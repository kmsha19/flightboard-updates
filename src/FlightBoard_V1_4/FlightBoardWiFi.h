#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <esp_system.h>
#include <esp_wifi.h>

// V1.9 : connexion non bloquante, configuration locale et reseau de secours.
class FlightBoardWiFi {
public:
  explicit FlightBoardWiFi(WebServer& server) : server_(server) {}

  // Facultatif : suspendre les commandes reseau pendant une mise a jour OTA.
  void setBusyCheck(bool (*check)()) { busyCheck_ = check; }

  void begin(const char* defaultSsid, const char* defaultPassword) {
    storageReady_ = credentials_.begin("flightboard-net", false);
    JsonDocument saved;
    if (storageReady_ &&
        !deserializeJson(saved, credentials_.getString("station", "")) &&
        saved["ssid"].is<String>() && saved["password"].is<String>()) {
      ssid_ = saved["ssid"].as<String>();
      password_ = saved["password"].as<String>();
      if (!validCredentials(ssid_, password_, password_.isEmpty())) {
        ssid_ = "";
        password_ = "";
      }
    }

    // Compatibilite V1.8 : Secrets.h sert seulement de solution initiale.
    if (ssid_.isEmpty() && defaultSsid && defaultPassword &&
        String(defaultSsid).indexOf("REMPLACER") < 0 &&
        String(defaultPassword).indexOf("REMPLACER") < 0 &&
        validCredentials(defaultSsid, defaultPassword, !strlen(defaultPassword))) {
      ssid_ = defaultSsid;
      password_ = defaultPassword;
    }

    apPassword_ = storageReady_ ? credentials_.getString("apkey", "") : "";
    if (apPassword_.length() != 16) {
      char key[17];
      snprintf(key, sizeof(key), "%08lx%08lx",
               (unsigned long)esp_random(), (unsigned long)esp_random());
      apPassword_ = key;
      if (storageReady_) credentials_.putString("apkey", apPassword_);
    }

    char name[24];
    snprintf(name, sizeof(name), "FlightBoard-%04X",
             (unsigned int)(ESP.getEfuseMac() & 0xFFFF));
    apSsid_ = name;

    WiFi.persistent(false);
    WiFi.setHostname("flightboard");
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    server_.on("/api/wifi", HTTP_GET, [this]() { touch(); sendStatus(); });
    server_.on("/api/wifi", HTTP_POST, [this]() {
      if (rejectBusy()) return;
      touch();
      configure();
    });
    server_.on("/api/wifi/scan", HTTP_GET, [this]() {
      if (rejectBusy()) return;
      touch();
      sendScan();
    });
    server_.on("/api/wifi/keepalive", HTTP_GET, [this]() {
      touch();
      JsonDocument doc;
      doc["ok"] = true;
      sendJson(200, doc);
    });

    if (ssid_.isEmpty()) {
      message_ = "Choisis ton reseau Wi-Fi pour demarrer.";
      startAccessPoint();
    } else {
      startConnection(false);
    }
  }

