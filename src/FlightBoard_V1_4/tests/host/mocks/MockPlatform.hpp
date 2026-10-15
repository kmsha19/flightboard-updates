#pragma once
#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <deque>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#define ARDUINOJSON_ENABLE_ARDUINO_STREAM 0
#define ARDUINOJSON_ENABLE_ARDUINO_PRINT 0
class String {
  std::string value_;
public:
  String() = default;
  String(const char* s):value_(s?s:""){}
  String(std::string s):value_(std::move(s)){}
  String(int value):value_(std::to_string(value)){}
  String(double value,int precision){std::ostringstream out;out<<std::fixed<<std::setprecision(precision)<<value;value_=out.str();}
  String& operator=(const char* s){value_=s?s:"";return *this;}
  const char* c_str() const{return value_.c_str();}
  size_t length() const{return value_.size();}
  bool isEmpty() const{return value_.empty();}
  void trim(){auto a=value_.find_first_not_of(" \t\r\n");if(a==std::string::npos){value_.clear();return;}auto b=value_.find_last_not_of(" \t\r\n");value_=value_.substr(a,b-a+1);}
  void toUpperCase(){for(auto& c:value_)c=std::toupper((unsigned char)c);}
  String substring(size_t a,size_t b)const{return value_.substr(a,b-a);}
  friend String operator+(const String&a,const String&b){return a.value_+b.value_;}
  char operator[](size_t i)const{return value_[i];}
  int indexOf(const char* s)const{auto i=value_.find(s);return i==std::string::npos?-1:static_cast<int>(i);}
  bool concat(const char* s){if(s)value_+=s;return true;}
  friend bool operator==(const String&a,const String&b){return a.value_==b.value_;}
  friend bool operator!=(const String&a,const String&b){return !(a==b);}
  friend std::ostream& operator<<(std::ostream&o,const String&s){return o<<s.value_;}
};
inline unsigned long testNow=0;
inline unsigned long millis(){return testNow;}
inline void delay(unsigned long duration){testNow=(uint32_t)(testNow+duration);}
struct SerialMock {
  std::ostringstream output;
  template<class T> void print(const T&v){output<<v;}
  template<class T> void println(const T&v){output<<v<<'\n';}
  void println(){output<<'\n';}
  void reset(){output.str("");output.clear();}
  void printf(const char*,...){}
};
inline SerialMock Serial;
struct EspMock {uint64_t getEfuseMac(){return 0x12345678;}};
inline EspMock ESP;
inline uint32_t esp_random(){static uint32_t n=0x11111111;return ++n;}
class IPAddress {
  uint32_t value_=0;
public:
  IPAddress()=default;
  IPAddress(int a,int b,int c,int d):value_((a<<24)|(b<<16)|(c<<8)|d){}
  String toString()const{return std::to_string((value_>>24)&255)+"."+std::to_string((value_>>16)&255)+"."+std::to_string((value_>>8)&255)+"."+std::to_string(value_&255);}
  friend bool operator!=(IPAddress a,IPAddress b){return a.value_!=b.value_;}
  friend bool operator==(IPAddress a,IPAddress b){return !(a!=b);}
  friend std::ostream&operator<<(std::ostream&o,const IPAddress&i){return o<<i.toString();}
};
constexpr int WIFI_STA=1, WIFI_AP_STA=3, WL_IDLE_STATUS=0, WL_CONNECTED=3, WIFI_SCAN_RUNNING=-1, WIFI_SCAN_FAILED=-2, WIFI_AUTH_OPEN=0;
struct WiFiMock {
  struct Network {String ssid;int rssi;int auth;};
  int modeValue=0,statusValue=WL_IDLE_STATUS,stationCount=0,beginCalls=0,disconnectCalls=0,scanCalls=0,scanDeleteCalls=0,apStartCalls=0,apStopCalls=0;
  int scanStartResult=WIFI_SCAN_RUNNING,scanCompleteResult=WIFI_SCAN_FAILED;
  bool apConfigSuccess=true,apSuccess=true;
  String currentSsid,lastBeginSsid,lastBeginPassword,apSsid,apPassword;
  IPAddress ip;
  std::vector<Network> networks;
  void persistent(bool){}
  void setHostname(const char*){}
  void mode(int m){modeValue=m;}
  void setAutoReconnect(bool){}
  int status()const{return statusValue;}
  IPAddress localIP()const{return ip;}
  String SSID()const{return currentSsid;}
  String SSID(int i)const{return networks.at(i).ssid;}
  int RSSI(int i)const{return networks.at(i).rssi;}
  int encryptionType(int i)const{return networks.at(i).auth;}
  int softAPgetStationNum()const{return stationCount;}
  bool softAPConfig(IPAddress,IPAddress,IPAddress){return apConfigSuccess;}
  bool softAP(const char*s,const char*p){++apStartCalls;apSsid=s;apPassword=p;return apSuccess;}
  bool softAPdisconnect(bool){++apStopCalls;return true;}
  bool disconnect(bool=false,bool=false,unsigned long=100){++disconnectCalls;statusValue=WL_IDLE_STATUS;ip={};return true;}
  int begin(const char*s,const char*p){++beginCalls;lastBeginSsid=s;lastBeginPassword=p;return statusValue;}
  int scanNetworks(bool){++scanCalls;scanCompleteResult=scanStartResult;return scanStartResult;}
  int scanComplete()const{return scanCompleteResult;}
  void scanDelete(){++scanDeleteCalls;scanCompleteResult=WIFI_SCAN_FAILED;}
  void connect(const char*s){statusValue=WL_CONNECTED;currentSsid=s;ip=IPAddress(192,168,1,42);}
};
inline WiFiMock WiFi;
struct DNSServer {void processNextRequest(){} bool start(int,const char*,IPAddress){return true;}void stop(){}};
struct MdnsMock {int starts=0,stops=0;bool begin(const char*){++starts;return true;}void addService(const char*,const char*,int){}void end(){++stops;}};
inline MdnsMock MDNS;
inline int scanStops=0;
inline int esp_wifi_scan_stop(){++scanStops;return 0;}
struct Preferences {
  static inline std::map<std::string,String> values;
  static inline std::vector<std::pair<std::string,String>> writes;
  static inline bool available=true,failStationWrite=false;
  static inline std::string failKey;
  static inline unsigned int uintWriteAttempts = 0;
  bool begin(const char*,bool){return available;}
  String getString(const char*k,const String&d=String()){auto i=values.find(k);return i==values.end()?d:i->second;}
  int getInt(const char*k,int d=0){auto i=values.find(k);return i==values.end()?d:std::atoi(i->second.c_str());}
  uint32_t getUInt(const char*k,uint32_t d=0){
    auto i=values.find(k);
    if(!available||i==values.end()||i->second.isEmpty())return d;
    uint64_t value=0;
    for(size_t n=0;n<i->second.length();++n){
      const char c=i->second[n];
      if(c<'0'||c>'9')return d;
      value=value*10+(c-'0');
      if(value>UINT32_MAX)return d;
    }
    return static_cast<uint32_t>(value);
  }
  double getDouble(const char*k,double d=0){auto i=values.find(k);return i==values.end()?d:std::atof(i->second.c_str());}
  size_t putString(const char*k,const String&v){if(!available||(failStationWrite&&std::string(k)=="station")||failKey==k)return 0;values[k]=v;writes.emplace_back(k,v);return v.length();}
  size_t putUInt(const char*k,uint32_t value){
    ++uintWriteAttempts;
    if(!available||failKey==k)return 0;
    values[k]=std::to_string(value);writes.emplace_back(k,values[k]);return sizeof(uint32_t);
  }
};
constexpr int HTTP_GET=0,HTTP_POST=1;
struct WebServer {
  std::map<std::pair<std::string,int>,std::function<void()>> routes;
  std::map<std::string,String> args,headers;
  int responseCode=0;
  String responseBody;
  void on(const char*p,int method,std::function<void()>handler){routes[{p,method}]=std::move(handler);}
  String arg(const char*k){auto i=args.find(k);return i==args.end()?String():i->second;}
  void sendHeader(const char*k,const char*v){headers[k]=v;}
  void send(int code,const char*,const String&body){responseCode=code;responseBody=body;}
  void request(const char*p,int method=HTTP_GET,const String&body=String(),bool refresh=false){args.clear();headers.clear();responseCode=0;responseBody="";args["plain"]=body;if(refresh)args["refresh"]="1";routes.at({p,method})();}
};
