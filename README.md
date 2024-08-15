# SCRUM 4 Progress

## Progress 1: Initial BLE Server-Client Connection

- **Server Advertising**: The ESP32 BLE server begins advertising its presence.
- **Client Discovery**: The client searches for available BLE servers.
- **Connection Establishment**: The client selects the ESP32 BLE server and initiates a connection.
- **Passkey Transmission**: The client generates a passkey and sends it to the ESP32 BLE server via a characteristic.
- **Passkey Verification**: The ESP32 BLE server receives the passkey from the client and compares it with its own generated passkey.
- **Authentication Confirmation**: If the passkeys match, the ESP32 BLE server prints "Authenticated" to the screen.

## Progress 2: Passkey Authentication

- **Server Advertising**: The ESP32 BLE server begins advertising its presence.
- **Client Discovery**: The client searches for available BLE servers.
- **Connection Attempt**: The client selects the ESP32 BLE server and attempts to connect.
- **Passkey Generation**: The ESP32 BLE server generates a passkey independently, and the client generates its own passkey.
- **User Interaction**: A popup appears on the client for passkey entry.
- **Passkey Input**: The user inputs the passkey into the popup, based on the client's generated passkey, and sends it to the ESP32 BLE server.
- **Passkey Comparison**: The ESP32 BLE server compares the received passkey from the client with its own generated passkey.
- **Authentication and Bonding**: If the passkeys match, the devices are authenticated and bonded.

## Progress 3: Authentication with AES CBC Encryption

- **Steps Followed**: All previous steps from Progress 2 are repeated.
- **Client Reconnection**: Reruns the Python code(client) and selects the ESP32 BLE server.
- **Data Encryption**: The client encrypts data using AES-128 in CBC mode before attempting to connect to the server.
- **Data Transmission**: Upon successful connection, the client sends the encrypted data to the ESP32 BLE server.
- **Data Decryption**: The ESP32 BLE server decrypts the received data.
- **Output**: The decrypted data is written to the serial monitor for verification.
