import math
import os
import serial

CHUNK_SIZE = 500  # 32 bytes data
FILE_PATH = 'duos24_latest_release/src/compile/target/duos'
SERIAL_PORT = "/dev/tty.usbmodem1203"

class TerminalServer:
    def __init__(self):
        self.serial_instance = serial.Serial(port=SERIAL_PORT, baudrate=115200)
        print("Terminal Server Connected to Serial Port:", self.serial_instance.name)
        with open('version.txt', 'r') as file:
            self.latest_version = float(file.read())
            print("Server OS version: ",self.latest_version)

    def run(self):
        try:
            while True:
                value = self.serial_instance.readline().decode('utf-8', errors='ignore').strip()
                print("me - " + value)
                
                cmd = value.split()
                
                if cmd[0] == "CHECK_VERSION":
                    self.check_version(cmd)
                elif cmd[0] == "GET_UPDATE":
                    self.get_update()
        except KeyboardInterrupt:
            print("Keyboard Interrupt detected. Exiting...")
        finally:
            self.serial_instance.close()

    def check_version(self, cmd):
        current_version = float(cmd[1])
        if math.isclose(current_version, self.latest_version, rel_tol=1e-9):
            response = "NO_UPDATES_AVAILABLE\n".encode('utf-8')
        else:
            response = f"UPDATE_AVAILABLE {self.latest_version}\n".encode('utf-8')
        self.serial_instance.write(response)

    def get_update(self):
        file_size = os.path.getsize(FILE_PATH)
        print(f"File size: {file_size} Bytes")
        packet = str(file_size).encode('utf-8') + b'$'
        
        self.serial_instance.write(packet)
        ack = self.serial_instance.readline().decode('utf-8', errors='ignore').strip()
        print(ack)
        
        if ack == "ACK":
            print("File size sent successfully")
            
            with open(FILE_PATH, 'rb') as file:
                current_chunk_number = 1
                total_chunk_number = int(math.ceil(file_size / CHUNK_SIZE))
                
                while file_size:
                    chunk = file.read(CHUNK_SIZE)
                    packet = chunk
                    
                    self.serial_instance.write(packet)
                    ack = self.serial_instance.readline().decode('utf-8', errors='ignore').strip()
                    print(ack)
                    
                    if ack != "NACK":
                        print(f"{current_chunk_number} / {total_chunk_number} chunks are sent successfully")
                        current_chunk_number += 1
                        
                        if file_size >= CHUNK_SIZE:
                            file_size -= CHUNK_SIZE
                        else:
                            file_size = 0
                    elif ack == "NACK":
                        print(f"Error while sending chunk number {current_chunk_number}")
                        break
            
            if file_size == 0:
                print("File sent successfully")
                    
        elif ack == "NACK":
            print("Error while sending file size")

if __name__ == "__main__":
    updater = TerminalServer()
    updater.run()