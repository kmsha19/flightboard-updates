#include "MockPlatform.hpp"
#include "pgmspace.h"
#include <ArduinoJson.h>
#include "HTTPClient.h"
#include "freertos/FreeRTOS.h"
#include <ctime>
#include <stdexcept>
static time_t weatherEpoch = 1792000000;
time_t weatherTime(time_t* output) { if(output)*output=weatherEpoch;return weatherEpoch; }
#define time weatherTime
#define private public
#include "ZcsWeather.h"
#undef private
#undef time

static int assertions=0;
#define CHECK(expr) do{++assertions;if(!(expr))throw std::runtime_error(std::string(__func__)+":"+std::to_string(__LINE__)+": " #expr);}while(0)
void reset(){
  testNow=0;weatherEpoch=1792000000;WiFi=WiFiMock();WiFi.connect("Mock");
  HTTPClient::responseStatus=200;HTTPClient::responseSize=-2;HTTPClient::responseJson="{}";
  HTTPClient::beginSuccess=true;HTTPClient::gets=HTTPClient::ends=0;
  HTTPClient::headers.clear();MockHTTPStream::readDelayMs=0;MockHTTPStream::bytesRead=0;
  MockHTTPStream::holdOpenAtEnd=false;MockHTTPStream::gapsBeforeByte.clear();
  queueFailuresAfter=-1;taskCreateSuccess=true;stopWorkerAfterSend=false;queueDeletes=0;taskDeletes=0;
  for(auto*q:fakeQueues)delete q;fakeQueues.clear();
}
JsonDocument body(){
  JsonDocument d;auto c=d["current"].to<JsonObject>();
  c["temperature_2m"]=19.4;c["wind_speed_10m"]=12.3;c["weather_code"]=2;c["is_day"]=1;c["time"]=(uint32_t)weatherEpoch;
  auto u=d["current_units"].to<JsonObject>();u["temperature_2m"]="°C";u["wind_speed_10m"]="km/h";u["time"]="unixtime";
  return d;
}
bool decode(const JsonDocument& d,ZcsWeatherData& out){return ZcsWeather::decode(d.as<JsonObjectConst>(),out,(uint32_t)weatherEpoch);}
std::string bodyText(){std::string s;serializeJson(body(),s);return s;}
ZcsWeather::Result fetch(const std::string& raw){HTTPClient::responseJson=raw;ZcsWeather::Result r;ZcsWeather::fetch({48.5,2.5,0},r);return r;}
void test_values_units_and_time(){
  reset();ZcsWeatherData out;auto d=body();CHECK(decode(d,out));CHECK(out.temperatureC==19.4);CHECK(out.windKmh==12.3);CHECK(out.isDay);CHECK(out.updatedEpoch==(uint32_t)weatherEpoch);
  for(int code:{0,1,2,3,45,48,51,53,55,56,57,61,63,65,66,67,71,73,75,77,80,81,82,85,86,95,96,99}){d=body();d["current"]["weather_code"]=code;CHECK(decode(d,out));CHECK(out.code==code);}
  for(int code:{-1,4,50,52,60,70,90,97,100,999}){d=body();d["current"]["weather_code"]=code;CHECK(!decode(d,out));}
  for(const char* key:{"temperature_2m","wind_speed_10m","weather_code","is_day","time"}){
    d=body();d["current"].remove(key);CHECK(!decode(d,out));
    d=body();d["current"][key]="1";CHECK(!decode(d,out));
    d=body();d["current"][key]=nullptr;CHECK(!decode(d,out));
    d=body();d["current"][key]=true;CHECK(!decode(d,out));
  }
  for(const char* key:{"temperature_2m","wind_speed_10m","time"}){d=body();d["current_units"][key]="wrong";CHECK(!decode(d,out));d["current_units"].remove(key);CHECK(!decode(d,out));}
  for(double temp:{-100.,80.}){d=body();d["current"]["temperature_2m"]=temp;CHECK(decode(d,out));}
  for(double temp:{-100.01,80.01,double(NAN),double(INFINITY)}){d=body();d["current"]["temperature_2m"]=temp;CHECK(!decode(d,out));}
  for(double wind:{0.,500.}){d=body();d["current"]["wind_speed_10m"]=wind;CHECK(decode(d,out));}
  for(double wind:{-0.1,500.1,double(NAN),double(INFINITY)}){d=body();d["current"]["wind_speed_10m"]=wind;CHECK(!decode(d,out));}
  for(int day:{0,1}){d=body();d["current"]["is_day"]=day;CHECK(decode(d,out));CHECK(out.isDay==(day==1));}
  for(double day:{-1.,2.,.5}){d=body();d["current"]["is_day"]=day;CHECK(!decode(d,out));}
  for(int offset:{-7200,900}){d=body();d["current"]["time"]=(uint32_t)(weatherEpoch+offset);CHECK(decode(d,out));}
  for(int offset:{-7201,901}){d=body();d["current"]["time"]=(uint32_t)(weatherEpoch+offset);CHECK(!decode(d,out));}
  d=body();d["current"]["time"]=1704067199UL;CHECK(!decode(d,out));
  out.temperatureC=77;d=body();d["current"]["weather_code"]=999;CHECK(!decode(d,out));CHECK(out.temperatureC==77);
}
void test_bounded_tls_fetch(){
  reset();auto r=fetch(bodyText());CHECK(r.ok);CHECK(std::abs(r.data.temperatureC-19.4)<1e-6);CHECK(HTTPClient::ends==1);
  CHECK(HTTPClient::lastUrl.indexOf("https://api.open-meteo.com/v1/forecast?latitude=48.50&longitude=2.50")==0);
  CHECK(HTTPClient::headers["Accept-Encoding"]=="identity");CHECK(WiFiClientSecure::caCertificate);CHECK(std::strstr(WiFiClientSecure::caCertificate,"BEGIN CERTIFICATE"));
  for(int status:{-1,429,503}){HTTPClient::responseStatus=status;r=fetch(bodyText());CHECK(!r.ok);CHECK(std::string(r.error)==(status<0?"network":"http"));}
  HTTPClient::responseStatus=200;
  for(const char* raw:{"{}","null","{\"current\":!}"}){r=fetch(raw);CHECK(!r.ok);}
  HTTPClient::responseSize=2049;r=fetch(bodyText());CHECK(!r.ok);CHECK(std::string(r.error)=="size");
  reset();MockHTTPStream::gapsBeforeByte={{0,100},{15,700}};r=fetch(bodyText());CHECK(r.ok);CHECK(testNow==800);
  reset();MockHTTPStream::gapsBeforeByte={{15,2500}};r=fetch(bodyText());CHECK(!r.ok);CHECK(std::string(r.error)=="timeout");CHECK(testNow==2000);
  reset();HTTPClient::responseSize=-1;r=fetch(std::string(2049,' '));CHECK(!r.ok);CHECK(MockHTTPStream::bytesRead==2048);
  reset();testNow=0xffffff00UL;MockHTTPStream::gapsBeforeByte={{0,600}};r=fetch(bodyText());CHECK(r.ok);CHECK(testNow==344);
  reset();WiFi.disconnect();r=fetch(bodyText());CHECK(!r.ok);CHECK(std::string(r.error)=="wifi");CHECK(HTTPClient::gets==0);
  reset();weatherEpoch=0;r=fetch(bodyText());CHECK(!r.ok);CHECK(std::string(r.error)=="time");CHECK(HTTPClient::gets==0);
}
void finish(ZcsWeather& weather){const int deleted=taskDeletes;workerFunction(workerContext);CHECK(taskDeletes==deleted+1);weather.update();}
void test_cache_cadence_and_generation(){
  reset();ZcsWeather w;JsonDocument d;w.toJson(d.to<JsonObject>());CHECK(!d["available"].as<bool>());CHECK(d["temperatureC"].isNull());CHECK(d["ageSeconds"].as<int>()==-1);
  CHECK(!w.request(NAN,0));CHECK(!w.request(0,181));CHECK(w.request(48.5,2.5));CHECK(w.pending());CHECK(!w.request(0,0));
  HTTPClient::responseJson=bodyText();finish(w);CHECK(!w.pending());CHECK(w.available());CHECK(!w.stale(1799999));CHECK(w.stale(1800000));
  w.toJson(d.to<JsonObject>());CHECK(d["status"]=="ready");CHECK(std::abs(d["temperatureC"].as<double>()-19.4)<1e-6);
  testNow=899999;CHECK(!w.request(48.5,2.5));testNow=900000;CHECK(w.request(48.5,2.5));
  HTTPClient::responseStatus=503;finish(w);CHECK(w.available());CHECK(std::abs(w.data().temperatureC-19.4)<1e-6);
  testNow=1800000;w.toJson(d.to<JsonObject>());CHECK(d["status"]=="stale");CHECK(d["stale"].as<bool>());CHECK(w.request(48.5,2.5));
  w.resetLocation();CHECK(w.pending());CHECK(!w.available());CHECK(!w.request(1,2));
  HTTPClient::responseStatus=200;HTTPClient::responseJson=bodyText();finish(w);CHECK(!w.available());CHECK(!w.pending());
  CHECK(w.request(1,2));auto old=body();old["current"]["time"]=(uint32_t)(weatherEpoch-1900);serializeJson(old,HTTPClient::responseJson);finish(w);CHECK(w.available());CHECK(w.stale((uint32_t)testNow));CHECK(w.ageSeconds((uint32_t)testNow)==1900);
  reset();ZcsWeather wrapped;testNow=0xfffffff0UL;CHECK(wrapped.request(0,0));HTTPClient::responseJson=bodyText();finish(wrapped);testNow=(uint32_t)(testNow+900000);CHECK(wrapped.request(0,0));
}
void test_worker_failures(){
  for(int failure:{0,1}){reset();queueFailuresAfter=failure;ZcsWeather w;CHECK(!w.request(0,0));CHECK(!w.pending());CHECK(fakeQueues.empty());}
  reset();taskCreateSuccess=false;ZcsWeather w;CHECK(!w.request(0,0));CHECK(fakeQueues.size()==2);CHECK(fakeQueues[0]->items.empty());taskCreateSuccess=true;CHECK(!w.request(0,0));testNow=900000;CHECK(w.request(0,0));
}
int main(){test_values_units_and_time();test_bounded_tls_fetch();test_cache_cadence_and_generation();test_worker_failures();std::cout<<"ZCS weather: "<<assertions<<" assertions passed\n";}
