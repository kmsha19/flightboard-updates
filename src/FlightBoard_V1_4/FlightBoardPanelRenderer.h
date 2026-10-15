#pragma once

// Portable 128 x 64 renderer. No framebuffer, allocation, driver or Arduino API.
// Input strings and logo context only need to remain valid during renderPanel().
// The driver applies effective day/night brightness once, after RGB565 rendering.
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ZcsWidgets.h"

namespace flightboard_panel {

constexpr int kWidth = 128, kHeight = 64;
constexpr uint32_t kFrameIntervalMs = 80;  // 12.5 fps for the idle animation.
enum class Mode : uint8_t { Auto, Manual, Clock };
enum class App : uint8_t { Aviation, Clock, Animation, Weather, Message, Timer, Score, Pixel };
enum class Visual : uint8_t { Lava, Stars, Rain, Plain, Aurora, Ocean, Warp };
enum class RouteStatus : uint8_t { NotRequested, Loading, Known, Unknown, Error, Unconfirmed };
enum class Scene : uint8_t { Sleeping, Connecting, Idle, Unavailable, Aircraft, Animation, Weather, Widget };

struct PanelCanvas {
  void* context = nullptr;
  void (*pixel)(void* context, int16_t x, int16_t y, uint16_t rgb565) = nullptr;
};

struct PanelLogo {
  const void* context = nullptr;
  // Read one row-major 24 x 16 pixel. The adapter may use pgm_read_word().
  uint16_t (*read)(const void* context, uint16_t index) = nullptr;
};

struct PanelAircraft {
  const char* callsign = nullptr;
  const char* aircraftType = nullptr;
  PanelLogo logo;
  RouteStatus routeStatus = RouteStatus::NotRequested;
  // Populate only after the same route plausibility check used by /api/state.
  const char* origin = nullptr;
  const char* destination = nullptr;
  double distanceKm = NAN;
  double speedKmh = NAN;
  int32_t altitudeFt = 0;
  bool speedAvailable = false;
  bool altitudeAvailable = false;
};

struct PanelWeather {
  bool available = false, stale = false;
  double temperatureC = NAN, windKmh = NAN;
  int code = -1;
  bool isDay = true;
};

struct PanelFrame {
  App app = App::Aviation;
  Visual visual = Visual::Lava;
  Mode mode = Mode::Auto;
  bool stateAvailable = true;
  bool wifiConnected = false;
  bool radarHealthy = false;
  int32_t ageSeconds = -1;
  bool sleeping = false;
  bool timeSynced = false;
  const char* clock = nullptr;  // Local HH:MM, supplied by the existing scheduler.
  bool animate = true;
  const PanelAircraft* aircraft = nullptr;  // Already selected by auto/manual mode.
  const PanelWeather* weather = nullptr;
  const ZcsWidgetFrame* widget = nullptr;
};

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return uint16_t((uint16_t(r & 0xf8) << 8) | (uint16_t(g & 0xfc) << 3) | (b >> 3));
}

inline Scene panelScene(const PanelFrame& frame) {
  if (frame.sleeping) return Scene::Sleeping;
  if (!frame.stateAvailable) return Scene::Connecting;
  if (frame.app == App::Clock) return Scene::Idle;
  if (frame.app == App::Animation) return Scene::Animation;
  if (frame.app == App::Weather) return Scene::Weather;
  if (frame.app == App::Message || frame.app == App::Timer || frame.app == App::Score || frame.app == App::Pixel) return Scene::Widget;
  const bool usable = frame.wifiConnected && frame.radarHealthy &&
      frame.ageSeconds >= 0 && frame.ageSeconds <= 90 && frame.aircraft;
  if (frame.mode == Mode::Clock || (frame.mode == Mode::Auto && !usable)) return Scene::Idle;
  return usable ? Scene::Aircraft : Scene::Unavailable;
}

inline bool weatherCodeValid(int code) {
  switch (code) {
    case 0: case 1: case 2: case 3: case 45: case 48: case 51: case 53:
    case 55: case 56: case 57: case 61: case 63: case 65: case 66: case 67:
    case 71: case 73: case 75: case 77: case 80: case 81: case 82: case 85:
    case 86: case 95: case 96: case 99: return true;
    default: return false;
  }
}

