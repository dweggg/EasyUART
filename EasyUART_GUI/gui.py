import tkinter as tk
from tkinter import ttk
import threading
from serial_interface import SerialInterface  # Import your serial interface module

class SerialGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("EasyUART - Serial Port GUI")
        self.root.geometry("600x500")
        self.root.configure(bg="#f0f0f0")

        # Frame for Serial Port settings
        self.port_frame = ttk.LabelFrame(root, text="Serial Port Settings", padding=(10, 10))
        self.port_frame.grid(column=0, row=0, padx=10, pady=10, sticky="ew")

        # Port selection
        self.port_label = ttk.Label(self.port_frame, text="Select Port:")
        self.port_label.grid(column=0, row=0, sticky="w")

        self.port_combobox = ttk.Combobox(self.port_frame, state='readonly', width=20)
        self.port_combobox.grid(column=1, row=0)

        # Baud rate selection
        self.baud_label = ttk.Label(self.port_frame, text="Baud Rate:")
        self.baud_label.grid(column=0, row=1, sticky="w")

        self.baud_combobox = ttk.Combobox(self.port_frame, state='readonly', values=[9600, 115200, 19200], width=20)
        self.baud_combobox.grid(column=1, row=1)
        self.baud_combobox.current(0)  # Default to 9600

        # Open/Close Button
        self.connect_button = ttk.Button(self.port_frame, text="Connect", command=self.connect)
        self.connect_button.grid(column=0, row=2, columnspan=2, pady=(5, 0))

        # Text area for communication
        self.text_area = tk.Text(root, wrap='word', height=15, width=70, bg="#ffffff", font=("Arial", 10))
        self.text_area.grid(column=0, row=1, padx=10, pady=10)

        # Send data entry
        self.send_entry = ttk.Entry(root, width=60, font=("Arial", 10))
        self.send_entry.grid(column=0, row=2, padx=10, pady=5)

        self.send_button = ttk.Button(root, text="Send", command=self.send_data)
        self.send_button.grid(column=0, row=3, padx=10, pady=(5, 10))

        # Status Bar
        self.status_bar = ttk.Label(root, text="Status: Disconnected", relief=tk.SUNKEN, anchor='w')
        self.status_bar.grid(column=0, row=4, sticky="ew", padx=10)

        # Initialize Serial Interface
        self.serial_interface = SerialInterface(self)  # Initialize the serial interface
        self.update_ports()  # Now we can safely call update_ports() after initializing serial_interface
        self.running = False

    def update_ports(self):
        self.port_combobox['values'] = self.serial_interface.list_ports()
        if self.serial_interface.list_ports():
            self.port_combobox.current(0)  # Select the first port

    def connect(self):
        if self.serial_interface.is_connected():
            self.serial_interface.disconnect()
            self.connect_button.config(text="Connect")
            self.status_bar.config(text="Status: Disconnected")
        else:
            port = self.port_combobox.get()
            baudrate = int(self.baud_combobox.get())
            self.serial_interface.connect(port, baudrate)
            self.connect_button.config(text="Disconnect")
            self.status_bar.config(text=f"Status: Connected to {port} at {baudrate} baud")
            threading.Thread(target=self.read_serial, daemon=True).start()

    def read_serial(self):
        while self.serial_interface.is_connected():
            data = self.serial_interface.read()
            if data:
                self.text_area.insert(tk.END, f"Received: {data}\n")
                self.text_area.see(tk.END)

    def send_data(self):
        if self.serial_interface.is_connected():
            data = self.send_entry.get()
            self.serial_interface.write(data.encode('utf-8'))
            self.text_area.insert(tk.END, f"Sent: {data}\n")
            self.send_entry.delete(0, tk.END)
