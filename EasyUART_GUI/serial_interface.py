import serial
import serial.tools.list_ports

class SerialInterface:
    def __init__(self):
        self.serial_port = None

    def list_ports(self):
        """List available serial ports."""
        return [port.device for port in serial.tools.list_ports.comports()]

    def connect(self, port, baudrate):
        """Connect to the specified serial port."""
        self.serial_port = serial.Serial(port, baudrate, timeout=1)

    def disconnect(self):
        """Disconnect from the serial port."""
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            self.serial_port = None

    def is_connected(self):
        """Check if connected to the serial port."""
        return self.serial_port is not None and self.serial_port.is_open

    def read(self):
        """Read data from the serial port."""
        try:
            if self.serial_port and self.serial_port.in_waiting:
                return self.serial_port.read(self.serial_port.in_waiting).hex()  # Return hex data
            return None
        except serial.SerialException as e:
            print(f"Serial exception occurred: {e}")
            self.disconnect()  # Optionally disconnect if an error occurs
            return None
        except Exception as e:
            print(f"An error occurred: {e}")
            return None


    def write(self, data):
        """Write data to the serial port."""
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.write(data)
