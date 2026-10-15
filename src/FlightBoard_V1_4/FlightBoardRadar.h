#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <math.h>
#include <new>
#include "AirlineLogos.h"

constexpr int MAX_AIRCRAFT = 40;
constexpr unsigned long UPDATE_MS = 15000UL;
constexpr unsigned long RADAR_RATE_LIMIT_BACKOFF_MS = 120000UL;

// Une limitation concerne la session entiere, meme si la position ou le Wi-Fi
// change. Les succes ne retablissent pas immediatement une cadence trop rapide.
// Cet etat est utilise uniquement par la boucle principale, jamais par la tache.
class FlightBoardRadarCadence {
public:
  // A appeler au demarrage, une fois le namespace NVS ouvert par les reglages.
  void begin(Preferences& storage) {
    storage_ = &storage;
    const uint32_t saved = storage.getUInt("radarMs", 0);
    savedIntervalMs_ = saved >= UPDATE_MS && saved <= 120000UL && saved % 5000UL == 0
      ? saved : 0;
    intervalMs_ = savedIntervalMs_ ? savedIntervalMs_ : UPDATE_MS;
    consecutiveSuccesses_ = 0;
    cooldownMs_ = 0;
    saveFailed_ = false;
  }

  unsigned long intervalMs() const { return intervalMs_; }
  bool isSaved() const { return savedIntervalMs_ == intervalMs_; }
  void resetValidation() { consecutiveSuccesses_ = 0; }

  unsigned long complete(int httpStatus, uint32_t now,
                         unsigned long requestedRetryMs = UPDATE_MS,
                         bool usableResponse = false) {
    if (httpStatus == 429) {
      resetValidation();
      intervalMs_ = intervalMs_ < 120000UL ? intervalMs_ + 5000UL : 120000UL;
      cooldownStartedMs_ = now;
      cooldownMs_ = requestedRetryMs > RADAR_RATE_LIMIT_BACKOFF_MS
        ? requestedRetryMs : RADAR_RATE_LIMIT_BACKOFF_MS;
      return cooldownMs_;
    }
    cooldownMs_ = 0;
    if (httpStatus == 200 && usableResponse) {
      if (consecutiveSuccesses_ < 3) ++consecutiveSuccesses_;
      if (consecutiveSuccesses_ == 3) saveValidatedInterval(now);
    } else {
      resetValidation();
    }
    return requestedRetryMs > intervalMs_ ? requestedRetryMs : intervalMs_;
  }

  unsigned long remainingCooldownMs(uint32_t now) const {
    const uint32_t elapsed = now - cooldownStartedMs_;
    return elapsed < cooldownMs_ ? cooldownMs_ - elapsed : 0UL;
  }

  bool allowsRequest(uint32_t now) {
    if (remainingCooldownMs(now)) return false;
    cooldownMs_ = 0;
    return true;
  }

private:
  Preferences* storage_ = nullptr;
  unsigned long intervalMs_ = UPDATE_MS;
  unsigned long savedIntervalMs_ = 0;
  uint32_t cooldownStartedMs_ = 0;
  unsigned long cooldownMs_ = 0;
  uint8_t consecutiveSuccesses_ = 0;
  bool saveFailed_ = false;
  uint32_t lastSaveAttemptMs_ = 0;

  void saveValidatedInterval(uint32_t now) {
    if (!storage_ || isSaved() ||
        (saveFailed_ && (uint32_t)(now - lastSaveAttemptMs_) < 300000UL)) return;
    lastSaveAttemptMs_ = now;
    if (storage_->putUInt("radarMs", (uint32_t)intervalMs_) == sizeof(uint32_t)) {
      savedIntervalMs_ = intervalMs_;
      saveFailed_ = false;
      Serial.printf("Cadence radar memorisee : %lu s\n", intervalMs_ / 1000UL);
    } else {
      saveFailed_ = true;
      Serial.println("Memorisation cadence radar impossible; nouvel essai dans 5 min.");
    }
  }
};

