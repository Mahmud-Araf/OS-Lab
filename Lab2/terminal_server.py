import math
import os
import struct
import serial

CHUNK_SIZE = 4
FILE_PATH = 'duos24/src/compile/target/duos'
SERIAL_PORT = '/dev/tty.usbmodem1303'
POLYNOMIAL = 0x04C11DB7

class TerminalServer:
    def __init__(self):
        self.serial_instance = serial.Serial(port=SERIAL_PORT, baudrate=115200)
        print("server - Terminal Server Connected to Serial Port:", self.serial_instance.name)
        with open('version.txt', 'r') as file:
            self.latest_version = float(file.read())
            print("server - Server OS version:", self.latest_version)

    def calc_crc_32(self, data, crc=0xFFFFFFFF) -> int:
        """Calculate CRC32 to match STM32 hardware CRC behavior."""
        for i in range(0, len(data), 4):
            chunk = data[i:i+4]
            if len(chunk) < 4:
                chunk = chunk.ljust(4, b'\x00')  # Pad with zeros if less than 4 bytes
            crc = self.crc32(struct.unpack('>I', chunk)[0], crc)
        return crc

    def crc32(self, data, crc=0xFFFFFFFF) -> int:
        crc = crc ^ data
        for i in range(32):
            if (crc & 0x80000000) != 0:
                crc = (crc << 1) ^ POLYNOMIAL
            else:
                crc = crc << 1
        return crc & 0xFFFFFFFF

    def run(self):
        """Run the terminal server."""
        try:
            while True:
                value = self.serial_instance.readline().decode('utf-8', errors='ignore').strip()
                print("mc - " + value)
                
                cmd = value.split()
                
                if cmd[0] == "CHECK_VERSION":
                    self.check_version(cmd)
                elif cmd[0] == "GET_UPDATE":
                    self.get_update()
        except KeyboardInterrupt:
            print()
            print("server - Keyboard Interrupt detected. Exiting...")
        finally:
            self.serial_instance.close()

    def check_version(self, cmd):
        """Check if the firmware version matches the latest version."""
        current_version = float(cmd[1])
        if math.isclose(current_version, self.latest_version, rel_tol=1e-9):
            response = "NO_UPDATES_AVAILABLE\n".encode('utf-8')
        else:
            response = f"UPDATE_AVAILABLE {self.latest_version}\n".encode('utf-8')
        self.serial_instance.write(response)

    def get_update(self):
        """Send the firmware update file in chunks with CRC validation."""
        file_size = os.path.getsize(FILE_PATH)
        print(f"server - File size: {file_size} Bytes")
        packet = str(file_size).encode('utf-8') + b'$'
        
        self.serial_instance.write(packet)
        ack = self.serial_instance.readline().decode('utf-8', errors='ignore').strip()
        print("mc -",ack)
        
        if ack == "ACK":
            print("server - File size sent successfully")
            
            with open(FILE_PATH, 'rb') as file:
                current_chunk_number = 1
                total_chunk_number = int(math.ceil(file_size / CHUNK_SIZE))

                # crc = 0xFFFFFFFF
                
                while file_size:
                    chunk = file.read(CHUNK_SIZE)
                    padded_chunk = chunk + b'\x00' * (CHUNK_SIZE - len(chunk)) 
                    
                    # Calculate CRC
                    crc = self.calc_crc_32(padded_chunk)
                    
                    # Send CRC
                    self.serial_instance.write(struct.pack('>I', crc))
                    
                    # Send data chunk
                    self.serial_instance.write(chunk)
                    ack = self.serial_instance.readline().decode('utf-8', errors='ignore').strip()
                    print("mc -",ack)
                    
                    if ack == "ACK":
                        print(f"server - {current_chunk_number} / {total_chunk_number} chunks sent successfully")
                        current_chunk_number += 1
                        
                        if file_size >= CHUNK_SIZE:
                            file_size -= CHUNK_SIZE
                        else:
                            file_size = 0
                    elif ack == "NACK":
                        print(f"server - Error while sending chunk number {current_chunk_number}")
                        break
                    elif ack == "RESEND":
                        i=0
                        while True:
                            print(f"server - Resending chunk number {current_chunk_number}")
                            self.serial_instance.write(struct.pack('>I', crc))
                            self.serial_instance.write(chunk)
                            ack = self.serial_instance.readline().decode('utf-8', errors='ignore').strip()
                            print("mc -",ack)   
                            i+=1    
                            if ack == "ACK":
                                print(f"server - {current_chunk_number} / {total_chunk_number} chunks sent successfully")
                                current_chunk_number += 1
                                
                                if file_size >= CHUNK_SIZE:
                                    file_size -= CHUNK_SIZE
                                else:
                                    file_size = 0
                                break
                            elif i==3:
                                print(f"server - Error while sending chunk number {current_chunk_number}")
                                break

                        
                        
            
            if file_size == 0:
                print("server - File sent successfully")
                    
        elif ack == "NACK":
            print("server - Error while sending file size")

if __name__ == "__main__":
    updater = TerminalServer()
    updater.run()