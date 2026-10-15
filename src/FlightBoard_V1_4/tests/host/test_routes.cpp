#include "MockPlatform.hpp"
#include "pgmspace.h"
#include <ArduinoJson.h>
#include <stdexcept>
#include <new>
#include "HTTPClient.h"
#include "freertos/FreeRTOS.h"
#define private public
#include "FlightBoardRoutes.h"
#undef private

static int assertions=0;
#define CHECK(expr) do {++assertions;if(!(expr))throw std::runtime_error(std::string(__func__)+":"+std::to_string(__LINE__)+": " #expr);}while(0)
using Status=FlightRouteStatus;
const char* validJson=R"({"callsign":"AFR1234","airport_codes":"LFPG-EDDB","_airports":[{"iata":"CDG","icao":"LFPG","name":"Charles de Gaulle International Airport","location":"Paris","lat":49.0097,"lon":2.5479},{"iata":"BER","icao":"EDDB","name":"Berlin Brandenburg Airport","location":"Berlin","lat":52.35139,"lon":13.49389}]})";


void reset(){
  testNow=10000;WiFi=WiFiMock();WiFi.connect("Mock");HTTPClient::responseStatus=200;
  HTTPClient::responseSize=-2;HTTPClient::headers.clear();
  MockHTTPStream::readDelayMs=0;MockHTTPStream::bytesRead=0;
  MockHTTPStream::holdOpenAtEnd=false;MockHTTPStream::gapsBeforeByte.clear();
  HTTPClient::responseJson=validJson;HTTPClient::beginSuccess=true;HTTPClient::gets=HTTPClient::ends=0;
  queueFailuresAfter=-1;taskCreateSuccess=true;stopWorkerAfterSend=false;queueDeletes=0;
  for(auto*q:fakeQueues)delete q;fakeQueues.clear();WiFiClientSecure::caCertificate=nullptr;
}
FlightBoardRoutes::Result fetch(const std::string& json=validJson,const char* callsign="AFR1234"){
  HTTPClient::responseJson=json;FlightBoardRoutes::Job job{};
  CHECK(FlightBoardRoutes::makeJob("abc123",callsign,job));
  FlightBoardRoutes::Result result;FlightBoardRoutes::fetch(job,result);return result;
}

void worker(){stopWorkerAfterSend=true;try{workerFunction(workerContext);}catch(const WorkerStopped&){}stopWorkerAfterSend=false;}
void finish(FlightBoardRoutes& routes){worker();routes.update();}


void test_schema_and_tls(){
  reset();auto r=fetch();CHECK(r.route.status==Status::Known);
  CHECK(std::string(r.route.origin.code)=="CDG");CHECK(std::string(r.route.destination.code)=="BER");
  CHECK(std::string(r.route.origin.city)=="Paris");CHECK(r.route.ttlMs==1800000);
  CHECK(std::abs(r.route.origin.lat-49.0097)<0.00001);CHECK(std::abs(r.route.destination.lon-13.49389)<0.00001);
  CHECK(HTTPClient::lastUrl=="https://vrs-standing-data.adsb.lol/routes/AF/AFR1234.json");
  CHECK(HTTPClient::ends==1);CHECK(WiFiClientSecure::caCertificate);
  CHECK(std::string(WiFiClientSecure::caCertificate)==FlightBoardRoutes::rootCertificate());
  CHECK(std::string(WiFiClientSecure::caCertificate).find("BEGIN CERTIFICATE")!=std::string::npos);
  CHECK(HTTPClient::headers["Accept-Encoding"]=="identity");
  JsonDocument d;deserializeJson(d,validJson);d["callsign"]=" afr1234 ";
  d["_airports"][0]["iata"]=nullptr;d["_airports"][0]["location"]=nullptr;
  std::string text;serializeJson(d,text);r=fetch(text);CHECK(r.route.status==Status::Known);
  CHECK(std::string(r.route.origin.code)=="LFPG");CHECK(std::string(r.route.origin.city).empty());
}


