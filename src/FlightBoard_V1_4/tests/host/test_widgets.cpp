#include "ZcsWidgets.h"
#include "FlightBoardPanelRenderer.h"
#include <array>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace flightboard_panel;
static int assertions = 0;
#define CHECK(expr) do { ++assertions; if (!(expr)) throw std::runtime_error(std::string(__func__) + ":" + std::to_string(__LINE__) + ": " #expr); } while (0)

void test_timer() {
  ZcsTimer timer(90);
  CHECK(timer.durationSeconds() == 90);
  CHECK(timer.remainingSeconds(1000) == 90);
  CHECK(!timer.running(1000));
  CHECK(!timer.pause(1000));
  CHECK(timer.start(1000));
  CHECK(timer.remainingSeconds(1001) == 90);
  CHECK(timer.remainingSeconds(1999) == 90);
  CHECK(timer.remainingSeconds(2000) == 89);
  CHECK(timer.pause(2499));
  CHECK(timer.remainingSeconds(99999) == 89);  // Paused time cannot drain the countdown.
  CHECK(!timer.running(99999));
  CHECK(timer.resume(100000));
  CHECK(!timer.resume(100000));
  CHECK(timer.remainingSeconds(100500) == 89); // Fractional seconds survive pause/resume.
  CHECK(timer.remainingSeconds(100501) == 88);
  CHECK(timer.remainingSeconds(200000) == 0);
  CHECK(!timer.running(200000));
  CHECK(!timer.resume(200001));
  CHECK(timer.reset(200001));
  CHECK(timer.remainingSeconds(200001) == 90);
  CHECK(!timer.running(200001));
  for (const uint32_t invalid : {0U, 6000U, 65537U, UINT32_MAX}) {
    CHECK(!timer.start(invalid, 300000));
    CHECK(timer.durationSeconds() == 90);
    CHECK(timer.remainingSeconds(300000) == 90);
  }
  CHECK(timer.start(5999, 400000));
  CHECK(timer.remainingSeconds(400000) == 5999);
  CHECK(timer.restorePaused(300, 125));
  CHECK(timer.durationSeconds() == 300);
  CHECK(timer.remainingSeconds(400000) == 125);
  CHECK(!timer.running(400000));
  CHECK(!timer.restorePaused(300, 301));
  CHECK(!timer.restorePaused(0, 0));
  CHECK(!timer.restorePaused(6000));
  CHECK(timer.remainingSeconds(400000) == 125);
  CHECK(timer.restorePaused(0));
  CHECK(!timer.resume(400000));
  CHECK(ZcsTimer(65537).durationSeconds() == 300);

  // Prepare on a copy: a failed persistence attempt must not change active state.
  timer = ZcsTimer(10); timer.start(500);
  auto next = timer; CHECK(next.pause(1500));
  CHECK(next.remainingSeconds(2500) == 9);
  CHECK(timer.running(2500)); CHECK(timer.remainingSeconds(2500) == 8);
  timer = next; CHECK(!timer.running(2500)); CHECK(timer.remainingSeconds(2500) == 9);

  // One uninterrupted timeline crosses uint32 millis wrap and then expires exactly once.
  timer = ZcsTimer(12);
  const uint32_t start = UINT32_MAX - 2500;
  timer.start(start);
  for (uint32_t elapsed = 0; elapsed <= 15000; elapsed += 125) {
    const uint32_t expected = elapsed >= 12000 ? 0 : (12000 - elapsed + 999) / 1000;
    CHECK(timer.remainingSeconds(start + elapsed) == expected);
    CHECK(timer.running(start + elapsed) == (expected != 0));
  }
  CHECK(timer.remainingSeconds(start) == 0); // Completed state cannot resurrect at a later wrap.
  CHECK(!timer.running(start));
}