struct AircraftInfo {
  String hex, callsign, aircraftType;
  double lat = 0, lon = 0, distanceKm = 0, trackDeg = 0, speedKmh = 0;
  bool trackAvailable = false, altitudeAvailable = false, speedAvailable = false;
  int altitudeFt = 0;
  const AirlineAsset* airline = &DEFAULT_AIRLINE;
};

struct RadarJob {
  double lat, lon;
  int radiusKm;
  uint32_t generation;
};

struct RadarResult {
  AircraftInfo aircraft[MAX_AIRCRAFT];
  uint32_t generation = 0;
  int count = 0;
  bool healthy = false;
  unsigned long retryMs = UPDATE_MS;
  const char* error = "none";
  int httpStatus = 0;
  int contentLength = -1;
  size_t bytesRead = 0;
  int firstByte = -1;
};

// Le telechargement HTTPS ne bloque jamais les commandes de la tablette.
// Seules les files echangent les resultats : aucun String partage entre taches.
class FlightBoardRadar {
public:
  bool begin() {
    if (available()) return true;
    requests_ = xQueueCreate(1, sizeof(RadarJob));
    results_ = xQueueCreate(1, sizeof(RadarResult*));
    if (!requests_ || !results_ ||
        xTaskCreate(run, "flight-radar", 16384, this, 1, &task_) != pdPASS) {
      if (requests_) vQueueDelete(requests_);
      if (results_) vQueueDelete(results_);
      requests_ = results_ = nullptr;
      return false;
    }
    return true;
  }

  bool request(double lat, double lon, int radius, uint32_t generation) {
    if (!requests_ || pending_) return false;
    const RadarJob job{lat, lon, radius, generation};
    if (xQueueSend(requests_, &job, 0) != pdTRUE) return false;
    pending_ = true;
    return true;
  }

  bool take(RadarResult*& result) {
    if (!results_ || xQueueReceive(results_, &result, 0) != pdTRUE) return false;
    pending_ = false;
    return true;
  }

  bool pending() const { return pending_; }
  bool available() const { return requests_ && results_; }

private:
  static constexpr size_t MAX_BODY_BYTES = 256UL * 1024UL;
  static constexpr uint32_t BODY_TIMEOUT_MS = 15000UL;
  static constexpr uint32_t READ_TIMEOUT_MS = 2000UL;
  QueueHandle_t requests_ = nullptr, results_ = nullptr;
  TaskHandle_t task_ = nullptr;
  bool pending_ = false;

  // Un flux lent ne doit pas conserver pending_ indefiniment. La lecture reste
  // progressive : aucune copie complete du JSON n'est allouee en memoire.
  template<class StreamType> struct LimitedReader {
    StreamType& stream;
    size_t remaining;
    uint32_t startedMs = (uint32_t)millis();
    uint32_t lastDataMs = startedMs;
    bool timedOut = false, limitReached = false;
    size_t bytesRead = 0;
    int firstByte = -1;
    uint32_t configuredTimeout = 0;

    int read() {
      char c;
      return readBytes(&c, 1) == 1 ? (uint8_t)c : -1;
    }

    size_t readBytes(char* buffer, size_t count) {
      size_t received = 0;
      while (received < count) {
        const uint32_t elapsed = (uint32_t)millis() - startedMs;
        if (elapsed >= BODY_TIMEOUT_MS) { timedOut = true; break; }
        if (!remaining) { limitReached = true; break; }
        const uint32_t idle = (uint32_t)millis() - lastDataMs;
        if (idle >= READ_TIMEOUT_MS) { timedOut = true; break; }
        const uint32_t budget = BODY_TIMEOUT_MS - elapsed;
        const uint32_t idleBudget = READ_TIMEOUT_MS - idle;
        const uint32_t timeout = budget < idleBudget ? budget : idleBudget;
        if (timeout != configuredTimeout) {
          stream.setTimeout(timeout);
          configuredTimeout = timeout;
        }
        const size_t got = stream.readBytes(buffer + received, 1);
        if (got) {
          if (firstByte < 0) firstByte = (uint8_t)buffer[received];
          bytesRead += got;
        }
        if ((uint32_t)millis() - startedMs >= BODY_TIMEOUT_MS) {
          timedOut = true;
          break;
        }
        if (!got) {
          if ((uint32_t)millis() - lastDataMs >= READ_TIMEOUT_MS) {
            timedOut = true;
            break;
          }
          if (!stream.connected()) break;
          // ESP32 NetworkClientSecure peut retourner zero avant le prochain
          // paquet TLS, meme si readBytes() n'a pas attendu son timeout.
          delay(1);
          continue;
        }
        lastDataMs = (uint32_t)millis();
        --remaining;
        ++received;
      }
      return received;
    }
  };

