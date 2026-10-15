#pragma once
#include <Arduino.h>
#include "ZcsTls.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <math.h>
#include <new>
#include <time.h>

struct ZcsWeatherData {
  double temperatureC = 0, windKmh = 0;
  int code = 0;
  bool isDay = true;
  uint32_t updatedEpoch = 0;
};

// Open-Meteo Forecast API, current values in Celsius and km/h.
// https://open-meteo.com/en/docs (non-commercial, attribution in the UI).
// Single pending job; only the main loop accesses the cache. The main loop
// excludes radar/routes HTTPS while this worker runs (including app changes).
class ZcsWeather {
public:
  bool available() const { return available_; }
  bool pending() const { return pending_; }
  const ZcsWeatherData& data() const { return data_; }
  uint32_t ageSeconds(uint32_t now) const {
    return initialAgeSeconds_ + (uint32_t)(now - receivedMs_) / 1000UL;
  }
  bool stale(uint32_t now) const { return available_ && ageSeconds(now) >= 1800UL; }

  void resetLocation() {
    ++generation_;
    available_ = false;
    attempted_ = false;
    error_ = "none";
    // An old in-flight request stays pending until collected, preserving TLS
    // exclusion; its generation will never populate the new location's cache.
  }

  bool request(double lat, double lon) {
    const uint32_t now = millis();
    if (pending_ || WiFi.status() != WL_CONNECTED ||
        time(nullptr) < 1704067200 || !isfinite(lat) || !isfinite(lon) ||
        lat < -90 || lat > 90 || lon < -180 || lon > 180 ||
        (attempted_ && (uint32_t)(now - requestedMs_) < REFRESH_MS)) return false;
    attempted_ = true;
    requestedMs_ = now;
    if (!begin()) { error_ = "memory"; return false; }
    const Job job{lat, lon, generation_};
    if (xQueueSend(requests_, &job, 0) != pdTRUE) { error_ = "worker"; return false; }
    if (xTaskCreate(run, "zcs-weather", 12288, this, 1, &task_) != pdPASS) {
      Job unused;
      xQueueReceive(requests_, &unused, 0);
      error_ = "memory";
      return false;
    }
    pending_ = true;
    return true;
  }

  void update() {
    Result* result = nullptr;
    if (!results_ || xQueueReceive(results_, &result, 0) != pdTRUE) return;
    pending_ = false;
    if (!result) { error_ = "memory"; return; }
    if (result->generation == generation_) {
      error_ = result->error;
      if (result->ok) {
        data_ = result->data;
        available_ = true;
        receivedMs_ = (uint32_t)millis();
        const time_t now = time(nullptr);
        initialAgeSeconds_ = now > data_.updatedEpoch ? (uint32_t)(now - data_.updatedEpoch) : 0;
      }
    }
    delete result;
  }

  void toJson(JsonObject object) const {
    const uint32_t now = millis();
    const bool clockReady = time(nullptr) >= 1704067200;
    const char* status = available_ ? (stale(now) ? "stale" : "ready") :
      (!clockReady && now > 60000UL) ? "error" :
      pending_ || (!attempted_ && WiFi.status() == WL_CONNECTED) ? "loading" : "error";
    object["status"] = status;
    object["available"] = available_;
    object["stale"] = stale(now);
    object["source"] = "Open-Meteo";
    object["error"] = WiFi.status() != WL_CONNECTED ? "wifi" :
      !clockReady ? "time" : error_;
    object["ageSeconds"] = available_ ? (int32_t)ageSeconds(now) : -1;
    if (available_) {
      object["temperatureC"] = data_.temperatureC;
      object["windKmh"] = data_.windKmh;
      object["code"] = data_.code;
      object["isDay"] = data_.isDay;
      object["updatedEpoch"] = data_.updatedEpoch;
    }
  }

private:
  static constexpr uint32_t REFRESH_MS = 15UL * 60000UL;
  static constexpr uint32_t BODY_TIMEOUT_MS = 10000UL, READ_TIMEOUT_MS = 2000UL;
  static constexpr size_t MAX_BODY_BYTES = 2048;
  struct Job { double lat, lon; uint32_t generation; };
  struct Result {
    ZcsWeatherData data;
    uint32_t generation = 0;
    bool ok = false;
    const char* error = "none";
  };
  QueueHandle_t requests_ = nullptr, results_ = nullptr;
  TaskHandle_t task_ = nullptr;
  ZcsWeatherData data_;
  uint32_t generation_ = 0, requestedMs_ = 0, receivedMs_ = 0, initialAgeSeconds_ = 0;
  bool pending_ = false, attempted_ = false, available_ = false;
  const char* error_ = "none";

  bool begin() {
    if (requests_ && results_) return true;
    requests_ = xQueueCreate(1, sizeof(Job));
    results_ = xQueueCreate(1, sizeof(Result*));
    if (!requests_ || !results_) {
      if (requests_) vQueueDelete(requests_);
      if (results_) vQueueDelete(results_);
      requests_ = results_ = nullptr;
      return false;
    }
    return true;
  }

  static bool weatherCode(int code) {
    switch (code) {
      case 0: case 1: case 2: case 3: case 45: case 48:
      case 51: case 53: case 55: case 56: case 57:
      case 61: case 63: case 65: case 66: case 67:
      case 71: case 73: case 75: case 77:
      case 80: case 81: case 82: case 85: case 86:
      case 95: case 96: case 99: return true;
      default: return false;
    }
  }

