#pragma once
#include "MockPlatform.hpp"
#include "WiFiClientSecure.h"
struct MockHTTPStream : std::istringstream {
  inline static uint32_t readDelayMs = 0;
  inline static size_t bytesRead = 0;
  inline static bool holdOpenAtEnd = false;
  inline static std::map<size_t,uint32_t> gapsBeforeByte;
  uint32_t timeoutMs = 1000;
  size_t consumed = 0, lastGap = (size_t)-1;
  bool gapActive = false;
  uint32_t gapStartedMs = 0;
  void setTimeout(uint32_t timeout) { timeoutMs = timeout; }
  bool connected() { return holdOpenAtEnd || gapActive || !eof(); }
  size_t readBytes(char* buffer,size_t count){
    const auto gap = gapsBeforeByte.find(consumed);
    if (gap != gapsBeforeByte.end() && lastGap != consumed) {
      if (!gapActive) { gapActive = true; gapStartedMs = (uint32_t)testNow; }
      if ((uint32_t)testNow - gapStartedMs < gap->second) return 0;
      gapActive = false;
      lastGap = consumed;
    }
    const uint32_t delay = readDelayMs < timeoutMs ? readDelayMs : timeoutMs;
    testNow = (uint32_t)(testNow + delay);
    if (readDelayMs >= timeoutMs) return 0;
    read(buffer,count);
    const size_t got = static_cast<size_t>(gcount());
    bytesRead += got;
    consumed += got;
    return got;
  }
};
struct HTTPClient {
  inline static int responseStatus=200,gets=0,ends=0;
  inline static int responseSize=-2; // -2: infer, -1: no Content-Length.
  inline static bool beginSuccess=true;
  inline static std::map<std::string,std::string> headers;
  inline static std::map<std::string,std::string> responseHeaders;
  inline static std::string responseJson="{\"ac\":[]}";
  inline static String lastUrl;
  inline static bool http10 = false;
  inline static int connectTimeout = 0, readTimeout = 0;
  MockHTTPStream stream;
  std::vector<std::string> collectedHeaders;
  void useHTTP10(bool value){http10=value;}
  void setConnectTimeout(int value){connectTimeout=value;}
  void setTimeout(int value){readTimeout=value;}
  bool begin(WiFiClientSecure&,const String& url){lastUrl=url;return beginSuccess;}
  void addHeader(const char* name,const char* value){headers[name]=value;}
  void collectHeaders(const char* names[],size_t count){
    for(size_t i=0;i<count;++i)collectedHeaders.emplace_back(names[i]);
  }
  String header(const char* name){
    for(const auto& collected:collectedHeaders)if(collected==name)return responseHeaders[name];
    return "";
  }
  void setUserAgent(const char*){}
  int GET(){++gets;stream.str(responseJson);stream.clear();return responseStatus;}
  int getSize(){return responseSize==-2?static_cast<int>(responseJson.size()):responseSize;}
  void end(){++ends;}
  MockHTTPStream& getStream(){return stream;}
};
