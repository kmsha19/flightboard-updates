#include "MockPlatform.hpp"
#include <ArduinoJson.h>
#include <stdexcept>
#include <new>
#include "HTTPClient.h"
#include "freertos/FreeRTOS.h"
#include "AirlineLogos.h"
#define private public
#include "FlightBoardRadar.h"
#undef private
static int assertions=0;
#define CHECK(expr) do {++assertions;if(!(expr))throw std::runtime_error(std::string(__func__)+":"+std::to_string(__LINE__)+": " #expr);}while(0)
void reset(){testNow=0;WiFi=WiFiMock();WiFi.connect("Mock");HTTPClient::responseStatus=200;HTTPClient::responseSize=-2;HTTPClient::headers.clear();HTTPClient::responseHeaders.clear();MockHTTPStream::readDelayMs=0;MockHTTPStream::bytesRead=0;MockHTTPStream::holdOpenAtEnd=false;MockHTTPStream::gapsBeforeByte.clear();HTTPClient::responseJson="{\"ac\":[]}";HTTPClient::beginSuccess=true;HTTPClient::gets=HTTPClient::ends=0;queueFailuresAfter=-1;taskCreateSuccess=true;stopWorkerAfterSend=false;queueDeletes=0;for(auto*q:fakeQueues)delete q;fakeQueues.clear();}
RadarResult fetch(const std::string& json,int radius=50){HTTPClient::responseJson=json;RadarResult result;FlightBoardRadar::fetch({0,0,radius,7},result);return result;}
void test_top40_unsorted(){reset();JsonDocument doc;auto ac=doc["ac"].to<JsonArray>();for(int n=60;n>=1;--n){auto p=ac.add<JsonObject>();p["hex"]=std::to_string(n);p["flight"]=" AFR842 ";p["lat"]=n*.005;p["lon"]=0;p["seen_pos"]=5;p["gs"]=100;p["track"]=50;p["alt_baro"]=32000;p["t"]="A320";}
  std::string text;serializeJson(doc,text);auto r=fetch(text);CHECK(r.healthy);CHECK(r.count==40);for(int i=0;i<40;++i){CHECK(r.aircraft[i].hex==String(i+1));CHECK(i==0||r.aircraft[i].distanceKm>r.aircraft[i-1].distanceKm);}CHECK(r.aircraft[0].callsign=="AFR842");CHECK(r.aircraft[0].airline==&AIRLINES[0]);CHECK(std::abs(r.aircraft[0].speedKmh-185.2)<1e-8);CHECK(r.aircraft[0].altitudeFt==32000);CHECK(r.aircraft[0].aircraftType=="A320");CHECK(HTTPClient::ends==1);CHECK(HTTPClient::lastUrl=="https://api.adsb.lol/v2/lat/0.0000/lon/0.0000/dist/27");}
void test_invalid_positions_and_fields(){reset();JsonDocument d;auto ac=d["ac"].to<JsonArray>();
  auto plane=[&](const char*hex,double lat,double lon,double age){auto p=ac.add<JsonObject>();p["hex"]=hex;p["lat"]=lat;p["lon"]=lon;p["seen_pos"]=age;return p;};
  plane("good",.01,.01,60)["flight"]=" ";
  plane("stale",.01,0,60.01);plane("future",0,0,-1);plane("latbad",91,0,0);plane("lonbad",0,181,0);plane("far",1,0,0);plane("",0,0,0);
  plane("noloc",0,0,0).remove("lat");plane("noage",0,0,0).remove("seen_pos");plane("badtype",0,0,0)["lat"]="0";
  auto p=plane("good2",.02,0,0);p["gs"]=-2;p["track"]="unknown";p["alt_baro"]="ground";
  std::string text;serializeJson(d,text);auto r=fetch(text,5);CHECK(r.healthy);CHECK(r.count==2);CHECK(r.aircraft[0].hex=="good");CHECK(r.aircraft[0].callsign=="good");CHECK(r.aircraft[0].airline==&DEFAULT_AIRLINE);CHECK(!r.aircraft[1].speedAvailable);CHECK(!r.aircraft[1].trackAvailable);CHECK(!r.aircraft[1].altitudeAvailable);CHECK(r.aircraft[0].distanceKm<2);}
