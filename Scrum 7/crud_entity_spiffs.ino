#include "SPIFFS.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <time.h>

// Konfigurasi Wi-Fi
const char* ssid = "UmmuHadi-2F";
const char* password = "238988rizami";

// UUIDs yang tersedia
const char* uuid1 = "123e4567-e89b-12d3-a456-426614174000";
const char* uuid2 = "223a8299-t98f-a18k-156h-123812938199";

// Fungsi untuk menghubungkan ke Wi-Fi
void connectToWiFi() {
  Serial.print("Menghubungkan ke Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nTerhubung ke Wi-Fi");
}

// Fungsi untuk menginisialisasi waktu (NTP)
void initializeTime() {
  configTime(25200, 0, "pool.ntp.org");  // Konfigurasi dengan offset GMT+7 (25200 detik) dan tanpa DST (Daylight Saving Time)
  Serial.print("Menunggu sinkronisasi waktu");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWaktu telah disinkronisasi.");
}

// Fungsi untuk mendapatkan epoch time
long getEpochTime() {
  time_t now;
  time(&now);
  return now;
}

void setup() {
  Serial.begin(115200);
  SPIFFS.begin(true);

  // Hubungkan ke Wi-Fi
  connectToWiFi();

  // Inisialisasi waktu
  initializeTime();
}

void loop() {
  // Tambahkan perintah untuk petunjuk penggunaan
  Serial.println("\nMasukkan perintah (a = Add, r = Read, e = Remove, d = Delete): ");
  while (!Serial.available());
  String command = Serial.readStringUntil('\n');
  command.trim(); // Menghapus spasi atau karakter newline di awal/akhir string

  if (command.equalsIgnoreCase("a")) {
    addJsonData();
  } else if (command.equalsIgnoreCase("r")) {
    readJsonFile();
  } else if (command.equalsIgnoreCase("e")) {
    removeJsonObject();
  } else if (command.equalsIgnoreCase("d")) {
    deleteJsonFile();
  } else {
    Serial.println("Perintah tidak dikenali!");
  }
 
}

void addJsonData() {
  Serial.println("\nPilih UUID (1 untuk UUID1, 2 untuk UUID2): ");
  while (!Serial.available());

  String uuidSelection = Serial.readStringUntil('\n');
  uuidSelection.trim();

  const char* selectedUUID;

  if (uuidSelection.equals("1")) {
    selectedUUID = uuid1;
  } else if (uuidSelection.equals("2")) {
    selectedUUID = uuid2;
  } else {
    Serial.println("Perintah tidak dikenali! Keluar dari fungsi addJsonData.");
    return;
  }

  // Baca file JSON yang ada atau buat JSON baru jika file tidak ada
  File file = SPIFFS.open("/data.json", FILE_READ);
  StaticJsonDocument<1024> jsonDoc;
  if (file) {
    DeserializationError error = deserializeJson(jsonDoc, file);
    if (error) {
      Serial.println("Gagal mengurai file JSON. Membuat JSON baru.");
    }
    file.close();
  } else {
    Serial.println("File JSON tidak ditemukan. Membuat JSON baru.");
  }

  // Pastikan ada array di bawah kunci UUID yang dipilih
  JsonArray jsonArray;
  if (jsonDoc.containsKey(selectedUUID)) {
    jsonArray = jsonDoc[selectedUUID].as<JsonArray>();
  } else {
    jsonArray = jsonDoc.createNestedArray(selectedUUID);
  }

  // Buat objek JSON baru dan tambahkan ke array
  JsonObject newEntry = jsonArray.createNestedObject();
  newEntry["timestamp"] = getEpochTime();

  JsonObject data = newEntry.createNestedObject("data");
  if (strcmp(selectedUUID, uuid1) == 0) {
    // UUID1: Tambahkan objek dengan struktur untuk UUID1
    data["type_connection"] = "RTU";
    data["data_type"] = "Coils";
    data["id"] = jsonArray.size(); // ID berdasarkan jumlah elemen dalam array (berdasarkan urutan)
    data["address"] = "0x01";
    data["value"] = "0xFF";
  } else if (strcmp(selectedUUID, uuid2) == 0) {
    // UUID2: Tambahkan objek dengan struktur untuk UUID2
    data["connection_method"] = "Ethernet";
    data["mac_address"] = "00:1B:44:11:3A:B7";
    data["ip_address"] = "192.168.1.100";
  }

  // Simpan JSON ke file
  file = SPIFFS.open("/data.json", FILE_WRITE);
  if (!file) {
    Serial.println("Gagal membuka file untuk ditulis.");
    return;
  }
  serializeJson(jsonDoc, file);
  file.close();
  Serial.println("Data JSON baru telah ditambahkan.");
}

