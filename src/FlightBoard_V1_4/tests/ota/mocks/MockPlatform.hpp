#pragma once
#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#define PROGMEM
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#define ARDUINOJSON_ENABLE_ARDUINO_STREAM 0
#define ARDUINOJSON_ENABLE_ARDUINO_PRINT 0
class String {
  std::string value_;
public:
  String() = default;
  String(const char* s):value_(s?s:""){}
  String(std::string s):value_(std::move(s)){}
  String& operator=(const char* s){value_=s?s:"";return *this;}
  const char* c_str() const{return value_.c_str();}
  size_t length() const{return value_.size();}
  bool isEmpty() const{return value_.empty();}
  char operator[](size_t i)const{return value_[i];}
  int indexOf(const char* s)const{auto i=value_.find(s);return i==std::string::npos?-1:static_cast<int>(i);}
  bool endsWith(const char* s)const { std::string end=s; return value_.size()>=end.size() && value_.compare(value_.size()-end.size(),end.size(),end)==0; }
  void toLowerCase(){std::transform(value_.begin(),value_.end(),value_.begin(),[](unsigned char c){return std::tolower(c);});}
  bool concat(const char* s){if(s)value_+=s;return true;}
  friend bool operator==(const String&a,const String&b){return a.value_==b.value_;}
  friend bool operator!=(const String&a,const String&b){return !(a==b);}
  friend std::ostream& operator<<(std::ostream&o,const String&s){return o<<s.value_;}
};

inline unsigned long testNow=1000;
inline unsigned long millis(){return testNow;}
inline int64_t esp_timer_get_time(){return testNow*1000LL;}
struct EspMock {int restarts=0; uint32_t getFreeHeap(){return 140000;} void restart(){++restarts;}};
inline EspMock ESP;
inline uint32_t esp_random(){static uint32_t n=0x12345678;return ++n;}
constexpr int HTTP_GET=0,HTTP_POST=1;
enum HTTPUploadStatus {UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END, UPLOAD_FILE_ABORTED};
struct HTTPUpload {HTTPUploadStatus status=UPLOAD_FILE_START;String name,filename;size_t totalSize=0,currentSize=0;uint8_t buf[2048]={};};
struct WebServer {
 std::map<std::pair<std::string,int>,std::function<void()>> routes;
 std::function<void()> uploadHandler;
 std::map<std::string,String> requestHeaders,responseHeaders;
 std::vector<String> collected;
 HTTPUpload currentUpload;
 String responseBody;int responseCode=0,argsCount=0;
 void on(const char*p,int method,std::function<void()>h){routes[{p,method}]=h;}
 void on(const char*p,int method,std::function<void()>h,std::function<void()>u){on(p,method,h);uploadHandler=u;}
 int headers()const{return collected.size();} String headerName(int i){return collected.at(i);}
 String header(const char*key)const{auto p=requestHeaders.find(key);return p==requestHeaders.end()?String():p->second;}
 void collectHeaders(const char**keys,size_t n){collected.clear();for(size_t i=0;i<n;++i)collected.emplace_back(keys[i]);}
 void sendHeader(const char*k,const char*v){responseHeaders[k]=v;}
 void send(int code,const char*,const String&body){responseCode=code;responseBody=body;}
 void send_P(int code,const char*,const char*body,size_t size){responseCode=code;responseBody=std::string(body,size);}
 void request(const char*p,int method=HTTP_GET){responseCode=0;responseBody="";routes.at({p,method})();}
 int args()const{return argsCount;}
 HTTPUpload&upload(){return currentUpload;}
 void event(HTTPUploadStatus status){currentUpload.status=status;uploadHandler();}
};
constexpr int U_FLASH=0;
struct UpdateMock {
 size_t expected=0;std::vector<uint8_t>written;int begins=0,ends=0,aborts=0;
 bool begun=false,bootActivated=false,failBegin=false,failWrite=false,failEnd=false;
 bool begin(size_t size,int){++begins;expected=size;written.clear();begun=!failBegin;return begun;}
 size_t write(uint8_t*data,size_t size){if(failWrite||!begun)return 0;written.insert(written.end(),data,data+size);return size;}
 bool end(bool even){++ends;if(even)throw std::runtime_error("unsafe partial end");if(!begun||failEnd||written.size()!=expected)return false;bootActivated=true;begun=false;return true;}
 void abort(){++aborts;begun=false;}
};
inline UpdateMock Update;
struct esp_partition_t {int address,type,subtype;size_t size;};
constexpr int ESP_PARTITION_TYPE_APP=0,ESP_PARTITION_TYPE_DATA=1,ESP_PARTITION_SUBTYPE_DATA_OTA=0,ESP_PARTITION_SUBTYPE_APP_OTA_MIN=16,ESP_PARTITION_SUBTYPE_APP_OTA_MAX=32;
inline esp_partition_t running{0x10000,0,16,1310720},next{0x150000,0,17,1310720},otadata{0xe000,1,0,8192};
inline bool noNext=false,noOtaData=false,samePartition=false;
inline const esp_partition_t*esp_ota_get_next_update_partition(const void*){return noNext?nullptr:(samePartition?&running:&next);}
inline const esp_partition_t*esp_ota_get_running_partition(){return &running;}
inline const esp_partition_t*esp_partition_find_first(int,int,const char*){return noOtaData?nullptr:&otadata;}
