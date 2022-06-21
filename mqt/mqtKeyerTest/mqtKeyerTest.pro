#-------------------------------------------------
#
# Project created by QtCreator 2017-07-05T13:31:47
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../keyerbase.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui charts
QT       += widgets
QT       += network

TARGET = mqtKeyerTest
TEMPLATE = app

SOURCES += \
        main.cpp \
        mqtktMainWindow.cpp \
        mqtktWaveShowDialog.cpp

HEADERS += \
        mqtktMainWindow.h \
        mqtktWaveShowDialog.h

FORMS += \
        mqtktMainWindow.ui \
        mqtktWaveShowDialog.ui
