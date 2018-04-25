import sys
import ctypes
from PyQt5.QtWidgets import QApplication, QWidget, QMessageBox, QFontDialog, QPushButton, QCheckBox
from PyQt5.QtCore import Qt, QEvent, QTimer

class ForwardWidget(QWidget):

    def __init__(self):
        super().__init__()

        self.forwardDll = ctypes.CDLL('./HelloPyQT.dll')
        self.initUI()

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.updateRender)
        self.timer.start(30)

    def __del__(self):
        self.forwardDll.Forward_Destructor()

    def initUI(self):
        self.setGeometry(300, 300, 640, 480)
        self.setWindowTitle("HelloPyQT")

        self.show()

        Forward_Constructor = self.forwardDll.Forward_Constructor
        Forward_Constructor.argtypes = [ctypes.c_uint32, ctypes.c_int32, ctypes.c_int32]
        Forward_Constructor(int(self.winId()), self.width(), self.height())
    
    def event(self, evt):
        #self.forwardDll.Forward_Update()
        return super().event(evt)

    def updateRender(self):
        self.forwardDll.Forward_Update()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    wnd = ForwardWidget()
    sys.exit(app.exec_())