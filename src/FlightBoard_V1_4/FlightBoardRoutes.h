#pragma once
#include <Arduino.h>
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

enum class FlightRouteStatus : uint8_t { NotRequested, Loading, Known, Unknown, Error };

struct FlightRouteAirport {
  char code[5] = {}, icao[5] = {}, city[65] = {}, name[97] = {};
  double lat = NAN, lon = NAN;
};

struct FlightRoute {
  char hex[7] = {}, callsign[9] = {};
  FlightRouteStatus status = FlightRouteStatus::NotRequested;
  FlightRouteAirport origin, destination;
  uint32_t updatedMs = 0, ttlMs = 0;
};

// Cache et files appartiennent a la boucle principale. Le worker ne touche
// jamais au cache : seuls des jobs POD et des pointeurs de resultat transitent.
class FlightBoardRoutes {
public:
  static constexpr size_t CACHE_SIZE = 32;

  bool begin() {
    if (requests_ && results_) return true;
    requests_ = xQueueCreate(1, sizeof(Job));
    results_ = xQueueCreate(1, sizeof(Result*));
    if (!requests_ || !results_ ||
        xTaskCreate(run, "flight-routes", 12288, this, 1, &task_) != pdPASS) {
      if (requests_) vQueueDelete(requests_);
      if (results_) vQueueDelete(results_);
      requests_ = results_ = nullptr;
      return false;
    }
    return true;
  }

  bool request(const String& hex, const String& callsign) {
    Job job{};
    if (!makeJob(hex, callsign, job) || !requests_ || pending_ ||
        WiFi.status() != WL_CONNECTED || time(nullptr) < 1704067200) return false;
    const FlightRoute* cached = find(job);
    if (cached && fresh(*cached)) return false;
    if ((requested_ && elapsed(lastRequestMs_) < MIN_REQUEST_MS) ||
        (backoffMs_ && elapsed(backoffAt_) < backoffMs_)) return false;
    if (xQueueSend(requests_, &job, 0) != pdTRUE) return false;
    active_ = job;
    pending_ = requested_ = true;
    lastRequestMs_ = (uint32_t)millis();
    FlightRoute& slot = reserve(job);
    slot.status = FlightRouteStatus::Loading;
    return true;
  }

  void update() {
    Result* result = nullptr;
    if (!results_ || xQueueReceive(results_, &result, 0) != pdTRUE) return;
    if (!pending_) { delete result; return; }
    FlightRoute& slot = reserve(active_);
    if (result && strcmp(result->route.hex, active_.hex) == 0 &&
        strcmp(result->route.callsign, active_.callsign) == 0) {
      slot = result->route;
      if (result->rateLimited) {
        backoffAt_ = (uint32_t)millis();
        backoffMs_ = RATE_LIMIT_MS;
      }
    } else {
      slot.status = FlightRouteStatus::Error;
      slot.ttlMs = ERROR_MS;
    }
    slot.updatedMs = (uint32_t)millis();
    pending_ = false;
    delete result;
  }

  bool pending() const { return pending_; }

  const FlightRoute* get(const String& hex, const String& callsign) const {
    Job job{};
    if (!makeJob(hex, callsign, job)) return nullptr;
    const FlightRoute* route = find(job);
    return route && fresh(*route) ? route : nullptr;
  }

  // The physical panel must apply exactly the same plausibility filter as JSON.
  const FlightRoute* confirmed(const String& hex, const String& callsign,
      double lat, double lon, double track = NAN, double speedKmh = NAN) const {
    const FlightRoute* route = get(hex, callsign);
    return route && route->status == FlightRouteStatus::Known &&
      !plausibility(*route, lat, lon, track, speedKmh) ? route : nullptr;
  }

