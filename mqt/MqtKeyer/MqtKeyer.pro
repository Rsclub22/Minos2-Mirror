#-------------------------------------------------
#
# Project created by QtCreator 2016-09-16T11:01:45
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../keyerbase.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network widgets charts

TARGET = MqtKeyer
TEMPLATE = app

win32:RC_ICONS += ../MinosKeyer.ico

INCLUDEPATH += $$PWD/../rtaudio
INCLUDEPATH += $$PWD/../Chunkware

unix:!macos{DEFINES += __LINUX_ALSA__}
win32{DEFINES += __WINDOWS_DS__}

SOURCES += main.cpp\
        KeyerMain.cpp \
    keyerAbout.cpp \
    keyconf.cpp \
    KeyerRPCServer.cpp \
    keyers.cpp \
    portcon.cpp \
    windowMonitor.cpp

HEADERS  += KeyerMain.h \
    keyerAbout.h \
    keyconf.h \
    KeyerRPCServer.h \
    keyers.h \
    portcon.h \
    windowMonitor.h

FORMS    += KeyerMain.ui \
    keyerAbout.ui \
    windowMonitor.ui

