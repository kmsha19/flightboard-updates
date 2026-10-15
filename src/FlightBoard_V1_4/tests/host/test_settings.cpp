#include "FlightBoardDisplay.h"
#include "FlightBoardFavorites.h"
#include "AirlineLogos.h"
#include <stdexcept>
#include <new>
#include "settings_under_test.inc"
static int assertions=0;
#define CHECK(expr) do{++assertions;if(!(expr))throw std::runtime_error(std::string(__func__)+":"+std::to_string(__LINE__)+": " #expr);}while(0)
void reset(){Preferences::values.clear();Preferences::writes.clear();Preferences::available=true;Preferences::failKey.clear();settings=BoardSettings();selectedHex="";favorites.~FlightBoardFavorites();new(&favorites)FlightBoardFavorites(storage);}
JsonDocument validBody(){JsonDocument d;d["place"]=" Test ";d["lat"]=48.5;d["lon"]=2;d["radiusKm"]=20;d["brightness"]=65;return d;}
bool parse(const JsonDocument& d,BoardSettings& next){return parseSettings(d.as<JsonObjectConst>(),next);}
void test_base_parse_and_optional_retention(){reset();auto d=validBody();BoardSettings next;next.timezone="london";next.nightEnabled=true;next.mode=2;CHECK(parse(d,next));CHECK(next.place=="Test");CHECK(next.lat==48.5);CHECK(next.lon==2);CHECK(next.radiusKm==20);CHECK(next.brightness==65);CHECK(next.timezone=="london");CHECK(next.nightEnabled);CHECK(next.mode==2);CHECK(Preferences::writes.empty());}
void test_numeric_and_type_validation(){reset();for(const char* key:{"lat","lon","radiusKm","brightness"}){auto d=validBody();d[key]="20";BoardSettings next;CHECK(!parse(d,next));d[key]=true;CHECK(!parse(d,next));d[key]=nullptr;CHECK(!parse(d,next));}
  for(const char*key:{"radiusKm","brightness","nightStart","nightEnd","nightBrightness"}){auto d=validBody();d[key]=22.5;BoardSettings next;CHECK(!parse(d,next));}
  for(const char*key:{"place","timezone","nightAction"}){auto d=validBody();d[key]=42;BoardSettings next;CHECK(!parse(d,next));}
  auto d=validBody();d["nightEnabled"]="true";BoardSettings next;CHECK(!parse(d,next));d["nightEnabled"]=1;CHECK(!parse(d,next));
  d=validBody();d["lat"]=NAN;CHECK(!parse(d,next));d=validBody();d["lon"]=INFINITY;CHECK(!parse(d,next));
}
void test_embedded_null_rejected(){reset();for(const auto& pair:std::vector<std::pair<const char*,const char*>>{{"place","\"Niort\\u0000ignored\""},{"timezone","\"paris\\u0000ignored\""},{"nightAction","\"dim\\u0000ignored\""}}){auto d=validBody();JsonDocument value;CHECK(!deserializeJson(value,pair.second));d[pair.first]=value.as<JsonVariantConst>();BoardSettings next;CHECK(!parse(d,next));}}
void test_boundaries_and_schedule(){reset();for(const auto& pair:std::vector<std::pair<const char*,double>>{{"lat",-90.1},{"lat",90.1},{"lon",-180.1},{"lon",180.1},{"radiusKm",4},{"radiusKm",51},{"brightness",9},{"brightness",101},{"nightStart",-1},{"nightStart",1440},{"nightEnd",-1},{"nightEnd",1440},{"nightBrightness",9},{"nightBrightness",101}}){auto d=validBody();d[pair.first]=pair.second;BoardSettings next;CHECK(!parse(d,next));}
  auto d=validBody();d["place"]=" \t ";BoardSettings next;CHECK(!parse(d,next));d["place"]=std::string(33,'a');CHECK(!parse(d,next));
  d=validBody();d["timezone"]="invalid";CHECK(!parse(d,next));d=validBody();d["nightAction"]="invalid";CHECK(!parse(d,next));
  next=BoardSettings();d=validBody();d["nightEnabled"]=true;d["nightStart"]=600;d["nightEnd"]=600;CHECK(!parse(d,next));d["nightEnabled"]=false;CHECK(parse(d,next));d["nightEnabled"]=true;d["nightStart"]=1439;d["nightEnd"]=0;CHECK(parse(d,next));d["nightAction"]="sleep";CHECK(parse(d,next));CHECK(next.nightAction=="sleep");
  for(const char* zone:{"paris","london","utc","newyork","dubai"}){d=validBody();d["timezone"]=zone;CHECK(parse(d,next));CHECK(next.timezone==zone);}
}
void test_legacy_migration(){reset();Preferences::values["lat"]="49.0097";Preferences::values["lon"]="2.5479";Preferences::values["radius"]="20";Preferences::values["bright"]="70";Preferences::values["mode"]="2";Preferences::values["place"]="CDG perso";loadSettings();CHECK(settings.lat==49.0097);CHECK(settings.lon==2.5479);CHECK(settings.radiusKm==20);CHECK(settings.brightness==70);CHECK(settings.mode==2);CHECK(settings.place=="CDG perso");CHECK(settings.timezone=="paris");CHECK(!settings.nightEnabled);CHECK(favorites.get("cdg"));CHECK(Preferences::values["place"]=="CDG perso");}
void test_v2_persist_and_restart(){reset();BoardSettings next;next.place="Maison";next.lat=48;next.lon=2;next.brightness=80;next.timezone="dubai";next.nightEnabled=true;next.nightStart=1350;next.nightEnd=390;next.nightAction="sleep";next.mode=2;CHECK(persistSettings(next));CHECK(Preferences::values.find("settings2")!=Preferences::values.end());CHECK(settings.place=="Niort");loadSettings();CHECK(settings.place=="Maison");CHECK(settings.timezone=="dubai");CHECK(settings.nightEnabled);CHECK(settings.nightStart==1350);CHECK(settings.nightEnd==390);CHECK(settings.nightAction=="sleep");CHECK(settings.mode==2);
  settings.mode=1;CHECK(persistSettings(settings));selectedHex="abc";favorites.~FlightBoardFavorites();new(&favorites)FlightBoardFavorites(storage);loadSettings();CHECK(settings.mode==0);CHECK(selectedHex.isEmpty());}
