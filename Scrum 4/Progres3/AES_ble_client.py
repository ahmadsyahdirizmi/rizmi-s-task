import hashlib
import asyncio
from datetime import datetime
from bleak import BleakScanner, BleakClient, BleakError
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad
import json
import base64

# Secret key (same as on the ESP32)
SECRET_KEY = "SURIOTA"

# Original Key and IV (Ensure these are exactly 16 bytes)
AES_KEY = b'Bandung Bondowos'  # Example key, exactly 16 bytes
IV = b'bandung bondowos'  # Example IV, exactly 16 bytes

# Function to get the current time in the format used by the ESP32
def get_current_time():
    return datetime.now().strftime("%H%M%d%m%Y")

# Function to format MAC address by removing ":" and converting to lowercase
def format_mac_address(mac_address):
    return mac_address.replace(":", "").lower()

# Function to generate the passkey
def generate_passkey(mac_address):
    mac_address = format_mac_address(mac_address)  # Format MAC address
    current_time = get_current_time()
    combined_string = mac_address + SECRET_KEY + current_time
    
    # Print combined string for debugging
    print(f"Combined String: {combined_string}")

    sha256_hash = hashlib.sha256(combined_string.encode()).digest()
    # Convert the first 3 bytes of the hash to a 6-digit integer passkey
    hash_value = (sha256_hash[0] << 16) | (sha256_hash[1] << 8) | sha256_hash[2]
    passkey = hash_value % 1000000

    # Ensure the passkey is 6 digits
    if passkey < 100000:
        passkey += 100000

    return str(passkey)

# Function to encrypt data using AES-CBC
def encrypt_data(data):
    cipher = AES.new(AES_KEY, AES.MODE_CBC, IV)
    ciphertext = cipher.encrypt(pad(data.encode('utf-8'), AES.block_size))
    return base64.b64encode(IV + ciphertext).decode('utf-8')

async def main():
    # Scan for BLE devices and select one to connect to
    print("Scanning for BLE devices...")
    devices = await BleakScanner.discover()

    print("Found devices:")
    for idx, device in enumerate(devices):
        print(f"{idx + 1}. Name: {device.name}, Address: {device.address}")

    device_num = int(input("Enter the number of the device you want to connect to: ")) - 1
    selected_device = devices[device_num]
    mac_address = selected_device.address

    # Generate passkey
    passkey = generate_passkey(mac_address)
    print(f"Generated Passkey: {passkey}")
    
    # Prepare the data to be sent
    data = {
        "type_connection": "RTU",
        "data_type": "Coils",
        "id": 1,
        "address": "0x01",
        "value": "0xFF"
    }
    json_data = json.dumps(data)
    encrypted_data = encrypt_data(json_data)
    print(f"Encrypted Data: {encrypted_data}")

    # Connect to the BLE device and send the encrypted data
    async with BleakClient(selected_device) as client:
        # Check if the connection is established before sending data
        if client.is_connected:
            print("Connection established. Preparing to send data...")
            
            # Wait a moment to ensure that the connection is fully established and secured
            await asyncio.sleep(5)
            
            # Send the encrypted data to the characteristic
            characteristic_uuid = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
            await client.write_gatt_char(characteristic_uuid, encrypted_data.encode('utf-8'))

            print("Data sent successfully.")
        else:
            print("Failed to connect to the device.")

if __name__ == "__main__":
    asyncio.run(main())
