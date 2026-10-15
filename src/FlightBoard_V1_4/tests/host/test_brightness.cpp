#include "FlightBoardDisplay.h"
#include "FlightBoardFavorites.h"
#include "AirlineLogos.h"
#include <stdexcept>
#include <new>

struct UpdateState {
  bool active = false;
  bool busy() const { return active; }
} boardUpdate;
WebServer webServer;
#include "settings_under_test.inc"
void sendError(int status, const String& message) {
  webServer.send(status, "application/json", message);
}
#include "brightness_under_test.inc"

static int assertions = 0;
#define CHECK(expr) do { ++assertions; if (!(expr)) throw std::runtime_error(std::string(__func__)+":"+std::to_string(__LINE__)+": " #expr); } while (0)

void reset() {
  Preferences::values.clear();
  Preferences::writes.clear();
  Preferences::available = true;
  Preferences::failKey.clear();
  settings = BoardSettings();
  savedBrightness = settings.brightness;
  savedNightBrightness = settings.nightBrightness;
  brightnessChangedMs = 0;
  brightnessSaveError = false;
  selectedHex = "AFR123";
  boardUpdate.active = false;
  webServer = WebServer();
  testNow = 1000;
  favorites.~FlightBoardFavorites();
  new (&favorites) FlightBoardFavorites(storage);
}

void request(const String& body) {
  webServer.args["plain"] = body;
  webServer.responseCode = 0;
  changeBrightness();
}

void setBrightness(int value) {
  request(String("{\"brightness\":") + String(value) + "}");
}

void setNightBrightness(int value) {
  request(String("{\"nightBrightness\":") + String(value) + "}");
}

JsonDocument saved() {
  JsonDocument doc;
  CHECK(!deserializeJson(doc, Preferences::values["settings2"]));
  return doc;
}

void test_strict_requests_and_ota_guard() {
  reset();
  for (const char* invalid : {"", "{", "[]", "null", "{}", "{\"brightness\":9}",
       "{\"brightness\":101}", "{\"brightness\":-10}", "{\"brightness\":9999999999}",
       "{\"brightness\":65.5}", "{\"brightness\":\"65\"}", "{\"brightness\":true}",
       "{\"brightness\":null}", "{\"nightBrightness\":9}", "{\"nightBrightness\":101}",
       "{\"nightBrightness\":-10}", "{\"nightBrightness\":9999999999}",
       "{\"nightBrightness\":65.5}", "{\"nightBrightness\":\"65\"}",
       "{\"nightBrightness\":true}", "{\"nightBrightness\":null}",
       "{\"brightness\":85,\"nightBrightness\":null}", "{\"brightness\":null,\"nightBrightness\":25}",
       "{\"brightness\":85,\"nightBrightness\":9}", "{\"brightness\":9,\"nightBrightness\":25}",
       "{\"brightness\":85,\"nightBrightness\":\"25\"}", "{\"brightness\":true,\"nightBrightness\":25}"}) {
    request(invalid);
    CHECK(webServer.responseCode == 400);
    CHECK(settings.brightness == 60);
    CHECK(savedBrightness == 60);
    CHECK(settings.nightBrightness == 15);
    CHECK(savedNightBrightness == 15);
    CHECK(brightnessChangedMs == 0);
    CHECK(Preferences::writes.empty());
  }
  request(std::string(1025, ' '));
  CHECK(webServer.responseCode == 400);
  boardUpdate.active = true;
  setBrightness(85);
  CHECK(webServer.responseCode == 503);
  CHECK(settings.brightness == 60);
  setNightBrightness(25);
  CHECK(webServer.responseCode == 503);
  CHECK(settings.nightBrightness == 15);
  CHECK(brightnessChangedMs == 0);
  CHECK(Preferences::writes.empty());
}

void test_immediate_update_is_brightness_only() {
  reset();
  settings.app = "weather";
  settings.mode = 1;
  settings.aviationLogoOnly = true;
  settings.aviationType = "A320";
  settings.nightEnabled = true;
  settings.nightBrightness = 20;
  savedNightBrightness = 20;
  for (int value = 10; value <= 100; value += 5) {
    ++testNow;
    setBrightness(value);
    CHECK(webServer.responseCode == 200);
    CHECK(settings.brightness == value);
    CHECK(brightnessChangedMs == testNow);
    CHECK(savedBrightness == 60);
    CHECK(Preferences::writes.empty());
    CHECK(settings.app == "weather");
    CHECK(settings.mode == 1);
    CHECK(selectedHex == "AFR123");
    CHECK(settings.aviationLogoOnly);
    CHECK(settings.aviationType == "A320");
    CHECK(settings.nightEnabled);
    CHECK(settings.nightBrightness == 20);
    const auto dim = flightBoardDisplayState(value, true, 1320, 420, 20, false, true, 0);
    CHECK(dim.brightness == std::min(value, 20));
    const auto sleep = flightBoardDisplayState(value, true, 1320, 420, 20, true, true, 0);
    CHECK(sleep.brightness == 0);
    JsonDocument response;
    CHECK(!deserializeJson(response, webServer.responseBody));
    CHECK(response["brightness"] == value);
    CHECK(response["brightnessPending"] == (value != savedBrightness));
    CHECK(response["brightnessSaveError"] == false);
  }
  request("{\"brightness\":75,\"app\":\"aviation\",\"lat\":0,\"lon\":0,\"radiusKm\":5,\"mode\":0}");
  CHECK(webServer.responseCode == 200);
  CHECK(settings.brightness == 75);
  CHECK(settings.app == "weather");
  CHECK(settings.lat == 46.324);
  CHECK(settings.lon == -0.459);
  CHECK(settings.radiusKm == 30);
  CHECK(settings.mode == 1);
  CHECK(selectedHex == "AFR123");
}

