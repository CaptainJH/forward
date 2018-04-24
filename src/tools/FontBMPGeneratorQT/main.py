import sys
import ctypes
from PyQt5.QtWidgets import QApplication, QWidget, QMessageBox, QFontDialog, QPushButton, QCheckBox
from PyQt5.QtGui import QFontDatabase, QFont, QFontMetrics, QImage, QPainter, QPen
from PyQt5.QtCore import Qt, QRectF


gExportPNG = False
gFontFolder = ''
gGlyphInfo = []

def ExportCPPFile(name, width, height, image):
    outFileName = gFontFolder + name + '.cpp'
    className = name
    cn = name
    numTexels = width * height

    fileText = ''
    fileText += "// Heqi Ju, Project Forward \n"
    fileText += "// Copyright (c) 2015-2018\n"
    fileText += "// File Version: automatically generated\n"
    fileText += "\n"
    fileText += "#include \"{:s}.h\"\n".format(name)
    fileText += "using namespace forward;\n"
    fileText += "\n"
    fileText += "\n"
    fileText += "{:s}::~{:s}()\n".format(cn, cn)
    fileText += "{\n"
    fileText += "}\n"
    fileText += "\n"
    fileText += "{:s}::{:s}(i32 maxMessageLength)\n".format(cn, cn)
    fileText += "    :\n"
    fileText += "    Font(msWidth, msHeight, (i8 const*)msTexels, msCharacterData,\n"
    fileText += "        maxMessageLength)\n"
    fileText += "{\n"
    fileText += "}\n"
    fileText += "\n"
    fileText += "\n"
    fileText += "i32 {:s}::msWidth = {:d};\n".format(cn, width)
    fileText += "i32 {:s}::msHeight = {:d};\n".format(cn, height)
    fileText += "\n"
    fileText += "u8 {:s}::msTexels[{:d}] =\n".format(cn, numTexels)
    fileText += "{\n"

    #print(fileText)   

    numPerRow = 16
    numPerRowM1 = numPerRow - 1
    numPixel = 0
    for y in range(height):
        for x in range(width):
            r = image.pixelColor(x, y).red()
            fileText += str(r) + ','
            if numPixel % numPerRow == numPerRowM1:
                fileText += '\n'
            numPixel += 1
    fileText += "\n"
    fileText += "};"
    fileText += "\n"

    fileText += "f32 {:s}::msCharacterData[{:d}] =\n".format(cn, len(gGlyphInfo))
    fileText += "{\n"
    numPerRow = 8
    numPerRowM1 = numPerRow - 1
    index = 0
    for glyph in gGlyphInfo:
        fileText += str(glyph) + 'f,'
        if (index % numPerRow) == numPerRowM1:
            fileText += "\n"
        index += 1

    fileText += "};\n"

    file = open(outFileName, 'w')
    file.write(fileText)
    file.close()

def ExportHeaderFile(name, width, height):
    outFileName = gFontFolder + name + '.h'
    className = name

    fileText = ''
    fileText += "// Heqi Ju, Project Forward \n"
    fileText += "// Copyright (c) 2015-2018\n"
    fileText += "// File Version: automatically generated\n"
    fileText += "\n"
    fileText += "#pragma once\n"
    fileText += "\n"
    fileText += "#include \"Font.h\"\n"
    fileText += "\n"
    fileText += "namespace forward\n"
    fileText += "{\n"
    fileText += "\n"
    fileText += "class " + className + " : public Font\n"
    fileText += "{\n"
    fileText += "public:\n"
    fileText += "    virtual ~" + className + "();\n"
    fileText += "    " + className + "(i32 maxMessageLength);\n"
    fileText += "\n"
    fileText += "private:\n"
    fileText += "    static i32 msWidth;\n"
    fileText += "    static i32 msHeight;\n"
    fileText += "    static u8 msTexels[];\n"
    fileText += "    static f32 msCharacterData[];\n"
    fileText += "};\n"
    fileText += "\n"
    fileText += "}\n"   

    #print(fileText)

    file = open(outFileName, 'w')
    file.write(fileText)
    file.close()

def ExportCB_Checked(state):
    global gExportPNG
    gExportPNG = state > 0

def NomalizeFontName(fontName):
    fontName = fontName.replace(' ', '_')
    return fontName

def GenerateFontExportFileName(fontName, weight, size):
    ret = 'Font' + NomalizeFontName(fontName) + 'W' + str(weight) + 'H' + str(size)
    return ret


def ExportBtn_Clicked():
    global gGlyphInfo
    font, ok = QFontDialog.getFont()
    if ok:
        msg = ''' !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~'''
        #msg = ''
        #i = 0
        #while (i < 128):
        #    ch = chr(i)
        #    msg += ch
        #    i += 1

        print(msg)
        metric = QFontMetrics(font)
        rect = metric.boundingRect(msg)
        width = rect.width()
        height = rect.height()
        startY = rect.y()
        rect.setX(0)
        rect.setY(0)

        image = QImage(width, height, QImage.Format_ARGB32_Premultiplied)
        painter = QPainter(image)
        painter.setRenderHint(QPainter.TextAntialiasing)
        painter.setFont(font)
        painter.fillRect(image.rect(), Qt.white)
        painter.drawText(image.rect(), Qt.AlignLeft | Qt.AlignBottom, msg)


        pen = QPen(Qt.red)
        painter.setPen(pen)
        start = 0
        dx = 1.0 / width
        gGlyphInfo.append(0.5 * dx)
        for c in msg:
            charWidth = metric.width(c)
            #r = QRectF(x, 0, charWidth, height)
            #painter.drawRect(r)
            #print('{:s}, {:d}'.format(c, charWidth))

            # Place a blank pixel at the start of each character.
            #charWidth += 1 
            end = start + charWidth
            gGlyphInfo.append(( end + 0.5) * dx)
            start += charWidth
        painter.end()

        ExportHeaderFile(GenerateFontExportFileName(font.family(), font.weight(), font.pointSize()), width, height)
        ExportCPPFile(GenerateFontExportFileName(font.family(), font.weight(), font.pointSize()), width, height, image)

        if gExportPNG:
            image.save(NomalizeFontName(font.family()) + ".png")

    
app = QApplication(sys.argv)

w = QWidget()
w.resize(250, 150)
w.move(300, 300)
w.setWindowTitle('FontGenerator')
print(int(w.winId()))

forwardDll = ctypes.CDLL('../../../libs/forwardDX11_Dll.dll')

forwardDll.Forward_Constructor()

GetFontFolder_Func = forwardDll.FileSystem_GetFontFolder
GetFontFolder_Func.restype = ctypes.c_wchar_p
gFontFolder = GetFontFolder_Func()
print(gFontFolder)

export_btn = QPushButton('Export', w)
export_btn.resize(export_btn.sizeHint())
export_btn.move(50, 50)
export_btn.clicked.connect(ExportBtn_Clicked)

export_cb = QCheckBox('Export PNG', w)
export_cb.move(50, 80)
export_cb.stateChanged.connect(ExportCB_Checked)

w.show()

sys.exit(app.exec_())
forwardDll.Forward_Destructor()