void test_errors_and_backoff(){reset();for(const char*raw:{"invalid","{}","{\"ac\":{}}","null"}){auto r=fetch(raw);CHECK(!r.healthy);CHECK(r.count==0);CHECK(r.retryMs==15000);}auto r=fetch("{\"ac\":[]}");CHECK(r.healthy);CHECK(r.count==0);CHECK(r.retryMs==15000);
  HTTPClient::responseStatus=429;r=fetch("{}");CHECK(!r.healthy);CHECK(r.retryMs==120000);
  HTTPClient::responseStatus=503;r=fetch("{}");CHECK(!r.healthy);CHECK(r.retryMs==15000);
  HTTPClient::beginSuccess=false;int gets=HTTPClient::gets;r=fetch("{}");CHECK(!r.healthy);CHECK(HTTPClient::gets==gets);
  HTTPClient::beginSuccess=true;WiFi.disconnect();r=fetch("{}");CHECK(!r.healthy);CHECK(HTTPClient::gets==gets);
}
void test_queue_handoff_and_generation(){reset();FlightBoardRadar radar;CHECK(!radar.request(0,0,30,1));CHECK(radar.begin());CHECK(fakeQueues.size()==2);CHECK(radar.request(1,2,30,43));CHECK(radar.pending());CHECK(!radar.request(9,9,5,99));CHECK(fakeQueues[0]->items.size()==1);RadarJob queued{};std::memcpy(&queued,fakeQueues[0]->items[0].data(),sizeof queued);CHECK(queued.lat==1&&queued.lon==2&&queued.radiusKm==30&&queued.generation==43);RadarResult* result=nullptr;CHECK(!radar.take(result));CHECK(radar.pending());stopWorkerAfterSend=true;try{workerFunction(workerContext);}catch(const WorkerStopped&){}CHECK(radar.take(result));CHECK(!radar.pending());CHECK(result);CHECK(result->generation==43);CHECK(result->healthy);CHECK(result->count==0);delete result;result=nullptr;CHECK(!radar.take(result));CHECK(radar.request(2,3,10,44));}
void test_queue_creation_failures(){for(int failure:{0,1}){reset();queueFailuresAfter=failure;FlightBoardRadar radar;CHECK(!radar.begin());CHECK(!radar.request(1,2,30,1));CHECK(fakeQueues.empty());}reset();taskCreateSuccess=false;FlightBoardRadar radar;CHECK(!radar.begin());CHECK(queueDeletes==2);CHECK(fakeQueues.empty());CHECK(!radar.request(1,2,30,1));}
void test_distance_boundaries(){CHECK(FlightBoardRadar::distance(0,0,0,0)==0);CHECK(std::abs(FlightBoardRadar::distance(0,0,0,180)-20015.0868)<.1);CHECK(std::abs(FlightBoardRadar::distance(0,179.9,0,-179.9)-22.23898)<.01);CHECK(std::isfinite(FlightBoardRadar::distance(90,0,-90,180)));}