void test_idle_save_coalesces_drag_and_unchanged_values() {
  reset();
  setBrightness(30);
  testNow = 1050; setBrightness(90);
  testNow = 1100; setBrightness(45);
  testNow = 1999; saveBrightnessIfIdle();
  CHECK(Preferences::writes.empty());
  testNow = 2000; saveBrightnessIfIdle();
  CHECK(Preferences::writes.size() == 1);
  CHECK(savedBrightness == 45);
  CHECK(saved()["brightness"] == 45);
  CHECK(!brightnessSaveError);
  const auto changedAt = brightnessChangedMs;
  testNow = 2500; setBrightness(45);
  CHECK(brightnessChangedMs == changedAt);
  testNow = 10000; saveBrightnessIfIdle();
  CHECK(Preferences::writes.size() == 1);
  reset();
  setBrightness(80);
  testNow += 300; setBrightness(60);
  testNow += 1000; saveBrightnessIfIdle();
  CHECK(Preferences::writes.empty());
  CHECK(savedBrightness == 60);
  CHECK(settings.brightness == 60);
}

void test_live_night_brightness_respects_schedule_and_day_ceiling() {
  reset();
  settings.app = "weather";
  settings.visual = "aurora";
  settings.nightEnabled = true;
  settings.nightStart = 23 * 60;
  settings.nightEnd = 6 * 60;
  for (int value = 10; value <= 100; value += 5) {
    ++testNow;
    setNightBrightness(value);
    CHECK(webServer.responseCode == 200);
    CHECK(settings.nightBrightness == value);
    CHECK(settings.brightness == 60);
    CHECK(savedNightBrightness == 15);
    CHECK(brightnessChangedMs == testNow);
    CHECK(Preferences::writes.empty());
    CHECK(settings.app == "weather");
    CHECK(settings.visual == "aurora");
    CHECK(settings.nightEnabled);
    CHECK(settings.nightStart == 1380);
    CHECK(settings.nightEnd == 360);
    CHECK(settings.nightAction == "dim");
    CHECK(settings.lat == 46.324);
    CHECK(settings.lon == -0.459);
    CHECK(selectedHex == "AFR123");
    const auto dim = flightBoardDisplayState(60, true, 1380, 360, value, false, true, 0);
    CHECK(dim.nightActive);
    CHECK(dim.brightness == std::min(60, value));
    const auto day = flightBoardDisplayState(60, true, 1380, 360, value, false, true, 720);
    CHECK(!day.nightActive);
    CHECK(day.brightness == 60);
    const auto sleep = flightBoardDisplayState(60, true, 1380, 360, value, true, true, 0);
    CHECK(sleep.sleeping);
    CHECK(sleep.brightness == 0);
    const auto disabled = flightBoardDisplayState(60, false, 1380, 360, value, false, true, 0);
    CHECK(disabled.brightness == 60);
    const auto unsynced = flightBoardDisplayState(60, true, 1380, 360, value, false, false, 0);
    CHECK(unsynced.brightness == 60);
    JsonDocument response;
    CHECK(!deserializeJson(response, webServer.responseBody));
    CHECK(response["brightness"] == 60);
    CHECK(response["nightBrightness"] == value);
    CHECK(response["brightnessPending"] == (value != savedNightBrightness));
    CHECK(response["brightnessSaveError"] == false);
  }
  request("{\"nightBrightness\":25,\"nightEnabled\":false,\"nightStart\":0,\"nightEnd\":1,\"nightAction\":\"sleep\",\"app\":\"clock\"}");
  CHECK(webServer.responseCode == 200);
  CHECK(settings.nightBrightness == 25);
  CHECK(settings.nightEnabled);
  CHECK(settings.nightStart == 1380);
  CHECK(settings.nightEnd == 360);
  CHECK(settings.nightAction == "dim");
  CHECK(settings.app == "weather");
}

