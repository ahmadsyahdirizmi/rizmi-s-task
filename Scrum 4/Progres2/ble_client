import hashlib
import asyncio
from datetime import datetime
from bleak import BleakScanner, BleakClient, BleakError

# Secret key (same as on the ESP32)
SECRET_KEY = "SURIOTA"

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

async def main():
    # Scanning for BLE devices and selecting one to connect to
    print("Scanning for BLE devices...")
    devices = await BleakScanner.discover()

    print("Found devices:")
    for idx, device in enumerate(devices):
        print(f"{idx + 1}. Name: {device.name}, Address: {device.address}")

    device_num = int(input("Enter the number of the device you want to connect to: ")) - 1
    selected_device = devices[device_num]
    mac_address = selected_device.address

    # Generating the passkey
    passkey = generate_passkey(mac_address)
    print(f"Generated Passkey: {passkey}")

    # Attempt to connect to the BLE device
    try:
        async with BleakClient(selected_device) as client:
            print(f"Connected to {selected_device.name}")

            # Wait for the device to prompt for a passkey and authenticate
            print("Waiting for passkey authentication...")
            paired = False
            while not paired:
                if client.is_connected:
                    await asyncio.sleep(5)
                    paired = True
                else:
                    print("Client disconnected. Reconnecting...")
                    await client.connect()
                    await asyncio.sleep(1)

            print("Device authenticated. Disconnecting...")
            # No further actions needed, disconnecting now.

    except BleakError as e:
        print(f"Could not connect to device: {e}")
        print("Ensure that the device is paired and that it requires no additional authentication steps.")

if __name__ == "__main__":
    asyncio.run(main())