  // Renvoie true une seule fois a chaque nouvelle connexion a la box.
  bool update() {
    const unsigned long now = millis();
    if (apActive_) dns_.processNextRequest();
    updateScan();

    if (queued_) {
      // Laisser partir la reponse HTTP avant de changer de reseau.
      if (now - queuedAt_ >= 1200UL) {
        queued_ = false;
        startConnection(true);
      }
      return false;
    }

    const bool connected = WiFi.status() == WL_CONNECTED &&
                           WiFi.localIP() != IPAddress(0, 0, 0, 0);
    if (attempting_) {
      const String& target = testingNew_ ? pendingSsid_ : ssid_;
      if (connected && WiFi.SSID() == target) {
        if (testingNew_ && !saveCandidate()) {
          failConnection("Connexion testee, mais sauvegarde impossible. Ancien reseau conserve.");
          return false;
        }
        attempting_ = false;
        testingNew_ = false;
        pendingSsid_ = "";
        pendingPassword_ = "";
        if (!changeFailed_) message_ = "Wi-Fi connecte. Reseau memorise.";
      } else if (now - attemptAt_ >= CONNECT_TIMEOUT_MS) {
        failConnection(testingNew_
          ? "Connexion impossible. Verifie le mot de passe et le reseau 2,4 GHz. Ancien reseau conserve."
          : "Reseau indisponible. Utilise le Wi-Fi de secours pour le configurer.");
        return false;
      } else {
        return false;
      }
    }

    if (connected) {
      bool changed = !wasConnected_;
      if (changed) {
        wasConnected_ = true;
        apIdleAt_ = now;
        mdnsStarted_ = MDNS.begin("flightboard");
        if (mdnsStarted_) MDNS.addService("http", "tcp", 80);
        Serial.print("Tablette : http://");
        Serial.print(WiFi.localIP());
        Serial.println("/  (ou http://flightboard.local/)");
      }
      // Garder le portail tant qu'une tablette y est connectee.
      if (apActive_) {
        if (WiFi.softAPgetStationNum() || changeFailed_) apIdleAt_ = now;
        if (now - apIdleAt_ >= 60000UL) stopAccessPoint();
      }
      return changed;
    }

    if (wasConnected_) {
      wasConnected_ = false;
      if (mdnsStarted_) MDNS.end();
      mdnsStarted_ = false;
      message_ = "Connexion perdue. Reconnexion automatique en cours.";
      startAccessPoint();
    }
    if (!apActive_ && now - lastApAttempt_ >= 10000UL) {
      startAccessPoint();
    }
    if (!ssid_.isEmpty() && !scanning_ && now - attemptAt_ >= RETRY_MS) {
      startConnection(false);
    }
    return false;
  }

  bool allowsRadar() const {
    return WiFi.status() == WL_CONNECTED && !queued_ && !attempting_ &&
           !scanning_ && (!wifiUsed_ || millis() - lastWifiRequest_ >= 10000UL);
  }

  bool rescueActive() const { return apActive_; }

  // A public update contains no Secrets.h. Preserve legacy fallback credentials
  // locally before replacing the application, without exposing them to the tablet.
  bool prepareUpdate() {
    if (queued_ || attempting_ || testingNew_) return false;
    if (ssid_.isEmpty()) return true;
    if (!storageReady_) return false;
    JsonDocument saved;
    saved["ssid"] = ssid_;
    saved["password"] = password_;
    String json;
    serializeJson(saved, json);
    if (credentials_.getString("station", "") == json) return true;
    if (credentials_.putString("station", json) != json.length()) return false;
    return credentials_.getString("station", "") == json;
  }

private:
  static constexpr unsigned long CONNECT_TIMEOUT_MS = 25000UL;
  static constexpr unsigned long RETRY_MS = 30000UL;
  static constexpr int MAX_NETWORKS = 20;
  struct Network { String ssid; int rssi; bool open; };

  WebServer& server_;
  bool (*busyCheck_)() = nullptr;
  Preferences credentials_;
  DNSServer dns_;
  String ssid_, password_, pendingSsid_, pendingPassword_;
  String apSsid_, apPassword_, message_;
  Network networks_[MAX_NETWORKS];
  int networkCount_ = 0;
  bool storageReady_ = false, apActive_ = false, mdnsStarted_ = false;
  bool attempting_ = false, testingNew_ = false, queued_ = false;
  bool wasConnected_ = false, changeFailed_ = false;
  bool scanning_ = false, scanFinished_ = false, wifiUsed_ = false;
  String scanError_;
  unsigned long attemptAt_ = 0, queuedAt_ = 0, scanAt_ = 0;
  unsigned long apIdleAt_ = 0, lastWifiRequest_ = 0, lastApAttempt_ = 0;

  static bool validCredentials(const String& ssid, const String& password, bool open) {
    if (ssid.isEmpty() || ssid.length() > 32) return false;
    if (open) return password.isEmpty();
    if (password.length() >= 8 && password.length() <= 63) return true;
    if (password.length() != 64) return false;
    for (size_t i = 0; i < 64; ++i) {
      if (!isxdigit((unsigned char)password[i])) return false;
    }
    return true;
  }

  static bool containsNul(JsonString value) {
    for (size_t i = 0; i < value.size(); ++i) {
      if (!value.c_str()[i]) return true;
    }
    return false;
  }

  void touch() { wifiUsed_ = true; lastWifiRequest_ = millis(); }

