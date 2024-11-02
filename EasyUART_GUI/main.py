from PyQt5 import QtWidgets
from gui import EasyUARTApp
import qdarktheme  # Import the theme

if __name__ == "__main__":
    qdarktheme.enable_hi_dpi()
    app = QtWidgets.QApplication([])
    window = EasyUARTApp()
    window.show()
    app.exec()
    