  static void fail(RadarResult& result, const char* code, const char* detail = "") {
    result.error = code;
    Serial.printf("ADSB.lol erreur : %s %s\n", code, detail);
  }

  static unsigned long retryAfterMs(String value) {
    value.trim();
    // Retry-After en secondes est independant de l'horloge NTP. Une date HTTP
    // ou une valeur invalide conserve la pause minimale de deux minutes.
    constexpr unsigned long MAX_SECONDS = 2147483UL;
    unsigned long seconds = 0;
    if (value.isEmpty()) return RADAR_RATE_LIMIT_BACKOFF_MS;
    for (size_t i = 0; i < value.length(); ++i) {
      if (value[i] < '0' || value[i] > '9') return RADAR_RATE_LIMIT_BACKOFF_MS;
      const unsigned long digit = value[i] - '0';
      seconds = seconds > (MAX_SECONDS - digit) / 10UL
        ? MAX_SECONDS : seconds * 10UL + digit;
    }
    const unsigned long wait = seconds * 1000UL;
    return wait > RADAR_RATE_LIMIT_BACKOFF_MS ? wait : RADAR_RATE_LIMIT_BACKOFF_MS;
  }

  static double distance(double lat1, double lon1, double lat2, double lon2) {
    const double r = PI / 180.0;
    double a = pow(sin((lat2 - lat1) * r / 2), 2) +
      cos(lat1 * r) * cos(lat2 * r) * pow(sin((lon2 - lon1) * r / 2), 2);
    a = constrain(a, 0.0, 1.0);
    return 12742.0 * atan2(sqrt(a), sqrt(1.0 - a));
  }

  static void run(void* context) {
    auto* self = static_cast<FlightBoardRadar*>(context);
    RadarJob job;
    for (;;) {
      if (xQueueReceive(self->requests_, &job, portMAX_DELAY) != pdTRUE) continue;
      RadarResult* result = new (std::nothrow) RadarResult();
      if (result) {
        result->generation = job.generation;
        fetch(job, *result);
      }
      xQueueSend(self->results_, &result, portMAX_DELAY);
    }
  }

