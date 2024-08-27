#include "SPIFFS.h"
#include <ArduinoJson.h>

// UUIDs yang tersedia
const char* uuid1 = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
const char* uuid2 = "aeb5483e-36e1-4688-b7f5-ea07361b26b9";
const char* uuid3 = "ceb5483e-36e1-4688-b7f5-ea07361b26c0";
const char* uuid4 = "deb5483e-36e1-4688-b7f5-ea07361b26d1";

void setup() {
  Serial.begin(115200);
  initSPIFFS();
}

void loop() {
  Serial.println("Masukkan perintah (c = Create, r = Read, u = Update, d = Delete, x = Remove): ");
  while (!Serial.available());
  String command = Serial.readStringUntil('\n');
  command.trim();

  if (command.equalsIgnoreCase("c")) {
    createJsonData();
  } else if (command.equalsIgnoreCase("r")) {
    readJsonFile();
  } else if (command.equalsIgnoreCase("u")) {
    updateJsonData();
  } else if (command.equalsIgnoreCase("d")) {
    deleteJsonData();
  } else if (command.equalsIgnoreCase("x")) {
    removeJsonFile();
  } else {
    Serial.println("Perintah tidak dikenali!");
  }
}

void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Gagal memulai SPIFFS.");
    while (true); // Stop execution if SPIFFS initialization fails
  }
}

const char* getSelectedUUID() {
  Serial.println("\nPilih UUID (1, 2, 3, 4): ");
  while (!Serial.available());
  String uuidSelection = Serial.readStringUntil('\n');
  uuidSelection.trim();

  if (uuidSelection.equals("1")) return uuid1;
  if (uuidSelection.equals("2")) return uuid2;
  if (uuidSelection.equals("3")) return uuid3;
  if (uuidSelection.equals("4")) return uuid4;
  
  Serial.println("UUID tidak valid.");
  return nullptr;
}

void createJsonData() {
  const char* selectedUUID = getSelectedUUID();
  if (!selectedUUID) return;

  File file = SPIFFS.open("/data.json", FILE_READ);
  StaticJsonDocument<2048> jsonDoc;

  // Inisialisasi file JSON jika kosong atau tidak ada
  if (!file || file.size() == 0) {
    Serial.println("File JSON kosong atau tidak ditemukan, membuat file baru.");
    file.close();
    file = SPIFFS.open("/data.json", FILE_WRITE);
    file.print("{}"); // Tambahkan objek JSON kosong
    file.close();
    file = SPIFFS.open("/data.json", FILE_READ);
  }

  DeserializationError error = deserializeJson(jsonDoc, file);
  file.close();

  if (error) {
    Serial.print("Gagal mengurai file JSON: ");
    Serial.println(error.c_str());
    return;
  }

  JsonArray jsonArray;
  if (jsonDoc.containsKey(selectedUUID)) {
    jsonArray = jsonDoc[selectedUUID].as<JsonArray>();
  } else {
    jsonArray = jsonDoc.createNestedArray(selectedUUID);
  }

  Serial.println("Masukkan data dalam format JSON (contoh: {\"device_name\":\"value\",\"connection_type\":\"value\"}):");
  while (!Serial.available());
  String jsonDataStr = Serial.readStringUntil('\n');

  StaticJsonDocument<256> newData;
  error = deserializeJson(newData, jsonDataStr);
  if (error) {
    Serial.print("Gagal mengurai data JSON baru: ");
    Serial.println(error.c_str());
    return;
  }

  jsonArray.add(newData);

  file = SPIFFS.open("/data.json", FILE_WRITE);
  if (!file) {
    Serial.println("Gagal membuka file untuk ditulis.");
    return;
  }
  serializeJson(jsonDoc, file);
  file.close();
  Serial.println("Data baru berhasil ditambahkan.");
}

void readJsonFile() {
  File file = SPIFFS.open("/data.json", FILE_READ);
  if (!file) {
    Serial.println("Gagal membuka file untuk dibaca.");
    return;
  }

  String jsonString = file.readString();
  file.close();

  StaticJsonDocument<2048> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, jsonString);

  if (error) {
    Serial.println("Gagal mengurai file JSON.");
    return;
  }

  serializeJsonPretty(jsonDoc, Serial);
  Serial.println("\nSelesai membaca file JSON.");
}

