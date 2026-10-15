#pragma once
struct WiFiClientSecure {
  inline static const char* caCertificate = nullptr;
  void setInsecure(){}
  void setHandshakeTimeout(int){}
  void setCACert(const char* certificate){caCertificate=certificate;}
};