  void toJson(JsonObject object, const String& hex, const String& callsign,
              double lat = NAN, double lon = NAN, double track = NAN,
              double speedKmh = NAN) const {
    object.remove("origin");
    object.remove("destination");
    object.remove("reason");
    Job job{};
    const bool eligible = makeJob(hex, callsign, job);
    const FlightRoute* route = eligible ? get(hex, callsign) : nullptr;
    const char* reason = route && route->status == FlightRouteStatus::Known
      ? plausibility(*route, lat, lon, track, speedKmh) : nullptr;
    object["status"] = reason ? "unconfirmed" : route ? statusName(route->status) :
      (eligible ? "not_requested" : "unknown");
    object["source"] = "adsb.lol-vrs";
    object["indicative"] = true;
    if (reason) object["reason"] = reason;
    if (route && route->status == FlightRouteStatus::Known && !reason) {
      airportJson(object["origin"].to<JsonObject>(), route->origin);
      airportJson(object["destination"].to<JsonObject>(), route->destination);
    }
  }

  static const char* statusName(FlightRouteStatus status) {
    switch (status) {
      case FlightRouteStatus::Loading: return "loading";
      case FlightRouteStatus::Known: return "known";
      case FlightRouteStatus::Unknown: return "unknown";
      case FlightRouteStatus::Error: return "error";
      default: return "not_requested";
    }
  }

private:
  static constexpr uint32_t KNOWN_MS = 30UL * 60000UL;
  static constexpr uint32_t UNKNOWN_MS = 10UL * 60000UL;
  static constexpr uint32_t ERROR_MS = 60000UL;
  static constexpr uint32_t MIN_REQUEST_MS = 5000UL;
  static constexpr uint32_t RATE_LIMIT_MS = 5UL * 60000UL;
  static constexpr size_t MAX_BODY = 8192;
  static constexpr uint32_t BODY_TIMEOUT_MS = 15000UL;
  static constexpr uint32_t READ_TIMEOUT_MS = 2000UL;
  struct Job { char hex[7] = {}, callsign[9] = {}; };
  struct Result { FlightRoute route; bool rateLimited = false; };
  FlightRoute cache_[CACHE_SIZE];
  QueueHandle_t requests_ = nullptr, results_ = nullptr;
  TaskHandle_t task_ = nullptr;
  Job active_;
  bool pending_ = false, requested_ = false;
  uint32_t lastRequestMs_ = 0, backoffAt_ = 0, backoffMs_ = 0;

  static uint32_t elapsed(uint32_t then) { return (uint32_t)millis() - then; }
  static bool fresh(const FlightRoute& r) {
    return r.status == FlightRouteStatus::Loading ||
      (r.status != FlightRouteStatus::NotRequested && elapsed(r.updatedMs) < r.ttlMs);
  }

  static bool coordinates(double lat, double lon) {
    return isfinite(lat) && isfinite(lon) && lat >= -90 && lat <= 90 &&
      lon >= -180 && lon <= 180;
  }

  static double angle(double lat1, double lon1, double lat2, double lon2) {
    const double radians = PI / 180.0;
    const double a = pow(sin((lat2 - lat1) * radians / 2), 2) +
      cos(lat1 * radians) * cos(lat2 * radians) *
      pow(sin((lon2 - lon1) * radians / 2), 2);
    return 2 * asin(sqrt(constrain(a, 0.0, 1.0)));
  }

  static double bearing(double lat1, double lon1, double lat2, double lon2) {
    const double radians = PI / 180.0;
    const double p1 = lat1 * radians, p2 = lat2 * radians;
    const double delta = (lon2 - lon1) * radians;
    return atan2(sin(delta) * cos(p2),
      cos(p1) * sin(p2) - sin(p1) * cos(p2) * cos(delta));
  }

