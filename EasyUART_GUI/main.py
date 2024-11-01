from PyQt5 import QtWidgets
from gui import EasyUARTApp

if __name__ == "__main__":
    app = QtWidgets.QApplication([])
    window = EasyUARTApp()
    window.show()
    app.exec_()
