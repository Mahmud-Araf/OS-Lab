import time
import os

BOOTLOADER_SIZE = 0x10000  # 64KB
BOOTLOADER_FILE = "target/bootloader"

while not os.path.exists(BOOTLOADER_FILE):
    print("file not found")
    time.sleep(1)

with open(BOOTLOADER_FILE,"rb") as f:
    raw_file = f.read()

bytes_to_pad = BOOTLOADER_SIZE - len(raw_file)

if bytes_to_pad < 0:
    print("bootloader.bin is too large")
    exit(1)

print(f"bytes to pad : {bytes_to_pad}")

padding = bytes([0xff for _ in range(bytes_to_pad)])

with open(BOOTLOADER_FILE, "wb") as f:
    f.write(raw_file + padding)

print("padding done...")