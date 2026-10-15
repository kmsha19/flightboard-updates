// Standalone: clang++ -std=c++17 -Wall -Wextra -Werror -I. tests/host/test_panel.cpp -o /tmp/test_panel
#include "FlightBoardPanelRenderer.h"
#include <array>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace flightboard_panel;
static int assertions = 0;
#define CHECK(expr) do { ++assertions; if (!(expr)) throw std::runtime_error(std::string(__func__) + ":" + std::to_string(__LINE__) + ": " #expr); } while (0)

struct Screen {
  std::array<uint16_t, kWidth * kHeight> pixels{};
  size_t writes = 0;
  static void pixel(void* context, int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= kWidth || y < 0 || y >= kHeight) throw std::runtime_error("Out-of-panel write");
    auto& screen = *static_cast<Screen*>(context);
    screen.pixels[y * kWidth + x] = color;
    ++screen.writes;
  }
  PanelCanvas canvas() { return {this, pixel}; }
  void draw(const PanelFrame& frame, uint32_t now = 0) { writes = 0; renderPanel(canvas(), frame, now); }
  uint16_t at(int x, int y) const { return pixels[y * kWidth + x]; }
};

struct Logo {
  std::array<uint16_t, 384> pixels{};
  mutable size_t reads = 0;
  Logo() { for (size_t i = 0; i < pixels.size(); ++i) pixels[i] = uint16_t(i * 37); }
  static uint16_t read(const void* context, uint16_t index) {
    auto& logo = *static_cast<const Logo*>(context);
    if (index >= logo.pixels.size()) throw std::runtime_error("Logo index overflow");
    ++logo.reads;
    return logo.pixels[index];
  }
};

PanelAircraft makePlane() {
  PanelAircraft plane;
  plane.callsign = "AFR1234";
  plane.aircraftType = "A320";
  plane.routeStatus = RouteStatus::Known;
  plane.origin = "CDG"; plane.destination = "BER";
  plane.distanceKm = 8.4; plane.speedKmh = 819.6; plane.speedAvailable = true;
  plane.altitudeFt = 34000; plane.altitudeAvailable = true;
  return plane;
}
PanelFrame makeFrame(const PanelAircraft* plane = nullptr) {
  PanelFrame frame;
  frame.wifiConnected = true; frame.radarHealthy = true; frame.ageSeconds = 0;
  frame.aircraft = plane; frame.timeSynced = true; frame.clock = "14:32";
  return frame;
}

void test_data_rules() {
  auto plane = makePlane();
  auto frame = makeFrame(&plane);
  CHECK(panelScene(frame) == Scene::Aircraft);
  CHECK(!panelAnimated(frame));
  for (const auto mode : {Mode::Auto, Mode::Manual}) {
    frame.mode = mode;
    const Scene absent = mode == Mode::Auto ? Scene::Idle : Scene::Unavailable;
    frame.ageSeconds = 90; CHECK(panelScene(frame) == Scene::Aircraft);
    frame.ageSeconds = 91; CHECK(panelScene(frame) == absent);
    frame.ageSeconds = -1; CHECK(panelScene(frame) == absent);
    frame.ageSeconds = 0; frame.radarHealthy = false; CHECK(panelScene(frame) == absent);
    frame.radarHealthy = true; frame.wifiConnected = false; CHECK(panelScene(frame) == absent);
    frame.wifiConnected = true; frame.aircraft = nullptr; CHECK(panelScene(frame) == absent);
    frame.aircraft = &plane;
  }
  frame.mode = Mode::Clock; CHECK(panelScene(frame) == Scene::Idle); CHECK(panelAnimated(frame));
  frame.animate = false; CHECK(!panelAnimated(frame));
  frame.stateAvailable = false; CHECK(panelScene(frame) == Scene::Connecting);
  frame.sleeping = true; CHECK(panelScene(frame) == Scene::Sleeping);
  CHECK(!panelAnimated(frame));
  Screen screen; screen.pixels.fill(0xffff); screen.draw(frame);
  CHECK(screen.writes == 8192);
  CHECK(std::all_of(screen.pixels.begin(), screen.pixels.end(), [](uint16_t p) { return p == 0; }));
  renderPanel({}, frame, 0);  // Missing driver is safe.
}

