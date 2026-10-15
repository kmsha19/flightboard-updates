#include "ZcsContentStore.h"
#include <string.h>

bool ZcsContentStore::validId(const char* id) {
  if (!id) return false;
  const size_t length = strnlen(id, 33);
  if (!length || length > 32) return false;
  for (size_t i = 0; i < length; ++i) {
    const char c = id[i];
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-')) return false;
  }
  return true;
}

bool ZcsContentStore::decodeSha(const char* text, uint8_t* bytes) {
  if (!text || strnlen(text, 65) != 64) return false;
  for (size_t i = 0; i < 64; ++i) {
    const char c = text[i];
    const int n = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 :
                  c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
    if (n < 0) return false;
    if (!(i & 1)) bytes[i / 2] = uint8_t(n << 4);
    else bytes[i / 2] |= uint8_t(n);
  }
  return true;
}

uint32_t ZcsContentStore::crc32(const uint8_t* data, size_t length) {
  uint32_t crc = 0xFFFFFFFF;
  while (length--) {
    crc ^= *data++;
    for (unsigned bit = 0; bit < 8; ++bit) crc = (crc >> 1) ^ (0xEDB88320U & (0U - (crc & 1)));
  }
  return ~crc;
}

bool ZcsContentStore::hashPayload(int bank, uint32_t length, uint8_t* digest) const {
  mbedtls_sha256_context hash;
  mbedtls_sha256_init(&hash);
  bool ok = mbedtls_sha256_starts(&hash, 0) == 0;
  uint8_t block[512];
  for (uint32_t offset = 0; ok && offset < length;) {
    const size_t size = length - offset < sizeof(block) ? length - offset : sizeof(block);
    ok = esp_partition_read(partition_, payloadAddress(bank, offset), block, size) == ESP_OK &&
         mbedtls_sha256_update(&hash, block, size) == 0;
    offset += size;
  }
  if (ok) ok = mbedtls_sha256_finish(&hash, digest) == 0;
  mbedtls_sha256_free(&hash);
  return ok;
}

bool ZcsContentStore::loadBank(int bank, Header& header) const {
  if (esp_partition_read(partition_, uint32_t(bank) * BANK_BYTES, &header, sizeof(header)) != ESP_OK ||
      header.magic != MAGIC || header.schema != SCHEMA || header.count > MAX_ENTRIES || header.used > CAPACITY ||
      header.crc != crc32(reinterpret_cast<const uint8_t*>(&header) + 4, offsetof(Header, crc) - 4)) return false;
  uint32_t end = 0;
  for (size_t i = 0; i < header.count; ++i) {
    const Entry& entry = header.entries[i];
    if (!validId(entry.id) || !entry.length || entry.offset != end || entry.length > header.used - end) return false;
    for (size_t j = 0; j < i; ++j) if (!strcmp(entry.id, header.entries[j].id)) return false;
    end += entry.length;
  }
  if (end != header.used) return false;
  uint8_t digest[32];
  return hashPayload(bank, header.used, digest) && !memcmp(digest, header.sha256, sizeof(digest));
}

bool ZcsContentStore::begin() {
  abort();
  active_ = {};
  activeBank_ = -1;
  error_ = "";
  partition_ = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, "spiffs");
  if (!partition_ || partition_->size < 2 * BANK_BYTES) {
    partition_ = nullptr;
    return fail("storage_unavailable");
  }
  Header candidate;
  for (int bank = 0; bank < 2; ++bank) {
    if (loadBank(bank, candidate) && (activeBank_ < 0 || int32_t(candidate.generation - active_.generation) > 0)) {
      active_ = candidate;
      activeBank_ = bank;
    }
  }
  // Empty/corrupt data does not cause an erase. The rescue UI and LED logos remain available.
  return true;
}

const ZcsContentStore::Entry* ZcsContentStore::find(const char* id) const {
  if (!validId(id)) return nullptr;
  for (size_t i = 0; i < active_.count; ++i) if (!strcmp(id, active_.entries[i].id)) return &active_.entries[i];
  return nullptr;
}

bool ZcsContentStore::read(const char* id, uint32_t offset, void* out, size_t length) const {
  const Entry* entry = find(id);
  if (!partition_ || !entry || offset > entry->length || length > entry->length - offset || (!out && length)) return fail("read_bounds");
  return !length || esp_partition_read(partition_, payloadAddress(activeBank_, entry->offset + offset), out, length) == ESP_OK || fail("read_failed");
}

