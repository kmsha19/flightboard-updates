#pragma once

// Public HTTPS feed. This header must never contain credentials or access tokens.
// The tablet downloads the manifest and firmware; the ESP32 keeps its NVS settings.
#ifndef FLIGHTBOARD_RELEASE_MANIFEST_URL
#define FLIGHTBOARD_RELEASE_MANIFEST_URL "https://raw.githubusercontent.com/kmsha19/flightboard-updates/main/latest.json"
#endif
