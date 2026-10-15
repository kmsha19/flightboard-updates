#pragma once
#include <Arduino.h>
#include <new>
#define NO_GFX
#include "assets/vendor/waveshare-hub75/ESP32-HUB75-MatrixPanel-I2S-DMA.h"

// Verified Waveshare ESP32-S3-RGB-Matrix, single 128x64 1/32 HUB75 panel.
// The panel has been visually validated; apply the user's day/night setting.
class FlightBoardPanel {
public:
  static constexpr int BRIGHTNESS_LIMIT = 100;

  static void blankOutput() {
    // Preload OE high before enabling output. Arduino digitalWrite() before
    // pinMode() is ignored by core 3.3.12, so use the driver's GPIO API here.
    gpio_set_level(GPIO_NUM_2, 1);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
  }

  bool begin() {
    if (display_) return ready_;
    blankOutput();
    // Leave room for Wi-Fi, the HTTPS workers and an OTA upload.
    if (heap_caps_get_free_size(MALLOC_CAP_INTERNAL) < 140000 ||
        heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA) < 80000)
      return false;
    HUB75_I2S_CFG config(128, 64, 1);
    // This K716 panel shows B,R,G for the standard R,G,B startup pattern
    // (confirmed on the real display). Reassign only the six colour signals:
    // board R/G/B pins 4/5/6 and 7/15/16 -> logical R/G/B 5/6/4 and 15/16/7.
    // RGB565 assets and the tablet retain standard RGB; address/OE/CLK stay put.
    config.gpio = {5, 6, 4, 15, 16, 7, 18, 8, 3, 42, 9, 40, 2, 41};
    config.line_decoder = HUB75_I2S_CFG::TYPE138;
    config.clkphase = false;
    // Standard HUB75 first: the photo does not identify an FM6126A chip.
    // Do not send vendor-specific register writes without that confirmation.
    config.driver = HUB75_I2S_CFG::SHIFTREG;
    config.double_buff = true;
    config.setPixelColorDepthBits(4);
    config.i2sspeed = HUB75_I2S_CFG::HZ_8M;
    config.min_refresh_rate = 60;
    display_ = new (std::nothrow) MatrixPanel_I2S_DMA(config);
    if (!display_) return false;
    ready_ = display_->begin();
    if (!ready_) return false; // No retry loop or partially initialized DMA reuse.
    display_->setBrightness8(0); // begin() initializes black buffers first.
    return true;
  }

  bool ready() const { return ready_; }
  int brightness() const { return brightness_; }
  uint32_t frames() const { return frames_; }
  uint32_t renderMicros() const { return renderMicros_; }

  void setBrightness(int requested) {
    const int next = constrain(requested, 0, BRIGHTNESS_LIMIT);
    if (!ready_ || next == brightness_) return;
    display_->setBrightness8(next * 255 / 100);
    brightness_ = next;
  }

  static void pixel(void* context, int16_t x, int16_t y, uint16_t color) {
    auto* panel = static_cast<FlightBoardPanel*>(context);
    if (panel->ready_) panel->display_->drawPixel(x, y, color);
  }

  void present(uint32_t renderMicros) {
    if (!ready_) return;
    display_->flipDMABuffer();
    renderMicros_ = renderMicros;
    ++frames_;
  }

private:
  MatrixPanel_I2S_DMA* display_ = nullptr;
  bool ready_ = false;
  int brightness_ = -1;
  uint32_t frames_ = 0, renderMicros_ = 0;
};