void test_validation_and_pixels() {
  CHECK(zcsValidAscii("Bonjour 123", 12, 1));
  CHECK(!zcsValidAscii("", 120, 1));
  CHECK(!zcsValidAscii(nullptr, 120, 1));
  CHECK(!zcsValidAscii("bonjour\n", 120, 1));
  CHECK(!zcsValidAscii("caf\xc3\xa9", 120, 1));
  const std::string message120(120, 'A'), message121(121, 'A');
  CHECK(zcsValidMessage({message120.c_str(), 0}));
  CHECK(!zcsValidMessage({message121.c_str(), 0xffff}));
  CHECK(zcsMessageWidth("A") == 10);
  CHECK(zcsMessageWidth("A A") == 30);
  CHECK(zcsMessageWidth("::") == 6);
  CHECK(zcsMessageWidth(message120.c_str()) == 1438);
  CHECK(zcsMessageWidth(message121.c_str()) == 0);
  for (int c = 32; c <= 126; ++c) {
    const char text[] = {char(c), 0};
    CHECK(zcsMessageWidth(text) == detail::textWidth(text, false, 2));
  }
  ZcsScoreFrame score{"HOME", "AWAY", "TERMINE", 999, 0, true, false};
  CHECK(zcsValidScore(score)); score.homeScore = 1000; CHECK(!zcsValidScore(score));
  score.homeScore = 1; score.home = "1234567890123"; CHECK(!zcsValidScore(score));
  score.home = "HOME"; score.status = "1234567890123456789012345"; CHECK(!zcsValidScore(score));

  const uint16_t palette[] = {0xf800, 0x07e0, 0x001f};
  const uint8_t packed[] = {0x01, 0x22, 0x10};
  ZcsPixelFrame pixel{3, 1, 2, 3, 100, palette, packed, sizeof(packed)};
  CHECK(zcsValidPixel(pixel));
  CHECK(zcsPixelAt(pixel, 0, 0, 0) == 0xf800); // Palette0 is opaque color, not transparency.
  CHECK(zcsPixelAt(pixel, 1, 0, 99) == 0x07e0);
  CHECK(zcsPixelAt(pixel, 2, 0, 99) == 0x001f);
  CHECK(zcsPixelAt(pixel, 0, 0, 100) == 0x001f); // Odd frame size continues into the next nibble.
  CHECK(zcsPixelAt(pixel, 1, 0, 100) == 0x07e0);
  CHECK(zcsPixelAt(pixel, 2, 0, 100) == 0xf800);
  CHECK(zcsPixelAt(pixel, 0, 0, 200) == 0xf800);
  CHECK(zcsPixelAt(pixel, 3, 0, 0) == 0);
  CHECK(zcsPixelAt(pixel, 0, 1, 0) == 0);
  for (const uint16_t duration : {0, 79, 2001}) {
    auto bad = pixel; bad.frameMs = duration; CHECK(!zcsValidPixel(bad)); CHECK(zcsPixelAt(bad, 0, 0, 0) == 0);
  }
  auto bad = pixel; bad.dataBytes--; CHECK(!zcsValidPixel(bad));
  bad = pixel; bad.paletteSize = 2; CHECK(!zcsValidPixel(bad)); CHECK(zcsPixelAt(bad, 2, 0, 0) == 0);
  bad = pixel; bad.width = 0; CHECK(!zcsValidPixel(bad));
  bad = pixel; bad.height = 65; CHECK(!zcsValidPixel(bad));
  bad = pixel; bad.frameCount = 33; CHECK(!zcsValidPixel(bad));
  bad = pixel; bad.palette = nullptr; CHECK(!zcsValidPixel(bad));
  const uint8_t odd[] = {0x01, 0x2f};
  pixel = {1, 1, 3, 3, 80, palette, odd, sizeof(odd)};
  CHECK(zcsValidPixel(pixel)); // Unused padding nibble does not become a fourth pixel.
  CHECK(zcsPixelAt(pixel, 0, 0, UINT32_MAX) == palette[(UINT32_MAX / 80) % 3]);
  std::vector<uint8_t> maximum(32768, 0);
  pixel = {64, 64, 16, 3, 2000, palette, maximum.data(), maximum.size()};
  CHECK(zcsValidPixel(pixel));
  pixel.frameCount = 17; CHECK(!zcsValidPixel(pixel));
}

