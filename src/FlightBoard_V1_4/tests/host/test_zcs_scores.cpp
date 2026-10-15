#include "MockPlatform.hpp"
#include "pgmspace.h"
#include <ArduinoJson.h>
#include "HTTPClient.h"
#include "freertos/FreeRTOS.h"
#include <ctime>
#include <stdexcept>
static time_t scoreEpoch=1792000000;
time_t scoreTime(time_t* out){if(out)*out=scoreEpoch;return scoreEpoch;}
#define time scoreTime
#define private public
#include "ZcsScores.h"
#undef private
#undef time

static int assertions=0;
#define CHECK(expr) do{++assertions;if(!(expr))throw std::runtime_error(std::string(__func__)+":"+std::to_string(__LINE__)+": " #expr);}while(0)
void reset(){
  testNow=0;scoreEpoch=1792000000;WiFi=WiFiMock();WiFi.connect("Mock");
  HTTPClient::responseStatus=200;HTTPClient::responseSize=-2;HTTPClient::responseJson="{}";
  HTTPClient::beginSuccess=true;HTTPClient::gets=HTTPClient::ends=0;
  HTTPClient::headers.clear();HTTPClient::http10=false;HTTPClient::connectTimeout=HTTPClient::readTimeout=0;
  WiFiClientSecure::caCertificate=nullptr;WiFiClientSecure::insecure=false;WiFiClientSecure::handshakeTimeout=0;
  MockHTTPStream::readDelayMs=0;MockHTTPStream::bytesRead=0;MockHTTPStream::holdOpenAtEnd=false;MockHTTPStream::gapsBeforeByte.clear();
  queueFailuresAfter=-1;taskCreateSuccess=true;stopWorkerAfterSend=false;queueDeletes=taskDeletes=0;
  for(auto*q:fakeQueues)delete q;fakeQueues.clear();
}
JsonDocument body(const char* state="in",bool reversed=false){
  JsonDocument d;auto e=d["events"][0];e["id"]="123456";
  auto c=e["competitions"][0];c["status"]["type"]["state"]=state;c["status"]["displayClock"]="90'+3";
  auto h=c["competitors"][reversed?1:0],a=c["competitors"][reversed?0:1];
  h["homeAway"]="home";h["team"]["abbreviation"]="psg";h["score"]="2";
  a["homeAway"]="away";a["team"]["abbreviation"]="OM";a["score"]="0";
  return d;
}
bool decode(const JsonDocument& d,ZcsScoreData& out,const char* id="123456"){return ZcsScores::decode(d.as<JsonObjectConst>(),id,out);}
std::string text(const JsonDocument& doc){std::string out;serializeJson(doc,out);return out;}
ZcsScores::Result fetch(const std::string& raw){HTTPClient::responseJson=raw;ZcsScores::Result r;ZcsScores::fetch({"fra.1","123456","20261015",0},r);return r;}
bool request(ZcsScores& s){return s.request("fra.1","123456","20261015");}
void finish(ZcsScores& scores){const int deleted=taskDeletes;workerFunction(workerContext);CHECK(taskDeletes==deleted+1);scores.update();}

