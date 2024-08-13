#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include "time.h"
#include "mbedtls/md.h"

// Secret key
#define SECRET_KEY "RIZMI"

// UUIDs for BLE Service and Characteristics
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// Function to get current time
String getCurrentTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time, using default time");
    return "000000"; // Default time if NTP time sync fails
  }
  char timeStr[20];
  strftime(timeStr, sizeof(timeStr), "%H%M%d%m%Y", &timeinfo);
  return String(timeStr);
}

// Function to format MAC Address
String formatMacAddress(String macAddress) {
  macAddress.replace(":", "");
  macAddress.toLowerCase();
  return macAddress;
}

// Function to generate passkey
String generatePasskey(String macAddress) {
  String currentTime = getCurrentTime();
  macAddress = formatMacAddress(macAddress); // Format MAC address
  String combinedString = macAddress + SECRET_KEY + currentTime;
  Serial.print("Combined String: ");
  Serial.println(combinedString);

  // Hash using SHA-256
  byte shaResult[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char*)combinedString.c_str(), combinedString.length());
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);

  // Take the first 6 digits of the hash as the passkey
  char passkey[7];
  sprintf(passkey, "%02x%02x%02x", shaResult[0], shaResult[1], shaResult[2]);
  return String(passkey);
}

// BLE Server Callbacks to handle connection and disconnection
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected");
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected");
      pServer->getAdvertising()->start(); // Restart advertising
    }
};

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi for NTP
  WiFi.begin("UmmuHadi-2F", "238988rizami");  // Change to your WiFi credentials
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set up NTP
  configTime(25200, 0, "pool.ntp.org", "time.nist.gov");

  // Wait for NTP time sync
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting for NTP time sync...");
    delay(1000);
  }
  
  // Initialize BLE
  BLEDevice::init("ESP32 BLE Secure Server");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  pServer->getAdvertising()->start();
  
  // Generate initial passkey
  String macAddress = BLEDevice::getAddress().toString().c_str();
  String passkey = generatePasskey(macAddress);
  Serial.print("Generated Passkey: ");
  Serial.println(passkey);
}

void loop() {
  // Check connection status
  if (deviceConnected) {
    String receivedPasskey = pCharacteristic->getValue().c_str();

    // Check if received passkey is not NULL
    if (receivedPasskey.length() > 0) {
      String macAddress = BLEDevice::getAddress().toString().c_str();
      String generatedPasskey = generatePasskey(macAddress);
    
      if (receivedPasskey == generatedPasskey) {
        Serial.println("Authentication successful, passkeys match!");
        // Proceed with authenticated connection logic
      } else {
        Serial.println("Authentication failed, passkeys do not match!");
      }
      
      // Clear the received passkey after processing
      pCharacteristic->setValue(""); 
    }
  }
  delay(1000);
}
