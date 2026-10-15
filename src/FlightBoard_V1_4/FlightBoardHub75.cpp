// Compile the pinned Waveshare driver exactly once, without Adafruit GFX.
// Its main .cpp already includes the ESP32-S3 DMA implementation.
#define NO_GFX
#include "assets/vendor/waveshare-hub75/ESP32-HUB75-MatrixPanel-I2S-DMA.cpp"
#include "assets/vendor/waveshare-hub75/ESP32-HUB75-MatrixPanel-leddrivers.cpp"
