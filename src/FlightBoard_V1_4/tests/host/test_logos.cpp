#include "MockPlatform.hpp"
#include <array>
#include <set>
#include <stdexcept>
#include "AirlineLogos.h"

static int assertions = 0;
#define CHECK(expr) do { ++assertions; if (!(expr)) throw std::runtime_error( \
  std::string(__func__) + ":" + std::to_string(__LINE__) + ": " #expr); } while (0)

void test_requested_airlines_and_normalization() {
  for (const char* call : {"BEL418", "VRE720", "ETH704", "CRL900"}) {
    const AirlineAsset* airline = findAirline(call);
    CHECK(airline != &DEFAULT_AIRLINE);
    CHECK(std::string(airline->code) == std::string(call, 3));
    CHECK(airline->name && std::strlen(airline->name) > 3);
  }
  const AirlineAsset* clean = findAirline("VRE720");
  CHECK(findAirline(" \t vre720\r\n") == clean);
  CHECK(findAirline("bEl418") == findAirline("BEL418"));
  CHECK(findAirline("AFR842") == &AIRLINES[0]);
}

void test_operating_company_aliases() {
  // Callsigns are operating-company ICAO identifiers, which can differ from
  // the commercial brand shown to passengers (e.g. Transavia France TVF).
  for (const auto& pair : std::vector<std::pair<const char*, const char*>>{
      {"TVF42", "TRA42"}, {"EJU42", "EZY42"}, {"EZS42", "EZY42"},
      {"HOP42", "AFR42"}, {"DHA42", "BCS42"}, {"DHK42", "BCS42"},
      {"DHV42", "BCS42"}, {"MAY42", "RYR42"}, {"RUK42", "RYR42"}}) {
    const AirlineAsset* operating = findAirline(pair.first);
    const AirlineAsset* brand = findAirline(pair.second);
    CHECK(operating != &DEFAULT_AIRLINE);
    CHECK(brand != &DEFAULT_AIRLINE);
    CHECK(operating->pixels == brand->pixels);
    CHECK(operating->image == brand->image);
    CHECK(operating->imageSize == brand->imageSize);
    CHECK(std::strcmp(operating->id, brand->id) == 0);
  }
}

void test_unrecognized_callsigns() {
  CHECK(findAirline("RYL42") == &DEFAULT_AIRLINE); // Royal Air Philippines, not Ryanair.
  CHECK(findAirline("BOX42") == &DEFAULT_AIRLINE); // AeroLogic has its own brand.
  for (const char* call : {"", " \r\n", "AF", "AFR", "BEL", "VRE", "N12345",
      "ZZZ123", "F-HABC", "XAFR123", "AF123", "123456"}) {
    CHECK(findAirline(call) == &DEFAULT_AIRLINE);
  }
}

void check_asset(const AirlineAsset& asset) {
  CHECK(asset.pixels != nullptr);
  CHECK(asset.image != nullptr);
  CHECK(asset.imageSize >= 20);
  CHECK(std::memcmp(asset.image, "RIFF", 4) == 0);
  CHECK(std::memcmp(asset.image + 8, "WEBP", 4) == 0);
  CHECK(std::memcmp(asset.image + 12, "VP8L", 4) == 0);
  CHECK(asset.name && std::strlen(asset.name));
  CHECK(asset.id && std::strlen(asset.id));
}

void test_embedded_assets_and_unique_codes() {
  std::set<std::string> codes;
  for (const AirlineAsset& airline : AIRLINES) {
    CHECK(std::strlen(airline.code) == 3);
    CHECK(codes.insert(airline.code).second);
    check_asset(airline);
  }
  check_asset(DEFAULT_AIRLINE);
}

