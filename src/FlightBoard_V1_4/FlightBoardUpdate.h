#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <WebServer.h>
#include <esp_app_desc.h>
#include <esp_app_format.h>
#include <esp_ota_ops.h>
#include <esp_random.h>
#include <esp_timer.h>
#include <vector>
#include "FlightBoardRelease.h"
#include "FlightBoardHashGzip.h"

// Mise a jour locale : une seule image d'application, sur la partition inactive.
// Le jeton protege les formulaires d'autres sites ; ce n'est pas un compte utilisateur.
class FlightBoardUpdate {
public:
  explicit FlightBoardUpdate(WebServer& server) : server_(server) {}

  void setPrepareCheck(bool (*check)()) { prepareCheck_ = check; }

  void begin() {
    char token[33];
    snprintf(token, sizeof(token), "%08lx%08lx%08lx%08lx",
             (unsigned long)esp_random(), (unsigned long)esp_random(),
             (unsigned long)esp_random(), (unsigned long)esp_random());
    token_ = token;

    // collectHeaders remplace la liste : conserver les cles deja enregistrees.
    std::vector<String> names;
    for (int i = 0; i < server_.headers(); ++i) names.push_back(server_.headerName(i));
    names.push_back("X-Update-Token");
    names.push_back("X-Firmware-Size");
    std::vector<const char*> headers;
    for (const String& name : names) headers.push_back(name.c_str());
    server_.collectHeaders(headers.data(), headers.size());

    server_.on("/api/system", HTTP_GET, [this]() { sendSystem(); });
    server_.on("/api/update-hash.js", HTTP_GET, [this]() {
      server_.sendHeader("Cache-Control", "no-cache");
      server_.sendHeader("Content-Encoding", "gzip");
      server_.sendHeader("X-Content-Type-Options", "nosniff");
      server_.send_P(200, "application/javascript", (const char*)FLIGHTBOARD_HASH_GZIP,
                    sizeof(FLIGHTBOARD_HASH_GZIP));
    });
    server_.on("/api/update", HTTP_POST,
               [this]() { finishRequest(); }, [this]() { receiveUpload(); });
  }

  void update() {
    if (restartPending_) {
      if (millis() - restartAt_ >= 1800UL) ESP.restart();
    } else if (requestSeen_ && millis() - lastChunkAt_ > 30000UL) {
      // Secours si le parseur quitte une requete tronquee sans callback final.
      fail(408, "Transfert interrompu. Recommence la mise a jour.");
      clearRequest();
    }
  }

  bool busy() const { return receiving_ || restartPending_; }

private:
  // L'Arduino ESP32 fournit souvent project_name=arduino-lib-builder dans app_desc.
  // Ce marqueur stable identifie donc les vrais firmwares Flight Board V2+.
  static const char* firmwareFamily() { return "FlightBoard.ESP32S3.Application.v2"; }
  static constexpr size_t PREFIX_SIZE = sizeof(esp_image_header_t) +
                                        sizeof(esp_image_segment_header_t) +
                                        sizeof(esp_app_desc_t);
  WebServer& server_;
  bool (*prepareCheck_)() = nullptr;
  String token_, error_;
  int errorStatus_ = 400;
  bool requestSeen_ = false, receiving_ = false, fileComplete_ = false;
  bool updateStarted_ = false, restartPending_ = false, prefixValid_ = false;
  bool identityFound_ = false, layoutComplete_ = false;
  unsigned long lastChunkAt_ = 0, restartAt_ = 0;
  size_t expectedSize_ = 0, receivedSize_ = 0, prefixSize_ = 0;
  size_t markerMatched_ = 0, nextSegmentAt_ = 0;
  uint8_t segmentsRemaining_ = 0, hashAppended_ = 0;
  uint8_t prefix_[PREFIX_SIZE] = {}, segmentHeader_[sizeof(esp_image_segment_header_t)] = {};

  static size_t firmwareCapacity() {
    const esp_partition_t* next = esp_ota_get_next_update_partition(nullptr);
    const esp_partition_t* running = esp_ota_get_running_partition();
    const esp_partition_t* otaData = esp_partition_find_first(
      ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_OTA, nullptr);
    if (!next || !running || !otaData || next->address == running->address ||
        next->type != ESP_PARTITION_TYPE_APP ||
        next->subtype < ESP_PARTITION_SUBTYPE_APP_OTA_MIN ||
        next->subtype >= ESP_PARTITION_SUBTYPE_APP_OTA_MAX) return 0;
    return next->size;
  }

  bool validToken() const {
    const String supplied = server_.header("X-Update-Token");
    if (supplied.length() != token_.length() || token_.isEmpty()) return false;
    volatile uint8_t difference = 0;
    for (size_t i = 0; i < token_.length(); ++i) {
      difference = difference | (uint8_t)(supplied[i] ^ token_[i]);
    }
    return difference == 0;
  }

