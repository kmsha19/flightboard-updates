#pragma once
#include "ZcsWeather.h"
#include <ctype.h>

// Public ESPN scoreboard. Only the selected match is retained. No API key.
struct ZcsScoreData {
  char home[13] = "", away[13] = "", status[25] = "";
  uint16_t homeScore = 0, awayScore = 0;
};
class ZcsScores {
 public:
  static bool validSelection(const String& league, const String& event, const String& date) {
    if (league != "fra.1" && league != "eng.1" && league != "uefa.champions") return false;
    if (event.length() < 5 || event.length() > 12 || date.length() != 8) return false;
    for (size_t i=0;i<event.length();++i) if (event[i]<'0'||event[i]>'9') return false;
    for (size_t i=0;i<date.length();++i) if (date[i]<'0'||date[i]>'9') return false;
    const int year=date.substring(0,4).toInt(), month=date.substring(4,6).toInt(), day=date.substring(6,8).toInt();
    const int days[]={0,31,28+(year%4==0&&(year%100!=0||year%400==0)),31,30,31,30,31,31,30,31,30,31};
    return year>=2024 && year<=2099 && month>=1 && month<=12 && day>=1 && day<=days[month];
  }
  bool pending() const { return pending_; }
  bool available() const { return available_; }
  bool stale(uint32_t now) const { return available_ && (error_ || uint32_t(now-received_)>=180000); }
  const ZcsScoreData& data() const { return data_; }
  void reset() { ++generation_; available_=attempted_=false; error_=false; }
  bool request(const String& league,const String& event,const String& date) {
    const uint32_t now=millis();
    if (pending_||!validSelection(league,event,date)||WiFi.status()!=WL_CONNECTED||time(nullptr)<1704067200||
        (attempted_&&uint32_t(now-requested_)<60000)) return false;
    requested_=now;attempted_=true;
    if(!queue_)queue_=xQueueCreate(1,sizeof(Result*));
    if(!queue_){error_=true;return false;}
    job_={};snprintf(job_.league,sizeof(job_.league),"%s",league.c_str());
    snprintf(job_.event,sizeof(job_.event),"%s",event.c_str());snprintf(job_.date,sizeof(job_.date),"%s",date.c_str());
    job_.generation=generation_;
    if(xTaskCreate(run,"zcs-scores",12288,this,1,nullptr)!=pdPASS){error_=true;return false;}
    pending_=true;return true;
  }
  void update() {
    Result* r=nullptr;
    if(!queue_||xQueueReceive(queue_,&r,0)!=pdTRUE)return;
    pending_=false;
    if(!r){error_=true;return;}
    if(r->generation==generation_){error_=!r->ok;if(r->ok){data_=r->data;available_=true;received_=millis();}}
    delete r;
  }
  void toJson(JsonObject o) const {
    o["available"]=available_;o["stale"]=stale(millis());o["source"]="ESPN";
    o["loading"]=pending_;o["error"]=error_;o["ageSeconds"]=available_?int32_t(uint32_t(millis()-received_)/1000):-1;
    if(available_){o["home"]=data_.home;o["away"]=data_.away;o["homeScore"]=data_.homeScore;o["awayScore"]=data_.awayScore;o["status"]=data_.status;}
  }
 private:
  struct Job {char league[16],event[13],date[9];uint32_t generation;};
  struct Result {ZcsScoreData data;uint32_t generation=0;bool ok=false;};
  Job job_{}; QueueHandle_t queue_=nullptr;
  uint32_t generation_=0,requested_=0,received_=0;
  bool pending_=false,available_=false,attempted_=false,error_=false;
  ZcsScoreData data_;
  static bool scoreValue(JsonVariantConst v,uint16_t& out){
    if(!v.is<JsonString>())return false;const JsonString s=v.as<JsonString>();
    if(s.size()<1||s.size()>3||strlen(s.c_str())!=s.size())return false;
    unsigned n=0;for(size_t i=0;i<s.size();++i){if(s.c_str()[i]<'0'||s.c_str()[i]>'9')return false;n=n*10+s.c_str()[i]-'0';}out=n;return true;
  }
  static bool abbreviation(JsonVariantConst v,char* out){
    if(!v.is<JsonString>())return false;const JsonString s=v.as<JsonString>();
    if(s.size()<1||s.size()>12||strlen(s.c_str())!=s.size())return false;
    for(size_t i=0;i<s.size();++i){char c=s.c_str()[i];if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c==' '||c=='-'))return false;out[i]=toupper(c);}out[s.size()]=0;return true;
  }
  static bool plainString(JsonVariantConst value) {
    return value.is<JsonString>() && strlen(value.as<JsonString>().c_str()) == value.as<JsonString>().size();
  }
  static bool decode(JsonObjectConst root,const char* event,ZcsScoreData& out){
    for(JsonObjectConst e:root["events"].as<JsonArrayConst>()){
      if(!plainString(e["id"])||strcmp(e["id"]|"",event)!=0)continue;
      const auto comp=e["competitions"][0]; const auto teams=comp["competitors"].as<JsonArrayConst>();
      if(teams.size()!=2)return false;ZcsScoreData next;bool home=false,away=false;
      for(JsonObjectConst t:teams){if(!plainString(t["homeAway"]))return false;const char* side=t["homeAway"]|"";
        if(!strcmp(side,"home")&&!home){home=abbreviation(t["team"]["abbreviation"],next.home)&&scoreValue(t["score"],next.homeScore);}
        else if(!strcmp(side,"away")&&!away){away=abbreviation(t["team"]["abbreviation"],next.away)&&scoreValue(t["score"],next.awayScore);}else return false;
      }
      if(!plainString(comp["status"]["type"]["state"]))return false;
      const char* state=comp["status"]["type"]["state"]|"";
      if(!strcmp(state,"pre"))strcpy(next.status,"A VENIR");
      else if(!strcmp(state,"post"))strcpy(next.status,"TERMINE");
      else if(!strcmp(state,"in")){
        const char* clock=comp["status"]["displayClock"]|"";char clean[9]={};size_t n=0;
        for(size_t i=0;clock[i]&&i<12&&n<8;++i)if((clock[i]>='0'&&clock[i]<='9')||clock[i]==':'||clock[i]=='+')clean[n++]=clock[i];
        snprintf(next.status,sizeof(next.status),"EN COURS %s",clean);
      }else return false;
      if(!home||!away)return false;out=next;return true;
    }return false;
  }
  template<class StreamType> struct Reader {
    StreamType& stream;size_t left;uint32_t start=millis();bool failed=false;
    int read(){char c;return readBytes(&c,1)==1?uint8_t(c):-1;}
    size_t readBytes(char* b,size_t n){size_t got=0;while(got<n){if(!left||uint32_t(millis()-start)>=10000){failed=true;break;}stream.setTimeout(1000);size_t r=stream.readBytes(b+got,1);if(!r){failed=true;break;}--left;++got;}return got;}
  };
  static void run(void* context){auto* self=static_cast<ZcsScores*>(context);const Job job=self->job_;auto* r=new(std::nothrow)Result();
    if(r){r->generation=job.generation;fetch(job,*r);}xQueueSend(self->queue_,&r,portMAX_DELAY);vTaskDelete(nullptr);}
  static void fetch(const Job& job,Result& result){
    WiFiClientSecure client;client.setCACert(ZCS_ISRG_ROOT_CA);client.setHandshakeTimeout(6);
    HTTPClient http;http.useHTTP10(true);http.setConnectTimeout(6000);http.setTimeout(8000);
    String url=String("https://site.api.espn.com/apis/site/v2/sports/soccer/")+job.league+"/scoreboard?dates="+job.date;
    if(!http.begin(client,url))return;http.addHeader("Accept","application/json");http.addHeader("Accept-Encoding","identity");
    int status=http.GET();if(status!=200){http.end();return;}
    int bytes=http.getSize();if(bytes==0||bytes>524288){http.end();return;}
    JsonDocument filter;auto e=filter["events"][0];e["id"]=true;auto c=e["competitions"][0];
    c["status"]["type"]["state"]=true;c["status"]["displayClock"]=true;
    auto t=c["competitors"][0];t["homeAway"]=true;t["score"]=true;t["team"]["abbreviation"]=true;
    auto& stream=http.getStream();Reader<decltype(stream)> reader{stream,bytes>0?size_t(bytes):524288};JsonDocument doc;
    auto err=deserializeJson(doc,reader,DeserializationOption::Filter(filter),DeserializationOption::NestingLimit(16));
    http.end();result.ok=!err&&!reader.failed&&decode(doc.as<JsonObjectConst>(),job.event,result.data);
  }
};