void test_logos_blend_into_dark_surfaces() {
  std::set<const void*> checked;
  for (size_t index = 0; index <= AIRLINE_COUNT; ++index) {
    const AirlineAsset& asset = index == AIRLINE_COUNT ? DEFAULT_AIRLINE : AIRLINES[index];
    if (!checked.insert(asset.pixels).second) continue;
    // The panel has no alpha channel. Its margin must be unlit, never a white
    // card (or an opaque brand-colored frame added around the source emblem).
    for (size_t x = 0; x < AIRLINE_LOGO_W; ++x) {
      CHECK(readAirlinePixel(asset, x) == 0);
      CHECK(readAirlinePixel(asset, (AIRLINE_LOGO_H - 1) * AIRLINE_LOGO_W + x) == 0);
    }
    for (size_t y = 0; y < AIRLINE_LOGO_H; ++y) {
      CHECK(readAirlinePixel(asset, y * AIRLINE_LOGO_W) == 0);
      CHECK(readAirlinePixel(asset, y * AIRLINE_LOGO_W + AIRLINE_LOGO_W - 1) == 0);
    }
    // Actual WebP decode and RGBA equivalence are checked by
    // tests/test_logo_images.py against the retained source PNGs.
  }
  // White elements inside an emblem remain visible: removing a white backing
  // must never punch out the Swiss cross, Corsair sails, or Tahiti flower.
  for (const char* call : {"SWR42", "CRL42", "THT42", "DLH42"}) {
    const AirlineAsset& asset = *findAirline(call);
    size_t bright = 0;
    for (size_t i = 0; i < AIRLINE_LOGO_W * AIRLINE_LOGO_H; ++i) {
      const uint16_t c = readAirlinePixel(asset, i);
      bright += ((c >> 11) & 31) >= 24 && ((c >> 5) & 63) >= 48 && (c & 31) >= 24;
    }
    CHECK(bright >= 3);
  }
}

// Immutable fingerprints of all 384 RGB565 pixels from the V2.0.10 public
// staging header, before palette packing. FNV-1a covers little-endian pixel bytes.
void test_lossless_palette_against_v2010() {
  const std::map<std::string, uint64_t> expected = {
    {"logo_afr", UINT64_C(0x7c6f300a9aa3baad)},
    {"logo_bel", UINT64_C(0x2c2e2a43e77305bb)},
    {"logo_vre", UINT64_C(0x4eaccb7c4152f6f2)},
    {"logo_eth", UINT64_C(0xa0e3460d611ade3b)},
    {"logo_crl", UINT64_C(0xacb7d4407650a66f)},
    {"logo_fwi", UINT64_C(0xe55529badff2c61d)},
    {"logo_fbu", UINT64_C(0x61da8cdb425a73a4)},
    {"logo_reu", UINT64_C(0xa06578f9719043e0)},
    {"logo_tht", UINT64_C(0x87b0bb6fe75fa689)},
    {"logo_mau", UINT64_C(0x81066619c5deb288)},
    {"logo_szn", UINT64_C(0x47355e148cb95a4a)},
    {"logo_ram", UINT64_C(0x1603296f41d72ca5)},
    {"logo_dah", UINT64_C(0xa977de1df2ab7be6)},
    {"logo_tar", UINT64_C(0xc8b3c102bc7b4805)},
    {"logo_lbt", UINT64_C(0x10d34919209de09c)},
    {"logo_aca", UINT64_C(0xfee268e2523a16fe)},
    {"logo_tsc", UINT64_C(0x1074e59f447f4281)},
    {"logo_swr", UINT64_C(0x1659418b54fa2726)},
    {"logo_lgl", UINT64_C(0x9cf3532f026cafa5)},
    {"logo_tra", UINT64_C(0xc843f3c8686b64cc)},
    {"logo_skk", UINT64_C(0xdd7d824b2f04b03f)},
    {"logo_crc", UINT64_C(0xcf655e1b0ea64204)},
    {"logo_qtr", UINT64_C(0x5b2292d22d877b19)},
    {"logo_thy", UINT64_C(0x3567b19ba8502a2b)},
    {"logo_etd", UINT64_C(0xad023c9f8d900d16)},
    {"logo_kqa", UINT64_C(0x693bf2371ae0980c)},
    {"logo_rwd", UINT64_C(0xb415ff4d454bea31)},
    {"logo_ryr", UINT64_C(0x9cae515d887bcd63)},
    {"logo_ezy", UINT64_C(0xa4d4e75eb4acc5e9)},
    {"logo_ups", UINT64_C(0x2c352de88d78a968)},
    {"logo_bcs", UINT64_C(0x287bd01432dc7125)},
    {"logo_dlh", UINT64_C(0x00dc390165e5fd0d)},
    {"logo_klm", UINT64_C(0xe1cec3d3679be9d6)},
    {"logo_baw", UINT64_C(0x52b341a85883ed6b)},
    {"logo_ibe", UINT64_C(0xdfb53384e9c8797e)},
    {"logo_uae", UINT64_C(0x23f17a6e974d61f5)},
    {"logo_default", UINT64_C(0xa7940ff193ff821f)},
  };
  std::set<std::string> checked;
  for (size_t index = 0; index <= AIRLINE_COUNT; ++index) {
    const auto& asset = index == AIRLINE_COUNT ? DEFAULT_AIRLINE : AIRLINES[index];
    if (!checked.insert(asset.id).second) continue;
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    for (uint16_t pixel = 0; pixel < 384; ++pixel) {
      const uint16_t value = readAirlinePixel(asset, pixel);
      for (uint8_t byte : {uint8_t(value), uint8_t(value >> 8)})
        hash = (hash ^ byte) * UINT64_C(0x100000001b3);
    }
    CHECK(expected.count(asset.id) == 1);
    CHECK(hash == expected.at(asset.id));
    CHECK(readAirlinePixel(asset, 384) == 0);
    CHECK(readAirlinePixel(asset, UINT16_MAX) == 0);
  }
  CHECK(checked.size() == expected.size());
}

