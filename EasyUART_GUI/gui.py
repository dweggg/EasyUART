from PyQt5 import QtWidgets, QtCore, QtGui
import pyqtgraph as pg
from serial_interface import SerialInterface
from database import Database
import threading

class EasyUARTApp(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("EasyUART - Serial Port GUI")
        self.setGeometry(100, 100, 1000, 600)

        # Main widget and layout
        main_widget = QtWidgets.QWidget()
        self.setCentralWidget(main_widget)
        layout = QtWidgets.QVBoxLayout()
        main_widget.setLayout(layout)

        # Tab widget
        self.tabs = QtWidgets.QTabWidget()
        layout.addWidget(self.tabs)

        # Serial Interface and Database Editor tabs
        self.serial_tab = QtWidgets.QWidget()
        self.database_tab = QtWidgets.QWidget()
        self.tabs.addTab(self.serial_tab, "Serial Interface")
        self.tabs.addTab(self.database_tab, "Database Editor")

        # Initialize Serial Interface
        self.serial_interface = SerialInterface()
        self.init_serial_interface()
        
        # Initialize Database Editor
        self.database = Database()
        self.init_database_editor()

    def init_serial_interface(self):
        serial_layout = QtWidgets.QVBoxLayout(self.serial_tab)

        # Serial Port Settings
        port_frame = QtWidgets.QGroupBox("Serial Port Settings")
        serial_layout.addWidget(port_frame)
        port_layout = QtWidgets.QFormLayout(port_frame)
        
        self.port_combobox = QtWidgets.QComboBox()
        self.port_combobox.addItems(self.serial_interface.list_ports())
        self.baud_combobox = QtWidgets.QComboBox()
        self.baud_combobox.addItems(["9600", "115200", "19200"])
        
        port_layout.addRow("Select Port:", self.port_combobox)
        port_layout.addRow("Baud Rate:", self.baud_combobox)

        # Connect button
        self.connect_button = QtWidgets.QPushButton("Connect")
        self.connect_button.clicked.connect(self.connect_serial)
        serial_layout.addWidget(self.connect_button)
        
        # Serial data display
        self.serial_text_area = QtWidgets.QTextEdit()
        self.serial_text_area.setReadOnly(True)
        serial_layout.addWidget(self.serial_text_area)

        # Send data
        self.send_entry = QtWidgets.QLineEdit()
        self.send_button = QtWidgets.QPushButton("Send")
        self.send_button.clicked.connect(self.send_data)
        serial_layout.addWidget(self.send_entry)
        serial_layout.addWidget(self.send_button)

        # Status Bar
        self.status_bar = QtWidgets.QLabel("Status: Disconnected")
        serial_layout.addWidget(self.status_bar)
        
        # Plot widget for live data
        self.plot_widget = pg.GraphicsLayoutWidget()
        self.plot_widget.setBackground('black')
        self.plot = self.plot_widget.addPlot(title="Real-Time Plot")
        self.curve = self.plot.plot(pen='y')
        self.plot.showGrid(x=True, y=True)
        self.plot.setLabel('left', 'Value')
        self.plot.setLabel('bottom', 'Time')
        serial_layout.addWidget(self.plot_widget)

        # Set up data for plotting
        self.plot_data = [0] * 100
        self.plot_timer = QtCore.QTimer()
        self.plot_timer.timeout.connect(self.update_plot)
        self.plot_timer.start(100)

    def init_database_editor(self):
        db_layout = QtWidgets.QVBoxLayout(self.database_tab)

        # Variable entry form
        var_form = QtWidgets.QFormLayout()
        self.var_name_entry = QtWidgets.QLineEdit()
        self.db_baud_combobox = QtWidgets.QComboBox()
        self.db_baud_combobox.addItems(["9600", "115200", "19200"])
        self.data_type_combobox = QtWidgets.QComboBox()
        self.data_type_combobox.addItems(["int", "float", "string"])
        
        var_form.addRow("Variable Name:", self.var_name_entry)
        var_form.addRow("Baud Rate:", self.db_baud_combobox)
        var_form.addRow("Data Type:", self.data_type_combobox)
        db_layout.addLayout(var_form)

        # Add variable button
        self.add_variable_button = QtWidgets.QPushButton("Add Variable")
        self.add_variable_button.clicked.connect(self.add_variable)
        db_layout.addWidget(self.add_variable_button)

        # List of variables
        self.variable_list = QtWidgets.QListWidget()
        db_layout.addWidget(self.variable_list)

    def connect_serial(self):
        if self.serial_interface.is_connected():
            self.serial_interface.disconnect()
            self.connect_button.setText("Connect")
            self.status_bar.setText("Status: Disconnected")
        else:
            port = self.port_combobox.currentText()
            baudrate = int(self.baud_combobox.currentText())
            self.serial_interface.connect(port, baudrate)
            self.connect_button.setText("Disconnect")
            self.status_bar.setText(f"Status: Connected to {port} at {baudrate} baud")
            threading.Thread(target=self.read_serial, daemon=True).start()

    def read_serial(self):
        while self.serial_interface.is_connected():
            data = self.serial_interface.read()
            if data:
                self.serial_text_area.append(f"Received: {data}")
                self.serial_text_area.moveCursor(QtGui.QTextCursor.End)

    def send_data(self):
        if self.serial_interface.is_connected():
            data = self.send_entry.text()
            self.serial_interface.write(data.encode('utf-8'))
            self.serial_text_area.append(f"Sent: {data}")
            self.send_entry.clear()

    def add_variable(self):
        var_name = self.var_name_entry.text()
        baud_rate = int(self.db_baud_combobox.currentText())
        data_type = self.data_type_combobox.currentText()

        if var_name:
            self.database.add_variable(var_name, baud_rate, data_type)
            self.variable_list.addItem(f"{var_name} - {baud_rate} - {data_type}")
            self.var_name_entry.clear()

    def update_plot(self):
        # Simulating real-time data; replace with actual data source
        self.curve.setData(self.plot_data)
