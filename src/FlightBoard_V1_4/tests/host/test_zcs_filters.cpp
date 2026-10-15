#include "FlightBoardDisplay.h"
#include "FlightBoardFavorites.h"
#include "FlightBoardRadar.h"
#include "ZcsWidgets.h"
#include <stdexcept>
#include "settings_under_test.inc"
#include "filters_under_test.inc"
static String requestBody;
static int responseStatus=0;
bool readJsonBody(JsonDocument& body){return !deserializeJson(body,requestBody);}
void sendError(int status,const String&){responseStatus=status;}
void sendRadarState(){responseStatus=200;}
static JsonDocument activePack;
static JsonDocument preparedPack;
static ZcsTimer widgetTimer;
static int prepareCalls = 0, activationCalls = 0, discardCalls = 0;
static bool preparationFails = false;
bool readPack(const char* id, JsonDocument& pack, bool = false) {
  if (!strcmp(id, "test-message")) { pack["id"] = id; pack["type"] = "message"; pack["text"] = "HELLO"; return true; }
  if (!strcmp(id, "test-timer")) { pack["id"] = id; pack["type"] = "timer"; pack["durationSeconds"] = 45; return true; }
  if (!strcmp(id, "test-pixel")) { pack["id"] = id; pack["type"] = "pixel"; return true; }
  return false;
}
bool preparePack(const String& id) { ++prepareCalls; preparedPack.clear(); return !preparationFails && (id.isEmpty() || readPack(id.c_str(), preparedPack)); }
void commitPreparedPack() { ++activationCalls; activePack=std::move(preparedPack); preparedPack.clear(); }
void discardPreparedPack() { ++discardCalls; preparedPack.clear(); }
#include "zcs_handlers_under_test.inc"

static int assertions=0;
#define CHECK(expr) do { ++assertions; if (!(expr)) throw std::runtime_error(std::string(__func__)+":"+std::to_string(__LINE__)+": " #expr); } while (0)