  static bool decode(JsonObjectConst body, ZcsWeatherData& output, uint32_t nowEpoch) {
    const JsonObjectConst current = body["current"].as<JsonObjectConst>();
    const JsonObjectConst units = body["current_units"].as<JsonObjectConst>();
    if (!current || !units || strcmp(units["temperature_2m"] | "", "°C") != 0 ||
        strcmp(units["wind_speed_10m"] | "", "km/h") != 0 ||
        strcmp(units["time"] | "", "unixtime") != 0 ||
        !current["temperature_2m"].is<double>() || !current["wind_speed_10m"].is<double>() ||
        !current["weather_code"].is<int>() || !current["is_day"].is<int>() ||
        !current["time"].is<uint32_t>()) return false;
    ZcsWeatherData next;
    next.temperatureC = current["temperature_2m"].as<double>();
    next.windKmh = current["wind_speed_10m"].as<double>();
    next.code = current["weather_code"].as<int>();
    const int day = current["is_day"].as<int>();
    next.isDay = day == 1;
    next.updatedEpoch = current["time"].as<uint32_t>();
    if (!isfinite(next.temperatureC) || !isfinite(next.windKmh) ||
        next.temperatureC < -100 || next.temperatureC > 80 ||
        next.windKmh < 0 || next.windKmh > 500 || !weatherCode(next.code) ||
        (day != 0 && day != 1) || next.updatedEpoch < 1704067200UL ||
        next.updatedEpoch > nowEpoch + 900UL ||
        (nowEpoch > next.updatedEpoch && nowEpoch - next.updatedEpoch > 7200UL)) return false;
    output = next;
    return true;
  }

  template<class StreamType> struct LimitedReader {
    StreamType& stream;
    size_t remaining;
    uint32_t startedMs = (uint32_t)millis(), lastDataMs = startedMs;
    bool timedOut = false, limitReached = false;
    int read() { char c; return readBytes(&c, 1) == 1 ? (uint8_t)c : -1; }
    size_t readBytes(char* buffer, size_t count) {
      size_t received = 0;
      while (received < count) {
        const uint32_t now = millis();
        const uint32_t elapsed = now - startedMs, idle = now - lastDataMs;
        if (elapsed >= BODY_TIMEOUT_MS || idle >= READ_TIMEOUT_MS) { timedOut = true; break; }
        if (!remaining) { limitReached = true; break; }
        const uint32_t budget = BODY_TIMEOUT_MS - elapsed, idleBudget = READ_TIMEOUT_MS - idle;
        stream.setTimeout(budget < idleBudget ? budget : idleBudget);
        const size_t got = stream.readBytes(buffer + received, 1);
        if ((uint32_t)millis() - startedMs >= BODY_TIMEOUT_MS) { timedOut = true; break; }
        if (!got) {
          if (!stream.connected()) break;
          delay(1);
          continue;
        }
        lastDataMs = millis();
        --remaining;
        ++received;
      }
      return received;
    }
  };

  static void run(void* context) {
    auto* self = static_cast<ZcsWeather*>(context);
    Job job;
    if (xQueueReceive(self->requests_, &job, 0) == pdTRUE) {
      Result* result = new (std::nothrow) Result();
      if (result) {
        result->generation = job.generation;
        fetch(job, *result);
      }
      xQueueSend(self->results_, &result, portMAX_DELAY);
    }
    // Return the 12 KiB stack to the heap between weather refreshes, also when
    // the user returns to aviation. TLS objects have been destroyed by fetch.
    vTaskDelete(nullptr);
  }

  static void fetch(const Job& job, Result& result) {
    if (WiFi.status() != WL_CONNECTED) { result.error = "wifi"; return; }
    if (time(nullptr) < 1704067200) { result.error = "time"; return; }
    const String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(job.lat, 2) +
      "&longitude=" + String(job.lon, 2) +
      "&current=temperature_2m,weather_code,wind_speed_10m,is_day&temperature_unit=celsius&wind_speed_unit=kmh&timeformat=unixtime";
    // ISRG Root X1 from https://letsencrypt.org/certs/isrgrootx1.pem.
    // Verified 2026-10-15: api.open-meteo.com -> YR1 -> Root YR -> ISRG Root X1.
    // SHA-256 96BCEC06264976F37460779ACF28C5A7CFE8A3C0AAE11A8FFCEE05C0BDDF08C6.
    WiFiClientSecure client;
    client.setCACert(ZCS_ISRG_ROOT_CA);
    client.setHandshakeTimeout(6);
    HTTPClient http;
    http.useHTTP10(true);
    http.setConnectTimeout(6000);
    http.setTimeout(8000);
    if (!http.begin(client, url)) { result.error = "network"; return; }
    http.addHeader("Accept", "application/json");
    http.addHeader("Accept-Encoding", "identity");
    http.setUserAgent("ZCS-Control-ESP32/3.0.0");
    const int status = http.GET();
    if (status != 200) { result.error = status < 0 ? "network" : "http"; http.end(); return; }
    const int size = http.getSize();
    if (size == 0 || size > (int)MAX_BODY_BYTES) { result.error = "size"; http.end(); return; }
    auto& stream = http.getStream();
    LimitedReader<decltype(stream)> reader{stream, size > 0 ? (size_t)size : MAX_BODY_BYTES};
    JsonDocument doc;
    const auto error = deserializeJson(doc, reader, DeserializationOption::NestingLimit(6));
    http.end();
    if (reader.timedOut) result.error = "timeout";
    else if (reader.limitReached || error) result.error = "json";
    else if (!decode(doc.as<JsonObjectConst>(), result.data, (uint32_t)time(nullptr))) result.error = "schema";
    else result.ok = true;
  }
};