void test_diagnostic_causes() {
  reset();
  const std::vector<std::pair<std::string,const char*>> malformed = {
    {"", "empty_response"}, {"   ", "json_empty"}, {"{\"ac\":[", "json_incomplete"},
    {"{\"ac\":[{\"hex\":\"abc\"}", "json_incomplete"}, {"{\"ac\":!}", "json_invalid"},
    {"{\"message\":\"upstream unavailable\"}", "schema"}, {"{\"ac\":{}}", "schema"}
  };
  for (const auto& sample : malformed) {
    const auto r = fetch(sample.first);
    CHECK(!r.healthy); CHECK(r.count == 0); CHECK(r.httpStatus == 200);
    if (std::string(r.error) != sample.second)
      throw std::runtime_error(sample.first + ": expected " + sample.second + ", got " + r.error);
    CHECK(std::string(r.error) == sample.second);
  }
  auto r = fetch("{\"ac\":[],\"msg\":\"No error\",\"now\":1735689600}");
  CHECK(r.healthy); CHECK(r.count == 0); CHECK(std::string(r.error) == "none");
  CHECK(r.contentLength > 0); CHECK(r.bytesRead == (size_t)r.contentLength); CHECK(r.firstByte == '{');
  for (const int status : {-1, -11, 403, 429, 503}) {
    HTTPClient::responseStatus = status;
    r = fetch("{}");
    CHECK(!r.healthy); CHECK(r.httpStatus == status);
    CHECK(std::string(r.error) == (status < 0 ? "network" : "http"));
    CHECK(r.retryMs == (status == 429 ? 120000UL : 15000UL));
  }
  HTTPClient::beginSuccess = false;
  r = fetch("{}"); CHECK(std::string(r.error) == "http_begin"); CHECK(r.httpStatus == 0);
  WiFi.disconnect();
  r = fetch("{}"); CHECK(std::string(r.error) == "wifi"); CHECK(r.httpStatus == 0);
}

void test_bounded_network_body() {
  reset();
  HTTPClient::responseSize = 256 * 1024 + 1;
  auto r = fetch("{\"ac\":[]}");
  CHECK(!r.healthy); CHECK(std::string(r.error) == "response_too_large");
  CHECK(MockHTTPStream::bytesRead == 0); CHECK(HTTPClient::ends == 1);
  CHECK(r.contentLength == 256 * 1024 + 1); CHECK(r.bytesRead == 0); CHECK(r.firstByte == -1);
  HTTPClient::responseSize = -1;
  r = fetch(std::string(256 * 1024 + 1, ' '));
  CHECK(!r.healthy); CHECK(std::string(r.error) == "response_too_large");
  CHECK(MockHTTPStream::bytesRead == 256 * 1024);
  CHECK(r.contentLength == -1); CHECK(r.bytesRead == 256 * 1024); CHECK(r.firstByte == ' ');
  r = fetch(""); CHECK(std::string(r.error) == "json_empty");
  r = fetch("{\"ac\":[]}"); CHECK(r.healthy);
  HTTPClient::responseSize = 100;
  r = fetch("{\"ac\":["); CHECK(std::string(r.error) == "json_incomplete");

  reset(); MockHTTPStream::readDelayMs = 1000;
  r = fetch(std::string(20, ' ') + "{\"ac\":[]}");
  CHECK(!r.healthy); CHECK(std::string(r.error) == "timeout");
  CHECK(testNow == 15000); CHECK(MockHTTPStream::bytesRead < 20); CHECK(HTTPClient::ends == 1);
  reset(); MockHTTPStream::readDelayMs = 3000;
  r = fetch("{\"ac\":[]}"); CHECK(std::string(r.error) == "timeout"); CHECK(testNow == 2000);

  // A short successful response survives the millis() rollover.
  reset(); testNow = 0xfffffff0UL; MockHTTPStream::readDelayMs = 10;
  r = fetch("{\"ac\":[]}"); CHECK(r.healthy); CHECK(testNow < 100);
  reset(); testNow = 0xfffffff0UL; MockHTTPStream::readDelayMs = 1000;
  r = fetch(std::string(20, ' ') + "{\"ac\":[]}");
  CHECK(std::string(r.error) == "timeout"); CHECK(testNow < 15000);
}

void test_protocol_and_availability() {
  reset(); FlightBoardRadar radar;
  CHECK(!radar.available()); CHECK(radar.begin()); CHECK(radar.available());
  CHECK(radar.begin()); CHECK(fakeQueues.size() == 2);
  auto r = fetch("{\"ac\":[]}", 30); CHECK(r.healthy);
  CHECK(HTTPClient::lastUrl == "https://api.adsb.lol/v2/lat/0.0000/lon/0.0000/dist/17");
  CHECK(HTTPClient::headers["Accept-Encoding"] == "identity");
  r = fetch("{\"ac\":[{\"hex\":\"inside\",\"lat\":0.269,\"lon\":0,\"seen_pos\":1},"
    "{\"hex\":\"nm-only\",\"lat\":0.28,\"lon\":0,\"seen_pos\":1}]}", 30);
  CHECK(r.healthy); CHECK(r.count == 1); CHECK(r.aircraft[0].hex == "inside");
  reset(); taskCreateSuccess = false; FlightBoardRadar failed;
  CHECK(!failed.begin()); CHECK(!failed.available());
}