void test_day_night_changes_are_atomic_and_share_idle_write() {
  reset();
  setNightBrightness(35);
  testNow = 1300; setBrightness(85);
  testNow = 1500; setNightBrightness(25);
  CHECK(brightnessPending());
  CHECK(savedBrightness == 60);
  CHECK(savedNightBrightness == 15);
  testNow = 1600; setBrightness(85); // Same value must not extend the pending night save.
  CHECK(brightnessChangedMs == 1500);
  testNow = 2399; saveBrightnessIfIdle();
  CHECK(Preferences::writes.empty());
  testNow = 2400; saveBrightnessIfIdle();
  CHECK(Preferences::writes.size() == 1);
  CHECK(savedBrightness == 85);
  CHECK(savedNightBrightness == 25);
  CHECK(!brightnessPending());
  CHECK(saved()["brightness"] == 85);
  CHECK(saved()["nightBrightness"] == 25);
  testNow = 2500;
  request("{\"brightness\":40,\"nightBrightness\":20}");
  CHECK(webServer.responseCode == 200);
  CHECK(settings.brightness == 40);
  CHECK(settings.nightBrightness == 20);
  CHECK(brightnessChangedMs == 2500);
  testNow = 2600;
  request("{\"brightness\":75,\"nightBrightness\":null}");
  CHECK(webServer.responseCode == 400);
  CHECK(settings.brightness == 40);
  CHECK(settings.nightBrightness == 20);
  CHECK(brightnessChangedMs == 2500);
  request("{\"brightness\":null,\"nightBrightness\":45}");
  CHECK(webServer.responseCode == 400);
  CHECK(settings.brightness == 40);
  CHECK(settings.nightBrightness == 20);
  CHECK(brightnessChangedMs == 2500);
  testNow = 3399; saveBrightnessIfIdle();
  CHECK(Preferences::writes.size() == 1);
  testNow = 3400; saveBrightnessIfIdle();
  CHECK(Preferences::writes.size() == 2);
  CHECK(savedBrightness == 40);
  CHECK(savedNightBrightness == 20);

  reset();
  setNightBrightness(35);
  testNow += 300; setNightBrightness(15);
  testNow += 900; saveBrightnessIfIdle();
  CHECK(Preferences::writes.empty());
  CHECK(!brightnessPending());
}

void test_current_settings_and_other_saves_are_preserved() {
  reset();
  setBrightness(85);
  setNightBrightness(35);
  settings.app = "clock";
  settings.visual = "stars";
  settings.place = "Current place";
  testNow += 900; saveBrightnessIfIdle();
  auto doc = saved();
  CHECK(doc["brightness"] == 85);
  CHECK(doc["nightBrightness"] == 35);
  CHECK(doc["app"] == "clock");
  CHECK(doc["visual"] == "stars");
  CHECK(doc["place"] == "Current place");
  reset();
  setBrightness(90);
  setNightBrightness(25);
  BoardSettings next = settings;
  next.app = "weather";
  next.radiusKm = 50;
  CHECK(persistSettings(next));
  settings = next;
  CHECK(savedBrightness == 90);
  CHECK(savedNightBrightness == 25);
  testNow += 1000; saveBrightnessIfIdle();
  CHECK(Preferences::writes.size() == 1);
  CHECK(saved()["radiusKm"] == 50);
  CHECK(saved()["app"] == "weather");
  CHECK(saved()["nightBrightness"] == 25);
}

void test_failed_night_save_keeps_both_pending_until_recovered() {
  reset();
  request("{\"brightness\":90,\"nightBrightness\":25}");
  Preferences::failKey = "settings2";
  testNow = 1900; saveBrightnessIfIdle();
  CHECK(brightnessSaveError);
  CHECK(savedBrightness == 60);
  CHECK(savedNightBrightness == 15);
  CHECK(settings.brightness == 90);
  CHECK(settings.nightBrightness == 25);
  testNow = 2000; setBrightness(60);
  CHECK(brightnessSaveError); // Night is still dirty even after day has returned to its saved value.
  CHECK(brightnessPending());
  JsonDocument response;
  CHECK(!deserializeJson(response, webServer.responseBody));
  CHECK(response["brightnessPending"] == true);
  CHECK(response["brightnessSaveError"] == true);
  testNow = 6999; saveBrightnessIfIdle();
  CHECK(brightnessChangedMs == 2000);
  Preferences::failKey.clear();
  testNow = 7000; saveBrightnessIfIdle();
  CHECK(Preferences::writes.size() == 1);
  CHECK(savedBrightness == 60);
  CHECK(savedNightBrightness == 25);
  CHECK(!brightnessSaveError);
  CHECK(!brightnessPending());
  CHECK(saved()["nightBrightness"] == 25);

  reset();
  request("{\"brightness\":90,\"nightBrightness\":25}");
  Preferences::failKey = "settings2";
  testNow += 900; saveBrightnessIfIdle();
  CHECK(brightnessSaveError);
  testNow += 100; setNightBrightness(15);
  CHECK(brightnessSaveError); // The converse also preserves the pending day save.
  CHECK(brightnessPending());
  testNow += 100; setBrightness(60);
  CHECK(!brightnessSaveError);
  CHECK(!brightnessPending());
  testNow += 10000; saveBrightnessIfIdle();
  CHECK(Preferences::writes.empty());
}