struct Screen {
  std::array<uint16_t, 8192> pixels{};
  static void pixel(void* context, int16_t x, int16_t y, uint16_t color) {
    CHECK(x >= 0 && x < 128 && y >= 0 && y < 64);
    static_cast<Screen*>(context)->pixels[y * 128 + x] = color;
  }
  void draw(const PanelFrame& frame, uint32_t now = 0) { renderPanel({this, pixel}, frame, now); }
  uint16_t at(int x, int y) const { return pixels[y * 128 + x]; }
};
void test_rendering() {
  Screen screen;
  ZcsWidgetFrame widget;
  PanelFrame frame; frame.stateAvailable = true; frame.widget = &widget; frame.app = App::Message;
  CHECK(panelScene(frame) == Scene::Widget);
  widget.message = {"HELLO", 0xf800};
  CHECK(!panelAnimated(frame));
  screen.draw(frame); const auto shortMessage = screen.pixels;
  screen.draw(frame, 10000); CHECK(screen.pixels == shortMessage);
  CHECK(std::count(screen.pixels.begin(), screen.pixels.end(), uint16_t(0xf800)) > 40);
  const std::string longMessage(120, 'W'); widget.message.text = longMessage.c_str();
  CHECK(panelAnimated(frame)); screen.draw(frame, 0); const auto before = screen.pixels;
  screen.draw(frame, 6400); CHECK(screen.pixels != before);
  frame.animate = false; screen.draw(frame, 6400); CHECK(screen.pixels == before); frame.animate = true;
  widget.message.text = "invalid\n"; CHECK(!panelAnimated(frame)); screen.draw(frame);

  frame.app = App::Timer; widget.timer = {5999, true}; CHECK(panelAnimated(frame)); screen.draw(frame);
  const auto running = screen.pixels; widget.timer.running = false; CHECK(!panelAnimated(frame)); screen.draw(frame); CHECK(screen.pixels != running);
  widget.timer = {0, false}; screen.draw(frame); CHECK(!panelAnimated(frame));
  widget.timer.remainingSeconds = 100000; screen.draw(frame); // Invalid snapshots remain bounded.

  frame.app = App::Score; widget.score = {"HOME", "AWAY", "69 MIN", 999, 999, true, false};
  CHECK(!panelAnimated(frame)); screen.draw(frame); const auto score = screen.pixels;
  widget.score.stale = true; screen.draw(frame); CHECK(screen.pixels != score);
  CHECK(std::equal(score.begin(), score.begin() + 58 * 128, screen.pixels.begin())); // Status remains visible above the stale badge.
  widget.score.available = false; screen.draw(frame); CHECK(screen.pixels != score);

  const uint16_t palette[] = {0xf800, 0x001f}; const uint8_t data[] = {0x01, 0x10};
  widget.pixel = {2, 1, 2, 2, 100, palette, data, sizeof(data)}; frame.app = App::Pixel;
  CHECK(panelAnimated(frame)); screen.draw(frame, 0);
  CHECK(screen.at(0, 0) == 0xf800 && screen.at(63, 63) == 0xf800);
  CHECK(screen.at(64, 0) == 0x001f && screen.at(127, 63) == 0x001f);
  screen.draw(frame, 100); CHECK(screen.at(0, 0) == 0x001f && screen.at(127, 63) == 0xf800);
  widget.pixel.width = 1; widget.pixel.height = 2; screen.draw(frame, 0);
  CHECK(screen.at(47, 0) == 0 && screen.at(48, 0) == 0xf800 && screen.at(79, 31) == 0xf800 && screen.at(80, 0) == 0);
  CHECK(screen.at(48, 32) == 0x001f && screen.at(79, 63) == 0x001f);
  widget.pixel.dataBytes = 0; CHECK(!panelAnimated(frame)); screen.draw(frame);
  frame.widget = nullptr; CHECK(!panelAnimated(frame)); screen.draw(frame);
  frame.widget = &widget;
  for (const auto app : {App::Message, App::Timer, App::Score, App::Pixel}) {
    frame.app = app; frame.sleeping = true; screen.pixels.fill(0xffff); screen.draw(frame, 1000);
    CHECK(panelScene(frame) == Scene::Sleeping); CHECK(!panelAnimated(frame));
    CHECK(std::all_of(screen.pixels.begin(), screen.pixels.end(), [](uint16_t value) { return value == 0; }));
  }
}
int main() { test_timer(); test_validation_and_pixels(); test_rendering(); std::cout << "Portable widgets: " << assertions << " assertions passed\n"; }