inline bool panelAnimated(const PanelFrame& frame) {
  const auto scene = panelScene(frame);
  const auto* w = frame.weather;
  if (scene == Scene::Widget) {
    if (!frame.animate || !frame.widget) return false;
    const auto& widget = *frame.widget;
    return frame.app == App::Message ? zcsMessageWidth(widget.message.text) > 120 :
      frame.app == App::Timer ? widget.timer.running && widget.timer.remainingSeconds > 0 :
      frame.app == App::Pixel && zcsPixelDimensionsValid(widget.pixel) && widget.pixel.frameCount > 1;
  }
  return frame.animate && ((scene == Scene::Weather && w && w->available &&
    isfinite(w->temperatureC) && isfinite(w->windKmh) &&
    weatherCodeValid(w->code)) ||
    (frame.visual != Visual::Plain && (scene == Scene::Idle || scene == Scene::Animation)));
}

namespace detail {
constexpr uint16_t kMint = rgb565(169, 216, 198);
constexpr uint16_t kWhite = rgb565(237, 240, 233);
constexpr uint16_t kMuted = rgb565(162, 176, 176);
constexpr uint16_t kAmber = rgb565(230, 191, 129);
constexpr uint16_t kDivider = rgb565(32, 49, 44);

// The same original bitmap rows as FlightBoardWeb.h; no font dependency.
// A-Z, 0-9, space, dash, colon, dot, comma, slash, greater-than, ?, !, apostrophe.
constexpr uint8_t kFont5[46][7] = {
  {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{15,16,16,16,16,16,15},
  {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
  {15,16,16,23,17,17,15},{17,17,17,31,17,17,17},{31,4,4,4,4,4,31},
  {7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
  {17,27,21,21,17,17,17},{17,25,25,21,19,19,17},{14,17,17,17,17,17,14},
  {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
  {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
  {17,17,17,17,17,10,4},{17,17,17,21,21,27,17},{17,17,10,4,10,17,17},
  {17,17,10,4,4,4,4},{31,1,2,4,8,16,31},
  {14,17,17,17,17,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
  {30,1,1,6,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
  {14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
  {14,17,17,15,1,1,14},{0,0,0,0,0,0,0},{0,0,0,7,0,0,0},
  {0,1,0,0,1,0,0},{0,0,0,0,0,0,1},{0,0,0,0,0,2,1},
  {1,1,2,2,2,4,4},{0,4,2,1,2,4,0},{14,17,1,2,4,0,4},
  {1,1,1,1,1,0,1},{1,1,0,0,0,0,0}
};
constexpr uint8_t kFont3[46][5] = {
  {2,5,7,5,5},{6,5,6,5,6},{3,4,4,4,3},{6,5,5,5,6},
  {7,4,7,4,7},{7,4,7,4,4},{3,4,5,5,3},{5,5,7,5,5},
  {7,2,2,2,7},{1,1,1,5,2},{5,5,6,5,5},{4,4,4,4,7},
  {5,7,7,5,5},{5,7,5,5,5},{2,5,5,5,2},{6,5,6,4,4},
  {2,5,5,7,3},{6,5,6,5,5},{3,4,2,1,6},{7,2,2,2,2},
  {5,5,5,5,7},{5,5,5,5,2},{5,5,5,7,7},{5,5,2,5,5},
  {5,5,2,2,2},{7,1,2,4,7},{2,5,5,5,2},{2,6,2,2,7},
  {6,1,2,4,7},{6,1,2,1,6},{5,5,7,1,1},{7,4,6,1,6},
  {3,4,6,5,2},{7,1,2,2,2},{2,5,2,5,2},{2,5,3,1,6},
  {0,0,0,0,0},{0,0,7,0,0},{0,1,0,1,0},{0,0,0,0,1},
  {0,0,0,1,1},{1,1,2,4,4},{4,2,1,2,4},{6,1,0,2,0},
  {1,1,1,0,1},{1,1,0,0,0}
};

inline uint8_t glyphIndex(char c) {
  if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
  if (c >= 'A' && c <= 'Z') return uint8_t(c - 'A');
  if (c >= '0' && c <= '9') return uint8_t(c - '0' + 26);
  switch (c) {
    case ' ': case '\t': case '\r': case '\n': return 36;
    case '-': return 37; case ':': return 38; case '.': return 39;
    case ',': return 40; case '/': return 41; case '>': return 42;
    case '!': return 44; case '\'': return 45;
    default: return 43;
  }
}

inline uint8_t glyphWidth(uint8_t index, bool small) {
  if (index == 36) return small ? 1 : 3;
  if (index == 38 || index == 39 || index == 44 || index == 45) return 1;
  if (index == 40) return small ? 1 : 2;
  if (index == 37 || index == 41 || index == 42) return 3;
  return small ? 3 : 5;
}

inline int textWidth(const char* text, bool small, uint8_t scale) {
  int width = 0;
  // Aircraft identifiers/codes are ASCII and bounded by the caller. A cap also
  // bounds work if an accidentally overlong string reaches this helper.
  for (size_t i = 0; text && text[i] && i < 64; ++i)
    width += glyphWidth(glyphIndex(text[i]), small) + 1;
  return width ? (width - 1) * scale : 0;
}

struct TextLayout {
  const char* text;
  bool small;
  uint8_t scale;
  int width;
};

inline TextLayout layout(const char* text, uint8_t size, int maxWidth) {
  TextLayout out{text ? text : "", size < 7, 1, 0};
  out.scale = uint8_t(size / (out.small ? 5 : 7));
  if (!out.scale) out.scale = 1;
  while (out.scale > 1 && textWidth(out.text, out.small, out.scale) > maxWidth) --out.scale;
  if (textWidth(out.text, out.small, out.scale) > maxWidth) {
    out.small = true;
    out.scale = 1;
  }
  if (textWidth(out.text, out.small, out.scale) > maxWidth) out.text = "...";
  out.width = textWidth(out.text, out.small, out.scale);
  return out;
}

inline void pixel(const PanelCanvas& canvas, int x, int y, uint16_t color) {
  if (canvas.pixel && x >= 0 && x < kWidth && y >= 0 && y < kHeight)
    canvas.pixel(canvas.context, int16_t(x), int16_t(y), color);
}

inline void rectangle(const PanelCanvas& canvas, int x, int y, int w, int h, uint16_t color) {
  for (int py = y; py < y + h; ++py)
    for (int px = x; px < x + w; ++px) pixel(canvas, px, py, color);
}

inline void paintText(const PanelCanvas& canvas, const TextLayout& text, int x, int y, uint16_t color) {
  for (size_t i = 0; text.text[i] && i < 64; ++i) {
    const uint8_t index = glyphIndex(text.text[i]);
    const uint8_t width = glyphWidth(index, text.small);
    for (int row = 0; row < (text.small ? 5 : 7); ++row) {
      const uint8_t bits = text.small ? kFont3[index][row] : kFont5[index][row];
      for (int col = 0; col < width; ++col)
        if (bits & (1 << (width - col - 1)))
          rectangle(canvas, x + col * text.scale, y + row * text.scale, text.scale, text.scale, color);
    }
    x += (width + 1) * text.scale;
  }
}

inline void text(const PanelCanvas& canvas, const char* value, int x, int y, uint8_t size, uint16_t color) {
  paintText(canvas, layout(value, size, kWidth - x), x, y, color);
}

inline void centered(const PanelCanvas& canvas, const char* value, int center, int y,
                     int maxWidth, uint8_t size, uint16_t color) {
  const TextLayout l = layout(value, size, maxWidth);
  paintText(canvas, l, center - l.width / 2, y, color);
}

inline bool validClock(const char* clock) {
  return clock && strlen(clock) == 5 && clock[0] >= '0' && clock[0] <= '2' &&
      clock[1] >= '0' && clock[1] <= '9' && !(clock[0] == '2' && clock[1] > '3') &&
      clock[2] == ':' && clock[3] >= '0' && clock[3] <= '5' && clock[4] >= '0' && clock[4] <= '9';
}

inline bool validAirport(const char* code) {
  if (!code) return false;
  size_t n = 0;
  for (; n < 5 && code[n]; ++n)
    if (!((code[n] >= 'A' && code[n] <= 'Z') || (code[n] >= '0' && code[n] <= '9'))) return false;
  return n == 3 || n == 4;
}

inline float smooth(float n) {
  n = n < 0 ? 0 : n > 1 ? 1 : n;
  return n * n * (3 - 2 * n);
}

// Integer sine shared verbatim with the tablet: reproducible on every frame,
// including millis() wrap, without a texture, floating point or heap allocation.
inline int wave(int phase) {
  static constexpr int8_t curve[64] = {
    0,3,6,9,12,15,17,20,22,24,26,27,29,30,30,31,
    31,31,30,30,29,27,26,24,22,20,17,15,12,9,6,3,
    0,-3,-6,-9,-12,-15,-17,-20,-22,-24,-26,-27,-29,-30,-30,-31,
    -31,-31,-30,-30,-29,-27,-26,-24,-22,-20,-17,-15,-12,-9,-6,-3
  };
  return curve[(uint32_t(phase) & 255) >> 2];
}

inline void atmosphere(const PanelCanvas& canvas, Visual visual, uint32_t tick, bool clock) {
  const int t = int(tick % 65536);
  for (int x = 0; x < kWidth; ++x) {
    const int ridge = 20 + wave(x * 3 + t) / 3 + wave(x * 7 - t) / 8;
    const int ribbon = 34 + wave(x * 2 - t) / 3;
    const int swell = 22 + wave(x * 2 + t) / 7;
    const int middle = 35 + wave(x * 3 - t * 2) / 5;
    const int shore = 51 + wave(x * 2 + t * 3) / 5;
    for (int y = 0; y < kHeight; ++y) {
      int r = 0, g = 2, b = 9;
      if (visual == Visual::Aurora) {
        int a = 27 - abs(y - ridge) * 2, c = 20 - abs(y - ribbon) * 2;
        a = a > 0 ? a * (48 + wave(x * 11 + t)) / 64 : 0;
        c = c > 0 ? c : 0;
        r = c * 2; g = a * 5 + c; b += a * 3 + c * 5;
      } else if (y >= swell) {
        g = 18; b = 38;
        if (y >= middle) { g = 31; b = 64; }
        if (y >= shore) { g = 45; b = 73; }
        if (y == swell || y == middle || y == shore) { g += 35; b += 40; }
        if (abs(x - 96) < (y - 16) / 2 + 2 && (y + t / 3) % 9 < 2) { r = 25; g += 35; b += 45; }
      } else if ((x - 96) * (x - 96) + (y - 11) * (y - 11) < 30) {
        r = 98; g = 140; b = 162;
      }
      if (y < 17 && (x * 17 + y * 43) % 311 == 0) { r += 25; g += 30; b += 35; }
      if (clock && y >= 14 && y < 50) {
        const int shade = y >= 18 && y < 46 ? 1 : 2;
        r = r * shade / 4; g = g * shade / 4; b = b * shade / 4;
      }
      pixel(canvas, x, y, rgb565(uint8_t(r), uint8_t(g), uint8_t(b)));
    }
  }
}

inline void ambient(const PanelCanvas& canvas, const PanelFrame& frame, uint32_t nowMs) {
  const uint32_t tick = frame.animate ? nowMs / kFrameIntervalMs : 0;
  if (frame.visual == Visual::Aurora || frame.visual == Visual::Ocean) {
    atmosphere(canvas, frame.visual, tick, frame.app != App::Animation); return;
  }
  if (frame.visual != Visual::Lava) {
    rectangle(canvas, 0, 0, kWidth, kHeight, 0);
    if (frame.visual == Visual::Stars) {
      for (int i = 0; i < 54; ++i) {
        const int x = (i * 37 + 11 + int(tick / (2 + i % 5) % 128)) % 128;
        const int y = (i * i * 13 + 7) % 64;
        const uint16_t color = (tick / 8 + i) % 5 == 0 ? 0xffff : i % 3 == 0 ? 0x8ddf : 0x426b;
        pixel(canvas, x, y, color);
        if (i % 13 == 0) pixel(canvas, x, y + 1, 0x426b);
      }
    } else if (frame.visual == Visual::Rain) {
      for (int i = 0; i < 19; ++i) {
        const int x = i * 7;
        const int head = int((tick * (1 + i % 3) + i * 11) % 78) - 7;
        for (int j = 0; j < 8; ++j)
          rectangle(canvas, x, head - j * 2, 2, 1, j == 0 ? 0xb7f7 : uint16_t((56 - j * 6) << 5));
      }
    } else if (frame.visual == Visual::Warp) {
      for (int i = 0; i < 68; ++i) {
        const int dx = (i * 47 + 9) % 129 - 64, dy = (i * 29 + 3) % 65 - 32;
        const int z = 64 - int((tick * (1 + i % 2) + i * 19) % 64);
        const int x = 64 + dx * 16 / z, y = 32 + dy * 16 / z;
        const int tailX = 64 + dx * 16 / (z + 5), tailY = 32 + dy * 16 / (z + 5);
        const uint16_t color = z < 12 ? 0xffff : i % 4 == 0 ? 0x999f : 0x6cba;
        for (int j = 0; j <= 5; ++j)
          pixel(canvas, x + (tailX - x) * j / 5, y + (tailY - y) * j / 5, color);
      }
    }
    return;
  }
  // Quantized contour bands around drifting, gently warped fluid islands.
  // Fixed-point coordinates keep the ESP32 and browser bit-for-bit identical.
  const int t = int(tick % 65536);
  const auto flow = [](int phase) {
    const int f = phase & 3;
    return wave(phase) * (4 - f) + wave(phase + 4) * f;
  };
  const int centers[4][2] = {
    {80 + flow(t), 128 + flow(t / 2 + 71)},
    {432 + flow(t / 2 + 34), 384 + flow(t + 170)},
    {712 + flow(t + 128), 96 + flow(t / 2 + 210)},
    {1024 + flow(t / 2 + 83), 432 + flow(t + 29)}
  };
  static constexpr uint16_t colors[] = {0, rgb565(8,24,160), rgb565(0,52,234),
    rgb565(0,108,255), rgb565(0,172,250), rgb565(0,225,222), rgb565(32,250,198), 0};
  for (int y = 0; y < kHeight; ++y) {
    const int warpX = flow(y * 3 + t) / 6;
    for (int x = 0; x < kWidth; ++x) {
      const int px = x * 8 + warpX, py = y * 8 + flow(x * 2 - t) / 6;
      int field = 0;
      for (const auto& center : centers) {
        const int dx = px - center[0], dy = py - center[1];
        field += 691200 / (dx * dx + dy * dy + 4608);
      }
      const int band = field < 23 ? 0 : field < 30 ? 1 : field < 39 ? 2 :
        field < 51 ? 3 : field < 67 ? 4 : field < 87 ? 5 : field < 117 ? 6 : 7;
      pixel(canvas, x, y, colors[band]);
    }
  }
}

inline void idle(const PanelCanvas& canvas, const PanelFrame& frame, uint32_t nowMs) {
  ambient(canvas, frame, nowMs);
  const bool synced = frame.timeSynced && validClock(frame.clock);
  // Calm black backing keeps digits legible over the sparse retro animations.
  if (frame.visual == Visual::Stars || frame.visual == Visual::Rain || frame.visual == Visual::Warp)
    rectangle(canvas, 5, synced ? 16 : 12, 118, 32, 0);
  if (frame.visual == Visual::Lava) {
    for (int dy = -1; dy <= 1; dy += 2)
      for (int dx = -1; dx <= 1; dx += 2)
        centered(canvas, synced ? frame.clock : "--:--", 64 + dx,
                 (synced ? 18 : 14) + dy, 116, 30, 0);
  }
  centered(canvas, synced ? frame.clock : "--:--", 64, synced ? 18 : 14, 116, 30, 0xffff);
  if (!synced) centered(canvas, "ATTENTE HEURE", 64, 51, 116, 7, rgb565(195, 217, 230));
}

inline const char* weatherLabel(int code, bool day) {
  if (code == 0) return day ? "CIEL DEGAGE" : "NUIT CLAIRE";
  if (code == 1 || code == 2) return "ECLAIRCIES";
  if (code == 3) return "NUAGEUX";
  if (code == 45 || code == 48) return "BROUILLARD";
  if (code >= 51 && code <= 57) return "BRUINE";
  if ((code >= 71 && code <= 77) || code == 85 || code == 86) return "NEIGE";
  if (code == 95 || code == 96 || code == 99) return "ORAGE";
  if ((code >= 61 && code <= 67) || (code >= 80 && code <= 82)) return "PLUIE";
  return "METEO";
}

inline void disc(const PanelCanvas& canvas, int cx, int cy, int radius, uint16_t color) {
  for (int y = -radius; y <= radius; ++y)
    for (int x = -radius; x <= radius; ++x)
      if (x*x + y*y <= radius*radius) pixel(canvas, cx+x, cy+y, color);
}

inline void weatherIcon(const PanelCanvas& canvas, const PanelWeather& w, uint32_t tick) {
  const int code = w.code, t = int(tick % 65536), drift = wave(t * 2) / 15;
  if (code < 3) {
    disc(canvas, 23, 27, 8, w.isDay ? kAmber : 0x8ddf);
    if (!w.isDay) {
      disc(canvas, 27, 24, 7, 0);
      for (int i = 0; i < 5; ++i) {
        const int glow = 75 + wave(t * 3 + i * 47) * 2;
        pixel(canvas, 7 + (i * 13) % 33, 13 + (i * 7) % 26, rgb565(glow, glow, glow + 35));
      }
    } else {
      disc(canvas, 22, 26, 5, rgb565(255, 208, 96));
      for (int i = 0; i < 8; ++i) {
        const int dx = wave(i * 32 + t * 2), dy = wave(i * 32 + t * 2 + 64);
        for (int r = 11; r <= 13; ++r) pixel(canvas, 23 + dx * r / 31, 27 + dy * r / 31, kAmber);
      }
    }
  }
  if (code == 45 || code == 48) {
    for (int i = 0; i < 5; ++i) {
      const int shift = wave(t * 2 + i * 53) / 10;
      rectangle(canvas, 10 + shift, 19 + i * 4, 26, 2, i % 2 ? 0x426b : kMuted);
    }
    return;
  }
  if (code > 0) {
    disc(canvas, 15 + drift, 28, 5, kMuted); disc(canvas, 23 + drift, 24, 7, kMuted);
    disc(canvas, 31 + drift, 29, 6, kMuted); rectangle(canvas, 12 + drift, 28, 22, 7, kMuted);
    rectangle(canvas, 14 + drift, 33, 19, 2, 0x738e);
    if (code >= 51) for (int i = 0; i < 4; ++i) {
      const bool snow = (code >= 71 && code <= 77) || code == 85 || code == 86;
      const int x = 12 + i * 7 + (snow ? wave(t * 3 + i * 63) / 20 : 0);
      const int y = 36 + int((tick / (snow ? 3 : 1) + i * 3) % 6);
      pixel(canvas, x, y, 0x8ddf); pixel(canvas, x - 1, y + 1, 0x8ddf);
      if (snow) { pixel(canvas, x + 1, y + 1, 0xffff); pixel(canvas, x, y + 2, 0xffff); }
    }
    if (code == 95 || code == 96 || code == 99) {
      // Slowly breathe the small bolt instead of flashing the whole display.
      const int glow = 120 + wave(t * 2) * 2;
      const uint16_t bolt = rgb565(glow, glow * 3 / 4, glow / 4);
      rectangle(canvas, 22, 31, 3, 5, bolt); rectangle(canvas, 20, 35, 4, 3, bolt);
      pixel(canvas, 21, 38, bolt);
    }
  }
}

inline void weather(const PanelCanvas& canvas, const PanelFrame& frame, uint32_t nowMs) {
  const auto* w = frame.weather;
  text(canvas, w && w->stale ? "ANCIEN" : "METEO", 4, 3, 6, kMuted);
  if (frame.timeSynced && validClock(frame.clock)) text(canvas, frame.clock, 103, 3, 6, kMuted);
  if (!w || !w->available || !isfinite(w->temperatureC) || !isfinite(w->windKmh)) {
    centered(canvas, "METEO INDISPONIBLE", 64, 26, 120, 7, kWhite);
    centered(canvas, "EN ATTENTE DU RELEVE", 64, 47, 120, 6, kMuted);
    return;
  }
  const int code = w->code;
  weatherIcon(canvas, *w, panelAnimated(frame) ? nowMs / kFrameIntervalMs : 0);
  char temp[16], wind[24];
  snprintf(temp, sizeof(temp), "%ld C", (long)floor(w->temperatureC + .5));
  snprintf(wind, sizeof(wind), "VENT %ld KM/H", (long)floor(w->windKmh + .5));
  centered(canvas, temp, 85, 18, 78, 21, kWhite);
  rectangle(canvas, 3, 44, 122, 1, kDivider);
  centered(canvas, weatherLabel(code, w->isDay), 64, 48, 122, 6, kMint);
  centered(canvas, wind, 64, 57, 122, 6, kMuted);
}

inline void metric(const PanelCanvas& canvas, const char* label, const char* value,
                   const char* unit, int center, uint16_t color) {
  centered(canvas, label, center, 37, 38, 6, kMuted);
  centered(canvas, value, center, 44, 38, 10, color);
  centered(canvas, unit, center, 57, 38, 6, kMuted);
}

inline const char* aircraftTypeLabel(const char* type) {
  if (!type) return "TYPE --";
  size_t length = 0;
  for (; length < 5 && type[length]; ++length)
    if (!((type[length] >= 'A' && type[length] <= 'Z') ||
          (type[length] >= '0' && type[length] <= '9'))) return "TYPE --";
  return length >= 2 && length <= 4 ? type : "TYPE --";
}

inline void aircraft(const PanelCanvas& canvas, const PanelAircraft& plane) {
  char callsign[13] = {};
  if (plane.callsign && *plane.callsign) strncpy(callsign, plane.callsign, sizeof(callsign) - 1);
  else strcpy(callsign, "INCONNU");
  text(canvas, callsign, 31, 2, 10, kMint);
  text(canvas, aircraftTypeLabel(plane.aircraftType), 31, 11, 7, kMuted);
  if (plane.logo.read) for (uint16_t index = 0; index < 24 * 16; ++index)
    pixel(canvas, 3 + index % 24, 2 + index / 24, plane.logo.read(plane.logo.context, index));
  rectangle(canvas, 3, 19, 122, 1, kDivider);
  rectangle(canvas, 3, 35, 122, 1, kDivider);
  if (plane.routeStatus == RouteStatus::Known && validAirport(plane.origin) && validAirport(plane.destination)) {
    char route[12];
    snprintf(route, sizeof(route), "%s > %s", plane.origin, plane.destination);
    centered(canvas, route, 64, 21, 122, 14, kWhite);
  } else {
    const char* label = "TRAJET NON RENSEIGNE";
    if (plane.routeStatus == RouteStatus::NotRequested || plane.routeStatus == RouteStatus::Loading)
      label = "RECHERCHE DU TRAJET";
    else if (plane.routeStatus == RouteStatus::Error) label = "TRAJET INDISPONIBLE";
    else if (plane.routeStatus == RouteStatus::Unconfirmed) label = "TRAJET NON CONFIRME";
    centered(canvas, label, 64, 24, 122, 8, kMuted);
  }
  char distance[24] = "--", speed[24] = "--", altitude[24] = "--";
  // Keep float printf out of the firmware. Values too large for the metric
  // area use the same explicit ellipsis fallback as the tablet, never clipping.
  if (isfinite(plane.distanceKm)) {
    if (fabs(plane.distanceKm) > 9999999.9) strcpy(distance, "...");
    else {
      const uint32_t tenths = uint32_t(fabs(plane.distanceKm) * 10 + .5);
      const unsigned long whole = tenths / 10, fraction = tenths % 10;
      const char* sign = plane.distanceKm < 0 ? "-" : "";
      if (whole < 1000) snprintf(distance, sizeof(distance), "%s%lu,%lu", sign, whole, fraction);
      else if (whole < 1000000)
        snprintf(distance, sizeof(distance), "%s%lu %03lu,%lu", sign, whole / 1000, whole % 1000, fraction);
      else snprintf(distance, sizeof(distance), "%s%lu %03lu %03lu,%lu", sign, whole / 1000000,
                    (whole / 1000) % 1000, whole % 1000, fraction);
    }
  }
  if (plane.speedAvailable && isfinite(plane.speedKmh)) {
    if (plane.speedKmh < -2147483648.0 || plane.speedKmh > 2147483647.0) strcpy(speed, "...");
    else snprintf(speed, sizeof(speed), "%ld", (long)floor(plane.speedKmh + .5));
  }
  if (plane.altitudeAvailable) snprintf(altitude, sizeof(altitude), "%ld", (long)plane.altitudeFt);
  metric(canvas, "DIST", distance, "KM", 21, kMint);
  metric(canvas, "VIT", speed, "KM/H", 64, kWhite);
  metric(canvas, "ALT", altitude, "FT", 107, kAmber);
}
inline void widgetMessage(const PanelCanvas& canvas, const ZcsMessageFrame& message, uint32_t nowMs) {
  centered(canvas, "MESSAGE", 64, 3, 122, 6, kMuted);
  const int width = zcsMessageWidth(message.text);
  if (!width) { centered(canvas, "AUCUN MESSAGE", 64, 25, 122, 7, kMuted); return; }
  int x = width <= 120 ? (kWidth - width) / 2 : kWidth - int((nowMs / 50) % uint32_t(width + kWidth));
  for (size_t i = 0; message.text[i] && i < 120 && x < kWidth; ++i) {
    const uint8_t glyph = glyphIndex(message.text[i]);
    const int advance = (glyphWidth(glyph, false) + 1) * 2;
    if (x + advance > 0) {
      const char value[2] = {message.text[i], 0};
      paintText(canvas, {value, false, 2, advance - 2}, x, 25, message.color);
    }
    x += advance;
  }
}

inline void widgetTimer(const PanelCanvas& canvas, const ZcsTimerFrame& timer) {
  centered(canvas, "MINUTEUR", 64, 3, 122, 6, kMuted);
  char value[6] = "--:--";
  if (timer.remainingSeconds <= 5999)
    snprintf(value, sizeof(value), "%02u:%02u", unsigned(timer.remainingSeconds / 60), unsigned(timer.remainingSeconds % 60));
  centered(canvas, value, 64, 22, 122, 28, kWhite);
  const char* status = timer.remainingSeconds == 0 ? "TERMINE" : timer.running ? "EN COURS" : "PAUSE";
  centered(canvas, status, 64, 55, 122, 6, timer.running && timer.remainingSeconds ? kMint : kAmber);
}

inline void widgetScore(const PanelCanvas& canvas, const ZcsScoreFrame& score) {
  if (!score.available || !zcsValidScore(score)) {
    centered(canvas, "SCORE INDISPONIBLE", 64, 25, 122, 7, kMuted); return;
  }
  centered(canvas, score.home, 31, 5, 58, 7, kMint);
  centered(canvas, score.away, 97, 5, 58, 7, kMint);
  char value[8]; snprintf(value, sizeof(value), "%u-%u", unsigned(score.homeScore), unsigned(score.awayScore));
  centered(canvas, value, 64, 22, 122, 21, kWhite);
  centered(canvas, score.status, 64, 49, 122, 6, kMuted);
  if (score.stale) centered(canvas, "ANCIEN", 64, 58, 122, 6, kAmber);
}

inline void widgetPixel(const PanelCanvas& canvas, const ZcsPixelFrame& image, uint32_t nowMs) {
  if (!zcsPixelDimensionsValid(image)) {
    centered(canvas, "PIXEL INDISPONIBLE", 64, 25, 122, 7, kMuted); return;
  }
  const int scaleX = kWidth / image.width, scaleY = kHeight / image.height;
  const int scale = scaleX < scaleY ? scaleX : scaleY;
  const int left = (kWidth - image.width * scale) / 2, top = (kHeight - image.height * scale) / 2;
  for (uint8_t y = 0; y < image.height; ++y) for (uint8_t x = 0; x < image.width; ++x)
    rectangle(canvas, left + x * scale, top + y * scale, scale, scale, zcsPixelAt(image, x, y, nowMs));
}

inline void widget(const PanelCanvas& canvas, const PanelFrame& frame, uint32_t nowMs) {
  if (!frame.widget) { centered(canvas, "CONTENU INDISPONIBLE", 64, 25, 122, 7, kMuted); return; }
  const auto& content = *frame.widget;
  const uint32_t phase = frame.animate ? nowMs : 0;
  if (frame.app == App::Message) widgetMessage(canvas, content.message, phase);
  else if (frame.app == App::Timer) widgetTimer(canvas, content.timer);
  else if (frame.app == App::Score) widgetScore(canvas, content.score);
  else if (frame.app == App::Pixel) widgetPixel(canvas, content.pixel, phase);
}
}  // namespace detail

inline void renderPanel(const PanelCanvas& canvas, const PanelFrame& frame, uint32_t nowMs) {
  if (!canvas.pixel) return;
  const Scene scene = panelScene(frame);
  if (scene == Scene::Idle) {
    detail::idle(canvas, frame, nowMs);
    return;
  }
  if (scene == Scene::Animation) { detail::ambient(canvas, frame, nowMs); return; }
  detail::rectangle(canvas, 0, 0, kWidth, kHeight, 0);
  if (scene == Scene::Widget) detail::widget(canvas, frame, nowMs);
  else if (scene == Scene::Weather) detail::weather(canvas, frame, nowMs);
  else if (scene == Scene::Aircraft) detail::aircraft(canvas, *frame.aircraft);
  else if (scene == Scene::Connecting) {
    detail::text(canvas, "ZCS CONTROL", 20, 9, 11, detail::kMint);
    detail::text(canvas, "CONNEXION", 31, 31, 10, detail::kWhite);
    detail::text(canvas, "EN ATTENTE", 31, 46, 10, detail::kMuted);
  } else if (scene == Scene::Unavailable) {
    detail::text(canvas, "FLIGHT BOARD", 23, 10, 10, detail::kMint);
    detail::text(canvas, "VOL INDISPONIBLE", 17, 32, 9, detail::kWhite);
    detail::text(canvas, "CHOISIR UN AVION", 20, 49, 8, detail::kMuted);
  }
}

}  // namespace flightboard_panel