  static bool parseSize(const String& text, size_t& result) {
    if (text.isEmpty() || text.length() > 10) return false;
    result = 0;
    for (size_t i = 0; i < text.length(); ++i) {
      if (text[i] < '0' || text[i] > '9') return false;
      const size_t digit = (size_t)(text[i] - '0');
      if (result > (SIZE_MAX - digit) / 10) return false;
      result = result * 10 + digit;
    }
    return result > PREFIX_SIZE;
  }

  void sendJson(int status, JsonDocument& doc) {
    String body;
    serializeJson(doc, body);
    server_.sendHeader("Cache-Control", "no-store");
    server_.sendHeader("X-Content-Type-Options", "nosniff");
    server_.send(status, "application/json; charset=utf-8", body);
  }

  void sendSystem() {
    JsonDocument doc;
    const size_t capacity = firmwareCapacity();
    doc["version"] = "4.1.2";
    doc["firmwareFamily"] = firmwareFamily();
    doc["releaseManifestUrl"] = FLIGHTBOARD_RELEASE_MANIFEST_URL;
    doc["uptimeSeconds"] = (uint64_t)esp_timer_get_time() / 1000000ULL;
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["otaSupported"] = capacity > 0;
    doc["maxFirmwareBytes"] = capacity;
    doc["updateToken"] = token_;
    doc["updateBusy"] = busy();
    sendJson(200, doc);
  }

  void fail(int status, const String& message) {
    if (error_.isEmpty()) {
      error_ = message;
      errorStatus_ = status;
    }
    if (updateStarted_) Update.abort();
    updateStarted_ = false;
    receiving_ = false;
    fileComplete_ = false;
  }

  void clearRequest() {
    requestSeen_ = receiving_ = fileComplete_ = updateStarted_ = prefixValid_ = false;
    identityFound_ = layoutComplete_ = false;
    expectedSize_ = receivedSize_ = prefixSize_ = markerMatched_ = nextSegmentAt_ = 0;
    segmentsRemaining_ = hashAppended_ = 0;
    error_ = "";
    errorStatus_ = 400;
  }

  bool checkImageLength(size_t dataEnd) {
    if (dataEnd > expectedSize_ || expectedSize_ - dataEnd < 1) return false;
    const size_t paddedEnd = (dataEnd + 16U) & ~(size_t)15U;
    const size_t imageEnd = paddedEnd + (hashAppended_ ? 32U : 0U);
    layoutComplete_ = imageEnd == expectedSize_;
    return layoutComplete_;
  }

  bool inspectPrefix() {
    esp_image_header_t header;
    esp_image_segment_header_t segment;
    esp_app_desc_t description;
    memcpy(&header, prefix_, sizeof(header));
    memcpy(&segment, prefix_ + sizeof(header), sizeof(segment));
    memcpy(&description, prefix_ + sizeof(header) + sizeof(segment), sizeof(description));
    if (header.magic != ESP_IMAGE_HEADER_MAGIC || header.chip_id != ESP_CHIP_ID_ESP32S3 ||
        header.segment_count < 1 || header.segment_count > ESP_IMAGE_MAX_SEGMENTS ||
        header.hash_appended > 1 || description.magic_word != ESP_APP_DESC_MAGIC_WORD ||
        segment.data_len < sizeof(esp_app_desc_t) ||
        segment.data_len > expectedSize_ - sizeof(header) - sizeof(segment)) return false;
    segmentsRemaining_ = header.segment_count - 1;
    hashAppended_ = header.hash_appended;
    nextSegmentAt_ = sizeof(header) + sizeof(segment) + segment.data_len;
    if (!segmentsRemaining_ && !checkImageLength(nextSegmentAt_)) return false;
    prefixValid_ = true;
    return true;
  }

  bool inspectBytes(const uint8_t* data, size_t length) {
    const char* marker = firmwareFamily();
    const size_t markerLength = strlen(marker);
    for (size_t i = 0; i < length; ++i) {
      const size_t offset = receivedSize_ + i;
      if (!identityFound_) {
        markerMatched_ = data[i] == (uint8_t)marker[markerMatched_]
          ? markerMatched_ + 1 : (data[i] == (uint8_t)marker[0] ? 1 : 0);
        identityFound_ = markerMatched_ == markerLength;
      }
      if (!prefixValid_) {
        prefix_[prefixSize_++] = data[i];
        if (prefixSize_ == PREFIX_SIZE && !inspectPrefix()) return false;
      } else if (segmentsRemaining_ && offset >= nextSegmentAt_) {
        const size_t index = offset - nextSegmentAt_;
        if (index >= sizeof(segmentHeader_)) return false;
        segmentHeader_[index] = data[i];
        if (index + 1 == sizeof(segmentHeader_)) {
          esp_image_segment_header_t segment;
          memcpy(&segment, segmentHeader_, sizeof(segment));
          const size_t dataAt = offset + 1;
          if (dataAt > expectedSize_ || segment.data_len > expectedSize_ - dataAt) return false;
          nextSegmentAt_ = dataAt + segment.data_len;
          if (!--segmentsRemaining_ && !checkImageLength(nextSegmentAt_)) return false;
        }
      }
    }
    return true;
  }