void test_fonts_and_layout() {
  CHECK(sizeof(detail::kFont3) + sizeof(detail::kFont5) == 46 * 12);
  CHECK(detail::glyphIndex('a') == detail::glyphIndex('A'));
  CHECK(detail::glyphIndex('$') == detail::glyphIndex('?'));
  CHECK(detail::glyphIndex('\t') == detail::glyphIndex(' '));
  CHECK(detail::glyphIndex('!') != detail::glyphIndex('?'));
  CHECK(detail::glyphIndex('\'') != detail::glyphIndex('?'));
  CHECK(detail::glyphWidth(detail::glyphIndex('!'), false) == 1);
  CHECK(detail::glyphWidth(detail::glyphIndex('\''), true) == 1);
  auto l = detail::layout("LFPG > EDDB", 14, 122);
  CHECK(l.width == 118 && l.scale == 2 && !l.small);
  l = detail::layout("1234567", 10, 38);
  CHECK(l.small && l.scale == 1 && std::string(l.text) == "1234567");
  l = detail::layout("12345678901234567890", 10, 38);
  CHECK(std::string(l.text) == "..." && l.width <= 38);
  l = detail::layout("07:08", 30, 116);
  CHECK(l.width == 100 && l.scale == 4 && !l.small);
  Screen screen;
  detail::text(screen.canvas(), "A", 0, 0, 7, 0xffff);
  const char* rows[] = {".###.", "#...#", "#...#", "#####", "#...#", "#...#", "#...#"};
  for (int y = 0; y < 7; ++y) for (int x = 0; x < 5; ++x)
    CHECK((screen.at(x, y) == 0xffff) == (rows[y][x] == '#'));
  // All printable ASCII, long identifiers, and edge placements stay bounded.
  for (int c = 32; c <= 126; ++c) {
    const char value[] = {char(c), 0};
    detail::text(screen.canvas(), value, 126, 62, 14, 0xffff);
  }
  for (const char* code : {"CDG", "LFPG", "0A1", "K7S3"}) CHECK(detail::validAirport(code));
  for (const char* code : {"", "AB", "ABCDE", "cdg", "A!B"}) CHECK(!detail::validAirport(code));
  CHECK(!detail::validAirport(nullptr));
}

void test_plane_and_routes() {
  Logo logo;
  auto plane = makePlane(); plane.logo = {&logo, Logo::read};
  auto frame = makeFrame(&plane);
  Screen screen; screen.draw(frame);
  CHECK(logo.reads == 384);
  for (int i = 0; i < 384; ++i) CHECK(screen.at(3 + i % 24, 2 + i / 24) == logo.pixels[i]);
  CHECK(screen.at(3, 19) == detail::kDivider && screen.at(124, 35) == detail::kDivider);
  const auto initial = screen.pixels;
  plane.routeStatus = RouteStatus::Unconfirmed; screen.draw(frame);
  CHECK(screen.pixels != initial);
  const auto unconfirmed = screen.pixels;
  plane.origin = "XXX"; plane.destination = "YYY"; screen.draw(frame);
  CHECK(screen.pixels == unconfirmed);  // Never leak dubious codes.
  plane.routeStatus = RouteStatus::Known; plane.origin = nullptr; screen.draw(frame);
  const auto malformed = screen.pixels;
  plane.routeStatus = RouteStatus::Unknown; screen.draw(frame); CHECK(screen.pixels == malformed);
  plane.speedAvailable = false; plane.altitudeAvailable = false; plane.distanceKm = NAN;
  screen.draw(frame); const auto missing = screen.pixels;
  plane.distanceKm = INFINITY; plane.speedKmh = INFINITY; plane.speedAvailable = true;
  screen.draw(frame); CHECK(screen.pixels == missing);
  plane.altitudeAvailable = true; plane.altitudeFt = 0; screen.draw(frame);
  CHECK(screen.pixels != missing);  // Zero is available, not an absent measurement.
  plane.callsign = "VERY-LONG-CALLSIGN"; plane.speedKmh = 1234567; plane.distanceKm = 99999.9;
  plane.altitudeFt = INT32_MIN; screen.draw(frame);
  CHECK(screen.writes > 8192);
}