  // Un trajet de base de donnees reste indicatif. On masque seulement une
  // incoherence manifeste, sans transformer une geometrie plausible en preuve.
  static const char* plausibility(const FlightRoute& route, double lat, double lon,
                                  double track, double speedKmh) {
    if (!coordinates(lat, lon)) return "position_unavailable";
    const auto& a = route.origin;
    const auto& b = route.destination;
    if (!coordinates(a.lat, a.lon) || !coordinates(b.lat, b.lon))
      return "airport_coordinates_unavailable";
    constexpr double EARTH_KM = 6371.0;
    const double routeAngle = angle(a.lat, a.lon, b.lat, b.lon);
    const double routeKm = EARTH_KM * routeAngle;
    if (routeKm < 1.0 || routeKm > 19800.0) return "route_geometry_ambiguous";
    const double fromOrigin = angle(a.lat, a.lon, lat, lon);
    const double fromDestination = angle(b.lat, b.lon, lat, lon);
    const double headingDelta = bearing(a.lat, a.lon, lat, lon) -
      bearing(a.lat, a.lon, b.lat, b.lon);
    const double along = atan2(sin(fromOrigin) * cos(headingDelta), cos(fromOrigin));
    const double cross = asin(constrain(sin(fromOrigin) * sin(headingDelta), -1.0, 1.0));
    const double distanceKm = EARTH_KM * (along < 0 || along > routeAngle
      ? fmin(fromOrigin, fromDestination) : fabs(cross));
    // Large tolerance pour les detours et routes long-courriers.
    const double corridorKm = constrain(routeKm * 0.25, 150.0, 1000.0);
    if (distanceKm > corridorKm) return "position_mismatch";
    // Les virages de depart/approche et les avions lents ne sont pas juges au cap.
    if (isfinite(track) && track >= 0 && track < 360 &&
        isfinite(speedKmh) && speedKmh > 250 &&
        fromOrigin * EARTH_KM > 150 && fromDestination * EARTH_KM > 150) {
      const double towardDestination = bearing(lat, lon, b.lat, b.lon);
      const double delta = track * PI / 180.0 - towardDestination;
      const double difference = fabs(atan2(sin(delta), cos(delta))) * 180.0 / PI;
      if (difference > 140) return "direction_mismatch";
    }
    return nullptr;
  }