void updateJsonData() {
  const char* selectedUUID = getSelectedUUID();
  if (!selectedUUID) return;

  File file = SPIFFS.open("/data.json", FILE_READ);
  StaticJsonDocument<2048> jsonDoc;

  if (file) {
    DeserializationError error = deserializeJson(jsonDoc, file);
    file.close();
    if (error) {
      Serial.println("Gagal mengurai file JSON.");
      return;
    }
  } else {
    Serial.println("File JSON tidak ditemukan.");
    return;
  }

  if (!jsonDoc.containsKey(selectedUUID)) {
    Serial.println("UUID tidak ditemukan.");
    return;
  }

  JsonArray jsonArray = jsonDoc[selectedUUID].as<JsonArray>();

  Serial.println("Masukkan data dalam format JSON (contoh: {\"device_name\":\"value\",\"connection_type\":\"value\"}):");
  while (!Serial.available());
  String jsonDataStr = Serial.readStringUntil('\n');

  StaticJsonDocument<512> updatedData;
  DeserializationError error = deserializeJson(updatedData, jsonDataStr);
  if (error) {
    Serial.println("Gagal mengurai data JSON baru.");
    return;
  }

  bool updated = false;

  if (selectedUUID == uuid1) {
    const char* newDeviceName = updatedData["device_name"];
    for (JsonObject obj : jsonArray) {
      if (obj["device_name"] == newDeviceName) {
        obj.set(updatedData.as<JsonObject>());
        updated = true;
        break;
      }
    }
  } else if (selectedUUID == uuid2) {
    const char* newDataName = updatedData["data_name"];
    for (JsonObject obj : jsonArray) {
      if (obj["data_name"] == newDataName) {
        obj.set(updatedData.as<JsonObject>());
        updated = true;
        break;
      }
    }
  } else if (selectedUUID == uuid3 || selectedUUID == uuid4) {
    if (!jsonArray.isNull() && jsonArray.size() > 0) {
      jsonArray[0] = updatedData;
      updated = true;
    }
  }

  if (!updated) {
    Serial.println("Data tidak ditemukan atau UUID tidak sesuai.");
    return;
  }

  file = SPIFFS.open("/data.json", FILE_WRITE);
  if (!file) {
    Serial.println("Gagal membuka file untuk ditulis.");
    return;
  }
  serializeJson(jsonDoc, file);
  file.close();
  Serial.println("Data berhasil diperbarui.");
}

void deleteJsonData() {
  const char* selectedUUID = getSelectedUUID();
  if (!selectedUUID) return;

  File file = SPIFFS.open("/data.json", FILE_READ);
  StaticJsonDocument<2048> jsonDoc;

  if (file) {
    DeserializationError error = deserializeJson(jsonDoc, file);
    file.close();
    if (error) {
      Serial.println("Gagal mengurai file JSON.");
      return;
    }
  } else {
    Serial.println("File JSON tidak ditemukan.");
    return;
  }

  if (!jsonDoc.containsKey(selectedUUID)) {
    Serial.println("UUID tidak ditemukan.");
    return;
  }

  JsonArray jsonArray = jsonDoc[selectedUUID].as<JsonArray>();

  if (selectedUUID == uuid1) {
    Serial.println("Masukkan objek JSON untuk dihapus:");
    while (!Serial.available());
    String inputJson = Serial.readStringUntil('\n');
    inputJson.trim();

    StaticJsonDocument<256> inputDoc;
    DeserializationError error = deserializeJson(inputDoc, inputJson);
    if (error) {
      Serial.println("Gagal mengurai objek JSON input.");
      return;
    }

    String deviceNameToDelete = inputDoc["device_name"].as<String>();

    bool deleted = false;
    for (int i = 0; i < jsonArray.size(); i++) {
      JsonObject obj = jsonArray[i];
      if (obj["device_name"] == deviceNameToDelete) {
        jsonArray.remove(i);
        deleted = true;
        break;
      }
    }

    if (!deleted) {
      Serial.println("device_name tidak ditemukan.");
      return;
    }

  } else if (selectedUUID == uuid2) {
    Serial.println("Masukkan objek JSON untuk dihapus:");
    while (!Serial.available());
    String inputJson = Serial.readStringUntil('\n');
    inputJson.trim();

    StaticJsonDocument<256> inputDoc;
    DeserializationError error = deserializeJson(inputDoc, inputJson);
    if (error) {
      Serial.println("Gagal mengurai objek JSON input.");
      return;
    }

    String dataNameToDelete = inputDoc["data_name"].as<String>();

    bool deleted = false;
    for (int i = 0; i < jsonArray.size(); i++) {
      JsonObject obj = jsonArray[i];
      if (obj["data_name"] == dataNameToDelete) {
        jsonArray.remove(i);
        deleted = true;
        break;
      }
    }

    if (!deleted) {
      Serial.println("data_name tidak ditemukan.");
      return;
    }

  } else if (selectedUUID == uuid3 || selectedUUID == uuid4) {
    if (jsonArray.size() > 0) {
      jsonArray.remove(0);
    } else {
      Serial.println("Array kosong.");
      return;
    }
  }

  file = SPIFFS.open("/data.json", FILE_WRITE);
  if (!file) {
    Serial.println("Gagal membuka file untuk ditulis.");
    return;
  }
  serializeJson(jsonDoc, file);
  file.close();
  Serial.println("Data berhasil dihapus.");
}

void removeJsonFile() {
  if (SPIFFS.remove("/data.json")) {
    Serial.println("File JSON berhasil dihapus.");
  } else {
    Serial.println("Gagal menghapus file JSON.");
  }
}