void test_aircraft_type_header() {
  auto plane = makePlane();
  auto frame = makeFrame(&plane);
  Screen screen;
  const auto verifyHeader = [&]() {
    size_t titlePixels = 0, typePixels = 0;
    for (int y = 0; y < 19; ++y) for (int x = 31; x < 128; ++x) {
      const uint16_t color = screen.at(x, y);
      if (!color) continue;
      if (color == detail::kMint) { ++titlePixels; CHECK(y >= 2 && y < 9); }
      else { ++typePixels; CHECK(color == detail::kMuted && y >= 11 && y < 18); }
    }
    CHECK(titlePixels > 0 && typePixels > 0);
    for (int y : {0, 1, 9, 10, 18})
      for (int x = 31; x < 128; ++x) CHECK(screen.at(x, y) == 0);
    for (int x = 27; x < 31; ++x)
      for (int y = 0; y < 19; ++y) CHECK(screen.at(x, y) == 0);
    CHECK(screen.at(31, 19) == detail::kDivider);
  };
  const auto verifyType = [&](const char* expected) {
    Screen type;
    detail::text(type.canvas(), expected, 31, 11, 7, detail::kMuted);
    for (int y = 11; y < 18; ++y) for (int x = 31; x < 128; ++x)
      CHECK(screen.at(x, y) == type.at(x, y));
  };
  for (const char* type : {"A320", "B738", "B38M", "PC21", "A1", "AB3"}) {
    plane.aircraftType = type; screen.draw(frame);
    verifyHeader(); verifyType(type);
  }
  plane.aircraftType = nullptr; screen.draw(frame);
  verifyHeader(); verifyType("TYPE --");
  const auto absent = screen.pixels;
  for (const char* type : {"", "—", "a320", "A", "A320X", "B-38", " A320", "A320 ", "A32\n", "A320\n"}) {
    plane.aircraftType = type; screen.draw(frame);
    CHECK(screen.pixels == absent);
  }
  plane.callsign = "VERY-LONG-CALLSIGN"; plane.aircraftType = "B38M"; screen.draw(frame);
  verifyHeader(); verifyType("B38M");
}

void test_idle() {
  Screen screen;
  auto frame = makeFrame();
  CHECK(kFrameIntervalMs == 80);
  screen.draw(frame, 0); const auto zero = screen.pixels;
  screen.draw(frame, 79); CHECK(screen.pixels == zero);
  screen.draw(frame, 80); CHECK(screen.pixels != zero);
  screen.draw(frame, 3200); CHECK(screen.pixels != zero);
  const auto moving = screen.pixels;
  screen.draw(frame, 3200); CHECK(screen.pixels == moving);
  frame.animate = false; screen.draw(frame, UINT32_MAX); CHECK(screen.pixels == zero);
  frame.animate = true; screen.draw(frame, UINT32_MAX); CHECK(screen.writes < 8192 + 8192);
  screen.draw(frame, 0); CHECK(screen.pixels == zero);  // millis wrap never blocks rendering.
  size_t white = 0, blue = 0;
  for (int y = 0; y < 64; ++y) for (int x = 0; x < 128; ++x) {
    const uint16_t color = screen.at(x, y);
    if (color == 0xffff) {
      ++white; CHECK(x >= 14 && x < 114 && y >= 18 && y < 46);
    } else {
      CHECK((color >> 11) <= 4);  // Blue/cyan contours may have a faint red component.
      if (color) ++blue;
    }
  }
  CHECK(white > 500 && blue > 500);
  frame.timeSynced = false; screen.draw(frame, 0); const auto unsynced = screen.pixels;
  CHECK(unsynced != zero);
  for (const char* clock : {"", "24:00", "12:60", "1:23", "xx:xx", "14:320"}) {
    frame.timeSynced = true; frame.clock = clock; screen.draw(frame, 0);
    CHECK(screen.pixels == unsynced);
  }
  for (const char* clock : {"00:00", "09:59", "23:59"}) CHECK(detail::validClock(clock));
  CHECK(!detail::validClock(nullptr));
  frame.mode = Mode::Manual; screen.draw(frame); CHECK(screen.pixels != unsynced);
}

