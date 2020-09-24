#-------------------------------------------------
#
# Project created by QtCreator 2017-07-05T13:31:47
#
#-------------------------------------------------
include($$PWD/../mqt.pri)

QT       += core gui charts
QT       += widgets

TARGET = mqtKeyerTest
TEMPLATE = app

INCLUDEPATH += $$PWD/../Chunkware

SOURCES += \
        main.cpp \
        mqtktMainWindow.cpp \
    ../Chunkware/SimpleComp.cpp \
    ../Chunkware/SimpleCompProcess.inl \
    ../Chunkware/SimpleEnvelope.cpp

HEADERS += \
        mqtktMainWindow.h \
    ../Chunkware/SimpleComp.h \
    ../Chunkware/SimpleEnvelope.h \
    ../Chunkware/SimpleGain.h \
    ../Chunkware/SimpleHeader.h

FORMS += \
        mqtktMainWindow.ui
