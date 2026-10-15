#include "FlightBoardDisplay.h"
#include "FlightBoardFavorites.h"
#include "FlightBoardRadar.h"
#include <stdexcept>
#include "settings_under_test.inc"
#include "filters_under_test.inc"
static String requestBody;
static int responseStatus=0;
bool readJsonBody(JsonDocument& body){return !deserializeJson(body,requestBody);}
void sendError(int status,const String&){responseStatus=status;}
void sendRadarState(){responseStatus=200;}
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
  for(const char* app:{"clock","animation","weather"}){
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
int main(){test_filter_combinations();test_active_selection_uses_same_filters();test_atomic_handlers_and_pinned_filter();std::cout<<"ZCS aviation filters and app handlers: "<<assertions<<" assertions passed\n";}
