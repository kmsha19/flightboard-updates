#pragma once
struct WiFiClientSecure {
  inline static const char* caCertificate = nullptr;
  inline static bool insecure = false;
  inline static int handshakeTimeout = 0;
  void setInsecure(){insecure=true;}
  void setHandshakeTimeout(int value){handshakeTimeout=value;}
  void setCACert(const char* certificate){caCertificate=certificate;}
};
