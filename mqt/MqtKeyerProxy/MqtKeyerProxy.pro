include($$PWD/../mqt.pri)
include($$PWD/../keyerbase.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network
QT       += widgets
QT       += serialport

TARGET = MqtKeyerProxy
TEMPLATE = app

win32:RC_ICONS += ../minos.ico
mac:ICON=../minos.icns


SOURCES += main.cpp\
            kpmainwindow.cpp \
            kprpcserver.cpp \
            serialPTT.cpp

HEADERS += kpmainwindow.h \
    kprpcserver.h \
    serialPTT.h

FORMS += kpmainwindow.ui
