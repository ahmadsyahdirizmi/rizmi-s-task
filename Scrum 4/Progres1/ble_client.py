import hashlib
import asyncio
from datetime import datetime
from bleak import BleakScanner, BleakClient

# Secret key
SECRET_KEY = "RIZMI"

# Fungsi untuk mendapatkan waktu saat ini dalam format yang sama dengan ESP32
def get_current_time():
    return datetime.now().strftime("%H%M%d%m%Y")

# Fungsi untuk menghapus ":" dan mengubah huruf ke kecil pada MAC address
def format_mac_address(mac_address):
    return mac_address.replace(":", "").lower()

# Fungsi untuk menghasilkan passkey
def generate_passkey(mac_address):
    mac_address = format_mac_address(mac_address)  # Format MAC address
    current_time = get_current_time()
    combined_string = mac_address + SECRET_KEY + current_time
    
    # Print combined string
    print(f"Combined String: {combined_string}")

    sha256_hash = hashlib.sha256(combined_string.encode()).hexdigest()
    passkey = sha256_hash[:6]
    return passkey

async def main():
    # Memindai perangkat BLE dan memilih salah satu untuk terhubung
    print("Scanning for BLE devices...")
    devices = await BleakScanner.discover()

    print("Found devices:")
    for idx, device in enumerate(devices):
        print(f"{idx + 1}. Name: {device.name}, Address: {device.address}")

    device_num = int(input("Enter the number of the device you want to connect to: ")) - 1
    selected_device = devices[device_num]
    mac_address = selected_device.address

    # Menghasilkan passkey
    passkey = generate_passkey(mac_address)
    print(f"Generated Passkey: {passkey}")

    # Menghubungkan ke perangkat BLE dan mengirim passkey
    async with BleakClient(selected_device) as client:
        print(f"Connected to {selected_device.name}")
        # Mengirim passkey ke characteristic yang sesuai
        characteristic_uuid = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
        await client.write_gatt_char(characteristic_uuid, passkey.encode())

        print("Passkey sent, awaiting response...")

if __name__ == "__main__":
    asyncio.run(main())