void test_tls_receive_gaps_and_closed_eof() {
  const std::string body = "{\"ac\":[{\"hex\":\"abc123\",\"lat\":0.01,\"lon\":0,\"seen_pos\":1}]}";
  reset(); MockHTTPStream::gapsBeforeByte = {{0, 300}, {5, 700}, {25, 1200}};
  auto r = fetch(body);
  CHECK(r.healthy); CHECK(r.count == 1); CHECK(r.aircraft[0].hex == "abc123");
  CHECK(r.bytesRead == body.size()); CHECK(r.firstByte == '{'); CHECK(testNow == 2200);

  reset(); MockHTTPStream::gapsBeforeByte = {{5, 3000}};
  r = fetch(body);
  CHECK(!r.healthy); CHECK(std::string(r.error) == "timeout");
  CHECK(r.bytesRead == 5); CHECK(testNow == 2000); CHECK(HTTPClient::ends == 1);

  // A connection left open after an incomplete body must also time out.
  reset(); HTTPClient::responseSize = -1; MockHTTPStream::holdOpenAtEnd = true;
  r = fetch("{\"ac\":[");
  CHECK(std::string(r.error) == "timeout"); CHECK(testNow == 2000);
  CHECK(r.bytesRead == 7);
  // An actual close remains an immediate incomplete-input diagnosis.
  reset(); HTTPClient::responseSize = -1;
  r = fetch("{\"ac\":[");
  CHECK(std::string(r.error) == "json_incomplete"); CHECK(testNow == 0);
  CHECK(r.bytesRead == 7);

  // Individually short packet gaps cannot reset the global body deadline.
  reset();
  for (size_t i = 0; i < 20; ++i) MockHTTPStream::gapsBeforeByte[i] = 1000;
  r = fetch(std::string(20, ' ') + body);
  CHECK(std::string(r.error) == "timeout"); CHECK(testNow == 15000);
  CHECK(r.bytesRead < 20);

  reset(); testNow = 0xffffff00UL;
  MockHTTPStream::gapsBeforeByte = {{0, 600}, {5, 400}};
  r = fetch(body); CHECK(r.healthy); CHECK(testNow == 744);
}

void test_adaptive_session_cadence() {
  reset(); FlightBoardRadarCadence cadence;
  auto completeResponse = [&](int status) {
    HTTPClient::responseStatus = status;
    const auto result = fetch("{\"ac\":[]}");
    CHECK(result.httpStatus == status);
    CHECK(result.healthy == (status == 200));
    return cadence.complete(result.httpStatus, (uint32_t)testNow, result.retryMs);
  };
  CHECK(cadence.intervalMs() == 15000);
  CHECK(cadence.allowsRequest(0));
  CHECK(completeResponse(200) == 15000);
  testNow = 15000;
  CHECK(completeResponse(429) == 120000);
  CHECK(cadence.intervalMs() == 20000);
  CHECK(cadence.remainingCooldownMs(15000) == 120000);
  CHECK(!cadence.allowsRequest(15000));
  CHECK(!cadence.allowsRequest(134999));
  CHECK(cadence.remainingCooldownMs(134999) == 1);
  testNow = 135000;
  CHECK(cadence.allowsRequest((uint32_t)testNow));

  // Successful recovery and unrelated failures must retain the accepted rate.
  for (const int status : {200, 200, 200, -1, 503, 403}) {
    CHECK(completeResponse(status) == 20000);
    CHECK(cadence.intervalMs() == 20000);
    testNow += 20000;
  }
  // Every further 429 slows the session by five seconds, then stays at 120s.
  for (const unsigned long expected : {25000UL, 30000UL, 35000UL, 40000UL,
      45000UL, 50000UL, 55000UL, 60000UL, 65000UL, 70000UL, 75000UL, 80000UL,
      85000UL, 90000UL, 95000UL, 100000UL, 105000UL, 110000UL, 115000UL,
      120000UL, 120000UL, 120000UL}) {
    CHECK(completeResponse(429) == 120000);
    CHECK(cadence.intervalMs() == expected);
    CHECK(!cadence.allowsRequest((uint32_t)testNow));
    CHECK(!cadence.allowsRequest((uint32_t)(testNow + 119999UL)));
    testNow += 120000;
    CHECK(cadence.allowsRequest((uint32_t)testNow));
    CHECK(completeResponse(200) == expected);
    CHECK(cadence.intervalMs() == expected);
    testNow += expected;
  }

  // A new process/session can start at the requested nominal rate.
  FlightBoardRadarCadence restarted;
  CHECK(restarted.intervalMs() == 15000);
  CHECK(restarted.allowsRequest((uint32_t)testNow));
}