void test_validation(){
  reset();FlightBoardRoutes::Job job{};CHECK(FlightBoardRoutes::makeJob("abc123"," afr1234 ",job));
  CHECK(std::string(job.hex)=="ABC123");CHECK(std::string(job.callsign)=="AFR1234");
  for(const char* c:{"","ABC123","AB","TOOLONG123","AFR/1234","AFR?X","AFR\n123","AFR\t123","AFR 123","AFR%123"})CHECK(!FlightBoardRoutes::makeJob("abc123",c,job));
  CHECK(!FlightBoardRoutes::makeJob("abc123",String(std::string("AFR\0X",5)),job));
  CHECK(!FlightBoardRoutes::makeJob("abc1234","AFR1234",job));CHECK(!FlightBoardRoutes::makeJob("xxyyzz","AFR1234",job));
  for(const char* field:{"callsign","airport_codes","_airports"}){
    JsonDocument d;deserializeJson(d,validJson);d.remove(field);std::string text;serializeJson(d,text);
    CHECK(fetch(text).route.status==Status::Error);
  }
  for(const std::string& bad:{std::string("OTHER123"),std::string("AFR1234\0x",9)}){
    JsonDocument d;deserializeJson(d,validJson);d["callsign"]=bad;std::string text;serializeJson(d,text);
    CHECK(fetch(text).route.status==Status::Error);
  }
  for(const std::string& raw:std::vector<std::string>{"{}","invalid","null","{\"_airports\":{}}",std::string(9000,' ')})CHECK(fetch(raw).route.status==Status::Error);
  for(const char* field:{"name","location"}){
    JsonDocument d;deserializeJson(d,validJson);d["_airports"][0][field]=std::string(150,'x');
    std::string text;serializeJson(d,text);CHECK(fetch(text).route.status==Status::Error);
  }
  for(int count:{0,1,3}){
    JsonDocument d;deserializeJson(d,validJson);auto airports=d["_airports"].as<JsonArray>();
    if(count<2){while((int)airports.size()>count)airports.remove(0);}else airports.add<JsonObject>()["icao"]="EGLL";
    std::string text;serializeJson(d,text);CHECK(fetch(text).route.status==Status::Unknown);
  }
  JsonDocument d;deserializeJson(d,validJson);
  for(const std::string& codes:{std::string("EDDB-LFPG"),std::string("LFPG-EDDB\0x",11)}){
    d["airport_codes"]=codes;std::string text;serializeJson(d,text);CHECK(fetch(text).route.status==Status::Error);
  }
  d["airport_codes"]="LFPG-EDDB";d["_airports"][1]["icao"]="LFPG";
  std::string text;serializeJson(d,text);CHECK(fetch(text).route.status==Status::Error);
  d["_airports"][1]["icao"]="EDDB";d["_airports"][0]["name"]=std::string("Paris\0ignored",13);
  text.clear();serializeJson(d,text);CHECK(fetch(text).route.status==Status::Error);
  deserializeJson(d,validJson);d["_airports"][0]["icao"]="1234";
  text.clear();serializeJson(d,text);CHECK(fetch(text).route.status==Status::Error);
}

