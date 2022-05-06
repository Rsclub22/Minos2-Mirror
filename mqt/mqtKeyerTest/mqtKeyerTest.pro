#-------------------------------------------------
#
# Project created by QtCreator 2017-07-05T13:31:47
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui charts
QT       += widgets
QT       += network

TARGET = mqtKeyerTest
TEMPLATE = app

INCLUDEPATH += $$PWD/../KeyerBase

SOURCES += \
        ../KeyerBase/riff.cpp \
        main.cpp \
        mqtktMainWindow.cpp \
        mqtktWaveShowDialog.cpp

HEADERS += \
        ../KeyerBase/ddc.h \
        ../KeyerBase/riff.h \
        mqtktMainWindow.h \
        mqtktWaveShowDialog.h

FORMS += \
        mqtktMainWindow.ui \
        mqtktWaveShowDialog.ui


# include system libs last, so they get included for KeyerBase

unix:!macos{ LIBS += -lasound}
win32{ LIBS += -lole32 -lwinmm -luuid -lksuser -ldsound -lUser32}
