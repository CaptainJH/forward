import sys
import ctypes
from PyQt5.QtWidgets import QApplication, QWidget, QMessageBox
from PyQt5.QtGui import QFontDatabase, QFont, QFontMetrics, QImage, QPainter, QPen
from PyQt5.QtCore import Qt, QRectF

if __name__ == '__main__':
    
    app = QApplication(sys.argv)

    w = QWidget()
    w.resize(250, 150)
    w.move(300, 300)
    w.setWindowTitle('FontGenerator')
    print(int(w.winId()))

    forwardDll = ctypes.CDLL('../../../libs/forwardDX11_Dll.dll')

    #msg = QMessageBox.information(w, "Boom", "Boom", QMessageBox.Yes | QMessageBox.No)

    forwardDll.Forward_Constructor()

    GetFontFolder_Func = forwardDll.FileSystem_GetFontFolder
    GetFontFolder_Func.restype = ctypes.c_wchar_p
    ret = GetFontFolder_Func()
    print(ret)

    fonts = QFontDatabase().families()
    #for f in fonts:
    #    print(f)

    msg = "abcdefghijklmnopqrstuvwxyz"
    font = QFont("Tahoma")
    font.setUnderline(False)
    font.setPointSize(20)
    image = QImage(500, 100, QImage.Format_ARGB32_Premultiplied)
    painter = QPainter(image)
    painter.setFont(font)
    painter.fillRect(image.rect(), Qt.white)
    x = image.rect().x()
    y = image.rect().y()
    painter.drawText(image.rect(), Qt.AlignLeft | Qt.AlignTop, msg)
    metric = QFontMetrics(font)
    pen = QPen(Qt.red)
    painter.setPen(pen)
    for c in msg:
        r = QRectF(x, y, metric.width(c), metric.height())
        painter.drawRect(r)
        x += metric.width(c)
    image.save("output.png")

    w.show()
    
    sys.exit(app.exec_())
    forwardDll.Forward_Destructor()