  void startUpload(HTTPUpload& upload) {
    if (restartPending_) return;
    if (requestSeen_) {
      fail(400, "Envoie un seul fichier de programme par mise a jour.");
      return;
    }
    clearRequest();
    requestSeen_ = true;
    lastChunkAt_ = millis();
    if (!validToken()) {
      fail(403, "Session expiree. Recharge la page avant de recommencer.");
      return;
    }
    String filename = upload.filename;
    filename.toLowerCase();
    if (upload.name != "firmware" || !filename.endsWith(".bin")) {
      fail(400, "Choisis le fichier .ino.bin du programme Flight Board.");
      return;
    }
    const size_t capacity = firmwareCapacity();
    if (!capacity) {
      fail(409, "Mise a jour Wi-Fi indisponible avec ce partitionnement. Utilise le cable USB.");
      return;
    }
    if (!parseSize(server_.header("X-Firmware-Size"), expectedSize_) || expectedSize_ > capacity) {
      fail(413, "Taille du programme invalide ou superieure a la place disponible.");
      return;
    }
    if (prepareCheck_ && !prepareCheck_()) {
      fail(503, "Sauvegarde du Wi-Fi impossible ou connexion en cours. Reessaie une fois le reseau connecte.");
      return;
    }
    receiving_ = true;
    // Attendre l'en-tete d'application valide avant toute ecriture en flash.
  }

  void writeUpload(HTTPUpload& upload) {
    if (!receiving_ || !error_.isEmpty() || restartPending_) return;
    lastChunkAt_ = millis();
    if (upload.currentSize > expectedSize_ - receivedSize_) {
      fail(413, "Le fichier depasse la taille annoncee. Transfert annule.");
      return;
    }
    const size_t oldPrefixSize = prefixSize_;
    if (!inspectBytes(upload.buf, upload.currentSize)) {
      fail(400, "Image incompatible. Utilise le .ino.bin Flight Board pour ESP32-S3, sans merged ni bootloader.");
      return;
    }
    if (prefixValid_ && !updateStarted_) {
      if (!Update.begin(expectedSize_, U_FLASH)) {
        fail(500, "Impossible de preparer la mise a jour. Reessaie apres un redemarrage.");
        return;
      }
      updateStarted_ = true;
      if (oldPrefixSize && Update.write(prefix_, oldPrefixSize) != oldPrefixSize) {
        fail(500, "Echec de l'ecriture en memoire. Programme actuel conserve.");
        return;
      }
    }
    if (updateStarted_ && Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      fail(500, "Echec de l'ecriture en memoire. Programme actuel conserve.");
      return;
    }
    receivedSize_ += upload.currentSize;
  }

  void receiveUpload() {
    HTTPUpload& upload = server_.upload();
    if (restartPending_) return;
    switch (upload.status) {
      case UPLOAD_FILE_START: startUpload(upload); break;
      case UPLOAD_FILE_WRITE: writeUpload(upload); break;
      case UPLOAD_FILE_END:
        if (receiving_ && error_.isEmpty()) {
          lastChunkAt_ = millis();
          if (upload.totalSize != expectedSize_ || receivedSize_ != expectedSize_ ||
              !prefixValid_ || !layoutComplete_ || !identityFound_ || !updateStarted_) {
            fail(400, "Fichier incomplet ou ne correspondant pas a Flight Board V2. Transfert annule.");
          } else {
            fileComplete_ = true;
          }
        }
        // Ne pas activer ici : il peut rester une seconde partie ou un corps tronque.
        break;
      case UPLOAD_FILE_ABORTED:
        fail(400, "Transfert interrompu. Programme actuel conserve.");
        clearRequest();
        break;
    }
  }

  void finishRequest() {
    if (restartPending_) {
      JsonDocument doc;
      doc["error"] = "Redemarrage en cours. Attends quelques secondes.";
      sendJson(409, doc);
      return;
    }
    if (!validToken()) fail(403, "Session expiree. Recharge la page avant de recommencer.");
    if (!requestSeen_ || !fileComplete_ || !updateStarted_ || server_.args() != 0) {
      fail(400, "Envoie uniquement un fichier .ino.bin Flight Board complet.");
    }
    if (error_.isEmpty()) {
      // end(false) exige la taille exacte et verifie l'image avant de modifier le demarrage.
      if (!Update.end(false)) {
        fail(400, "Verification du programme echouee. Programme actuel conserve.");
      } else {
        updateStarted_ = false;
        receiving_ = false;
        restartPending_ = true;
        restartAt_ = millis();
      }
    }
    JsonDocument doc;
    if (restartPending_) {
      doc["ok"] = true;
      doc["message"] = "Programme verifie. Le Flight Board redemarre.";
      doc["restarting"] = true;
      sendJson(200, doc);
    } else {
      doc["error"] = error_;
      sendJson(errorStatus_, doc);
      clearRequest();
    }
  }
};