void test_selection(){
  for(const char* league:{"fra.1","eng.1","uefa.champions"})CHECK(ZcsScores::validSelection(league,"123456","20261015"));
  for(const char* league:{"","laliga","FRA.1","fra.1/../x","fra.1?evil"})CHECK(!ZcsScores::validSelection(league,"123456","20261015"));
  for(const char* id:{"12345","123456789012"})CHECK(ZcsScores::validSelection("fra.1",id,"20261015"));
  for(const char* id:{"","1234","1234567890123","1234a","12345 ","/123456","12345\n"})CHECK(!ZcsScores::validSelection("fra.1",id,"20261015"));
  CHECK(!ZcsScores::validSelection("fra.1",String(std::string("12345\0x",7)),"20261015"));
  for(const char* date:{"20240229","20260131","20260430","20261231","20991231"})CHECK(ZcsScores::validSelection("fra.1","123456",date));
  for(const char* date:{"20230228","21000101","20250229","20260230","20260431","20260001","20261301","20261000","2026-10-15","20261a15","2026101","202610150"})CHECK(!ZcsScores::validSelection("fra.1","123456",date));
}
void test_decode(){
  reset();ZcsScoreData out;
  for(bool reversed:{false,true})for(const auto& state:std::vector<std::pair<const char*,const char*>>{{"pre","A VENIR"},{"in","EN COURS 90+3"},{"post","TERMINE"}}){
    auto d=body(state.first,reversed);CHECK(decode(d,out));CHECK(std::string(out.home)=="PSG");CHECK(std::string(out.away)=="OM");CHECK(out.homeScore==2&&out.awayScore==0);CHECK(std::string(out.status)==state.second);
  }
  auto d=body();CHECK(!decode(d,out,"999999"));d["events"][0]["id"]=123456;CHECK(!decode(d,out));
  d=body();d["events"][0]["id"]=std::string("123456\0other",12);CHECK(!decode(d,out));
  d=body();d["events"][0]["competitions"][0]["status"]["type"]["state"]=std::string("in\0post",7);CHECK(!decode(d,out));
  d=body();d["events"][0]["competitions"][0]["competitors"][0]["homeAway"]=std::string("home\0away",9);CHECK(!decode(d,out));
  d=body();d["events"][1]=d["events"][0];d["events"][0]["id"]="999999";CHECK(decode(d,out));
  for(const char* value:{"0","000","999"}){d=body();d["events"][0]["competitions"][0]["competitors"][0]["score"]=value;CHECK(decode(d,out));CHECK(out.homeScore==std::atoi(value));}
  for(const char* value:{"","1000","-1","1.0","1 "," 1","NaN","+2","2x"}){d=body();d["events"][0]["competitions"][0]["competitors"][0]["score"]=value;CHECK(!decode(d,out));}
  for(const char* value:{"A","M-C","abc 123","ABCDEFGHIJKL"}){d=body();d["events"][0]["competitions"][0]["competitors"][0]["team"]["abbreviation"]=value;CHECK(decode(d,out));}
  for(const char* value:{"","ABCDEFGHIJKLM","P.S.G","équipe","PSG\n","A/B"}){d=body();d["events"][0]["competitions"][0]["competitors"][0]["team"]["abbreviation"]=value;CHECK(!decode(d,out));}
  for(const char* key:{"score","team"})for(int kind=0;kind<4;++kind){d=body();auto team=d["events"][0]["competitions"][0]["competitors"][0];JsonVariant v=std::string(key)=="team"?team["team"]["abbreviation"].as<JsonVariant>():team["score"].as<JsonVariant>();if(kind==0)v.set(123);else if(kind==1)v.set(nullptr);else if(kind==2)v.set(true);else v.set(std::string("2\0x",3));CHECK(!decode(d,out));}
  for(const char* side:{"home","neutral","","AWAY"}){d=body();d["events"][0]["competitions"][0]["competitors"][1]["homeAway"]=side;CHECK(!decode(d,out));}
  for(int count:{0,1,3}){d=body();auto teams=d["events"][0]["competitions"][0]["competitors"].as<JsonArray>();if(count<2)while(teams.size()>size_t(count))teams.remove(teams.size()-1);else teams.add(teams[0]);CHECK(!decode(d,out));}
  for(const char* state:{"","unknown","IN","cancelled"}){d=body(state);CHECK(!decode(d,out));}
  d=body();d["events"][0]["competitions"][0]["status"]["displayClock"]="abc12:34+56789";CHECK(decode(d,out));CHECK(std::string(out.status)=="EN COURS 12:34+56");
  out.homeScore=777;strcpy(out.home,"UNCHANGED");d=body();d["events"][0]["competitions"][0]["competitors"][1]["score"]="bad";CHECK(!decode(d,out));CHECK(out.homeScore==777&&std::string(out.home)=="UNCHANGED");
}
void test_tls_and_bounds(){
  reset();auto r=fetch(text(body()));CHECK(r.ok);CHECK(r.data.homeScore==2&&r.data.awayScore==0);CHECK(HTTPClient::ends==1);
  CHECK(HTTPClient::lastUrl=="https://site.api.espn.com/apis/site/v2/sports/soccer/fra.1/scoreboard?dates=20261015");
  CHECK(HTTPClient::headers["Accept"]=="application/json");CHECK(HTTPClient::headers["Accept-Encoding"]=="identity");
  CHECK(WiFiClientSecure::caCertificate==ZCS_ISRG_ROOT_CA);CHECK(!WiFiClientSecure::insecure);CHECK(WiFiClientSecure::handshakeTimeout==6);CHECK(HTTPClient::http10);CHECK(HTTPClient::connectTimeout==6000);CHECK(HTTPClient::readTimeout==8000);
  for(int status:{-1,301,401,429,503}){reset();HTTPClient::responseStatus=status;r=fetch(text(body()));CHECK(!r.ok);CHECK(HTTPClient::ends==1);CHECK(MockHTTPStream::bytesRead==0);}
  reset();HTTPClient::beginSuccess=false;r=fetch(text(body()));CHECK(!r.ok);CHECK(HTTPClient::gets==0);
  for(int size:{0,524289}){reset();HTTPClient::responseSize=size;r=fetch(text(body()));CHECK(!r.ok);CHECK(MockHTTPStream::bytesRead==0);CHECK(HTTPClient::ends==1);}
  for(const char* raw:{"{}","null","{!}","{\"events\":[","{\"events\":{}}"}){reset();r=fetch(raw);CHECK(!r.ok);CHECK(HTTPClient::ends==1);}
  reset();auto raw=text(body());raw.resize(raw.size()-12);r=fetch(raw);CHECK(!r.ok);
  reset();HTTPClient::responseSize=20;r=fetch(text(body()));CHECK(!r.ok);CHECK(MockHTTPStream::bytesRead==20);
  reset();HTTPClient::responseSize=-1;r=fetch(std::string(524289,' '));CHECK(!r.ok);CHECK(MockHTTPStream::bytesRead==524288);
  reset();HTTPClient::responseSize=-1;r=fetch(text(body()));CHECK(r.ok);
  reset();MockHTTPStream::readDelayMs=1000;r=fetch(text(body()));CHECK(!r.ok);CHECK(testNow==1000);
  reset();MockHTTPStream::readDelayMs=500;r=fetch(text(body()));CHECK(!r.ok);CHECK(testNow==10000);CHECK(MockHTTPStream::bytesRead==20);
  reset();testNow=0xfffffff0UL;MockHTTPStream::readDelayMs=1;r=fetch(text(body()));CHECK(r.ok);CHECK(testNow<1000);
  reset();auto many=body();many["events"][1]=many["events"][0];many["events"][0]["id"]="999999";many["events"][0]["competitions"][0]["competitors"][0]["score"]="999";r=fetch(text(many));CHECK(r.ok&&r.data.homeScore==2);
}
void test_cache_and_workers(){
  reset();ZcsScores s;JsonDocument j;s.toJson(j.to<JsonObject>());CHECK(j["available"]==false);CHECK(j["ageSeconds"]==-1);CHECK(j["source"]=="ESPN");CHECK(j["home"].isNull());
  CHECK(!s.request("invalid","123456","20261015"));WiFi.disconnect();CHECK(!request(s));WiFi.connect("Mock");scoreEpoch=0;CHECK(!request(s));scoreEpoch=1792000000;
  CHECK(request(s));CHECK(s.pending());CHECK(!request(s));HTTPClient::responseJson=text(body());finish(s);CHECK(!s.pending());CHECK(s.available());CHECK(!s.stale(179999));CHECK(s.stale(180000));
  s.toJson(j.to<JsonObject>());CHECK(j["home"]=="PSG");CHECK(j["homeScore"]==2);CHECK(j["error"]==false);
  testNow=59999;CHECK(!request(s));testNow=60000;CHECK(request(s));HTTPClient::responseStatus=503;finish(s);CHECK(s.available());CHECK(s.stale(testNow));CHECK(s.data().homeScore==2);
  testNow=120000;CHECK(request(s));HTTPClient::responseStatus=200;auto updated=body("post");updated["events"][0]["competitions"][0]["competitors"][0]["score"]="3";HTTPClient::responseJson=text(updated);finish(s);CHECK(s.data().homeScore==3);CHECK(!s.stale(testNow));CHECK(std::string(s.data().status)=="TERMINE");
  testNow=180000;CHECK(request(s));s.reset();CHECK(!s.available());CHECK(s.pending());CHECK(!request(s));HTTPClient::responseJson=text(body());finish(s);CHECK(!s.available());CHECK(!s.pending());CHECK(request(s));finish(s);CHECK(s.available());
  s.reset();CHECK(!s.available());CHECK(!s.stale(testNow));j.clear();s.toJson(j.to<JsonObject>());CHECK(j["home"].isNull());CHECK(j["ageSeconds"]==-1);
  reset();ZcsScores wrap;testNow=0xfffffff0UL;CHECK(request(wrap));HTTPClient::responseJson=text(body());finish(wrap);testNow=uint32_t(testNow+59999);CHECK(!request(wrap));++testNow;CHECK(request(wrap));finish(wrap);CHECK(!wrap.stale(testNow));CHECK(wrap.stale(uint32_t(testNow+180000)));
  reset();queueFailuresAfter=0;ZcsScores noQueue;CHECK(!request(noQueue));CHECK(!noQueue.pending());CHECK(fakeQueues.empty());CHECK(!request(noQueue));queueFailuresAfter=-1;testNow=60000;CHECK(request(noQueue));HTTPClient::responseJson=text(body());finish(noQueue);
  reset();taskCreateSuccess=false;ZcsScores noTask;CHECK(!request(noTask));CHECK(!noTask.pending());CHECK(fakeQueues.size()==1);CHECK(fakeQueues[0]->items.empty());taskCreateSuccess=true;CHECK(!request(noTask));testNow=60000;CHECK(request(noTask));HTTPClient::responseJson=text(body());finish(noTask);
  testNow+=60000;CHECK(request(noTask));ZcsScores::Result* failed=nullptr;CHECK(xQueueSend(noTask.queue_,&failed,0)==pdTRUE);noTask.update();CHECK(!noTask.pending());CHECK(noTask.available());CHECK(noTask.stale(testNow));
}
int main(){test_selection();test_decode();test_tls_and_bounds();test_cache_and_workers();reset();std::cout<<"ZCS scores: "<<assertions<<" assertions passed\n";}