void test_failed_save_keeps_live_value_and_retries_slowly() {
  reset();
  setBrightness(90);
  Preferences::failKey = "settings2";
  testNow = 1900; saveBrightnessIfIdle();
  CHECK(brightnessSaveError);
  CHECK(brightnessChangedMs == 1900);
  CHECK(savedBrightness == 60);
  CHECK(settings.brightness == 90);
  CHECK(Preferences::writes.empty());
  testNow = 6899; saveBrightnessIfIdle();
  CHECK(brightnessChangedMs == 1900);
  testNow = 6900; saveBrightnessIfIdle();
  CHECK(brightnessChangedMs == 6900);
  CHECK(brightnessSaveError);
  Preferences::failKey.clear();
  testNow = 11899; saveBrightnessIfIdle();
  CHECK(Preferences::writes.empty());
  testNow = 11900; saveBrightnessIfIdle();
  CHECK(savedBrightness == 90);
  CHECK(!brightnessSaveError);
  CHECK(Preferences::writes.size() == 1);
  CHECK(saved()["brightness"] == 90);

  reset();
  setBrightness(90);
  Preferences::failKey = "settings2";
  testNow += 900; saveBrightnessIfIdle();
  CHECK(brightnessSaveError);
  testNow += 100; setBrightness(60);
  CHECK(!brightnessSaveError);
  CHECK(settings.brightness == savedBrightness);
  testNow += 10000; saveBrightnessIfIdle();
  CHECK(Preferences::writes.empty());
  JsonDocument response;
  CHECK(!deserializeJson(response, webServer.responseBody));
  CHECK(response["brightnessPending"] == false);
  CHECK(response["brightnessSaveError"] == false);
}

void test_wraparound_and_reloaded_brightness() {
  reset();
  testNow = UINT32_MAX - 400;
  setBrightness(35);
  setNightBrightness(25);
  testNow = 498; saveBrightnessIfIdle();
  CHECK(Preferences::writes.empty());
  testNow = 499; saveBrightnessIfIdle();
  CHECK(Preferences::writes.size() == 1);
  CHECK(savedBrightness == 35);
  CHECK(savedNightBrightness == 25);
  settings = BoardSettings();
  savedBrightness = 60;
  savedNightBrightness = 15;
  loadSettings();
  CHECK(settings.brightness == 35);
  CHECK(savedBrightness == 35);
  CHECK(settings.nightBrightness == 25);
  CHECK(savedNightBrightness == 25);
  CHECK(!brightnessPending());
  const auto writes = Preferences::writes.size();
  testNow = 10000; saveBrightnessIfIdle();
  CHECK(Preferences::writes.size() == writes);
}

int main() {
  const std::vector<std::pair<const char*, std::function<void()>>> tests = {
    {"strict brightness request validation and OTA guard", test_strict_requests_and_ota_guard},
    {"live brightness preserves app, location, pin and night rules", test_immediate_update_is_brightness_only},
    {"drag is coalesced into one idle write and unchanged value is a no-op", test_idle_save_coalesces_drag_and_unchanged_values},
    {"night brightness follows schedule, sleep and day ceiling", test_live_night_brightness_respects_schedule_and_day_ceiling},
    {"interleaved day/night changes are atomic and coalesce into one write", test_day_night_changes_are_atomic_and_share_idle_write},
    {"idle writes use current settings and do not duplicate other saves", test_current_settings_and_other_saves_are_preserved},
    {"failed storage preserves live brightness and retries slowly", test_failed_save_keeps_live_value_and_retries_slowly},
    {"failed night save keeps shared dirty state until recovery", test_failed_night_save_keeps_both_pending_until_recovered},
    {"idle timer wraps safely and saved brightness survives reload", test_wraparound_and_reloaded_brightness}
  };
  int failed = 0;
  for (const auto& test : tests) {
    try { test.second(); std::cout << "PASS " << test.first << '\n'; }
    catch (const std::exception& error) { ++failed; std::cerr << "FAIL " << test.first << ": " << error.what() << '\n'; }
  }
  std::cout << tests.size() << " brightness scenarios, " << assertions << " assertions, " << failed << " failed\n";
  return failed ? 1 : 0;
}