void test_lava_background() {
  auto frame = makeFrame(); frame.visual = Visual::Lava;
  for (const uint32_t phase : {0U, 3200U, 8000U}) {
    Screen animation, clockBackground, clock, outline;
    frame.app = App::Animation; detail::ambient(animation.canvas(), frame, phase);
    frame.app = App::Clock; detail::ambient(clockBackground.canvas(), frame, phase);
    CHECK(animation.pixels == clockBackground.pixels); // No shaded central band.
    uint16_t brightestAnimation = 0;
    for (int y = 18; y < 46; ++y) for (int x = 14; x < 114; ++x)
      brightestAnimation = std::max<uint16_t>(brightestAnimation, animation.at(x, y) & 31);
    CHECK(brightestAnimation > 24);
    for (int dy : {-1, 1}) for (int dx : {-1, 1})
      detail::centered(outline.canvas(), "14:32", 64 + dx, 18 + dy, 116, 30, 1);
    detail::centered(outline.canvas(), "14:32", 64, 18, 116, 30, 2);
    clock.draw(frame, phase);
    size_t blackStroke = 0;
    for (size_t i = 0; i < clock.pixels.size(); ++i) {
      if (outline.pixels[i] == 2) CHECK(clock.pixels[i] == 0xffff);
      else if (outline.pixels[i] == 1) { CHECK(clock.pixels[i] == 0); ++blackStroke; }
      else CHECK(clock.pixels[i] == animation.pixels[i]); // Nothing changes outside the glyphs and their contour.
    }
    CHECK(blackStroke > 100);
  }
}

void test_zcs_scenes() {
  auto plane = makePlane(); auto frame = makeFrame(&plane);
  Screen screen;
  // An app choice is independent from Flight Board's own selection mode.
  frame.app = App::Clock; frame.mode = Mode::Manual;
  screen.draw(frame); const auto clock = screen.pixels;
  frame.aircraft = nullptr; frame.radarHealthy = false; frame.wifiConnected = false;
  screen.draw(frame); CHECK(screen.pixels == clock);
  frame.mode = Mode::Auto; screen.draw(frame); CHECK(screen.pixels == clock);
  for (const auto visual : {Visual::Lava, Visual::Stars, Visual::Rain, Visual::Plain, Visual::Aurora, Visual::Ocean, Visual::Warp}) {
    frame.visual = visual; frame.app = App::Animation;
    screen.draw(frame, 0); const auto initial = screen.pixels;
    screen.draw(frame, 79); CHECK(screen.pixels == initial);
    screen.draw(frame, 3200);
    CHECK((screen.pixels != initial) == (visual != Visual::Plain));
    CHECK(panelAnimated(frame) == (visual != Visual::Plain));
    frame.animate = false; screen.draw(frame, UINT32_MAX); CHECK(screen.pixels == initial);
    CHECK(!panelAnimated(frame)); frame.animate = true;
    frame.app = App::Clock; screen.draw(frame); CHECK(screen.pixels != initial);
  }
  PanelWeather weather;
  frame.app = App::Weather; frame.weather = &weather;
  screen.draw(frame); const auto unavailable = screen.pixels;
  weather.available = true; weather.temperatureC = 19.4; weather.windKmh = 12.3;
  weather.code = 0; weather.isDay = true;
  screen.draw(frame); const auto sunny = screen.pixels; CHECK(sunny != unavailable);
  CHECK(panelAnimated(frame));
  weather.stale = true; screen.draw(frame); CHECK(screen.pixels != sunny);
  weather.available = false; weather.stale = false; screen.draw(frame); CHECK(screen.pixels == unavailable);
  frame.weather = nullptr; screen.draw(frame); CHECK(screen.pixels == unavailable);
  // Every app obeys the same night schedule, including animation and weather.
  for (const auto app : {App::Aviation, App::Clock, App::Animation, App::Weather}) {
    frame.app = app; frame.sleeping = true; screen.pixels.fill(0xffff);
    screen.draw(frame, 3200); CHECK(!panelAnimated(frame));
    CHECK(std::all_of(screen.pixels.begin(), screen.pixels.end(), [](uint16_t p) { return p == 0; }));
  }
}