void readJsonFile() {
  File file = SPIFFS.open("/data.json", FILE_READ);
  if (!file) {
    Serial.println("Gagal membuka file untuk dibaca.");
    return;
  }

  // Baca file JSON
  String jsonString = file.readString();
  file.close();

  // Deserialize JSON dari string
  StaticJsonDocument<1024> jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, jsonString);

  if (error) {
    Serial.println("Gagal mengurai file JSON.");
    return;
  }

  // Tampilkan JSON ke Serial Monitor
  serializeJsonPretty(jsonDoc, Serial);
  Serial.println("\nSelesai membaca file JSON.");
}

void removeJsonObject() {
  Serial.println("\nPilih UUID (1 untuk UUID1, 2 untuk UUID2): ");
  while (!Serial.available());

  String uuidSelection = Serial.readStringUntil('\n');
  uuidSelection.trim();

  const char* selectedUUID;

  if (uuidSelection.equals("1")) {
    selectedUUID = uuid1;
  } else if (uuidSelection.equals("2")) {
    selectedUUID = uuid2;
  } else {
    Serial.println("Perintah tidak dikenali! Keluar dari fungsi removeJsonObject.");
    return;
  }

  // Baca file JSON yang ada
  File file = SPIFFS.open("/data.json", FILE_READ);
  StaticJsonDocument<1024> jsonDoc;
  if (file) {
    DeserializationError error = deserializeJson(jsonDoc, file);
    if (error) {
      Serial.println("Gagal mengurai file JSON.");
      file.close();
      return;
    }
    file.close();
  } else {
    Serial.println("File JSON tidak ditemukan.");
    return;
  }

  // Pastikan dokumen memiliki kunci UUID yang dipilih
  if (!jsonDoc.containsKey(selectedUUID)) {
    Serial.println("UUID tidak ditemukan.");
    return;
  }

  JsonArray jsonArray = jsonDoc[selectedUUID].as<JsonArray>();

  // Minta timestamp yang akan dihapus
  Serial.println("Masukkan timestamp yang ingin dihapus: ");
  while (!Serial.available());

  String timestampString = Serial.readStringUntil('\n');
  timestampString.trim();
  long timestampToRemove = timestampString.toInt();

  // Cari dan hapus objek dengan timestamp yang sesuai
  bool removed = false;
  for (size_t i = 0; i < jsonArray.size(); i++) {
    JsonObject obj = jsonArray[i];
    if (obj["timestamp"] == timestampToRemove) {
      jsonArray.remove(i);
      removed = true;
      break;
    }
  }

  if (removed) {
    // Simpan kembali JSON yang telah diperbarui ke file
    file = SPIFFS.open("/data.json", FILE_WRITE);
    if (!file) {
      Serial.println("Gagal membuka file untuk ditulis.");
      return;
    }
    serializeJson(jsonDoc, file);
    file.close();
    Serial.println("Objek dengan timestamp tersebut telah dihapus.");
  } else {
    Serial.println("Objek dengan timestamp tersebut tidak ditemukan.");
  }
}

void deleteJsonFile() {
  if (SPIFFS.remove("/data.json")) {
    Serial.println("File JSON telah dihapus.");
  } else {
    Serial.println("Gagal menghapus file JSON.");
  }
}
