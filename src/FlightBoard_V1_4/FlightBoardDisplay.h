#pragma once
#include <cstring>

// Regles d'affichage communes au firmware et aux tests sur ordinateur.
inline const char* flightBoardTimezone(const char* name) {
  if (!name) return nullptr;
  if (!strcmp(name, "paris")) return "CET-1CEST,M3.5.0,M10.5.0/3";
  if (!strcmp(name, "london")) return "GMT0BST,M3.5.0/1,M10.5.0";
  if (!strcmp(name, "utc")) return "UTC0";
  if (!strcmp(name, "newyork")) return "EST5EDT,M3.2.0,M11.1.0";
  if (!strcmp(name, "dubai")) return "GST-4";
  return nullptr;
}

inline bool flightBoardNightMinute(int minute, int start, int end) {
  if (minute < 0 || minute >= 1440 || start < 0 || start >= 1440 ||
      end < 0 || end >= 1440 || start == end) return false;
  return start < end ? minute >= start && minute < end
                     : minute >= start || minute < end;
}

struct FlightBoardDisplayState {
  int brightness;
  bool nightActive;
  bool sleeping;
};

inline FlightBoardDisplayState flightBoardDisplayState(
    int dayBrightness, bool nightEnabled, int nightStart, int nightEnd,
    int nightBrightness, bool sleepAtNight, bool timeSynced, int minute) {
  const bool night = nightEnabled && timeSynced &&
                    flightBoardNightMinute(minute, nightStart, nightEnd);
  const bool sleeping = night && sleepAtNight;
  const int brightness = sleeping ? 0 : night && nightBrightness < dayBrightness
                                             ? nightBrightness : dayBrightness;
  return {brightness, night, sleeping};
}
