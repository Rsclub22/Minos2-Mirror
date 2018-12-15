#-------------------------------------------------
#
# Project created by QtCreator 2016-11-10T21:16:20
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri) 
include($$PWD/../mqthamlib.pri)

QT       += core gui serialport
QT       += widgets
QT       += network

VERSION=0.6.0.0

win32:RC_ICONS += ../minos.ico

TARGET = MqtBandmap
TEMPLATE = app


SOURCES += main.cpp\
        bandmapmainwindow.cpp \
    rigcontrol.cpp \
    bandmap.cpp \
    callsignmarker.cpp \
    freqdial.cpp \
    textmarker.cpp \
    bmsetupdialog.cpp

HEADERS  += bandmapmainwindow.h \
    rigcontrol.h \
    bandmap.h \
    callsignmarker.h \
    freqdial.h \
    textmarker.h \
    bmsetupdialog.h

FORMS    += bandmapmainwindow.ui \
    bmsetupdialog.ui