void test_retry_after_and_cooldown_rollover() {
  reset(); HTTPClient::responseStatus = 429;
  const std::vector<std::pair<std::string,unsigned long>> headers = {
    {"", 120000}, {"0", 120000}, {"30", 120000}, {"120", 120000},
    {" 180 ", 180000}, {"600", 600000}, {"-5", 120000},
    {"180s", 120000}, {"1.5", 120000},
    {"Wed, 21 Oct 2026 07:28:00 GMT", 120000},
    {"99999999999999999999999", 2147483000UL}
  };
  for (const auto& sample : headers) {
    HTTPClient::responseHeaders["Retry-After"] = sample.first;
    auto result = fetch("{}");
    CHECK(result.httpStatus == 429);
    CHECK(result.retryMs == sample.second);
  }
  HTTPClient::responseHeaders["Retry-After"] = "180";
  auto result = fetch("{}");
  FlightBoardRadarCadence cadence;
  const uint32_t start = 0xfffffff0UL;
  CHECK(cadence.complete(result.httpStatus, start, result.retryMs) == 180000);
  CHECK(cadence.intervalMs() == 20000); // Recovery cadence differs from pause.
  CHECK(!cadence.allowsRequest(start));
  CHECK(!cadence.allowsRequest((uint32_t)(start + 179999UL)));
  CHECK(cadence.remainingCooldownMs((uint32_t)(start + 179999UL)) == 1);
  CHECK(cadence.allowsRequest((uint32_t)(start + 180000UL)));
  CHECK(cadence.remainingCooldownMs((uint32_t)(start + 180000UL)) == 0);
  CHECK(cadence.remainingCooldownMs(start) == 0); // Expiry stays cleared.
  // A non-429 response header must not request an unrelated extra wait.
  HTTPClient::responseStatus = 200;
  result = fetch("{\"ac\":[]}");
  CHECK(result.healthy); CHECK(result.retryMs == 15000);
  CHECK(cadence.complete(result.httpStatus, 180000, result.retryMs) == 20000);
}


void resetCadenceStorage() {
  Preferences::values.clear();
  Preferences::writes.clear();
  Preferences::available = true;
  Preferences::failKey.clear();
  Preferences::uintWriteAttempts = 0;
}

