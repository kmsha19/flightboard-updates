// Compile the actual widget API bodies; storage/network stay host-only.
#include "MockPlatform.hpp"
#include "pgmspace.h"
#include <ArduinoJson.h>
#include "HTTPClient.h"
#include "freertos/FreeRTOS.h"
#include <ctime>
#include <memory>
#include <stdexcept>
#define private public
#include "ZcsScores.h"
#undef private
#include "ZcsWidgets.h"
static int assertions=0;
#define CHECK(expr) do{++assertions;if(!(expr))throw std::runtime_error(std::string(__func__)+":"+std::to_string(__LINE__)+": " #expr);}while(0)
static Preferences storage;
static JsonDocument widgetConfig,activePack;
static ZcsTimer widgetTimer;
static ZcsScores scores;
static struct {String app="clock",contentId="";} settings;
static uint16_t activePalette[16]={};
static std::unique_ptr<uint8_t[]> activePixels;
static size_t activePixelBytes=0;
static bool webPackReady=false,logosPackReady=false;
static String contentToken,requestBody;
static int responseStatus=0,assetReads=0;
static constexpr char ZCS_WEB_SHA256[]="mock-web",AIRLINE_IMAGES_SHA256[]="mock-logo";
static constexpr uint32_t ZCS_WEB_SIZE=100,AIRLINE_IMAGES_SIZE=100;
bool readJsonBody(JsonDocument& body){if(deserializeJson(body,requestBody)){responseStatus=400;return false;}return true;}
void sendError(int status,const String&){responseStatus=status;}
void sendRadarState(){responseStatus=200;}
bool installedAssetMatches(const char*,const char*,uint32_t){++assetReads;return true;}
bool activatePack(const String& id){activePack.clear();if(id=="tea"){activePack["type"]="timer";activePack["durationSeconds"]=180;return true;}return id.isEmpty();}
#include "widgets_under_test.inc"