void test_storage_failures_and_fallback(){reset();BoardSettings next;next.place="Old";CHECK(persistSettings(next));String stored=Preferences::values["settings2"];Preferences::failKey="settings2";next.place="New";CHECK(!persistSettings(next));CHECK(Preferences::values["settings2"]==stored);CHECK(settings.place=="Niort");
  reset();Preferences::values["place"]="Legacy";Preferences::values["settings2"]="invalid json";loadSettings();CHECK(settings.place=="Legacy");
  reset();Preferences::values["lat"]="190";loadSettings();CHECK(settings.lat==46.324);CHECK(settings.place=="Niort");
  reset();auto d=validBody();d["timezone"]="invalid";String raw;serializeJson(d,raw);Preferences::values["settings2"]=raw;Preferences::values["place"]="Legacy";loadSettings();CHECK(settings.place=="Legacy");}
void test_zcs_defaults_and_legacy_retention(){
  reset();
  CHECK(settings.app=="aviation");CHECK(settings.visual=="lava");
  CHECK(!settings.aviationLogoOnly);CHECK(settings.aviationAirline.isEmpty());
  CHECK(settings.aviationType.isEmpty());CHECK(settings.aviationMaxAltitudeFt==0);
  auto d=validBody();d["mode"]=2;String raw;serializeJson(d,raw);
  Preferences::values["settings2"]=raw;loadSettings();
  CHECK(settings.app=="aviation");CHECK(settings.visual=="lava");CHECK(settings.mode==2);
  Preferences::writes.clear();
  BoardSettings next=settings;next.app="weather";next.visual="stars";
  next.aviationLogoOnly=true;next.aviationAirline="logo_afr";
  next.aviationType="B38M";next.aviationMaxAltitudeFt=18000;
  CHECK(parse(d,next));CHECK(next.app=="weather");CHECK(next.visual=="stars");
  CHECK(next.aviationLogoOnly);CHECK(next.aviationAirline=="logo_afr");
  CHECK(next.aviationType=="B38M");CHECK(next.aviationMaxAltitudeFt==18000);
  CHECK(Preferences::writes.empty());
}
void test_zcs_strict_validation(){
  reset();
  for(const char* app:{"aviation","clock","animation","weather","message","timer","score","pixel"}){
    auto d=validBody();d["app"]=app;BoardSettings next;CHECK(parse(d,next));CHECK(next.app==app);
  }
  for(const char* visual:{"lava","stars","rain","plain","aurora","ocean","warp","chrono"}){
    auto d=validBody();d["visual"]=visual;BoardSettings next;CHECK(parse(d,next));CHECK(next.visual==visual);
  }
  for(const char* key:{"app","visual","aviationLogoOnly","aviationAirline","aviationType","aviationMaxAltitudeFt"}){
    auto d=validBody();BoardSettings next;d[key]=nullptr;CHECK(!parse(d,next));
    d[key].to<JsonArray>();CHECK(!parse(d,next));
  }
  for(const char* key:{"app","visual","aviationAirline","aviationType"}){
    auto d=validBody();BoardSettings next;d[key]=42;CHECK(!parse(d,next));d[key]=true;CHECK(!parse(d,next));
    JsonDocument value;CHECK(!deserializeJson(value,"\"aviation\\u0000ignored\""));d[key]=value.as<JsonVariantConst>();CHECK(!parse(d,next));
  }
  for(const auto& invalid:std::vector<std::pair<const char*,const char*>>{
      {"app","Weather"},{"app","weather "},{"app","sports"},{"app",""},
      {"visual","Stars"},{"visual","other"},{"visual",""},
      {"aviationAirline","logo_default"},{"aviationAirline","AFR"},{"aviationAirline","logo_unknown"},
      {"aviationType","a320"},{"aviationType","A"},{"aviationType","A320X"},
      {"aviationType","A320\n"},{"aviationType","B-38"},{"aviationType","—"}}){
    auto d=validBody();d[invalid.first]=invalid.second;BoardSettings next;CHECK(!parse(d,next));
  }
  for(const char* type:{"","A1","AB3","A320","B38M","PC21"}){
    auto d=validBody();d["aviationType"]=type;BoardSettings next;CHECK(parse(d,next));CHECK(next.aviationType==type);
  }
  for(size_t i=0;i<AIRLINE_COUNT;++i){
    auto d=validBody();d["aviationAirline"]=AIRLINES[i].id;BoardSettings next;
    CHECK(parse(d,next));CHECK(next.aviationAirline==AIRLINES[i].id);
  }
  for(int altitude:{0,1,10000,60000}){
    auto d=validBody();d["aviationMaxAltitudeFt"]=altitude;BoardSettings next;
    CHECK(parse(d,next));CHECK(next.aviationMaxAltitudeFt==altitude);
  }
  for(double altitude:{-1.,60001.,1.5}){
    auto d=validBody();d["aviationMaxAltitudeFt"]=altitude;BoardSettings next;CHECK(!parse(d,next));
  }
  for(const char* key:{"aviationLogoOnly","aviationMaxAltitudeFt"}){
    auto d=validBody();d[key]="1";BoardSettings next;CHECK(!parse(d,next));
  }
  auto d=validBody();d["aviationLogoOnly"]=1;BoardSettings next;CHECK(!parse(d,next));
  d=validBody();d["aviationMaxAltitudeFt"]=true;CHECK(!parse(d,next));
  CHECK(settings.app=="aviation");CHECK(settings.aviationType.isEmpty());CHECK(Preferences::writes.empty());
}
void test_zcs_restart_and_failed_write(){
  reset();BoardSettings next;next.app="weather";next.visual="rain";next.mode=2;
  next.aviationLogoOnly=true;next.aviationAirline="logo_bel";
  next.aviationType="B738";next.aviationMaxAltitudeFt=32000;
  CHECK(persistSettings(next));loadSettings();
  CHECK(settings.app=="weather");CHECK(settings.visual=="rain");CHECK(settings.mode==2);
  CHECK(settings.aviationLogoOnly);CHECK(settings.aviationAirline=="logo_bel");
  CHECK(settings.aviationType=="B738");CHECK(settings.aviationMaxAltitudeFt==32000);
  String raw=Preferences::values["settings2"];
  Preferences::failKey="settings2";next.app="animation";CHECK(!persistSettings(next));
  CHECK(Preferences::values["settings2"]==raw);CHECK(settings.app=="weather");
  Preferences::failKey.clear();auto d=validBody();d["app"]="sports";
  next=settings;CHECK(!parse(d,next));CHECK(Preferences::values["settings2"]==raw);
  CHECK(settings.app=="weather");CHECK(settings.aviationType=="B738");
}
void test_fonts_persistence(){
 reset();BoardSettings next;CHECK(next.fontClock=="classic");
 for(const char* font:{"classic","modern","digital","arcade","compact"}){
  auto doc=validBody();doc["fontClock"]=font;doc["fontMessage"]="arcade";CHECK(parse(doc,next));CHECK(next.fontClock==font);
  CHECK(persistSettings(next));loadSettings();CHECK(settings.fontClock==font);CHECK(settings.fontMessage=="arcade");CHECK(settings.fontWeather=="classic");
 }
 for(const char* key:{"fontClock","fontAviation","fontWeather","fontMessage","fontTimer","fontScore"}){
  auto doc=validBody();doc[key]="unknown";CHECK(!parse(doc,next));doc[key]=1;CHECK(!parse(doc,next));doc[key]=nullptr;CHECK(!parse(doc,next));
 }
 auto doc=validBody();deserializeJson(doc,"{\"fontClock\":\"modern\\u0000bad\"}");CHECK(!parse(doc,next));
}
int main(){std::vector<std::pair<const char*,std::function<void()>>> tests={{"Font validation, per-app defaults and persistence",test_fonts_persistence},{"ZCS defaults and legacy optional retention",test_zcs_defaults_and_legacy_retention},{"ZCS strict apps, visuals and aviation filter validation",test_zcs_strict_validation},{"ZCS restart and failed write preserve active settings",test_zcs_restart_and_failed_write},{"base fields trim and optional old settings retained",test_base_parse_and_optional_retention},{"strict numeric/string/bool and nonfinite validation",test_numeric_and_type_validation},{"embedded null rejected before Arduino String conversion",test_embedded_null_rejected},{"coordinate/brightness/time bounds and enabled zero duration",test_boundaries_and_schedule},{"V1.9 coordinate/place/mode migration",test_legacy_migration},{"V2 atomic JSON persistence and clock/manual reboot modes",test_v2_persist_and_restart},{"failed storage preserves settings and corrupt recovery",test_storage_failures_and_fallback}};int failed=0;for(auto&t:tests)try{t.second();std::cout<<"PASS "<<t.first<<'\n';}catch(const std::exception&e){++failed;std::cerr<<"FAIL "<<t.first<<": "<<e.what()<<'\n';}std::cout<<tests.size()<<" scenarios, "<<assertions<<" assertions, "<<failed<<" failed\n";return failed?1:0;}