void test_errors(){reset();HTTPClient::responseStatus=404;auto r=fetch();CHECK(r.route.status==Status::Unknown);CHECK(r.route.ttlMs==600000);CHECK(!r.rateLimited);
  HTTPClient::responseStatus=429;r=fetch();CHECK(r.route.status==Status::Error);CHECK(r.rateLimited);
  HTTPClient::responseStatus=503;r=fetch();CHECK(r.route.status==Status::Error);CHECK(r.route.ttlMs==60000);
  HTTPClient::responseStatus=200;HTTPClient::beginSuccess=false;auto gets=HTTPClient::gets;r=fetch();CHECK(r.route.status==Status::Error);CHECK(HTTPClient::gets==gets);
  HTTPClient::beginSuccess=true;WiFi.disconnect();r=fetch();CHECK(r.route.status==Status::Error);CHECK(HTTPClient::gets==gets);
}
void test_queue_cache_and_json(){reset();FlightBoardRoutes routes;CHECK(!routes.request("abc123","AFR1234"));CHECK(routes.begin());CHECK(routes.begin());CHECK(fakeQueues.size()==2);CHECK(routes.request("abc123","AFR1234"));CHECK(routes.pending());CHECK(!routes.request("def456","RYR4SG"));CHECK(routes.get("abc123","AFR1234")->status==Status::Loading);CHECK(!routes.get("abc123","RYR4SG"));
  FlightBoardRoutes::Job job{};memcpy(&job,fakeQueues[0]->items[0].data(),sizeof job);CHECK(std::string(job.hex)=="ABC123");CHECK(std::string(job.callsign)=="AFR1234");
  routes.update();CHECK(routes.pending());finish(routes);CHECK(!routes.pending());CHECK(routes.get("abc123","AFR1234")->status==Status::Known);CHECK(!routes.request("abc123","AFR1234"));
  JsonDocument d;routes.toJson(d.to<JsonObject>(),"abc123","AFR1234",50.2,8,70,700);CHECK(d["status"]=="known");CHECK(d["origin"]["code"]=="CDG");CHECK(d["indicative"]==true);CHECK(d["source"]=="adsb.lol-vrs");
  d.clear();routes.toJson(d.to<JsonObject>(),"abc123","abc123");CHECK(d["status"]=="unknown");CHECK(d["origin"].isNull());
  d.clear();routes.toJson(d.to<JsonObject>(),"def456","RYR4SG");CHECK(d["status"]=="not_requested");CHECK(d["destination"].isNull());
  testNow+=1799999;CHECK(routes.get("abc123","AFR1234"));++testNow;CHECK(!routes.get("abc123","AFR1234"));CHECK(routes.request("abc123","AFR1234"));finish(routes);
  CHECK(routes.get("abc123","AFR1234")->status==Status::Known);
}
void test_throttle_backoff_and_wrap(){reset();FlightBoardRoutes routes;CHECK(routes.begin());CHECK(routes.request("abc123","AFR1234"));finish(routes);testNow+=4999;CHECK(!routes.request("def456","RYR4SG"));++testNow;CHECK(routes.request("def456","RYR4SG"));HTTPClient::responseStatus=429;finish(routes);testNow+=299999;CHECK(!routes.request("abcdef","UAE1"));++testNow;CHECK(routes.request("abcdef","UAE1"));HTTPClient::responseStatus=404;finish(routes);CHECK(routes.get("abcdef","UAE1")->status==Status::Unknown);
  testNow+=599999;CHECK(routes.get("abcdef","UAE1"));++testNow;CHECK(!routes.get("abcdef","UAE1"));
  testNow=0xfffffff0UL;CHECK(routes.request("001122","AFR1234"));HTTPClient::responseStatus=200;finish(routes);testNow=0x00000010UL;CHECK(routes.get("001122","AFR1234"));CHECK(!routes.request("003344","AFR1234"));testNow=0x00002000UL;CHECK(routes.request("003344","AFR1234"));finish(routes);
}
void test_cache_eviction_and_failures(){reset();FlightBoardRoutes routes;CHECK(routes.begin());for(int i=0;i<40;++i){char hex[7];snprintf(hex,sizeof hex,"%06x",i);testNow+=5000;CHECK(routes.request(hex,"AFR1234"));finish(routes);CHECK(routes.get(hex,"AFR1234"));}CHECK(!routes.get("000000","AFR1234"));CHECK(routes.get("000027","AFR1234"));
  testNow+=5000;CHECK(routes.request("abcdef","AFR1234"));FlightBoardRoutes::Job job;CHECK(xQueueReceive(routes.requests_,&job,0)==pdTRUE);FlightBoardRoutes::Result* absent=nullptr;CHECK(xQueueSend(routes.results_,&absent,0)==pdTRUE);routes.update();CHECK(!routes.pending());CHECK(routes.get("abcdef","AFR1234")->status==Status::Error);
  for(int failure:{0,1}){reset();queueFailuresAfter=failure;FlightBoardRoutes r;CHECK(!r.begin());CHECK(fakeQueues.empty());CHECK(!r.request("abc123","AFR1234"));}
  reset();taskCreateSuccess=false;FlightBoardRoutes r;CHECK(!r.begin());CHECK(fakeQueues.empty());CHECK(queueDeletes==2);
}

void test_stream_limits_and_error_expiry(){
  reset();MockHTTPStream stream;stream.str("abcdef");FlightBoardRoutes::LimitedReader<MockHTTPStream> reader{stream,3};
  CHECK(reader.read()=='a');char bytes[8]={};CHECK(reader.readBytes(bytes,8)==2);CHECK(std::string(bytes)=="bc");CHECK(reader.read()==-1);
  reset();MockHTTPStream::gapsBeforeByte={{0,300},{20,600},{100,700}};
  CHECK(fetch().route.status==Status::Known);CHECK(testNow==11600);
  reset();MockHTTPStream::gapsBeforeByte={{20,3000}};
  CHECK(fetch().route.status==Status::Error);CHECK(testNow==12000);
  reset();HTTPClient::responseSize=-1;MockHTTPStream::holdOpenAtEnd=true;
  CHECK(fetch("{\"callsign\":").route.status==Status::Error);CHECK(testNow==12000);
  reset();HTTPClient::responseSize=-1;
  CHECK(fetch("{\"callsign\":").route.status==Status::Error);CHECK(testNow==10000);
  reset();MockHTTPStream::readDelayMs=1000;
  CHECK(fetch().route.status==Status::Error);CHECK(testNow==25000);
  reset();HTTPClient::responseSize=-1;
  CHECK(fetch(std::string(8193,' ')).route.status==Status::Error);CHECK(MockHTTPStream::bytesRead==8192);
  reset();FlightBoardRoutes routes;CHECK(routes.begin());CHECK(routes.request("abc123","AFR1234"));
  HTTPClient::responseStatus=503;finish(routes);CHECK(routes.get("abc123","AFR1234")->status==Status::Error);
  testNow+=59999;CHECK(!routes.request("abc123","AFR1234"));++testNow;CHECK(routes.request("abc123","AFR1234"));finish(routes);
}

