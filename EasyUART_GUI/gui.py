import tkinter as tk
from tkinter import ttk
import threading
from serial_interface import SerialInterface
from database import Database  # Import your database management class

class EasyUARTApp:
    def __init__(self, root):
        self.root = root
        self.root.title("EasyUART - Serial Port GUI")
        self.root.geometry("600x550")

        # Create Notebook (Tabs)
        self.notebook = ttk.Notebook(root)
        self.notebook.pack(fill='both', expand=True)

        # Create Serial Interface Page
        self.serial_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.serial_frame, text="Serial Interface")

        # Create Database Editor Page
        self.database_frame = ttk.Frame(self.notebook)
        self.notebook.add(self.database_frame, text="Database Editor")

        # Initialize Serial Interface
        self.serial_interface = SerialInterface(self)  # Initialize the serial interface
        self.create_serial_interface()
        self.create_database_editor()

    def create_serial_interface(self):
        # Frame for Serial Port settings
        self.port_frame = ttk.LabelFrame(self.serial_frame, text="Serial Port Settings", padding=(10, 10))
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
        self.text_area = tk.Text(self.serial_frame, wrap='word', height=15, width=70)
        self.text_area.grid(column=0, row=1, padx=10, pady=10)

        # Send data entry
        self.send_entry = ttk.Entry(self.serial_frame, width=60)
        self.send_entry.grid(column=0, row=2, padx=10, pady=5)

        self.send_button = ttk.Button(self.serial_frame, text="Send", command=self.send_data)
        self.send_button.grid(column=0, row=3, padx=10, pady=(5, 10))

        # Status Bar
        self.status_bar = ttk.Label(self.serial_frame, text="Status: Disconnected", relief=tk.SUNKEN, anchor='w')
        self.status_bar.grid(column=0, row=4, sticky="ew", padx=10)

        self.update_ports()  # Populate the combobox with available ports

    def create_database_editor(self):
        # Frame for Database settings
        self.db_frame = ttk.LabelFrame(self.database_frame, text="Database Variables", padding=(10, 10))
        self.db_frame.grid(column=0, row=0, padx=10, pady=10, sticky="ew")

        # Variable Name Entry
        self.var_name_label = ttk.Label(self.db_frame, text="Variable Name:")
        self.var_name_label.grid(column=0, row=0, sticky="w")

        self.var_name_entry = ttk.Entry(self.db_frame, width=30)
        self.var_name_entry.grid(column=1, row=0, padx=5, pady=5)

        # Baud Rate Selection for Database
        self.db_baud_label = ttk.Label(self.db_frame, text="Baud Rate:")
        self.db_baud_label.grid(column=0, row=1, sticky="w")

        self.db_baud_combobox = ttk.Combobox(self.db_frame, state='readonly', values=[9600, 115200, 19200], width=20)
        self.db_baud_combobox.grid(column=1, row=1)
        self.db_baud_combobox.current(0)  # Default to 9600

        # Data Type Selection
        self.data_type_label = ttk.Label(self.db_frame, text="Data Type:")
        self.data_type_label.grid(column=0, row=2, sticky="w")

        self.data_type_combobox = ttk.Combobox(self.db_frame, state='readonly', values=["int", "float", "string"], width=20)
        self.data_type_combobox.grid(column=1, row=2)
        self.data_type_combobox.current(0)  # Default to int

        # Add Variable Button
        self.add_variable_button = ttk.Button(self.db_frame, text="Add Variable", command=self.add_variable)
        self.add_variable_button.grid(column=0, row=3, columnspan=2, pady=(5, 0))

        # Listbox to show added variables
        self.variable_listbox = tk.Listbox(self.database_frame, height=10, width=70)
        self.variable_listbox.grid(column=0, row=1, padx=10, pady=10)

        # Initialize the Database
        self.database = Database()

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

    def add_variable(self):
        variable_name = self.var_name_entry.get()
        baud_rate = int(self.db_baud_combobox.get())
        data_type = self.data_type_combobox.get()

        if variable_name:
            self.database.add_variable(variable_name, baud_rate, data_type)
            self.variable_listbox.insert(tk.END, f"{variable_name} - {baud_rate} - {data_type}")
            self.var_name_entry.delete(0, tk.END)  # Clear the entry after adding
        else:
            print("Variable name cannot be empty.")