  static bool asciiId(const char* text, size_t size, char* output,
                      size_t minSize, size_t maxSize, bool hexadecimal = false) {
    if (!text || size == 0 || size > 32) return false;
    // Espaces ADS-B de remplissage acceptes, controles et NUL jamais acceptes.
    for (size_t i = 0; i < size; ++i)
      if ((uint8_t)text[i] < 32 || (uint8_t)text[i] >= 127) return false;
    size_t first = 0, last = size;
    while (first < last && text[first] == ' ') ++first;
    while (last > first && text[last - 1] == ' ') --last;
    if (last - first < minSize || last - first > maxSize) return false;
    for (size_t i = first; i < last; ++i) {
      char c = text[i];
      if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
      if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= (hexadecimal ? 'F' : 'Z')))) return false;
      output[i - first] = c;
    }
    output[last - first] = 0;
    return true;
  }

  static bool makeJob(const String& hex, const String& callsign, Job& job) {
    return asciiId(hex.c_str(), hex.length(), job.hex, 6, 6, true) &&
      asciiId(callsign.c_str(), callsign.length(), job.callsign, 3, 8) &&
      strcmp(job.hex, job.callsign) != 0;
  }

  const FlightRoute* find(const Job& job) const {
    for (const FlightRoute& entry : cache_)
      if (strcmp(entry.hex, job.hex) == 0 && strcmp(entry.callsign, job.callsign) == 0) return &entry;
    return nullptr;
  }

  FlightRoute& reserve(const Job& job) {
    FlightRoute* chosen = &cache_[0];
    for (FlightRoute& entry : cache_) {
      if (strcmp(entry.hex, job.hex) == 0 && strcmp(entry.callsign, job.callsign) == 0) return entry;
      if (entry.status == FlightRouteStatus::NotRequested) { chosen = &entry; break; }
      if (elapsed(entry.updatedMs) > elapsed(chosen->updatedMs)) chosen = &entry;
    }
    *chosen = FlightRoute{};
    memcpy(chosen->hex, job.hex, sizeof(job.hex));
    memcpy(chosen->callsign, job.callsign, sizeof(job.callsign));
    return *chosen;
  }

  static void airportJson(JsonObject out, const FlightRouteAirport& airport) {
    out["code"] = airport.code;
    out["city"] = airport.city;
    out["name"] = airport.name;
  }

  template<size_t N>
  static bool label(JsonVariantConst value, char (&output)[N], bool required) {
    if (!value.is<JsonString>()) return !required && value.isNull();
    const JsonString text = value.as<JsonString>();
    if (text.size() >= N) return false;
    size_t first = 0, last = text.size();
    for (size_t i = 0; i < last; ++i)
      if ((uint8_t)text.c_str()[i] < 32 || (uint8_t)text.c_str()[i] == 127) return false;
    while (first < last && text.c_str()[first] == ' ') ++first;
    while (last > first && text.c_str()[last - 1] == ' ') --last;
    if (required && first == last) return false;
    memcpy(output, text.c_str() + first, last - first);
    output[last - first] = 0;
    return true;
  }

  static bool airport(JsonVariantConst value, FlightRouteAirport& out) {
    if (!value.is<JsonObjectConst>()) return false;
    out.lat = out.lon = NAN;
    if (value["lat"].is<double>() && value["lon"].is<double>()) {
      const double lat = value["lat"].as<double>();
      const double lon = value["lon"].as<double>();
      if (coordinates(lat, lon)) { out.lat = lat; out.lon = lon; }
    }
    if (!value["icao"].is<JsonString>()) return false;
    const JsonString icao = value["icao"].as<JsonString>();
    if (!asciiId(icao.c_str(), icao.size(), out.icao, 4, 4)) return false;
    for (const char c : out.icao)
      if (c && (c < 'A' || c > 'Z')) return false;
    bool codeValid = false;
    if (value["iata"].is<JsonString>()) {
      const JsonString code = value["iata"].as<JsonString>();
      codeValid = asciiId(code.c_str(), code.size(), out.code, 3, 3);
      for (size_t i = 0; codeValid && out.code[i]; ++i)
        if (out.code[i] < 'A' || out.code[i] > 'Z') codeValid = false;
    }
    if (!codeValid) memcpy(out.code, out.icao, sizeof(out.code));
    return label(value["name"], out.name, true) &&
      label(value["location"], out.city, false);
  }

  static void parse(JsonVariantConst root, const Job& job, Result& result) {
    if (!root.is<JsonObjectConst>()) return;
    JsonVariantConst id = root["callsign"];
    if (!id.is<JsonString>()) return;
    const JsonString callsign = id.as<JsonString>();
    char normalized[9] = {};
    if (!asciiId(callsign.c_str(), callsign.size(), normalized, 3, 8) ||
        strcmp(normalized, job.callsign) != 0) return;
    if (!root["_airports"].is<JsonArrayConst>()) return;
    const JsonArrayConst airports = root["_airports"].as<JsonArrayConst>();
    // Une escale rend l'etape actuellement volee ambigue : ne pas l'inventer.
    if (airports.size() != 2) {
      result.route.status = FlightRouteStatus::Unknown;
      result.route.ttlMs = UNKNOWN_MS;
      return;
    }
    if (!airport(airports[0], result.route.origin) ||
        !airport(airports[1], result.route.destination) ||
        strcmp(result.route.origin.icao, result.route.destination.icao) == 0) return;
    char expected[10];
    snprintf(expected, sizeof(expected), "%s-%s", result.route.origin.icao,
      result.route.destination.icao);
    if (!root["airport_codes"].is<JsonString>()) return;
    const JsonString codes = root["airport_codes"].as<JsonString>();
    if (codes.size() != strlen(expected) || memcmp(codes.c_str(), expected, codes.size())) return;
    result.route.status = FlightRouteStatus::Known;
    result.route.ttlMs = KNOWN_MS;
  }

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

  static void run(void* context) {
    auto* self = static_cast<FlightBoardRoutes*>(context);
    Job job;
    for (;;) {
      if (xQueueReceive(self->requests_, &job, portMAX_DELAY) != pdTRUE) continue;
      Result* result = new (std::nothrow) Result();
      if (result) fetch(job, *result);
      xQueueSend(self->results_, &result, portMAX_DELAY);
    }
  }

  static void fetch(const Job& job, Result& result) {
    memcpy(result.route.hex, job.hex, sizeof(job.hex));
    memcpy(result.route.callsign, job.callsign, sizeof(job.callsign));
    result.route.status = FlightRouteStatus::Error;
    result.route.ttlMs = ERROR_MS;
    if (WiFi.status() != WL_CONNECTED || time(nullptr) < 1704067200) return;
    WiFiClientSecure client;
    client.setCACert(rootCertificate());
    client.setHandshakeTimeout(6);
    HTTPClient http;
    http.useHTTP10(true);
    http.setConnectTimeout(6000);
    http.setTimeout(6000);
    const String callsign(job.callsign);
    if (!http.begin(client, String("https://vrs-standing-data.adsb.lol/routes/") +
        callsign.substring(0, 2) + "/" + callsign + ".json")) return;
    http.addHeader("Accept", "application/json");
    http.addHeader("Accept-Encoding", "identity");
    http.setUserAgent("FlightBoard-ESP32/2.0.8");
    const int status = http.GET();
    if (status != 200) {
      if (status == 404) {
        result.route.status = FlightRouteStatus::Unknown;
        result.route.ttlMs = UNKNOWN_MS;
      }
      result.rateLimited = status == 429;
      http.end();
      return;
    }
    const int size = http.getSize();
    if (size == 0 || size > (int)MAX_BODY) { http.end(); return; }
    auto& stream = http.getStream();
    LimitedReader<decltype(stream)> reader{stream, size > 0 ? (size_t)size : MAX_BODY};
    JsonDocument filter;
    for (const char* key : {"callsign", "airport_codes"}) filter[key] = true;
    JsonObject fields = filter["_airports"].add<JsonObject>();
    for (const char* key : {"iata", "icao", "location", "name", "lat", "lon"}) fields[key] = true;
    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, reader,
      DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(8));
    if (!error && !reader.timedOut) parse(doc.as<JsonVariantConst>(), job, result);
    http.end();
  }

  static const char* rootCertificate() {
    // GTS Root R4, source officielle https://pki.goog/repo/certs/gtsr4.pem.
    // Racine publique, valable jusqu'en 2036 ; aucun secret ni certificat client.
    static const char certificate[] PROGMEM = R"PEM(
-----BEGIN CERTIFICATE-----
MIICCTCCAY6gAwIBAgINAgPlwGjvYxqccpBQUjAKBggqhkjOPQQDAzBHMQswCQYD
VQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEUMBIG
A1UEAxMLR1RTIFJvb3QgUjQwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAwMDAw
WjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2Vz
IExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjQwdjAQBgcqhkjOPQIBBgUrgQQAIgNi
AATzdHOnaItgrkO4NcWBMHtLSZ37wWHO5t5GvWvVYRg1rkDdc/eJkTBa6zzuhXyi
QHY7qca4R9gq55KRanPpsXI5nymfopjTX15YhmUPoYRlBtHci8nHc8iMai/lxKvR
HYqjQjBAMA4GA1UdDwEB/wQEAwIBhjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQW
BBSATNbrdP9JNqPV2Py1PsVq8JQdjDAKBggqhkjOPQQDAwNpADBmAjEA6ED/g94D
9J+uHXqnLrmvT/aDHQ4thQEd0dlq7A/Cr8deVl5c1RxYIigL9zC2L7F8AjEA8GE8
p/SgguMh1YQdc4acLa/KNJvxn7kjNuK8YAOdgLOaVsjh4rsUecrNIdSUtUlD
-----END CERTIFICATE-----
)PEM";
    return certificate;
  }
};