FlightRoute candidate(double aLat,double aLon,double bLat,double bLon){
  FlightRoute r;r.status=Status::Known;r.origin.lat=aLat;r.origin.lon=aLon;r.destination.lat=bLat;r.destination.lon=bLon;return r;
}
std::string reason(const FlightRoute& route,double lat,double lon,double track=NAN,double speed=NAN){
  const char* value=FlightBoardRoutes::plausibility(route,lat,lon,track,speed);return value?value:"";
}
void test_route_geometry(){
  auto r=candidate(0,0,0,10);
  CHECK(reason(r,0,5,90,700).empty());CHECK(reason(r,1.5,5).empty());
  CHECK(reason(r,10,5)=="position_mismatch");CHECK(reason(r,0,-10)=="position_mismatch");
  CHECK(reason(r,0,5,270,700)=="direction_mismatch");CHECK(reason(r,0,5,270,250).empty());
  CHECK(reason(r,0,5,270,NAN).empty());CHECK(reason(r,0,5,NAN,700).empty());
  CHECK(reason(r,0,1,270,700).empty());CHECK(reason(r,0,9,270,700).empty());CHECK(reason(r,0,5,180,700).empty());
  for(double bad:std::vector<double>{NAN,INFINITY,91.0})CHECK(reason(r,bad,5)=="position_unavailable");
  CHECK(reason(r,0,181)=="position_unavailable");r.origin.lat=NAN;CHECK(reason(r,0,5)=="airport_coordinates_unavailable");
  r=candidate(0,0,0,0);CHECK(reason(r,0,0)=="route_geometry_ambiguous");
  r=candidate(0,0,0,180);CHECK(reason(r,0,90)=="route_geometry_ambiguous");
  r=candidate(10,10,10,10.5);CHECK(reason(r,10.1,10.25,270,700).empty());CHECK(reason(r,20,20)=="position_mismatch");
  r=candidate(10,170,10,-170);CHECK(reason(r,10,179,90,800).empty());CHECK(reason(r,10,-179,90,800).empty());
  r=candidate(51.47,-.45,37.62,-122.38);
  CHECK(reason(r,64,-60,270,850).empty());CHECK(reason(r,44,-60)=="position_mismatch");
}
void test_geometry_rechecks_cache_and_missing_coordinates(){
  reset();FlightBoardRoutes routes;CHECK(routes.begin());CHECK(routes.request("abc123","AFR1234"));finish(routes);
  JsonDocument d;auto object=d.to<JsonObject>();routes.toJson(object,"abc123","AFR1234",50.2,8,70,700);CHECK(d["status"]=="known");
  routes.toJson(object,"abc123","AFR1234",46,-1,70,700);CHECK(d["status"]=="unconfirmed");CHECK(d["reason"]=="position_mismatch");
  CHECK(d["origin"].isNull());CHECK(d["destination"].isNull());CHECK(routes.get("abc123","AFR1234")->status==Status::Known);
  routes.toJson(object,"abc123","AFR1234",50.2,8,70,700);CHECK(d["status"]=="known");CHECK(d["reason"].isNull());CHECK(d["origin"]["code"]=="CDG");
  routes.toJson(object,"abc123","AFR1234");CHECK(d["status"]=="unconfirmed");CHECK(d["reason"]=="position_unavailable");CHECK(d["origin"].isNull());
  for(int i=0;i<3;++i){
    JsonDocument body;deserializeJson(body,validJson);
    if(i==0)body["_airports"][0].remove("lat");if(i==1)body["_airports"][0]["lat"]="49.0097";if(i==2)body["_airports"][0]["lon"]=181;
    std::string text;serializeJson(body,text);auto result=fetch(text);CHECK(result.route.status==Status::Known);
    routes.cache_[0]=result.route;routes.cache_[0].updatedMs=(uint32_t)testNow;
    routes.toJson(object,"abc123","AFR1234",50.2,8,70,700);CHECK(d["status"]=="unconfirmed");
    CHECK(d["reason"]=="airport_coordinates_unavailable");CHECK(d["origin"].isNull());CHECK(d["destination"].isNull());
  }
}
void test_reused_callsign_regression(){
  reset();
  const char* stale=R"({"callsign":"RYR3UF","airport_codes":"LQBK-EDDB","_airports":[{"icao":"LQBK","iata":"BNX","name":"Banja Luka Airport","location":"Banja Luka","lat":44.9414,"lon":17.2975},{"icao":"EDDB","iata":"BER","name":"Berlin Brandenburg Airport","location":"Berlin","lat":52.35139,"lon":13.49389}]})";
  const char* current=R"({"callsign":"RYR3UF","airport_codes":"LFRS-LEVC","_airports":[{"icao":"LFRS","iata":"NTE","name":"Nantes Atlantique Airport","location":"Nantes","lat":47.153198,"lon":-1.61073},{"icao":"LEVC","iata":"VLC","name":"Valencia Airport","location":"Valencia","lat":39.4893,"lon":-0.481625}]})";
  FlightBoardRoutes routes;auto old=fetch(stale,"RYR3UF");CHECK(old.route.status==Status::Known);
  routes.cache_[0]=old.route;routes.cache_[0].updatedMs=(uint32_t)testNow;JsonDocument d;auto object=d.to<JsonObject>();
  routes.toJson(object,"abc123","RYR3UF",46,-1,180,750);CHECK(d["status"]=="unconfirmed");CHECK(d["reason"]=="position_mismatch");
  CHECK(!routes.confirmed("abc123","RYR3UF",46,-1,180,750));
  CHECK(d["origin"].isNull());CHECK(d["destination"].isNull());
  auto newer=fetch(current,"RYR3UF");CHECK(newer.route.status==Status::Known);routes.cache_[0]=newer.route;routes.cache_[0].updatedMs=(uint32_t)testNow;
  routes.toJson(object,"abc123","RYR3UF",46,-1,180,750);CHECK(d["status"]=="known");CHECK(d["origin"]["code"]=="NTE");CHECK(d["destination"]["code"]=="VLC");
  const auto* panelRoute=routes.confirmed("abc123","RYR3UF",46,-1,180,750); CHECK(panelRoute); CHECK(std::string(panelRoute->origin.code)=="NTE");
  CHECK(!routes.confirmed("abc123","RYR3UF",NAN,-1)); CHECK(!routes.confirmed("abc123","OTHER123",46,-1));
  testNow += 1800000; CHECK(!routes.confirmed("abc123","RYR3UF",46,-1,180,750));
  CHECK(d["indicative"]==true);CHECK(d["source"]=="adsb.lol-vrs");CHECK(HTTPClient::lastUrl=="https://vrs-standing-data.adsb.lol/routes/RY/RYR3UF.json");
}

