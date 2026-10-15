#define main previous_wifi_main
#include "test_wifi.cpp"
#undef main
bool otaBusy = false;
static bool isOtaBusy() { return otaBusy; }
void test_ota_lock() {
 reset();seed();WebServer s;FlightBoardWiFi wifi(s);wifi.setBusyCheck(isOtaBusy);wifi.begin(nullptr,nullptr);WiFi.connect("Previous");wifi.update();
 CHECK(wifi.allowsRadar());
 const int beforeBegins=WiFi.beginCalls,beforeDisconnects=WiFi.disconnectCalls,beforeAps=WiFi.apStartCalls;
 otaBusy=true;
 s.request("/api/wifi",HTTP_POST,candidate());CHECK(s.responseCode==503);CHECK(response(s)["error"].is<String>());
 CHECK(WiFi.beginCalls==beforeBegins);CHECK(WiFi.disconnectCalls==beforeDisconnects);CHECK(WiFi.apStartCalls==beforeAps);CHECK(stationWrites()==0);CHECK(wifi.allowsRadar());
 s.request("/api/wifi/scan",HTTP_GET,"",true);CHECK(s.responseCode==503);CHECK(WiFi.scanCalls==0);CHECK(WiFi.scanDeleteCalls==0);CHECK(wifi.allowsRadar());
 s.request("/api/wifi");CHECK(s.responseCode==200);CHECK(response(s)["ssid"]=="Previous");CHECK(response(s)["pendingSsid"]=="");
 s.request("/api/wifi/keepalive");CHECK(s.responseCode==200);CHECK(response(s)["ok"]==true);
 otaBusy=false;s.request("/api/wifi",HTTP_POST,candidate());CHECK(s.responseCode==202);
 reset();seed();WebServer s2;FlightBoardWiFi wifi2(s2);wifi2.setBusyCheck(isOtaBusy);wifi2.begin(nullptr,nullptr);otaBusy=true;wifi2.setBusyCheck(nullptr);s2.request("/api/wifi",HTTP_POST,candidate());CHECK(s2.responseCode==202);
}
void test_ota_preserves_fallback() {
 reset();WebServer s;FlightBoardWiFi wifi(s);wifi.begin("Legacy","legacy-pass");
 CHECK(!wifi.prepareUpdate());CHECK(stationWrites()==0);
 WiFi.connect("Legacy");wifi.update();Preferences::failStationWrite=true;
 CHECK(!wifi.prepareUpdate());Preferences::failStationWrite=false;
 CHECK(wifi.prepareUpdate());CHECK(stored()["ssid"]=="Legacy");CHECK(stored()["password"]=="legacy-pass");
 const int writes=stationWrites();CHECK(wifi.prepareUpdate());CHECK(stationWrites()==writes);
 WebServer nextServer;FlightBoardWiFi publicFirmware(nextServer);publicFirmware.begin(nullptr,nullptr);
 CHECK(WiFi.lastBeginSsid=="Legacy");CHECK(WiFi.lastBeginPassword=="legacy-pass");
 WiFi.connect("Legacy");publicFirmware.update();nextServer.request("/api/wifi",HTTP_POST,candidate());
 CHECK(!publicFirmware.prepareUpdate());CHECK(stationWrites()==writes);
 reset();WebServer emptyServer;FlightBoardWiFi empty(emptyServer);empty.begin(nullptr,nullptr);CHECK(empty.prepareUpdate());
 reset();Preferences::available=false;WebServer unavailableServer;FlightBoardWiFi unavailable(unavailableServer);
 unavailable.begin("Legacy","legacy-pass");WiFi.connect("Legacy");unavailable.update();CHECK(!unavailable.prepareUpdate());
}
int main(){if(previous_wifi_main())return 1;try{test_ota_lock();test_ota_preserves_fallback();std::cout<<"PASS OTA Wi-Fi guard; "<<assertions<<" total assertions\n";return 0;}catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