void reset(){testNow=0;settings.app="clock";settings.contentId="";widgetConfig.clear();activePack.clear();widgetTimer=ZcsTimer(300);scores=ZcsScores();Preferences::available=true;Preferences::values.clear();Preferences::writes.clear();Preferences::failKey.clear();responseStatus=0;assetReads=0;webPackReady=logosPackReady=false;}
void request(const char* body,bool timer=false){requestBody=body;responseStatus=0;if(timer)changeTimer();else changeWidget();}
JsonDocument pack(const char* type="pixel"){
  JsonDocument d;d["id"]="sample";d["title"]="Mon contenu";d["type"]=type;d["version"]=1;
  d["text"]="C'EST BIEN!";d["color"]=65535;d["durationSeconds"]=300;
  d["width"]=3;d["height"]=1;d["frameCount"]=2;d["frameMs"]=100;d["palette"][0]=0;d["palette"][1]=65535;d["pixels"]="011100";return d;
}
std::string json(const JsonDocument& d){std::string out;serializeJson(d,out);return out;}
void test_pack_validation(){
  for(const char* type:{"message","timer","pixel"}){auto d=pack(type);CHECK(validPack(d,"sample"));CHECK(!validPack(d,"different"));}
  for(const char* key:{"id","title","type","version"}){auto d=pack();d.remove(key);CHECK(!validPack(d,"sample"));d=pack();d[key]=nullptr;CHECK(!validPack(d,"sample"));}
  for(const char* key:{"id","title","type"}){auto d=pack();d[key]=std::string("sample\0x",8);CHECK(!validPack(d,"sample"));}
  for(int version:{0,-1,65536}){auto d=pack();d["version"]=version;CHECK(!validPack(d,"sample"));}
  for(const char* key:{"width","height","frameCount","frameMs"})for(const char* bad:{"missing","type","negative","fraction"}){auto d=pack();if(!strcmp(bad,"missing"))d.remove(key);else if(!strcmp(bad,"type"))d[key]="1";else if(!strcmp(bad,"negative"))d[key]=-1;else d[key]=1.5;CHECK(!validPack(d,"sample"));}
  for(const auto& pair:std::vector<std::pair<const char*,int>>{{"width",0},{"width",65},{"height",0},{"height",65},{"frameCount",0},{"frameCount",33},{"frameMs",79},{"frameMs",2001}}){auto d=pack();d[pair.first]=pair.second;CHECK(!validPack(d,"sample"));}
  for(const char* pixels:{"","0","0111000","011102","01110x"}){auto d=pack();d["pixels"]=pixels;CHECK(!validPack(d,"sample"));}
  auto d=pack();d["pixels"]=std::string("011\0" "00",6);CHECK(!validPack(d,"sample"));
  for(int color:{-1,65536}){d=pack();d["palette"][0]=color;CHECK(!validPack(d,"sample"));}
  d=pack();d["palette"].to<JsonArray>();CHECK(!validPack(d,"sample"));d=pack();for(int i=0;i<17;++i)d["palette"][i]=0;CHECK(!validPack(d,"sample"));
  d=pack();d["width"]=64;d["height"]=64;d["frameCount"]=8;d["pixels"]=std::string(32768,'0');CHECK(validPack(d,"sample"));d["frameCount"]=17;d["pixels"]=std::string(69632,'0');CHECK(!validPack(d,"sample"));
  for(const char* text:{"","bad\nmessage","école"}){d=pack("message");d["text"]=text;CHECK(!validPack(d,"sample"));}
  d=pack("message");d["text"]=std::string(120,'A');CHECK(validPack(d,"sample"));d["text"]=std::string(121,'A');CHECK(!validPack(d,"sample"));
  for(int seconds:{0,6000,65537}){d=pack("timer");d["durationSeconds"]=seconds;CHECK(!validPack(d,"sample"));}
}
void test_configuration_atomicity(){
  reset();loadWidgets();CHECK(widgetConfig["text"]=="BONJOUR");CHECK(widgetTimer.durationSeconds()==300);CHECK(!widgetTimer.running(0));CHECK(webPackReady&&logosPackReady);CHECK(assetReads==2);CHECK(contentToken.length()==32);
  request("{\"app\":\"message\",\"text\":\"SALUT!\",\"color\":65535}");CHECK(responseStatus==200);CHECK(widgetConfig["text"]=="SALUT!");CHECK(settings.app=="clock");CHECK(Preferences::writes.size()==1);
  const String saved=Preferences::values["widget2"];const auto before=json(widgetConfig);
  for(const char* body:{"{\"app\":\"message\",\"text\":\"\",\"color\":0}","{\"app\":\"message\",\"text\":\"A\",\"color\":65536}","{\"app\":\"message\",\"text\":\"A\",\"color\":-1}","{\"app\":\"message\",\"text\":\"A\\u0000B\",\"color\":0}","{\"app\":\"message\\u0000timer\",\"text\":\"A\",\"color\":0}","{\"app\":\"timer\",\"durationSeconds\":65537}","{\"app\":\"timer\",\"durationSeconds\":0}","{\"app\":\"timer\",\"durationSeconds\":1.5}","{\"app\":\"timer\",\"durationSeconds\":\"10\"}","{\"app\":\"score\",\"league\":\"fra.1\",\"eventId\":\"123456\",\"date\":\"20260230\"}","{\"app\":\"score\",\"league\":\"fra.1\\u0000x\",\"eventId\":\"123456\",\"date\":\"20261015\"}"}){
    request(body);CHECK(responseStatus==400);CHECK(Preferences::values["widget2"]==saved);CHECK(json(widgetConfig)==before);CHECK(Preferences::writes.size()==1);
  }
  Preferences::failKey="widget2";request("{\"app\":\"timer\",\"durationSeconds\":45}");CHECK(responseStatus==503);CHECK(widgetTimer.durationSeconds()==300);CHECK(json(widgetConfig)==before);CHECK(Preferences::values["widget2"]==saved);
  Preferences::failKey.clear();request("{\"app\":\"timer\",\"durationSeconds\":45}");CHECK(responseStatus==200);CHECK(widgetTimer.durationSeconds()==45);CHECK(!widgetTimer.running(0));CHECK(settings.app=="clock");
  const auto generation=scores.generation_;Preferences::failKey="widget2";request("{\"app\":\"score\",\"league\":\"fra.1\",\"eventId\":\"123456\",\"date\":\"20261015\"}");CHECK(responseStatus==503);CHECK(scores.generation_==generation);
  Preferences::failKey.clear();request("{\"app\":\"score\",\"league\":\"fra.1\",\"eventId\":\"123456\",\"date\":\"20261015\"}");CHECK(responseStatus==200);CHECK(scores.generation_==generation+1);CHECK(widgetConfig["eventId"]=="123456");CHECK(settings.app=="clock");
}
void test_timer_actions_and_restore(){
  reset();loadWidgets();request("{\"app\":\"timer\",\"durationSeconds\":10}");request("{\"action\":\"start\"}",true);CHECK(responseStatus==200);CHECK(widgetTimer.running(0));
  testNow=1500;const String saved=Preferences::values["widget2"];Preferences::failKey="widget2";request("{\"action\":\"pause\"}",true);CHECK(responseStatus==503);CHECK(widgetTimer.running(testNow));CHECK(widgetTimer.remainingSeconds(testNow)==9);CHECK(Preferences::values["widget2"]==saved);
  Preferences::failKey.clear();request("{\"action\":\"pause\"}",true);CHECK(responseStatus==200);CHECK(!widgetTimer.running(testNow));CHECK(widgetTimer.remainingSeconds(90000)==9);
  JsonDocument persisted;CHECK(!deserializeJson(persisted,Preferences::values["widget2"]));CHECK(persisted["remainingSeconds"]==9);CHECK(persisted["durationSeconds"]==10);
  const size_t writes=Preferences::writes.size();request("{\"action\":\"pause\"}",true);CHECK(responseStatus==409);CHECK(Preferences::writes.size()==writes);request("{\"action\":\"start\\u0000x\"}",true);CHECK(responseStatus==400);CHECK(Preferences::writes.size()==writes);
  testNow=5000;request("{\"action\":\"resume\"}",true);CHECK(responseStatus==200);CHECK(widgetTimer.running(testNow));testNow=6000;CHECK(widgetTimer.remainingSeconds(testNow)==8);
  loadWidgets();CHECK(!widgetTimer.running(testNow));CHECK(widgetTimer.remainingSeconds(testNow)==9);CHECK(widgetTimer.durationSeconds()==10); // Restarts use the persisted action snapshot, paused.
  request("{\"action\":\"reset\"}",true);CHECK(responseStatus==200);CHECK(!widgetTimer.running(testNow));CHECK(widgetTimer.remainingSeconds(testNow)==10);
  request("{\"action\":\"start\"}",true);testNow+=10000;CHECK(widgetTimer.remainingSeconds(testNow)==0);request("{\"action\":\"resume\"}",true);CHECK(responseStatus==409);CHECK(!widgetTimer.running(testNow));
  for(const char* raw:{"not JSON","{}","{\"text\":false,\"color\":-1,\"durationSeconds\":65537}","{\"durationSeconds\":10,\"remainingSeconds\":9999}"}){reset();Preferences::values["widget2"]=raw;loadWidgets();CHECK(!widgetTimer.running(0));CHECK(ZcsTimer::validDuration(widgetTimer.durationSeconds()));CHECK(widgetTimer.remainingSeconds(0)<=widgetTimer.durationSeconds());CHECK(widgetConfig["text"]=="BONJOUR");}
  reset();settings.app="pixel";settings.contentId="missing";loadWidgets();CHECK(settings.app=="clock");CHECK(settings.contentId.isEmpty());
  reset();settings.app="timer";settings.contentId="tea";loadWidgets();CHECK(widgetTimer.durationSeconds()==180);CHECK(widgetTimer.remainingSeconds(0)==180);CHECK(!widgetTimer.running(0));
}
int main(){test_pack_validation();test_configuration_atomicity();test_timer_actions_and_restore();std::cout<<"Widget API handlers: "<<assertions<<" assertions passed\n";}
