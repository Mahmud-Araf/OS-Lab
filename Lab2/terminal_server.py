import serial
import zlib
import time
import os
import argparse
import json

# Command Definitions
HEADER_BYTE = 0xA5
CMD_START_TRANSFER = 0x01
CMD_FIRMWARE_SIZE = 0x02
CMD_OS_VERSIONS = 0x03
ACK = 0x06
NACK = 0x15
CHUNK_SIZE = 128
MAX_RETRIES = 25

def calculate_crc(chunk):
    """Calculate CRC-32 for the given chunk."""
    return zlib.crc32(chunk) & 0xFFFFFFFF  # Mask to get the lower 32-bits

def send_command(ser, command_type, payload=b''):
    """Send a structured command packet with a header, command type, payload size, and payload."""
    packet = bytes([HEADER_BYTE, command_type, len(payload)]) + payload
    checksum = sum(packet) & 0xFF
    packet += bytes([checksum])
    ser.write(packet)
    print(f"Sent command: {packet.hex()}")

def read_from_serial(ser):
    """Read a byte from the serial port and return it, along with its hex representation."""
    try:
        byte = ser.read(1)  # Read one byte
        if byte:
            # Convert byte to hex representation for logging
            hex_byte = byte.hex()
            
            # Attempt to decode the byte to an ASCII character
            try:
                ascii_char = byte.decode('ascii')  # Decode to ASCII
                print(f"{ascii_char}")
            except UnicodeDecodeError as e:
                print(e)
                #print what is error
               
                print(f"{hex_byte}")

            return byte
        else:
            # print("No byte received.")
            return None
    except serial.SerialException as e:
        print(f"Serial port error: {e}")
        return None
    except Exception as e:
        print(f"Unexpected error: {e}")
        return None

def send_file(file_name, serial_port):
    if not os.path.isfile(file_name):
        print(f"Error: File '{file_name}' not found.")
        return

    try:
        with open(file_name, 'rb') as f:
            data = f.read()
    except IOError as e:
        print(f"Error reading file '{file_name}': {e}")
        return

    try:
        with serial.Serial(serial_port, 115200, timeout=1) as ser:
            # Send Start Transfer command
            send_command(ser, CMD_START_TRANSFER)
            #time.sleep(1)  # Short delay to ensure the command is received
            ack = read_from_serial(ser)
            while 1:
                print(ack)
                if ack == bytes([ACK]):
                    print("ACK received for start transfer command.")
                    break
            
            

            # Send firmware size as a command
            file_size = len(data)
            send_command(ser, CMD_FIRMWARE_SIZE, file_size.to_bytes(4, 'little'))
            print(f"Sent file size: {file_size} bytes")
            ack = read_from_serial(ser)

            # Wait for ACK for file size
            retries = 0
            while retries < MAX_RETRIES:
                ack = read_from_serial(ser)
                if ack == bytes([ACK]):
                    print("ACK received for file size.")
                    break
                else:
                    # print("")
                    retries += 1
                    time.sleep(0.5)  # Short delay before retrying
            if retries == MAX_RETRIES:
                print("Failed to receive ACK for file size. Aborting transfer.")
                return

            offset = 0
            total_size = len(data)
            retries = 0

            print(f"Starting file transfer of '{file_name}', size: {total_size} bytes")

            while offset < total_size:
                chunk = data[offset:offset + CHUNK_SIZE]
                chunk_size = len(chunk)
                crc = calculate_crc(chunk)

                # Prepare payload with CRC, chunk size, and chunk data
                payload = crc.to_bytes(4, 'big') + chunk_size.to_bytes(1, 'big') + chunk

                # Send structured command packet for chunk transfer
                send_command(ser, CMD_START_TRANSFER, payload)

                # Wait for ACK/NACK response
                ack = read_from_serial(ser)
                if ack == bytes([ACK]):
                    offset += chunk_size
                    retries = 0  # Reset retries on successful transmission
                    print(f"Chunk {offset // CHUNK_SIZE} acknowledged.")
                elif ack == bytes([NACK]):
                    print(f"Chunk {offset // CHUNK_SIZE} not acknowledged. Retrying...")
                    retries += 1
                    if retries > MAX_RETRIES:
                        print("Max retries exceeded. Transfer aborted.")
                        return
                    time.sleep(0.5)  # Short delay before resending
                else:
                    print("No response or unexpected response received. Retrying...")
                    retries += 1
                    if retries > MAX_RETRIES:
                        print("Max retries exceeded. Transfer aborted.")
                        return
                    time.sleep(0.5)  # Short delay before resending

            print("File transfer completed successfully.")
    except serial.SerialException as e:
        print(f"Serial port error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

def send_os_versions(serial_port):
    """Send the OS versions as an array over the serial port."""
    try:
        with open('os_versions.json', 'r') as f:
            os_versions = json.load(f)
        versions = [entry['version'] for entry in os_versions]
        versions_str = json.dumps(versions)
        with serial.Serial(serial_port, 115200, timeout=1) as ser:
            send_command(ser, CMD_OS_VERSIONS, versions_str.encode('utf-8'))
            print(f"Sent OS versions: {versions_str}")
    except Exception as e:
        print(f"Error sending OS versions: {e}")

def receive_version_and_send_file(serial_port):
    """Receive the version from the serial port and send the corresponding OS file."""
    try:
        with open('os_versions.json', 'r') as f:
            os_versions = json.load(f)
        with serial.Serial(serial_port, 115200, timeout=1) as ser:
            version = read_from_serial(ser).decode('utf-8')
            print(f"Received version: {version}")
            for entry in os_versions:
                if entry['version'] == version:
                    send_file(entry['path'], serial_port)
                    return
            print(f"Version '{version}' not found.")
    except Exception as e:
        print(f"Error receiving version and sending file: {e}")

def main():
    parser = argparse.ArgumentParser(description='File transfer over serial.')
    parser.add_argument('port', type=str, help='The serial port to use')
    args = parser.parse_args()

    with serial.Serial(args.port, 115200, timeout=1) as ser:
        while True:
            user_input = read_from_serial(ser).decode('utf-8')
            if user_input == '1':
                send_os_versions(args.port)
            elif user_input == '2':
                receive_version_and_send_file(args.port)
            elif user_input == '3':
                print("Exiting program.")
                break
            else:
                print("Invalid input received. Please send '1' to send version info, '2' to send os file, or '3' to stop.")

if __name__ == '__main__':
    main()