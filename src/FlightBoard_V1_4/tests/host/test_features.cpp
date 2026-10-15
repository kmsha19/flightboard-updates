#include "FlightBoardDisplay.h"
#include "FlightBoardFavorites.h"
#include <cstdlib>
#include <ctime>
#include <stdexcept>

static int assertions = 0;
#define CHECK(expr) do { ++assertions; if(!(expr)) throw std::runtime_error(std::string(__func__)+":"+std::to_string(__LINE__)+": " #expr); } while(0)

void reset() {
  Preferences::values.clear();
  Preferences::writes.clear();
  Preferences::available=true;
  Preferences::failStationWrite=false;
  Preferences::failKey.clear();
}
JsonDocument view(const FlightBoardFavorites& favorites) {
  JsonDocument doc;
  favorites.toJson(doc);
  return doc;
}
int add(FlightBoardFavorites& favorites, const char* name, double lat=48.1, double lon=2.5, int radius=30) {
  JsonDocument doc;
  doc["name"]=name;
  doc["lat"]=lat;
  doc["lon"]=lon;
  doc["radiusKm"]=radius;
  return favorites.add(doc.as<JsonObjectConst>());
}
int addJson(FlightBoardFavorites& favorites, const char* raw) {
  JsonDocument doc;
  CHECK(!deserializeJson(doc,raw));
  return favorites.add(doc.as<JsonObjectConst>());
}
String snapshot(const FlightBoardFavorites& favorites) {
  JsonDocument doc=view(favorites);
  String result;
  serializeJson(doc,result);
  return result;
}
void test_schedule_all_minutes() {
  for(int i=0;i<1440;++i) {
    CHECK(flightBoardNightMinute(i,1320,420)==(i>=1320||i<420));
    CHECK(flightBoardNightMinute(i,600,900)==(i>=600&&i<900));
    CHECK(!flightBoardNightMinute(i,600,600));
    CHECK(flightBoardNightMinute(i,0,1)==(i==0));
  }
  CHECK(!flightBoardNightMinute(-1,1320,420));
  CHECK(!flightBoardNightMinute(1440,1320,420));
  CHECK(!flightBoardNightMinute(0,-1,420));
  CHECK(!flightBoardNightMinute(0,1320,1440));
}
void test_display_rules() {
  auto a=flightBoardDisplayState(60,true,1320,420,15,false,true,1320);
  CHECK(a.nightActive && !a.sleeping && a.brightness==15);
  a=flightBoardDisplayState(60,true,1320,420,15,true,true,0);
  CHECK(a.nightActive && a.sleeping && a.brightness==0);
  a=flightBoardDisplayState(60,true,1320,420,15,true,true,420);
  CHECK(!a.nightActive && !a.sleeping && a.brightness==60);
  a=flightBoardDisplayState(60,true,1320,420,15,true,false,0);
  CHECK(!a.nightActive && !a.sleeping && a.brightness==60);
  a=flightBoardDisplayState(60,false,1320,420,15,true,true,0);
  CHECK(!a.nightActive && !a.sleeping && a.brightness==60);
  a=flightBoardDisplayState(20,true,1320,420,60,false,true,0);
  CHECK(a.nightActive && !a.sleeping && a.brightness==20);
  a=flightBoardDisplayState(60,true,1320,1320,15,true,true,1320);
  CHECK(!a.nightActive && !a.sleeping && a.brightness==60);
}
void checkLocal(const char* zone, int month,int day,int hour,int minute,int expectedMonth,int expectedDay,int expectedHour,int expectedMinute,int dst) {
  const char* rule=flightBoardTimezone(zone);
  CHECK(rule);
  setenv("TZ",rule,1);
  tzset();
  std::tm utc={};
  utc.tm_year=126;
  utc.tm_mon=month-1;
  utc.tm_mday=day;
  utc.tm_hour=hour;
  utc.tm_min=minute;
  time_t epoch=timegm(&utc);
  std::tm local={};
  CHECK(localtime_r(&epoch,&local));
  CHECK(local.tm_mon==expectedMonth-1);
  CHECK(local.tm_mday==expectedDay);
  CHECK(local.tm_hour==expectedHour);
  CHECK(local.tm_min==expectedMinute);
  CHECK((local.tm_isdst>0)==(dst>0));
}
void test_timezones_and_dst() {
  CHECK(!flightBoardTimezone(nullptr));
  CHECK(!flightBoardTimezone("unsupported"));
  checkLocal("paris",1,15,12,0,1,15,13,0,0);
  checkLocal("paris",7,15,12,0,7,15,14,0,1);
  checkLocal("paris",3,29,0,59,3,29,1,59,0);
  checkLocal("paris",3,29,1,0,3,29,3,0,1);
  checkLocal("paris",10,25,0,59,10,25,2,59,1);
  checkLocal("paris",10,25,1,0,10,25,2,0,0);
  checkLocal("london",1,15,12,0,1,15,12,0,0);
  checkLocal("london",7,15,12,0,7,15,13,0,1);
  checkLocal("london",3,29,0,59,3,29,0,59,0);
  checkLocal("london",3,29,1,0,3,29,2,0,1);
  checkLocal("london",10,25,0,59,10,25,1,59,1);
  checkLocal("london",10,25,1,0,10,25,1,0,0);
  checkLocal("newyork",1,15,12,0,1,15,7,0,0);
  checkLocal("newyork",7,15,12,0,7,15,8,0,1);
  checkLocal("newyork",3,8,6,59,3,8,1,59,0);
  checkLocal("newyork",3,8,7,0,3,8,3,0,1);
  checkLocal("newyork",11,1,5,59,11,1,1,59,1);
  checkLocal("newyork",11,1,6,0,11,1,1,0,0);
  checkLocal("utc",7,15,23,59,7,15,23,59,0);
  checkLocal("dubai",1,15,22,0,1,16,2,0,0);
  checkLocal("dubai",7,15,22,0,7,16,2,0,0);
  checkLocal("paris",12,31,23,30,1,1,0,30,0);
}
void test_defaults_reload_and_named_update() {
  reset();Preferences storage;FlightBoardFavorites favorites(storage);favorites.begin();
  auto d=view(favorites);
  CHECK(d["max"]==8);CHECK(d["favorites"].size()==3);
  CHECK(favorites.get("niort"));CHECK(favorites.get("cdg"));CHECK(favorites.get("orly"));
  CHECK(Preferences::writes.size()==1);
  CHECK(add(favorites,"Maison")==200);
  auto added=view(favorites);
  CHECK(added["favorites"].size()==4);
  String id=added["favorites"][3]["id"].as<String>();
  CHECK(id.length()>0);CHECK(favorites.get(id));
  CHECK(add(favorites,"  Maison  ",49.1,3.5,20)==200);
  CHECK(view(favorites)["favorites"].size()==4);
  CHECK(favorites.get(id)->lat==49.1);
  CHECK(favorites.get(id)->radiusKm==20);
  FlightBoardFavorites reloaded(storage);reloaded.begin();
  CHECK(snapshot(favorites)==snapshot(reloaded));
  CHECK(reloaded.get(id)->name=="Maison");
}
void test_limit_and_delete() {
  reset();Preferences storage;FlightBoardFavorites favorites(storage);favorites.begin();
  for(int i=0;i<5;++i) CHECK(add(favorites,("Custom"+std::to_string(i)).c_str())==200);
  CHECK(view(favorites)["favorites"].size()==8);
  String prior=snapshot(favorites);
  size_t writes=Preferences::writes.size();
  CHECK(add(favorites,"Overflow")==409);
  CHECK(snapshot(favorites)==prior);CHECK(Preferences::writes.size()==writes);
  CHECK(add(favorites,"Custom0",46,2,10)==200);
  CHECK(view(favorites)["favorites"].size()==8);
  CHECK(favorites.remove("unknown")==404);
  while(view(favorites)["favorites"].size()) {
    String id=view(favorites)["favorites"][0]["id"].as<String>();
    CHECK(favorites.remove(id)==200);CHECK(!favorites.get(id));
  }
  CHECK(Preferences::values["favorites"]=="[]");
  FlightBoardFavorites empty(storage);empty.begin();
  CHECK(view(empty)["favorites"].size()==0);
  CHECK(add(empty,"Premier")==200);CHECK(view(empty)["favorites"].size()==1);
}
void test_invalid_inputs_no_mutation() {
  reset();Preferences storage;FlightBoardFavorites favorites(storage);favorites.begin();
  String prior=snapshot(favorites);size_t writes=Preferences::writes.size();
  CHECK(add(favorites,"")==400);CHECK(add(favorites," \t ")==400);
  CHECK(add(favorites,std::string(33,'x').c_str())==400);
  CHECK(add(favorites,"Outside",-91,0)==400);CHECK(add(favorites,"Outside",91,0)==400);
  CHECK(add(favorites,"Outside",0,-181)==400);CHECK(add(favorites,"Outside",0,181)==400);
  CHECK(add(favorites,"Outside",0,0,4)==400);CHECK(add(favorites,"Outside",0,0,51)==400);
  CHECK(add(favorites,"Outside",NAN,0)==400);CHECK(add(favorites,"Outside",0,INFINITY)==400);
  for(const char* raw:{"{}","[]","null","{\"name\":42,\"lat\":0,\"lon\":0,\"radiusKm\":30}","{\"name\":\"X\",\"lat\":\"0\",\"lon\":0,\"radiusKm\":30}","{\"name\":\"X\",\"lat\":0,\"lon\":0,\"radiusKm\":30.5}","{\"name\":\"a\\u0000b\",\"lat\":0,\"lon\":0,\"radiusKm\":30}"}) CHECK(addJson(favorites,raw)==400);
  CHECK(snapshot(favorites)==prior);CHECK(Preferences::writes.size()==writes);
  CHECK(add(favorites,std::string(32,'x').c_str(),-90,180,5)==200);
  CHECK(add(favorites,"Pole",90,-180,50)==200);
}
void test_flash_failure_atomicity() {
  reset();Preferences storage;FlightBoardFavorites favorites(storage);favorites.begin();
  String before=snapshot(favorites),stored=Preferences::values["favorites"];
  Preferences::failKey="favorites";
  CHECK(add(favorites,"Maison")==503);
  CHECK(snapshot(favorites)==before);CHECK(Preferences::values["favorites"]==stored);
  CHECK(add(favorites,"Niort",0,0,5)==503);
  CHECK(snapshot(favorites)==before);CHECK(Preferences::values["favorites"]==stored);
  CHECK(favorites.remove("cdg")==503);
  CHECK(snapshot(favorites)==before);CHECK(Preferences::values["favorites"]==stored);
  Preferences::failKey.clear();CHECK(favorites.remove("cdg")==200);
  CHECK(view(favorites)["favorites"].size()==2);
}
void test_corrupt_storage_recovery() {
  const std::vector<std::string> corrupt={"invalid","{}","null","[null]","[{\"id\":\"x\"}]",std::string(4097,'x'),
    "[{\"id\":\"x\",\"name\":\"A\",\"lat\":0,\"lon\":0,\"radiusKm\":30},{\"id\":\"x\",\"name\":\"B\",\"lat\":0,\"lon\":0,\"radiusKm\":30}]",
    "[{\"id\":\"\",\"name\":\"A\",\"lat\":0,\"lon\":0,\"radiusKm\":30}]",
    "[{\"id\":\"xxxxxxxxxxxxxxxxxxxxxxxxx\",\"name\":\"A\",\"lat\":0,\"lon\":0,\"radiusKm\":30}]"};
  for(const auto& raw:corrupt) {
    reset();Preferences::values["favorites"]=raw;Preferences storage;FlightBoardFavorites favorites(storage);favorites.begin();
    CHECK(view(favorites)["favorites"].size()==3);CHECK(favorites.get("niort"));CHECK(favorites.get("cdg"));
    JsonDocument persisted;CHECK(!deserializeJson(persisted,Preferences::values["favorites"]));CHECK(persisted.size()==3);
  }
  reset();Preferences::values["favorites"]="[]";Preferences storage;FlightBoardFavorites favorites(storage);favorites.begin();
  CHECK(view(favorites)["favorites"].size()==0);CHECK(Preferences::writes.empty());
}
int main() {
  const std::vector<std::pair<const char*,std::function<void()>>> tests={
    {"every minute: overnight, daytime, equal endpoints, one minute",test_schedule_all_minutes},
    {"display: brightness cap, sleep, unsynced fallback, endpoints",test_display_rules},
    {"five timezones and 2026 DST transitions/date rollover",test_timezones_and_dst},
    {"favorite defaults, reload, named replacement retains id",test_defaults_reload_and_named_update},
    {"favorite capacity, replacement at capacity, delete empty reload",test_limit_and_delete},
    {"favorite invalid types, coordinates, embedded null, no mutation",test_invalid_inputs_no_mutation},
    {"favorite failed flash keeps memory and prior stored value",test_flash_failure_atomicity},
    {"favorite corrupt storage and intentional empty list",test_corrupt_storage_recovery}
  };
  int failures=0;
  for(const auto& t:tests) try {t.second();std::cout<<"PASS "<<t.first<<'\n';} catch(const std::exception& e) {++failures;std::cerr<<"FAIL "<<t.first<<": "<<e.what()<<'\n';}
  std::cout<<tests.size()<<" scenarios, "<<assertions<<" assertions, "<<failures<<" failed\n";
  return failures?1:0;
}
