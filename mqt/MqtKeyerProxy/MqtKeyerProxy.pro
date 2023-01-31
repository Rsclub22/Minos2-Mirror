include($$PWD/../mqt.pri)
include($$PWD/../keyerbase.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network
QT       += widgets

TARGET = MqtKeyerProxy
TEMPLATE = app

win32:RC_ICONS += ../Minos.ico

unix:!macos{DEFINES += __LINUX_ALSA__}
win32{DEFINES += __WINDOWS_DS__}

#win32:LIBS += -lWs2_32

SOURCES += main.cpp\
            kpmainwindow.cpp \
            kprpcserver.cpp

HEADERS += kpmainwindow.h \
    kprpcserver.h

FORMS += kpmainwindow.ui