  bool rejectBusy() {
    if (!busyCheck_ || !busyCheck_()) return false;
    error(503, "Mise a jour du programme en cours. Attends le redemarrage avant de modifier le Wi-Fi.");
    return true;
  }

  bool startAccessPoint() {
    if (apActive_) return true;
    lastApAttempt_ = millis();
    WiFi.mode(WIFI_AP_STA);
    const IPAddress ip(192, 168, 4, 1);
    if (!WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0)) ||
        !WiFi.softAP(apSsid_.c_str(), apPassword_.c_str())) {
      Serial.println("Impossible de demarrer le Wi-Fi de secours.");
      return false;
    }
    apActive_ = true;
    apIdleAt_ = millis();
    dns_.start(53, "*", ip);
    Serial.print("Wi-Fi de secours : ");
    Serial.println(apSsid_);
    // Uniquement la cle du reseau de secours, jamais celle de la box.
    Serial.print("Cle de secours : ");
    Serial.println(apPassword_);
    Serial.println("Configuration : http://192.168.4.1/");
    return true;
  }

  void stopAccessPoint() {
    dns_.stop();
    WiFi.softAPdisconnect(true);
    apActive_ = false;
  }

  void startConnection(bool candidate) {
    if (mdnsStarted_) MDNS.end();
    mdnsStarted_ = false;
    wasConnected_ = false;
    WiFi.disconnect(false, false);
    testingNew_ = candidate;
    attempting_ = true;
    attemptAt_ = millis();
    WiFi.begin(candidate ? pendingSsid_.c_str() : ssid_.c_str(),
               candidate ? pendingPassword_.c_str() : password_.c_str());
    if (!changeFailed_) message_ = "Connexion Wi-Fi en cours...";
  }

  bool saveCandidate() {
    JsonDocument saved;
    saved["ssid"] = pendingSsid_;
    saved["password"] = pendingPassword_;
    String json;
    serializeJson(saved, json);
    // Une seule cle NVS : le nom et le mot de passe restent coherents.
    if (!storageReady_ || credentials_.putString("station", json) != json.length()) {
      return false;
    }
    ssid_ = pendingSsid_;
    password_ = pendingPassword_;
    return true;
  }

  void failConnection(const char* reason) {
    const bool candidateFailed = testingNew_;
    if (candidateFailed) changeFailed_ = true;
    testingNew_ = false;
    attempting_ = false;
    pendingSsid_ = "";
    pendingPassword_ = "";
    message_ = reason;
    WiFi.disconnect(false, false);
    // Retour rapide apres un essai rate ; sinon laisser le portail disponible.
    attemptAt_ = candidateFailed ? millis() - RETRY_MS + 2000UL : millis();
    startAccessPoint();
  }

  void sendJson(int code, JsonDocument& doc) {
    String json;
    serializeJson(doc, json);
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(code, "application/json; charset=utf-8", json);
  }

  void error(int code, const char* message) {
    JsonDocument doc;
    doc["error"] = message;
    sendJson(code, doc);
  }

  void sendStatus(int code = 200) {
    JsonDocument doc;
    const bool connected = WiFi.status() == WL_CONNECTED;
    doc["connected"] = connected;
    doc["ssid"] = connected ? WiFi.SSID() : ssid_;
    doc["ip"] = connected ? WiFi.localIP().toString() : "";
    doc["state"] = changeFailed_ ? "failed" :
      (queued_ || testingNew_ || (attempting_ && !apActive_)) ? "connecting" :
      connected ? "connected" : ssid_.isEmpty() ? "unconfigured" : "failed";
    doc["message"] = message_;
    doc["pendingSsid"] = pendingSsid_;
    doc["apActive"] = apActive_;
    doc["apSsid"] = apSsid_;
    doc["apIp"] = "192.168.4.1";
    doc["apPassword"] = apPassword_;
    doc["hostname"] = "flightboard.local";
    // Le mot de passe de la box n'est jamais renvoye au navigateur.
    sendJson(code, doc);
  }

  void configure() {
    if (queued_ || testingNew_ || scanning_) {
      error(409, "Attends la fin de la connexion ou de la recherche en cours.");
      return;
    }
    if (!storageReady_) {
      error(503, "Memoire indisponible. Impossible d'enregistrer un nouveau reseau.");
      return;
    }
    String raw = server_.arg("plain");
    JsonDocument body;
    if (raw.isEmpty() || raw.length() > 512 || deserializeJson(body, raw) ||
        !body.is<JsonObject>() || !body["ssid"].is<String>() ||
        !body["password"].is<String>() || !body["open"].is<bool>()) {
      error(400, "Nom du reseau, mot de passe et type de securite requis.");
      return;
    }
    const String ssid = body["ssid"].as<String>();
    const String password = body["password"].as<String>();
    if (containsNul(body["ssid"].as<JsonString>()) ||
        containsNul(body["password"].as<JsonString>()) ||
        !validCredentials(ssid, password, body["open"].as<bool>())) {
      error(400, "Reseau : 1 a 32 octets. Mot de passe : 8 a 63 octets ou cle hexadecimale de 64 caracteres. Reseau ouvert : mot de passe vide.");
      return;
    }
    if (!startAccessPoint()) {
      error(503, "Wi-Fi de secours indisponible. Connexion actuelle conservee.");
      return;
    }
    // Un ancien reseau absent ne doit jamais empecher sa reconfiguration.
    if (attempting_) {
      WiFi.disconnect(false, false);
      attempting_ = false;
    }
    pendingSsid_ = ssid;
    pendingPassword_ = password;
    changeFailed_ = false;
    queued_ = true;
    queuedAt_ = millis();
    message_ = "Test du nouveau reseau. Garde les informations de secours affichees.";
    sendStatus(202);
  }

  void updateScan() {
    if (!scanning_) return;
    const int count = WiFi.scanComplete();
    if (count == WIFI_SCAN_RUNNING && millis() - scanAt_ < 15000UL) return;
    networkCount_ = 0;
    scanError_ = "";
    if (count >= 0) {
      for (int i = 0; i < count; ++i) {
        String ssid = WiFi.SSID(i);
        if (ssid.isEmpty() || ssid == apSsid_) continue;
        const bool open = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
        const int rssi = WiFi.RSSI(i);
        int duplicate = -1;
        for (int j = 0; j < networkCount_; ++j) {
          if (networks_[j].ssid == ssid && networks_[j].open == open) duplicate = j;
        }
        if (duplicate >= 0) {
          if (rssi > networks_[duplicate].rssi) networks_[duplicate].rssi = rssi;
        } else if (networkCount_ < MAX_NETWORKS) {
          networks_[networkCount_++] = {ssid, rssi, open};
        }
      }
      for (int i = 1; i < networkCount_; ++i) {
        Network item = networks_[i];
        int j = i - 1;
        while (j >= 0 && networks_[j].rssi < item.rssi) {
          networks_[j + 1] = networks_[j];
          --j;
        }
        networks_[j + 1] = item;
      }
    } else {
      esp_wifi_scan_stop();
      scanError_ = "Recherche Wi-Fi impossible. Reessaie ou saisis le nom du reseau.";
    }
    WiFi.scanDelete();
    scanning_ = false;
    scanFinished_ = true;
  }

  void sendScan() {
    if (queued_ || testingNew_) {
      error(409, "Connexion en cours. Attends quelques secondes avant de rechercher.");
      return;
    }
    if (attempting_) {
      WiFi.disconnect(false, false);
      attempting_ = false;
      attemptAt_ = millis();
    }
    if (!scanning_ && (!scanFinished_ || server_.arg("refresh") == "1")) {
      WiFi.scanDelete();
      scanError_ = "";
      networkCount_ = 0;
      scanAt_ = millis();
      int result = WiFi.scanNetworks(true);
      scanning_ = result == WIFI_SCAN_RUNNING || result >= 0;
      scanFinished_ = !scanning_;
      if (!scanning_) scanError_ = "Recherche indisponible. Saisis le nom du reseau.";
      updateScan();
    }
    JsonDocument doc;
    doc["scanning"] = scanning_;
    JsonArray list = doc["networks"].to<JsonArray>();
    for (int i = 0; i < networkCount_; ++i) {
      JsonObject item = list.add<JsonObject>();
      item["ssid"] = networks_[i].ssid;
      item["rssi"] = networks_[i].rssi;
      item["open"] = networks_[i].open;
    }
    if (!scanError_.isEmpty()) doc["error"] = scanError_;
    sendJson(200, doc);
  }
};