void test_animated_weather_and_clock() {
  Screen screen, text;
  auto frame = makeFrame();
  frame.app = App::Clock;
  for (const auto visual : {Visual::Aurora, Visual::Ocean, Visual::Warp}) {
    frame.visual = visual;
    text.pixels.fill(0); detail::centered(text.canvas(), "14:32", 64, 18, 116, 30, 0xffff);
    for (uint32_t now : {0U, 80U, 1040U, 3200U, UINT32_MAX}) {
      screen.draw(frame, now);
      // White numerals stay exact; the moving scenery below them remains dim.
      for (int y = 18; y < 46; ++y) for (int x = 5; x < 123; ++x) {
        if (text.at(x, y)) CHECK(screen.at(x, y) == text.at(x, y));
        else { const auto c = screen.at(x, y); CHECK((c >> 11) <= 5 && ((c >> 5) & 63) <= 13 && (c & 31) <= 7); }
      }
    }
  }
  frame.app = App::Weather; frame.visual = Visual::Plain;
  PanelWeather weather;
  weather.available = true; weather.temperatureC = -3.6; weather.windKmh = 12.3;
  frame.weather = &weather;
  for (const int code : {0, 1, 2, 3, 45, 48, 51, 53, 55, 56, 57, 61, 63, 65, 66, 67,
                         71, 73, 75, 77, 80, 81, 82, 85, 86, 95, 96, 99}) {
    weather.code = code;
    for (const bool day : {false, true}) {
      weather.isDay = day; frame.animate = true;
      CHECK(panelAnimated(frame));
      screen.draw(frame); const auto initial = screen.pixels;
      screen.draw(frame, 79); CHECK(screen.pixels == initial);
      screen.draw(frame, 3200); CHECK(screen.pixels != initial);
      for (uint32_t now : {80U, 1040U, 3200U, UINT32_MAX}) {
        screen.draw(frame, now);
        for (int y = 0; y < 64; ++y) for (int x = 0; x < 128; ++x)
          if (x < 4 || x >= 44 || y < 11 || y >= 44) CHECK(screen.at(x, y) == initial[y * 128 + x]);
      }
      frame.animate = false; CHECK(!panelAnimated(frame));
      screen.draw(frame, UINT32_MAX); CHECK(screen.pixels == initial);
    }
  }
  frame.animate = true;
  for (const int code : {-1, 4, 52, 58, 62, 72, 79, 83, 87, 94, 97, 999}) {
    weather.code = code; CHECK(!panelAnimated(frame));
    screen.draw(frame); const auto initial = screen.pixels;
    screen.draw(frame, 3200); CHECK(screen.pixels == initial);
  }
  weather.code = 0; weather.available = false; CHECK(!panelAnimated(frame));
  weather.available = true; weather.temperatureC = NAN; CHECK(!panelAnimated(frame));
  weather.temperatureC = 12; weather.windKmh = INFINITY; CHECK(!panelAnimated(frame));
}

void test_chrono_clock() {
  Screen screen, day, night;
  auto frame = makeFrame();
  frame.app = App::Clock; frame.visual = Visual::Chrono; frame.mode = Mode::Manual;
  frame.date = "LUN 16 OCT 2026"; frame.nightActive = false;
  PanelWeather weather;
  weather.available = true; weather.temperatureC = 19.4; weather.windKmh = 12.3;
  weather.code = 0; weather.isDay = true;
  frame.weather = &weather;
  day.draw(frame);
  CHECK(panelScene(frame) == Scene::Idle);
  CHECK(!panelAnimated(frame));                 // Never moves, so it can stay on all night.
  screen.draw(frame, UINT32_MAX); CHECK(screen.pixels == day.pixels);
  frame.nightActive = true; night.draw(frame);
  CHECK(night.pixels != day.pixels);            // Night uses a warmer, dimmer palette.
  frame.nightActive = false;
  // The large time occupies the chrono centre with the shared clock glyphs.
  Screen time; detail::centered(time.canvas(), "14:32", 64, 15, 124, 30, 0xffff);
  size_t digits = 0;
  for (size_t i = 0; i < time.pixels.size(); ++i)
    if (time.pixels[i]) { ++digits; CHECK(day.pixels[i] != 0); }
  CHECK(digits > 100);
  // Missing date, unsynced time and missing weather degrade to explicit labels.
  frame.date = ""; screen.draw(frame); CHECK(screen.pixels != day.pixels); frame.date = "LUN 16 OCT 2026";
  frame.timeSynced = false; frame.clock = ""; screen.draw(frame); CHECK(screen.pixels != day.pixels);
  frame.timeSynced = true; frame.clock = "14:32"; screen.draw(frame); CHECK(screen.pixels == day.pixels);
  PanelWeather none; frame.weather = &none; screen.draw(frame); CHECK(screen.pixels != day.pixels);
  frame.weather = &weather;
  // Stale readings stay visible but change tone.
  weather.stale = true; screen.draw(frame); CHECK(screen.pixels != day.pixels);
  weather.stale = false;
  // The night schedule still blanks the panel before the clock is drawn.
  frame.sleeping = true; screen.pixels.fill(0xffff); screen.draw(frame);
  CHECK(std::all_of(screen.pixels.begin(), screen.pixels.end(), [](uint16_t p) { return p == 0; }));
}