bool ZcsContentStore::start(const char* id, uint32_t length, const uint8_t* expected, bool removing) {
  if (writing_) return fail("busy");
  error_ = "";
  if (!partition_) return fail("storage_unavailable");
  if (!validId(id) || length > CAPACITY || (!removing && !length)) return fail("invalid_content");
  const Entry* old = find(id);
  if (removing && (!strcmp(id, "__logos") || !strcmp(id, "__web") || !old)) return fail("protected_or_missing");
  const uint32_t preserved = active_.used - (old ? old->length : 0);
  const size_t count = active_.count - (old ? 1 : 0) + (removing ? 0 : 1);
  if (length > CAPACITY - preserved || count > MAX_ENTRIES) return fail("storage_full");
  if (activeBank_ >= 0) {
    uint8_t actual[32];
    if (!hashPayload(activeBank_, active_.used, actual) || memcmp(actual, active_.sha256, 32)) return fail("storage_corrupt");
  }
  pending_ = {};
  pending_.magic = 0xFFFFFFFF;
  pending_.schema = SCHEMA;
  pending_.generation = active_.generation + 1;
  pending_.used = preserved + length;
  pendingBank_ = activeBank_ == 0 ? 1 : 0;
  expectedLength_ = length;
  received_ = 0;
  removing_ = removing;
  if (expected) memcpy(expectedSha_, expected, 32);
  mbedtls_sha256_init(&uploadHash_);
  mbedtls_sha256_init(&bankHash_);
  writing_ = true;
  if (mbedtls_sha256_starts(&uploadHash_, 0) || mbedtls_sha256_starts(&bankHash_, 0)) return stop("hash_failed");
  const uint32_t eraseBytes = (HEADER_BYTES + pending_.used + HEADER_BYTES - 1) & ~(HEADER_BYTES - 1);
  if (esp_partition_erase_range(partition_, uint32_t(pendingBank_) * BANK_BYTES, eraseBytes) != ESP_OK) return stop("erase_failed");
  uint32_t offset = 0;
  uint8_t block[512];
  for (size_t i = 0; i < active_.count; ++i) {
    const Entry& entry = active_.entries[i];
    if (!strcmp(id, entry.id)) continue;
    Entry& next = pending_.entries[pending_.count++];
    next = entry;
    next.offset = offset;
    for (uint32_t pos = 0; pos < entry.length;) {
      const size_t size = entry.length - pos < sizeof(block) ? entry.length - pos : sizeof(block);
      if (esp_partition_read(partition_, payloadAddress(activeBank_, entry.offset + pos), block, size) != ESP_OK ||
          esp_partition_write(partition_, payloadAddress(pendingBank_, offset), block, size) != ESP_OK ||
          mbedtls_sha256_update(&bankHash_, block, size)) return stop("copy_failed");
      pos += size;
      offset += size;
    }
  }
  pendingOffset_ = offset;
  if (!removing) {
    Entry& entry = pending_.entries[pending_.count++];
    memcpy(entry.id, id, strlen(id) + 1);
    entry.offset = offset;
    entry.length = length;
  }
  return true;
}

bool ZcsContentStore::beginPut(const char* id, uint32_t length, const char* sha256Hex) {
  uint8_t expected[32];
  if (!decodeSha(sha256Hex, expected)) return fail("invalid_sha256");
  return start(id, length, expected, false);
}

bool ZcsContentStore::write(const uint8_t* data, size_t length) {
  if (!writing_ || removing_) return fail("no_upload");
  if ((!data && length) || length > expectedLength_ - received_) return stop("upload_bounds");
  if (!length) return true;
  if (esp_partition_write(partition_, payloadAddress(pendingBank_, pendingOffset_ + received_), data, length) != ESP_OK ||
      mbedtls_sha256_update(&uploadHash_, data, length) || mbedtls_sha256_update(&bankHash_, data, length)) return stop("write_failed");
  received_ += length;
  return true;
}

bool ZcsContentStore::readPending(uint32_t offset, void* out, size_t length) const {
  if (!writing_ || removing_ || offset > received_ || length > received_ - offset || (!out && length)) return fail("pending_bounds");
  return !length || esp_partition_read(partition_, payloadAddress(pendingBank_, pendingOffset_ + offset), out, length) == ESP_OK || fail("read_failed");
}

bool ZcsContentStore::finish(Validator validator, void* context) {
  if (!writing_) return fail("no_upload");
  if (received_ != expectedLength_) return stop("upload_incomplete");
  uint8_t actual[32];
  if (!removing_ && (mbedtls_sha256_finish(&uploadHash_, actual) || memcmp(actual, expectedSha_, 32))) return stop("sha256_mismatch");
  if (validator && !validator(*this, context)) return stop("content_rejected");
  if (!writing_) return fail("upload_aborted");
  if (mbedtls_sha256_finish(&bankHash_, pending_.sha256) ||
      !hashPayload(pendingBank_, pending_.used, actual) || memcmp(actual, pending_.sha256, 32)) return stop("storage_verify_failed");
  pending_.crc = crc32(reinterpret_cast<const uint8_t*>(&pending_) + 4, offsetof(Header, crc) - 4);
  const uint32_t address = uint32_t(pendingBank_) * BANK_BYTES;
  if (esp_partition_write(partition_, address + 4, reinterpret_cast<const uint8_t*>(&pending_) + 4, sizeof(Header) - 4) != ESP_OK) return stop("header_failed");
  Header verified;
  if (esp_partition_read(partition_, address, &verified, sizeof(verified)) != ESP_OK ||
      memcmp(&verified, &pending_, sizeof(verified))) return stop("header_verify_failed");
  const uint32_t magic = MAGIC;
  if (esp_partition_write(partition_, address, &magic, sizeof(magic)) != ESP_OK) return stop("commit_failed");
  uint32_t committed = 0;
  if (esp_partition_read(partition_, address, &committed, sizeof(committed)) != ESP_OK || committed != magic) return stop("commit_verify_failed");
  pending_.magic = magic;
  active_ = pending_;
  activeBank_ = pendingBank_;
  abort();
  error_ = "";
  return true;
}

void ZcsContentStore::abort() {
  if (writing_) {
    mbedtls_sha256_free(&uploadHash_);
    mbedtls_sha256_free(&bankHash_);
  }
  writing_ = false;
  pendingBank_ = -1;
}

bool ZcsContentStore::remove(const char* id) {
  return start(id, 0, nullptr, true) && finish();
}