void test_validated_cadence_persistence_and_reboot() {
  reset(); resetCadenceStorage();
  Preferences storage;
  FlightBoardRadarCadence cadence;
  cadence.begin(storage);
  CHECK(cadence.intervalMs() == 15000); CHECK(!cadence.isSaved());
  CHECK(Preferences::uintWriteAttempts == 0);
  auto response = [&](int status, const char* body, bool currentGeneration = true) {
    HTTPClient::responseStatus = status;
    const auto result = fetch(body);
    cadence.complete(result.httpStatus, (uint32_t)testNow, result.retryMs,
                     result.healthy && currentGeneration);
    testNow += cadence.intervalMs();
  };
  response(429, "{}");
  testNow += 120000;
  CHECK(cadence.intervalMs() == 20000); CHECK(!cadence.isSaved());
  // JSON errors, obsolete generations, HTTP/network failures and setting resets
  // all interrupt a partial success streak, even when the HTTP status is 200.
  for (int interruption = 0; interruption < 5; ++interruption) {
    response(200, "{\"ac\":[]}"); response(200, "{\"ac\":[]}");
    CHECK(Preferences::uintWriteAttempts == 0);
    if (interruption == 0) response(200, "{invalid");
    else if (interruption == 1) response(200, "{\"ac\":[]}", false);
    else if (interruption == 2) response(503, "{}");
    else if (interruption == 3) response(-1, "{}");
    else cadence.resetValidation();
  }
  response(200, "{\"ac\":[]}"); response(200, "{\"ac\":[]}");
  CHECK(Preferences::uintWriteAttempts == 0);
  response(200, "{\"ac\":[]}");
  CHECK(cadence.isSaved()); CHECK(Preferences::values["radarMs"] == "20000");
  CHECK(Preferences::uintWriteAttempts == 1); CHECK(Preferences::writes.size() == 1);
  for (int n = 0; n < 10; ++n) response(200, "{\"ac\":[]}");
  CHECK(Preferences::uintWriteAttempts == 1);

  FlightBoardRadarCadence restarted;
  restarted.begin(storage);
  CHECK(restarted.intervalMs() == 20000); CHECK(restarted.isSaved());
  for (int n = 0; n < 5; ++n) restarted.complete(200, 20000UL * n, 15000, true);
  CHECK(Preferences::uintWriteAttempts == 1);
  response(429, "{}"); testNow += 120000;
  CHECK(cadence.intervalMs() == 25000); CHECK(!cadence.isSaved());
  response(200, "{\"ac\":[]}"); response(200, "{\"ac\":[]}");
  CHECK(Preferences::uintWriteAttempts == 1);
  response(200, "{\"ac\":[]}");
  CHECK(cadence.isSaved()); CHECK(Preferences::values["radarMs"] == "25000");
  CHECK(Preferences::uintWriteAttempts == 2);
  FlightBoardRadarCadence newerRestart;
  newerRestart.begin(storage);
  CHECK(newerRestart.intervalMs() == 25000); CHECK(newerRestart.isSaved());
}

void test_cadence_storage_bounds_and_corruption() {
  reset();
  Preferences storage;
  for (const char* saved : {"15000", "20000", "115000", "120000"}) {
    resetCadenceStorage(); Preferences::values["radarMs"] = saved;
    FlightBoardRadarCadence cadence; cadence.begin(storage);
    CHECK(cadence.intervalMs() == (unsigned long)std::atoi(saved));
    CHECK(cadence.isSaved()); CHECK(Preferences::uintWriteAttempts == 0);
  }
  for (const char* corrupt : {"", "0", "10000", "14999", "15001", "19999",
      "120001", "125000", "4294967295", "-1", "bad", "15000.0"}) {
    resetCadenceStorage(); Preferences::values["radarMs"] = corrupt;
    FlightBoardRadarCadence cadence; cadence.begin(storage);
    CHECK(cadence.intervalMs() == 15000); CHECK(!cadence.isSaved());
    CHECK(Preferences::uintWriteAttempts == 0); // No repair writes at startup.
    cadence.complete(200, 0, 15000, true);
    cadence.complete(200, 15000, 15000, true);
    CHECK(Preferences::uintWriteAttempts == 0);
    cadence.complete(200, 30000, 15000, true);
    CHECK(cadence.isSaved()); CHECK(Preferences::values["radarMs"] == "15000");
    CHECK(Preferences::uintWriteAttempts == 1);
  }
}