  static void fetch(const RadarJob& job, RadarResult& result) {
    if (WiFi.status() != WL_CONNECTED) { fail(result, "wifi"); return; }
    // L'API attend des milles nautiques ; le filtre local conserve le rayon km exact.
    const int radiusNm = (int)ceil(job.radiusKm / 1.852);
    const String url = "https://api.adsb.lol/v2/lat/" + String(job.lat, 4) +
      "/lon/" + String(job.lon, 4) + "/dist/" + String(radiusNm);
    WiFiClientSecure client;
    client.setInsecure();
    client.setHandshakeTimeout(6);
    HTTPClient http;
    http.useHTTP10(true);
    http.setConnectTimeout(6000);
    http.setTimeout(8000); // Leave time for the server to prepare its headers.
    if (!http.begin(client, url)) { fail(result, "http_begin"); return; }
    http.addHeader("Accept", "application/json");
    http.addHeader("Accept-Encoding", "identity");
    const char* responseHeaders[] = {"Retry-After"};
    http.collectHeaders(responseHeaders, 1);
    http.setUserAgent("FlightBoard-ESP32/2.0.8 (contact: zcsprod.19@gmail.com)");
    int status = http.GET();
    result.httpStatus = status;
    Serial.printf("ADSB.lol HTTP : %d\n", status);
    if (status != 200) {
      result.retryMs = status == 429 ? retryAfterMs(http.header("Retry-After")) : UPDATE_MS;
      fail(result, status < 0 ? "network" : "http");
      http.end();
      return;
    }
    const int size = http.getSize();
    result.contentLength = size;
    if (size == 0 || size > (int)MAX_BODY_BYTES) {
      fail(result, size == 0 ? "empty_response" : "response_too_large");
      http.end();
      return;
    }
    auto& stream = http.getStream();
    const size_t bodyLimit = size > 0 ? (size_t)size : MAX_BODY_BYTES;
    LimitedReader<decltype(stream)> reader{stream, bodyLimit};
    JsonDocument filter;
    JsonObject fields = filter["ac"].add<JsonObject>();
    for (const char* key : {"hex", "flight", "lat", "lon", "track", "alt_baro", "gs", "t", "seen_pos"}) {
      fields[key] = true;
    }
    JsonDocument doc;
    const DeserializationError error = deserializeJson(
      doc, reader, DeserializationOption::Filter(filter),
      DeserializationOption::NestingLimit(12));
    result.bytesRead = reader.bytesRead;
    result.firstByte = reader.firstByte;
    http.end();
    if (reader.timedOut) { fail(result, "timeout", error.c_str()); return; }
    if (reader.limitReached && bodyLimit == MAX_BODY_BYTES) {
      fail(result, "response_too_large", error.c_str());
      return;
    }
    if (error) {
      const char* code = error == DeserializationError::EmptyInput ? "json_empty" :
        error == DeserializationError::IncompleteInput ? "json_incomplete" :
        error == DeserializationError::NoMemory ? "json_memory" : "json_invalid";
      fail(result, code, error.c_str());
      return;
    }
    if (!doc["ac"].is<JsonArray>()) { fail(result, "schema"); return; }

    for (JsonObject item : doc["ac"].as<JsonArray>()) {
      if (!item["lat"].is<double>() || !item["lon"].is<double>() ||
          !item["seen_pos"].is<double>()) continue;
      const double lat = item["lat"].as<double>();
      const double lon = item["lon"].as<double>();
      const double age = item["seen_pos"].as<double>();
      if (!isfinite(lat) || !isfinite(lon) || !isfinite(age) ||
          lat < -90 || lat > 90 || lon < -180 || lon > 180 || age < 0 || age > 60) continue;
      const double km = distance(job.lat, job.lon, lat, lon);
      if (!isfinite(km) || km > job.radiusKm) continue;
      // Garder les 40 plus proches, meme si l'API renvoie une liste non triee.
      if (result.count == MAX_AIRCRAFT && km >= result.aircraft[MAX_AIRCRAFT - 1].distanceKm) continue;
      AircraftInfo p;
      p.hex = item["hex"] | "";
      if (p.hex.isEmpty()) continue;
      p.callsign = item["flight"] | "";
      p.callsign.trim();
      if (p.callsign.isEmpty()) p.callsign = p.hex;
      p.aircraftType = item["t"] | "";
      p.lat = lat;
      p.lon = lon;
      p.distanceKm = km;
      p.trackAvailable = item["track"].is<double>() && isfinite(item["track"].as<double>());
      if (p.trackAvailable) p.trackDeg = item["track"].as<double>();
      p.altitudeAvailable = item["alt_baro"].is<int>();
      if (p.altitudeAvailable) p.altitudeFt = item["alt_baro"].as<int>();
      p.speedAvailable = item["gs"].is<double>() && isfinite(item["gs"].as<double>()) && item["gs"].as<double>() >= 0;
      if (p.speedAvailable) p.speedKmh = item["gs"].as<double>() * 1.852;
      p.airline = findAirline(p.callsign);
      int index = result.count < MAX_AIRCRAFT ? result.count++ : MAX_AIRCRAFT - 1;
      while (index > 0 && result.aircraft[index - 1].distanceKm > km) {
        result.aircraft[index] = result.aircraft[index - 1];
        --index;
      }
      result.aircraft[index] = p;
    }
    result.healthy = true;
  }
};
