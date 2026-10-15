#pragma once

// Portable widget data and monotonic timer. No Arduino, network, allocation or I/O.
#include <stddef.h>
#include <stdint.h>

inline bool zcsValidAscii(const char* value, size_t maximum, size_t minimum = 0) {
  if (!value) return minimum == 0;
  size_t length = 0;
  for (; length <= maximum && value[length]; ++length)
    if (value[length] < 32 || value[length] > 126) return false;
  return length >= minimum && length <= maximum;
}

struct ZcsMessageFrame {
  const char* text = nullptr;
  uint16_t color = 0xffff;
};
struct ZcsTimerFrame {
  uint32_t remainingSeconds = 0;
  bool running = false;
};
struct ZcsScoreFrame {
  const char* home = nullptr;
  const char* away = nullptr;
  const char* status = nullptr;
  uint16_t homeScore = 0, awayScore = 0;
  bool available = false, stale = false;
};
struct ZcsPixelFrame {
  uint8_t width = 0, height = 0, frameCount = 0, paletteSize = 0;
  uint16_t frameMs = 100;
  const uint16_t* palette = nullptr;
  const uint8_t* data = nullptr;
  size_t dataBytes = 0;
};
struct ZcsWidgetFrame {
  ZcsMessageFrame message;
  ZcsTimerFrame timer;
  ZcsScoreFrame score;
  ZcsPixelFrame pixel;
};

inline bool zcsValidMessage(const ZcsMessageFrame& message) {
  return zcsValidAscii(message.text, 120, 1);
}
inline bool zcsValidScore(const ZcsScoreFrame& score) {
  return zcsValidAscii(score.home, 12, 1) && zcsValidAscii(score.away, 12, 1) &&
      zcsValidAscii(score.status, 24) && score.homeScore <= 999 && score.awayScore <= 999;
}
inline size_t zcsPixelCount(const ZcsPixelFrame& pixel) {
  return size_t(pixel.width) * pixel.height * pixel.frameCount;
}
inline bool zcsPixelDimensionsValid(const ZcsPixelFrame& pixel) {
  const size_t count = zcsPixelCount(pixel);
  return pixel.width >= 1 && pixel.width <= 64 && pixel.height >= 1 && pixel.height <= 64 &&
      pixel.frameCount >= 1 && pixel.frameCount <= 32 && pixel.frameMs >= 80 && pixel.frameMs <= 2000 &&
      pixel.paletteSize >= 1 && pixel.paletteSize <= 16 && pixel.palette && pixel.data &&
      pixel.dataBytes <= 32768 && pixel.dataBytes == (count + 1) / 2;
}
inline bool zcsValidPixel(const ZcsPixelFrame& pixel) {
  if (!zcsPixelDimensionsValid(pixel)) return false;
  for (size_t i = 0; i < zcsPixelCount(pixel); ++i) {
    const uint8_t packed = pixel.data[i / 2];
    const uint8_t index = (i & 1) ? packed & 15 : packed >> 4;
    if (index >= pixel.paletteSize) return false;
  }
  return true;
}
inline uint16_t zcsPixelAt(const ZcsPixelFrame& pixel, uint8_t x, uint8_t y, uint32_t nowMs) {
  if (!zcsPixelDimensionsValid(pixel) || x >= pixel.width || y >= pixel.height) return 0;
  const size_t frame = (nowMs / pixel.frameMs) % pixel.frameCount;
  const size_t index = (frame * pixel.height + y) * pixel.width + x;
  const uint8_t packed = pixel.data[index / 2];
  const uint8_t paletteIndex = (index & 1) ? packed & 15 : packed >> 4;
  return paletteIndex < pixel.paletteSize ? pixel.palette[paletteIndex] : 0;
}

// Width of the existing 5x7 font at scale2. The renderer uses the same glyphs.
inline int zcsMessageWidth(const char* text) {
  if (!zcsValidAscii(text, 120, 1)) return 0;
  int width = 0;
  for (size_t i = 0; text[i]; ++i) {
    const char c = text[i];
    const int glyph = c == ' ' ? 3 : c == ':' || c == '.' || c == '!' || c == '\'' ? 1 : c == ',' ? 2 :
        c == '-' || c == '/' || c == '>' ? 3 : 5;
    width += (glyph + 1) * 2;
  }
  return width - 2;
}

class ZcsTimer {
 public:
  explicit ZcsTimer(uint32_t durationSeconds = 300)
      : durationSeconds_(validDuration(durationSeconds) ? durationSeconds : 300),
        remainingMs_(uint32_t(durationSeconds_) * 1000) {}
  static bool validDuration(uint32_t seconds) { return seconds >= 1 && seconds <= 5999; }
  uint16_t durationSeconds() const { return durationSeconds_; }
  void update(uint32_t nowMs) {
    if (!running_) return;
    const uint32_t elapsed = nowMs - updatedMs_;  // unsigned subtraction handles millis wrap.
    updatedMs_ = nowMs;
    if (elapsed >= remainingMs_) { remainingMs_ = 0; running_ = false; }
    else remainingMs_ -= elapsed;
  }
  uint32_t remainingSeconds(uint32_t nowMs) { update(nowMs); return (remainingMs_ + 999) / 1000; }
  bool running(uint32_t nowMs) { update(nowMs); return running_; }
  bool start(uint32_t nowMs) {
    remainingMs_ = uint32_t(durationSeconds_) * 1000; updatedMs_ = nowMs; running_ = true; return true;
  }
  bool start(uint32_t seconds, uint32_t nowMs) {
    if (!validDuration(seconds)) return false;
    durationSeconds_ = seconds; return start(nowMs);
  }
  bool pause(uint32_t nowMs) {
    update(nowMs);
    if (!running_) return false;
    running_ = false; return true;
  }
  bool resume(uint32_t nowMs) {
    if (running_ || !remainingMs_) return false;
    updatedMs_ = nowMs; running_ = true; return true;
  }
  bool reset(uint32_t nowMs) {
    remainingMs_ = uint32_t(durationSeconds_) * 1000; updatedMs_ = nowMs; running_ = false; return true;
  }
  bool restorePaused(uint32_t remainingSeconds) {
    if (remainingSeconds > 5999) return false;
    remainingMs_ = remainingSeconds * 1000; updatedMs_ = 0; running_ = false; return true;
  }
  bool restorePaused(uint32_t durationSeconds, uint32_t remainingSeconds) {
    if (!validDuration(durationSeconds) || remainingSeconds > durationSeconds) return false;
    durationSeconds_ = durationSeconds; return restorePaused(remainingSeconds);
  }
 private:
  uint16_t durationSeconds_;
  uint32_t remainingMs_, updatedMs_ = 0;
  bool running_ = false;
};
