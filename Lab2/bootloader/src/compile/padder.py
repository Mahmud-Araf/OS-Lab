BOOTLOADER_SIZE = 0x10000 # 64 KB
BOOTLOADER_FILE = "target/bootloader"

# Read the original file
with open(BOOTLOADER_FILE, "rb") as f:
    raw_file = f.read()

# Calculate how many bytes need to be padded
bytes_to_pad = BOOTLOADER_SIZE - len(raw_file)

# If the file is larger than the desired size, raise an error
if bytes_to_pad < 0:
   print("Bootloader is too large")
else :
    # Generate padding (0xFF for each padded byte)
    padding = bytes([0xFF] * bytes_to_pad)
    # Write the padded file
    with open(BOOTLOADER_FILE, "wb") as f:
         f.write(raw_file + padding)