// Optional raw fixtures are compared by test_panel_parity.cjs with the actual
// embedded tablet renderer. uint16 pixels are written explicitly little-endian.
void writeFixture(const char* path, const PanelFrame& frame, uint32_t time = 0) {
  Screen screen; screen.draw(frame, time);
  std::ofstream out(path, std::ios::binary);
  for (uint16_t pixel : screen.pixels) { out.put(char(pixel & 255)); out.put(char(pixel >> 8)); }
  if (!out) throw std::runtime_error("Unable to write fixture");
}

int main(int argc, char** argv) {
  test_data_rules(); test_fonts_and_layout(); test_plane_and_routes(); test_aircraft_type_header(); test_idle(); test_lava_background(); test_zcs_scenes(); test_animated_weather_and_clock(); test_chrono_clock();
  if (argc == 2) {
    const std::string dir = argv[1];
    auto plane = makePlane(); auto frame = makeFrame(&plane);
    writeFixture((dir + "/plane.bin").c_str(), frame);
    const Font fonts[] = {Font::Classic, Font::Modern, Font::Digital, Font::Arcade, Font::Compact};
    const App fontApps[] = {App::Aviation, App::Clock, App::Weather, App::Message, App::Timer, App::Score};
    for (int f=0; f<5; ++f) for(int a=0; a<6; ++a) {
      auto fontFrame=makeFrame(&plane); fontFrame.app=fontApps[a]; fontFrame.font=fonts[f];fontFrame.clockFont=fonts[f];
      PanelWeather fontWeather; fontWeather.available=true;fontWeather.temperatureC=-3.6;fontWeather.windKmh=12;fontWeather.code=0; fontFrame.weather=&fontWeather;
      ZcsWidgetFrame fontWidget; fontWidget.message={"C'EST BIEN!",0x07e0}; fontWidget.timer={5999,false};fontWidget.score={"PSG","OM","69 MIN",12,9,true,false};fontFrame.widget=&fontWidget;
      writeFixture((dir+"/font_"+std::to_string(f)+"_"+std::to_string(a)+".bin").c_str(),fontFrame);
      if(a==0){fontFrame.mode=Mode::Clock;writeFixture((dir+"/font_idle_"+std::to_string(f)+".bin").c_str(),fontFrame);}
      if(a==3){const std::string message(120,'W');fontWidget.message={message.c_str(),0xf800};writeFixture((dir+"/font_scroll_"+std::to_string(f)+".bin").c_str(),fontFrame,6400);}
    }

    for (const char* type : {"B738", "B38M", "PC21", "A1", "AB3"}) {
      plane.aircraftType = type;
      writeFixture((dir + "/type_" + type + ".bin").c_str(), frame);
    }
    plane.aircraftType = nullptr;
    writeFixture((dir + "/type_missing.bin").c_str(), frame);
    plane.aircraftType = "A320X";
    writeFixture((dir + "/type_invalid.bin").c_str(), frame);
    plane.aircraftType = "A320";
    plane.callsign = "VERY-LONG-CALLSIGN"; plane.origin = "LFPG"; plane.destination = "EDDB";
    plane.distanceKm = 49.9; plane.speedKmh = 1234567; plane.altitudeFt = 100000;
    writeFixture((dir + "/large.bin").c_str(), frame);
    plane.distanceKm = 1234.5; plane.speedKmh = -1.5; plane.altitudeFt = -1000;
    writeFixture((dir + "/negative.bin").c_str(), frame);
    plane.distanceKm = NAN; plane.speedAvailable = false; plane.altitudeAvailable = false;
    plane.routeStatus = RouteStatus::Unconfirmed;
    writeFixture((dir + "/missing.bin").c_str(), frame);
    frame.mode = Mode::Manual; frame.ageSeconds = 91;
    writeFixture((dir + "/stale.bin").c_str(), frame);
    frame.stateAvailable = false; writeFixture((dir + "/connecting.bin").c_str(), frame);
    frame.stateAvailable = true; frame.mode = Mode::Clock;
    writeFixture((dir + "/clock.bin").c_str(), frame);
    frame.timeSynced = false; writeFixture((dir + "/unsynced.bin").c_str(), frame);
    frame = makeFrame(&plane); frame.mode = Mode::Manual;
    const std::pair<Visual, const char*> visuals[] = {
      {Visual::Lava, "lava"}, {Visual::Stars, "stars"}, {Visual::Rain, "rain"}, {Visual::Plain, "plain"},
      {Visual::Aurora, "aurora"}, {Visual::Ocean, "ocean"}, {Visual::Warp, "warp"}
    };
    for (const auto& visual : visuals) for (const auto app : {App::Clock, App::Animation}) {
      frame.app = app; frame.visual = visual.first;
      const std::string name = dir + "/" + (app == App::Clock ? "clock_" : "animation_") + visual.second;
      writeFixture((name + ".bin").c_str(), frame);
      writeFixture((name + "_moving.bin").c_str(), frame, 3200);
      writeFixture((name + "_phase.bin").c_str(), frame, 1040);
      writeFixture((name + "_wrap.bin").c_str(), frame, UINT32_MAX);
      frame.animate = false;
      writeFixture((name + "_reduced.bin").c_str(), frame, 3200);
      frame.animate = true;
    }
    ZcsWidgetFrame widget;
    frame = makeFrame(); frame.widget = &widget; frame.app = App::Message;
    widget.message = {"BONJOUR", 0x07e0};
    writeFixture((dir + "/widget_message_short.bin").c_str(), frame);
    widget.message = {"", 0x07e0};
    writeFixture((dir + "/widget_message_empty.bin").c_str(), frame);
    widget.message = {"C'EST BIEN!", 0x07e0};
    writeFixture((dir + "/widget_message_punctuation.bin").c_str(), frame);
    const std::string longMessage(120, 'W'); widget.message = {longMessage.c_str(), 0xf800};
    writeFixture((dir + "/widget_message_long.bin").c_str(), frame, 6400);
    writeFixture((dir + "/widget_message_wrap.bin").c_str(), frame, UINT32_MAX);
    frame.animate = false; writeFixture((dir + "/widget_message_reduced.bin").c_str(), frame, 6400); frame.animate = true;
    frame.app = App::Timer; widget.timer = {60, true};
    writeFixture((dir + "/widget_timer_running.bin").c_str(), frame);
    widget.timer = {62, false}; writeFixture((dir + "/widget_timer_paused.bin").c_str(), frame);
    widget.timer = {0, false}; writeFixture((dir + "/widget_timer_finished.bin").c_str(), frame);
    widget.timer = {5999, true}; writeFixture((dir + "/widget_timer_large.bin").c_str(), frame);
    widget.timer = {100000, false}; writeFixture((dir + "/widget_timer_invalid.bin").c_str(), frame);
    frame.app = App::Score; widget.score = {"PSG", "OM", "69 MIN", 0, 0, true, false};
    writeFixture((dir + "/widget_score_zero.bin").c_str(), frame);
    widget.score.homeScore = 999; widget.score.awayScore = 999; widget.score.stale = true;
    writeFixture((dir + "/widget_score_stale.bin").c_str(), frame);
    widget.score = {"ABCDEFGHIJKL", "MNOPQRSTUVWX", "123456789012345678901234", 12, 9, true, false};
    writeFixture((dir + "/widget_score_labels.bin").c_str(), frame);
    widget.score.available = false; writeFixture((dir + "/widget_score_missing.bin").c_str(), frame);
    frame.app = App::Pixel;
    const uint16_t palette[] = {0xf800, 0x07e0, 0x001f}; const uint8_t packed[] = {0x01, 0x22, 0x10};
    widget.pixel = {3, 1, 2, 3, 100, palette, packed, sizeof(packed)};
    writeFixture((dir + "/widget_pixel_odd.bin").c_str(), frame);
    writeFixture((dir + "/widget_pixel_next.bin").c_str(), frame, 100);
    writeFixture((dir + "/widget_pixel_wrap.bin").c_str(), frame, UINT32_MAX);
    frame.animate = false; writeFixture((dir + "/widget_pixel_reduced.bin").c_str(), frame, 100); frame.animate = true;
    widget.pixel.width = 1; widget.pixel.height = 3;
    writeFixture((dir + "/widget_pixel_portrait.bin").c_str(), frame);
    widget.pixel.dataBytes = 0; writeFixture((dir + "/widget_pixel_invalid.bin").c_str(), frame);
    frame.widget = nullptr; writeFixture((dir + "/widget_missing.bin").c_str(), frame);
    frame.widget = &widget; frame.sleeping = true; writeFixture((dir + "/widget_sleep.bin").c_str(), frame);
    frame = makeFrame(&plane); frame.mode = Mode::Manual;
    PanelWeather weather;
    frame.app = App::Weather; frame.weather = &weather;
    writeFixture((dir + "/weather_missing.bin").c_str(), frame);
    weather.available = true; weather.temperatureC = 19.4; weather.windKmh = 12.3;
    weather.code = 0; weather.isDay = true;
    writeFixture((dir + "/weather_sun.bin").c_str(), frame);
    weather.isDay = false;
    writeFixture((dir + "/weather_night.bin").c_str(), frame);
    weather.isDay = true;
    for (const int code : {3, 45, 61, 71, 95, 999}) {
      weather.code = code;
      writeFixture((dir + "/weather_code_" + std::to_string(code) + ".bin").c_str(), frame);
    }
    for (const int code : {0, 1, 2, 3, 45, 48, 51, 53, 55, 56, 57, 61, 63, 65, 66, 67,
                           71, 73, 75, 77, 80, 81, 82, 85, 86, 95, 96, 99}) for (const bool day : {false, true}) {
      weather.code = code; weather.isDay = day;
      const std::string name = dir + "/weather_animated_" + std::to_string(code) + (day ? "_day" : "_night");
      writeFixture((name + ".bin").c_str(), frame, 3200);
      writeFixture((name + "_phase.bin").c_str(), frame, 1040);
      writeFixture((name + "_wrap.bin").c_str(), frame, UINT32_MAX);
      frame.animate = false; writeFixture((name + "_reduced.bin").c_str(), frame, 3200); frame.animate = true;
    }
    weather.isDay = true;
    weather.code = 61; weather.temperatureC = -3.6; weather.windKmh = 100.2;
    writeFixture((dir + "/weather_negative.bin").c_str(), frame);
    weather.stale = true;
    writeFixture((dir + "/weather_stale.bin").c_str(), frame);
    weather.temperatureC = NAN; weather.windKmh = NAN;
    writeFixture((dir + "/weather_nonfinite.bin").c_str(), frame);
    frame.sleeping = true;
    writeFixture((dir + "/zcs_sleep.bin").c_str(), frame);
    // Bedside clock (chrono): date, time and weather, still, day and night.
    frame = makeFrame(&plane); frame.mode = Mode::Manual;
    frame.app = App::Clock; frame.visual = Visual::Chrono;
    frame.date = "LUN 16 OCT 2026"; frame.nightActive = false;
    weather = PanelWeather();
    weather.available = true; weather.temperatureC = 19.4; weather.windKmh = 12.3;
    weather.code = 0; weather.isDay = true;
    frame.weather = &weather;
    writeFixture((dir + "/clock_chrono.bin").c_str(), frame);
    frame.nightActive = true;
    writeFixture((dir + "/clock_chrono_night.bin").c_str(), frame);
    frame.nightActive = false;
    {
      PanelFrame off = frame; off.timeSynced = false; off.clock = ""; off.date = "";
      writeFixture((dir + "/clock_chrono_unsynced.bin").c_str(), off);
      PanelFrame undated = frame; undated.date = "";
      writeFixture((dir + "/clock_chrono_nodate.bin").c_str(), undated);
      PanelWeather none; PanelFrame without = frame; without.weather = &none;
      writeFixture((dir + "/clock_chrono_noweather.bin").c_str(), without);
      PanelWeather stale = weather; stale.stale = true; stale.code = 61; stale.temperatureC = -3.6;
      PanelFrame staleFrame = frame; staleFrame.weather = &stale;
      writeFixture((dir + "/clock_chrono_stale.bin").c_str(), staleFrame);
      PanelWeather coded = weather; coded.code = 45; coded.isDay = false; coded.temperatureC = 2.5;
      PanelFrame codedFrame = frame; codedFrame.weather = &coded;
      writeFixture((dir + "/clock_chrono_code.bin").c_str(), codedFrame);
    }
    {
      const Font chronoFonts[] = {Font::Classic, Font::Modern, Font::Digital, Font::Arcade, Font::Compact};
      const char* chronoNames[] = {"classic", "modern", "digital", "arcade", "compact"};
      for (int i = 0; i < 5; ++i) {
        frame.clockFont = chronoFonts[i];
        writeFixture((dir + "/clock_chrono_font_" + std::string(chronoNames[i]) + ".bin").c_str(), frame);
      }
    }
  }
  std::cout << "Panel renderer: " << assertions << " assertions passed\n";
}
