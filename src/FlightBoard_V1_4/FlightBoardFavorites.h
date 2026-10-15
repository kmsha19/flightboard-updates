#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <math.h>
#include <esp_system.h>

struct FavoritePlace {
  String id;
  String name;
  double lat = 0;
  double lon = 0;
  int radiusKm = 30;
};

class FlightBoardFavorites {
public:
  static constexpr int MAX_PLACES = 8;
  explicit FlightBoardFavorites(Preferences& storage) : storage_(storage) {}

  void begin() {
    JsonDocument doc;
    const String raw = storage_.getString("favorites", "");
    if (raw.length() && raw.length() <= 4096 && !deserializeJson(doc, raw) &&
        doc.is<JsonArray>() && doc.size() <= MAX_PLACES) {
      bool valid = true;
      for (JsonObject entry : doc.as<JsonArray>()) {
        FavoritePlace p;
        if (!entry["id"].is<String>() || !parse(entry, p)) { valid = false; break; }
        p.id = entry["id"].as<String>();
        if (p.id.isEmpty() || p.id.length() > 24 || find(p.id) >= 0) {
          valid = false;
          break;
        }
        places_[count_++] = p;
      }
      if (valid) return; // Une liste volontairement vide reste vide.
    }
    count_ = 3;
    places_[0] = {"niort", "Niort", 46.324, -0.459, 30};
    places_[1] = {"cdg", "Paris-CDG", 49.0097, 2.5479, 30};
    places_[2] = {"orly", "Paris-Orly", 48.7262, 2.3652, 30};
    persist(places_, count_);
  }

  const FavoritePlace* get(const String& id) const {
    const int index = find(id);
    return index < 0 ? nullptr : &places_[index];
  }

  void toJson(JsonDocument& doc) const {
    doc["max"] = MAX_PLACES;
    JsonArray list = doc["favorites"].to<JsonArray>();
    writeArray(list, places_, count_);
  }

  // 200: ajoute/remplace ; 400: invalide ; 409: plein ; 503: sauvegarde impossible.
  int add(JsonObjectConst body) {
    FavoritePlace p;
    if (!parse(body, p)) return 400;
    int index = -1;
    for (int i = 0; i < count_; ++i) {
      if (places_[i].name == p.name) index = i;
    }
    if (index < 0 && count_ >= MAX_PLACES) return 409;
    if (index >= 0) {
      p.id = places_[index].id;
    } else {
      do {
        char id[16];
        snprintf(id, sizeof(id), "fav-%08lx", (unsigned long)esp_random());
        p.id = id;
      } while (find(p.id) >= 0);
    }
    FavoritePlace updated[MAX_PLACES];
    for (int i = 0; i < count_; ++i) updated[i] = places_[i];
    const int newCount = index < 0 ? count_ + 1 : count_;
    updated[index < 0 ? count_ : index] = p;
    if (!persist(updated, newCount)) return 503;
    for (int i = 0; i < newCount; ++i) places_[i] = updated[i];
    count_ = newCount;
    return 200;
  }

  int remove(const String& id) {
    const int index = find(id);
    if (index < 0) return 404;
    FavoritePlace updated[MAX_PLACES];
    int n = 0;
    for (int i = 0; i < count_; ++i) if (i != index) updated[n++] = places_[i];
    if (!persist(updated, n)) return 503;
    for (int i = 0; i < n; ++i) places_[i] = updated[i];
    places_[n] = FavoritePlace();
    count_ = n;
    return 200;
  }

private:
  Preferences& storage_;
  FavoritePlace places_[MAX_PLACES];
  int count_ = 0;

  int find(const String& id) const {
    for (int i = 0; i < count_; ++i) if (places_[i].id == id) return i;
    return -1;
  }

  static bool parse(JsonObjectConst entry, FavoritePlace& p) {
    if (!entry["name"].is<String>() || !entry["lat"].is<double>() ||
        !entry["lon"].is<double>() || !entry["radiusKm"].is<int>()) return false;
    JsonString name = entry["name"].as<JsonString>();
    for (size_t i = 0; i < name.size(); ++i) if (!name.c_str()[i]) return false;
    p.name = entry["name"].as<String>();
    p.name.trim();
    p.lat = entry["lat"].as<double>();
    p.lon = entry["lon"].as<double>();
    p.radiusKm = entry["radiusKm"].as<int>();
    return p.name.length() > 0 && p.name.length() <= 32 &&
           isfinite(p.lat) && p.lat >= -90 && p.lat <= 90 &&
           isfinite(p.lon) && p.lon >= -180 && p.lon <= 180 &&
           p.radiusKm >= 5 && p.radiusKm <= 50;
  }

  static void writeArray(JsonArray list, const FavoritePlace* places, int count) {
    for (int i = 0; i < count; ++i) {
      JsonObject p = list.add<JsonObject>();
      p["id"] = places[i].id;
      p["name"] = places[i].name;
      p["lat"] = places[i].lat;
      p["lon"] = places[i].lon;
      p["radiusKm"] = places[i].radiusKm;
    }
  }

  bool persist(const FavoritePlace* places, int count) {
    JsonDocument doc;
    writeArray(doc.to<JsonArray>(), places, count);
    String json;
    serializeJson(doc, json);
    return storage_.putString("favorites", json) == json.length();
  }
};