int main(){std::vector<std::pair<const char*,std::function<void()>>> tests={{"route schema, ICAO fallback and verified TLS",test_schema_and_tls},{"callsign matching, bounded fields, malformed data and ambiguous stops",test_validation},{"HTTP errors, unknown callsign and disconnected WiFi",test_errors},{"queued ownership, cache lifetime and API serialization",test_queue_cache_and_json},{"global throttle,429backoff, negative TTL and millis rollover",test_throttle_backoff_and_wrap},{"bounded cache eviction and memory/task failures",test_cache_eviction_and_failures},{"stream byte/time limits and transient error TTL",test_stream_limits_and_error_expiry},{"great-circle corridor, direction, nearby airports and dateline",test_route_geometry},{"cached routes rechecked with current positions and missing coordinates",test_geometry_rechecks_cache_and_missing_coordinates},{"RYR3UF stale BNX-BER suppressed; plausible NTE-VLC retained",test_reused_callsign_regression}};int failed=0;for(auto&t:tests)try{t.second();std::cout<<"PASS "<<t.first<<'\n';}catch(const std::exception&e){++failed;std::cerr<<"FAIL "<<t.first<<": "<<e.what()<<'\n';}std::cout<<tests.size()<<" scenarios, "<<assertions<<" assertions, "<<failed<<" failed\n";return failed?1:0;}