void test_cadence_storage_failure_and_recovery() {
  reset(); resetCadenceStorage();
  Preferences storage;
  Preferences::values["radarMs"] = "20000";
  FlightBoardRadarCadence cadence; cadence.begin(storage);
  cadence.complete(429, 0);
  Preferences::failKey = "radarMs";
  cadence.complete(200, 120000, 15000, true);
  cadence.complete(200, 145000, 15000, true);
  cadence.complete(200, 170000, 15000, true);
  CHECK(Preferences::uintWriteAttempts == 1); CHECK(Preferences::writes.empty());
  CHECK(cadence.intervalMs() == 25000); CHECK(!cadence.isSaved());
  CHECK(Preferences::values["radarMs"] == "20000");
  FlightBoardRadarCadence rebootAfterFailure; rebootAfterFailure.begin(storage);
  CHECK(rebootAfterFailure.intervalMs() == 20000); CHECK(rebootAfterFailure.isSaved());
  Preferences::failKey.clear();
  for (uint32_t now = 195000; now < 470000; now += 25000) {
    CHECK(cadence.complete(200, now, 15000, true) == 25000);
    CHECK(Preferences::uintWriteAttempts == 1);
  }
  cadence.complete(200, 469999, 15000, true);
  CHECK(Preferences::uintWriteAttempts == 1);
  cadence.complete(200, 470000, 15000, true);
  CHECK(cadence.isSaved()); CHECK(Preferences::uintWriteAttempts == 2);
  CHECK(Preferences::writes.size() == 1); CHECK(Preferences::values["radarMs"] == "25000");

  // Unavailable NVS never prevents radar operation; retry timing survives wrap.
  resetCadenceStorage(); Preferences::available = false;
  FlightBoardRadarCadence offline; offline.begin(storage);
  CHECK(offline.intervalMs() == 15000); CHECK(!offline.isSaved());
  const uint32_t failedAt = 0xfffffff0UL;
  offline.complete(200, (uint32_t)(failedAt - 30000UL), 15000, true);
  offline.complete(200, (uint32_t)(failedAt - 15000UL), 15000, true);
  CHECK(offline.complete(200, failedAt, 15000, true) == 15000);
  CHECK(Preferences::uintWriteAttempts == 1); CHECK(!offline.isSaved());
  Preferences::available = true;
  offline.complete(200, (uint32_t)(failedAt + 299999UL), 15000, true);
  CHECK(Preferences::uintWriteAttempts == 1);
  offline.complete(200, (uint32_t)(failedAt + 300000UL), 15000, true);
  CHECK(Preferences::uintWriteAttempts == 2); CHECK(offline.isSaved());
  FlightBoardRadarCadence rebootAfterRecovery; rebootAfterRecovery.begin(storage);
  CHECK(rebootAfterRecovery.intervalMs() == 15000); CHECK(rebootAfterRecovery.isSaved());
}

int main(){std::vector<std::pair<const char*,std::function<void()>>> tests={{"nearest40 from reversed60; airline and units",test_top40_unsorted},{"stale/invalid/outside positions and unavailable metrics",test_invalid_positions_and_fields},{"HTTP/JSON/connection errors and429 backoff",test_errors_and_backoff},{"single queued job and result ownership/generation",test_queue_handoff_and_generation},{"queue/task allocation failure cleanup",test_queue_creation_failures},{"distance zero/antipodes/dateline/poles",test_distance_boundaries},{"distinct HTTP, JSON and connection diagnostics",test_diagnostic_causes},{"bounded response bytes, slow/truncated bodies and rollover",test_bounded_network_body},{"nautical-mile API, exact km filtering and worker availability",test_protocol_and_availability},{"TLS packet gaps, open stalls, actual EOF and rollover",test_tls_receive_gaps_and_closed_eof},{"session cadence starts at15s and adds5s per429, retaining accepted rate",test_adaptive_session_cadence},{"Retry-After seconds, mandatory cooldown and millis rollover",test_retry_after_and_cooldown_rollover},{"three usable consecutive responses persist a cadence across reboot",test_validated_cadence_persistence_and_reboot},{"saved cadence bounds and corrupt NVS recovery",test_cadence_storage_bounds_and_corruption},{"NVS failure retains RAM operation and retries after five minutes",test_cadence_storage_failure_and_recovery}};int failed=0;for(auto&t:tests)try{t.second();std::cout<<"PASS "<<t.first<<'\n';}catch(const std::exception&e){++failed;std::cerr<<"FAIL "<<t.first<<": "<<e.what()<<'\n';}std::cout<<tests.size()<<" scenarios, "<<assertions<<" assertions, "<<failed<<" failed\n";return failed?1:0;}
