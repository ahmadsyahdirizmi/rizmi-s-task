#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <WiFi.h>
#include <mbedtls/md.h>
#include "time.h"
#include "mbedtls/aes.h"
#include "mbedtls/base64.h"

// AES Key and IV - same as on the client side
unsigned char aes_key[16] = {'B', 'a', 'n', 'd', 'u', 'n', 'g', ' ', 'B', 'o', 'n', 'd', 'o', 'w', 'o', 's'};
unsigned char iv[16] = {'b', 'a', 'n', 'd', 'u', 'n', 'g', ' ', 'b', 'o', 'n', 'd', 'o', 'w', 'o', 's'};

const char* ssid = "Wi-Fi SSID";
const char* password = "Wi-Fi Password";
const char* secretKey = "SURIOTA";

uint32_t PASSKEY;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  Serial.println("Connected to WiFi");
  configTime(25200, 0, "pool.ntp.org");  // Configure time via NTP
  bleInit();
}

void loop() {}

/////////////////////
//BLE Secure Server//
/////////////////////

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
      std::string receivedData = pCharacteristic->getValue();
      String decryptedData = decryptData(receivedData);
      Serial.print("Decrypted data: ");
      Serial.println(decryptedData);
    }
};

class ServerCallback: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println(" - ServerCallback - onConnect");
      String macAddress = BLEDevice::getAddress().toString().c_str();
      PASSKEY = generatePasskey(macAddress);
      uint32_t passkey = PASSKEY;
      esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println(" - ServerCallback - onDisconnect");
    }
};

class SecurityCallback : public BLESecurityCallbacks {
  uint32_t onPassKeyRequest() {    
    return PASSKEY;
  }

  void onPassKeyNotify(uint32_t pass_key) {
    Serial.print("     - Generated Passkey: ");
    Serial.println(pass_key);
  }

  bool onConfirmPIN(uint32_t pass_key) {
    vTaskDelay(5000);
    return true;
  }

  bool onSecurityRequest() {
    return true;
  }

  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
    if(cmpl.success) {
      Serial.println("   - SecurityCallback - Authentication Success");       
    } else {
      Serial.println("   - SecurityCallback - Authentication Failure*");
      pServer->removePeerDevice(pServer->getConnId(), true);
    }
    BLEDevice::startAdvertising();
  }
};

void bleSecurity() {
  esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;
  esp_ble_io_cap_t iocap = ESP_IO_CAP_OUT;          
  uint8_t key_size = 16;     
  uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
  
  esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));
}

void bleInit() {
  BLEDevice::init("BLE-Secure-Server");
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  BLEDevice::setSecurityCallbacks(new SecurityCallback());

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallback());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY 
                    );

  pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();

  bleSecurity();
}

////////////////////
// Additional Functions //
////////////////////

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

String formatMacAddress(String macAddress) {
  macAddress.replace(":", "");
  macAddress.toLowerCase();
  return macAddress;
}

int generatePasskey(String macAddress) {
  String currentTime = getCurrentTime();
  macAddress = formatMacAddress(macAddress);
  String combinedString = macAddress + secretKey + currentTime;
  Serial.print("     - Combined String: ");
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

  // Convert the first 3 bytes of the hash to a 6-digit integer passkey
  uint32_t hashValue = (shaResult[0] << 16) | (shaResult[1] << 8) | shaResult[2];
  int passkey = hashValue % 1000000;

  // Ensure the passkey is 6 digits
  if (passkey < 100000) {
    passkey += 100000;
  }
  return passkey;
}

// Function to decrypt data using AES-CBC
String decryptData(const std::string &encryptedData) {
    // Decode base64
    size_t olen;
    unsigned char decoded_data[128];
    mbedtls_base64_decode(decoded_data, sizeof(decoded_data), &olen, (unsigned char*)encryptedData.c_str(), encryptedData.length());

    // Extract IV from the decoded data
    unsigned char iv_copy[16];
    memcpy(iv_copy, decoded_data, 16);

    // Decrypt using AES-CBC
    unsigned char decrypted_data[128];
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, aes_key, 128);

    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, olen - 16, iv_copy, decoded_data + 16, decrypted_data);
    mbedtls_aes_free(&aes);

    // Remove padding
    size_t decryptedLen = removePadding(decrypted_data, olen - 16);
    decrypted_data[decryptedLen] = '\0';

    return String((char*)decrypted_data);
}

size_t removePadding(unsigned char* input, size_t inputLen) {
    size_t paddingLen = input[inputLen - 1];
    if (paddingLen > 16) {
        return inputLen; // Invalid padding
    }
    return inputLen - paddingLen;
}

