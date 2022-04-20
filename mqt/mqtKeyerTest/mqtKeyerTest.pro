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

INCLUDEPATH += $$PWD/../Chunkware
INCLUDEPATH += $$PWD/../KeyerBase

SOURCES += \
        ../Chunkware/SimpleComp.cpp \
        ../Chunkware/SimpleEnvelope.cpp \
        ../KeyerBase/riff.cpp \
        WaveShowDialog.cpp \
        main.cpp \
        mqtktMainWindow.cpp

HEADERS += \
        ../Chunkware/SimpleComp.h \
        ../Chunkware/SimpleCompProcess.inl \
        ../Chunkware/SimpleEnvelope.h \
        ../KeyerBase/ddc.h \
        ../KeyerBase/riff.h \
        WaveShowDialog.h \
        mqtktMainWindow.h

FORMS += \
        WaveShowDialog.ui \
        mqtktMainWindow.ui


# include system libs last, so they get included for KeyerBase

unix:!macos{ LIBS += -lasound}
win32{ LIBS += -lole32 -lwinmm -luuid -lksuser -ldsound -lUser32}