static std::vector<std::tuple<int16_t, int16_t, uint16_t>> drawn;
void record_pixel(int16_t x, int16_t y, uint16_t color) {
  drawn.emplace_back(x, y, color);
}

void test_panel_callback_geometry() {
  CHECK(AIRLINE_LOGO_W == 24);
  CHECK(AIRLINE_LOGO_H == 16);
  for (const char* callsign : {"BEL418", "VRE720", "ETH704", "CRL900", "ZZZ123"}) {
    const AirlineAsset& asset = *findAirline(callsign);
    for (const auto& origin : std::vector<std::pair<int16_t, int16_t>>{{3, 2}, {-7, -4}}) {
      drawn.clear();
      drawAirlineLogo(origin.first, origin.second, asset, record_pixel);
      CHECK(drawn.size() == 384);
      std::set<std::pair<int16_t, int16_t>> coordinates;
      bool visible = false;
      for (const auto& pixel : drawn) {
        const int16_t x = std::get<0>(pixel) - origin.first;
        const int16_t y = std::get<1>(pixel) - origin.second;
        CHECK(x >= 0 && x < AIRLINE_LOGO_W);
        CHECK(y >= 0 && y < AIRLINE_LOGO_H);
        CHECK(coordinates.insert({x, y}).second);
        CHECK(std::get<2>(pixel) == readAirlinePixel(asset, y * AIRLINE_LOGO_W + x));
        visible |= std::get<2>(pixel) != 0;
      }
      CHECK(visible);
    }
  }
}

int main() {
  const std::vector<std::pair<const char*, std::function<void()>>> tests = {
    {"requested carriers and normalized callsigns", test_requested_airlines_and_normalization},
    {"operating-company aliases share brand assets", test_operating_company_aliases},
    {"unknown and incomplete callsigns use neutral fallback", test_unrecognized_callsigns},
    {"embedded lossless WebP signatures and unambiguous ICAO codes", test_embedded_assets_and_unique_codes},
    {"transparent tablet assets and black panel margins preserve white brand details", test_logos_blend_into_dark_surfaces},
    {"palette decompression preserves every V2.0.10 logo exactly", test_lossless_palette_against_v2010},
    {"panel callback draws exactly 24 by 16 pixels at its origin", test_panel_callback_geometry},
  };
  int failed = 0;
  for (const auto& test : tests) {
    try { test.second(); std::cout << "PASS " << test.first << '\n'; }
    catch (const std::exception& error) { ++failed; std::cerr << "FAIL " << test.first << ": " << error.what() << '\n'; }
  }
  std::cout << tests.size() << " scenarios, " << assertions << " assertions, " << failed << " failed\n";
  return failed ? 1 : 0;
}
