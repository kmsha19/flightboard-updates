#pragma once
#include <stddef.h>
#include <stdint.h>
#include <esp_partition.h>
#include <mbedtls/sha256.h>

// Raw A/B store in the existing data partition. Firmware/NVS partitions are
// never modified. No heap allocation, filesystem, or automatic formatting.
class ZcsContentStore {
public:
  static constexpr uint32_t BANK_BYTES = 0xB0000;
  static constexpr uint32_t HEADER_BYTES = 4096;
  static constexpr uint32_t CAPACITY = BANK_BYTES - HEADER_BYTES;
  static constexpr size_t MAX_ENTRIES = 16;
  struct Entry { char id[33]; uint32_t offset; uint32_t length; };
  using Validator = bool (*)(ZcsContentStore&, void*);

  bool begin();
  const Entry* entries() const { return active_.entries; }
  size_t count() const { return active_.count; }
  const Entry* find(const char* id) const;
  bool read(const char* id, uint32_t offset, void* out, size_t length) const;
  bool beginPut(const char* id, uint32_t length, const char* sha256Hex);
  bool write(const uint8_t* data, size_t length);
  bool readPending(uint32_t offset, void* out, size_t length) const;
  bool finish(Validator validator = nullptr, void* context = nullptr);
  void abort();
  bool remove(const char* id);
  const char* error() const { return error_; }
  bool busy() const { return writing_; }
  bool available() const { return partition_ != nullptr; }
  uint32_t usedBytes() const { return active_.used; }
  uint32_t generation() const { return active_.generation; }

private:
  static constexpr uint32_t MAGIC = 0x3153435A; // ZCS1, programmed last.
  static constexpr uint16_t SCHEMA = 1;
  struct Header {
    uint32_t magic;
    uint16_t schema, count;
    uint32_t generation, used;
    uint8_t sha256[32];
    Entry entries[MAX_ENTRIES];
    uint32_t crc;
  };
  static_assert(sizeof(Entry) == 44 && sizeof(Header) == 756, "Store layout must remain stable");
  const esp_partition_t* partition_ = nullptr;
  Header active_{}, pending_{};
  int activeBank_ = -1, pendingBank_ = -1;
  uint32_t expectedLength_ = 0, received_ = 0, pendingOffset_ = 0;
  uint8_t expectedSha_[32]{};
  mbedtls_sha256_context uploadHash_{}, bankHash_{};
  bool writing_ = false, removing_ = false;
  mutable const char* error_ = "";

  static bool validId(const char* id);
  static bool decodeSha(const char* text, uint8_t* bytes);
  static uint32_t crc32(const uint8_t* data, size_t length);
  bool fail(const char* reason) const { error_ = reason; return false; }
  bool stop(const char* reason) { abort(); return fail(reason); }
  bool loadBank(int bank, Header& header) const;
  bool hashPayload(int bank, uint32_t length, uint8_t* digest) const;
  bool start(const char* id, uint32_t length, const uint8_t* expected, bool removing);
  uint32_t payloadAddress(int bank, uint32_t offset) const {
    return uint32_t(bank) * BANK_BYTES + HEADER_BYTES + offset;
  }
};