AircraftInfo plane(const char* callsign, const char* type, int altitude, bool altitudeAvailable=true) {
  AircraftInfo p;p.callsign=callsign;p.hex=callsign;p.aircraftType=type;
  p.airline=findAirline(callsign);p.altitudeFt=altitude;p.altitudeAvailable=altitudeAvailable;
  return p;
}
void test_filter_combinations() {
  const auto afr=plane("AFR123","A320",10000), hop=plane("HOP123","A320",9000),
    bel=plane("BEL123","A320",10000), unknown=plane("N12345","A320",5000),
    b738=plane("AFR999","B738",9000), high=plane("AFR321","A320",10001),
    noAltitude=plane("AFR456","A320",0,false);
  auto noLogo=unknown;noLogo.airline=nullptr;
  BoardSettings filter;
  for(const auto& p:{afr,hop,bel,unknown,b738,high,noAltitude,noLogo})CHECK(matchesAviationFilters(p,filter));
  filter.aviationLogoOnly=true;
  CHECK(matchesAviationFilters(afr,filter));CHECK(matchesAviationFilters(bel,filter));
  CHECK(!matchesAviationFilters(unknown,filter));CHECK(!matchesAviationFilters(noLogo,filter));
  filter.aviationAirline="logo_afr";
  CHECK(matchesAviationFilters(afr,filter));CHECK(matchesAviationFilters(hop,filter));
  CHECK(!matchesAviationFilters(bel,filter));
  filter.aviationType="A320";
  CHECK(matchesAviationFilters(afr,filter));CHECK(!matchesAviationFilters(b738,filter));
  filter.aviationMaxAltitudeFt=10000;
  CHECK(matchesAviationFilters(afr,filter));CHECK(matchesAviationFilters(hop,filter));
  CHECK(!matchesAviationFilters(high,filter));CHECK(!matchesAviationFilters(noAltitude,filter));
  auto negative=afr;negative.altitudeFt=-50;CHECK(matchesAviationFilters(negative,filter));
  auto absentType=afr;absentType.aircraftType="";CHECK(!matchesAviationFilters(absentType,filter));
  filter.aviationMaxAltitudeFt=0;CHECK(matchesAviationFilters(noAltitude,filter));
  filter.aviationType="";CHECK(matchesAviationFilters(absentType,filter));
}
void test_active_selection_uses_same_filters() {
  settings=BoardSettings();radarCount=4;
  radarPlanes[0]=plane("N12345","C172",3000);
  radarPlanes[1]=plane("BEL123","A320",31000);
  radarPlanes[2]=plane("AFR123","A320",10000);
  radarPlanes[3]=plane("HOP123","E190",9000);
  CHECK(activeAircraftIndex()==0);
  settings.aviationLogoOnly=true;CHECK(activeAircraftIndex()==1);
  settings.aviationMaxAltitudeFt=10000;CHECK(activeAircraftIndex()==2);
  settings.aviationType="E190";CHECK(activeAircraftIndex()==3);
  settings.aviationAirline="logo_bel";CHECK(activeAircraftIndex()==-1);
  settings.aviationAirline="logo_afr";selectedHex="HOP123";settings.mode=1;
  CHECK(selectedAircraftMatches(settings));CHECK(activeAircraftIndex()==3);
  settings.aviationType="A320";
  CHECK(!selectedAircraftMatches(settings));CHECK(activeAircraftIndex()==-1);
  selectedHex="AFR123";CHECK(selectedAircraftMatches(settings));CHECK(activeAircraftIndex()==2);
  for(const char* app:{"clock","animation","weather","message","timer","score","pixel"}){
    settings.app=app;CHECK(activeAircraftIndex()==-1);CHECK(settings.mode==1);CHECK(selectedHex=="AFR123");
  }
  settings.app="aviation";CHECK(activeAircraftIndex()==2);
  settings.mode=2;CHECK(activeAircraftIndex()==-1);
  settings.mode=0;CHECK(activeAircraftIndex()==2);
  radarCount=0;CHECK(activeAircraftIndex()==-1);CHECK(!selectedAircraftMatches(settings));
}
void test_atomic_handlers_and_pinned_filter(){
  settings=BoardSettings();settings.mode=1;selectedHex="AFR123";radarCount=1;
  radarPlanes[0]=plane("AFR123","A320",10000);
  Preferences::values.clear();Preferences::writes.clear();Preferences::failKey.clear();
  requestBody="{\"app\":\"weather\",\"visual\":\"rain\"}";changeApp();
  CHECK(responseStatus==200);CHECK(settings.app=="weather");CHECK(settings.visual=="rain");
  CHECK(settings.mode==1);CHECK(selectedHex=="AFR123");CHECK(Preferences::writes.size()==1);
  const String saved=Preferences::values["settings2"];
  for(const char* invalid:{"{\"app\":\"clock\",\"visual\":null}","{\"app\":\"clock\",\"visual\":\"rain\\u0000bad\"}","{\"app\":\"sports\"}","{\"app\":\"clock\",\"visual\":\"other\"}"}){
    requestBody=invalid;changeApp();CHECK(responseStatus==400);CHECK(settings.app=="weather");CHECK(settings.visual=="rain");CHECK(Preferences::values["settings2"]==saved);CHECK(Preferences::writes.size()==1);
  }
  Preferences::failKey="settings2";requestBody="{\"app\":\"animation\",\"visual\":\"stars\"}";changeApp();
  CHECK(responseStatus==503);CHECK(settings.app=="weather");CHECK(settings.visual=="rain");CHECK(Preferences::values["settings2"]==saved);
  Preferences::failKey.clear();requestBody="{\"app\":\"clock\"}";changeApp();CHECK(responseStatus==200);CHECK(settings.app=="clock");CHECK(settings.visual=="rain");
  requestBody="{\"logoOnly\":true,\"airline\":\"logo_afr\",\"type\":\"B738\",\"maxAltitudeFt\":0}";
  Preferences::failKey="settings2";changeAviationFilters();CHECK(responseStatus==503);CHECK(settings.mode==1);CHECK(selectedHex=="AFR123");CHECK(settings.aviationType=="");
  Preferences::failKey.clear();changeAviationFilters();CHECK(responseStatus==200);CHECK(settings.mode==0);CHECK(selectedHex.isEmpty());CHECK(settings.aviationType=="B738");CHECK(settings.app=="clock");
  settings.mode=1;selectedHex="AFR123";requestBody="{\"logoOnly\":true,\"airline\":\"logo_afr\",\"type\":\"A320\",\"maxAltitudeFt\":10000}";changeAviationFilters();
  CHECK(responseStatus==200);CHECK(settings.mode==1);CHECK(selectedHex=="AFR123");
  requestBody="{\"logoOnly\":true,\"airline\":\"logo_default\",\"type\":\"A320\",\"maxAltitudeFt\":0}";changeAviationFilters();
  CHECK(responseStatus==400);CHECK(settings.aviationAirline=="logo_afr");CHECK(selectedHex=="AFR123");
}
void test_widgets_and_installed_content() {
  settings=BoardSettings();settings.mode=1;selectedHex="AFR123";activePack.clear();
  Preferences::values.clear();Preferences::writes.clear();Preferences::failKey.clear();prepareCalls=activationCalls=discardCalls=0;preparationFails=false;preparedPack.clear();
  for(const char* app:{"message","timer","score"}) {
    requestBody=String("{\"app\":\"")+app+"\"}";changeApp();
    CHECK(responseStatus==200);CHECK(settings.app==app);CHECK(settings.contentId.isEmpty());CHECK(settings.mode==1);CHECK(selectedHex=="AFR123");
  }
  const String nativeSaved=Preferences::values["settings2"];
  for(const char* body:{"{\"app\":\"pixel\"}","{\"app\":\"pixel\",\"contentId\":\"missing\"}",
      "{\"app\":\"message\",\"contentId\":\"test-pixel\"}","{\"app\":\"pixel\",\"contentId\":null}",
      "{\"app\":\"pixel\",\"contentId\":\"test-pixel\\u0000bad\"}"}) {
    requestBody=body;changeApp();CHECK(responseStatus==400);CHECK(settings.app=="score");CHECK(Preferences::values["settings2"]==nativeSaved);
  }
  requestBody="{\"app\":\"pixel\",\"contentId\":\"test-pixel\"}";changeApp();
  CHECK(responseStatus==200);CHECK(settings.app=="pixel");CHECK(settings.contentId=="test-pixel");CHECK(activePack["type"]=="pixel");
  const String pixelSaved=Preferences::values["settings2"];const int calls=activationCalls, preparations=prepareCalls, discards=discardCalls;const size_t writes=Preferences::writes.size();
  Preferences::failKey="settings2";requestBody="{\"app\":\"message\",\"contentId\":\"test-message\"}";changeApp();
  CHECK(responseStatus==503);CHECK(settings.app=="pixel");CHECK(activePack["type"]=="pixel");CHECK(activationCalls==calls);CHECK(Preferences::values["settings2"]==pixelSaved);
  CHECK(prepareCalls==preparations+1);CHECK(discardCalls==discards+1);CHECK(preparedPack.isNull());CHECK(Preferences::writes.size()==writes);
  Preferences::failKey.clear();preparationFails=true;changeApp();
  CHECK(responseStatus==503);CHECK(settings.app=="pixel");CHECK(settings.contentId=="test-pixel");CHECK(activePack["type"]=="pixel");CHECK(Preferences::values["settings2"]==pixelSaved);
  CHECK(Preferences::writes.size()==writes);CHECK(activationCalls==calls);CHECK(preparedPack.isNull());
  preparationFails=false;widgetTimer=ZcsTimer(300);requestBody="{\"app\":\"timer\",\"contentId\":\"test-timer\"}";changeApp();
  CHECK(responseStatus==200);CHECK(widgetTimer.durationSeconds()==45);CHECK(!widgetTimer.running(0));CHECK(widgetTimer.remainingSeconds(0)==45);
  widgetTimer.start(0);changeApp();CHECK(widgetTimer.running(1000));CHECK(widgetTimer.remainingSeconds(1000)==44); // Opening the same installed timer never resets it.
  requestBody="{\"app\":\"clock\"}";changeApp();CHECK(responseStatus==200);CHECK(settings.contentId.isEmpty());CHECK(activePack.isNull());CHECK(widgetTimer.running(2000));
}
int main(){test_filter_combinations();test_active_selection_uses_same_filters();test_atomic_handlers_and_pinned_filter();test_widgets_and_installed_content();std::cout<<"ZCS aviation filters and app handlers: "<<assertions<<" assertions passed\n";}
