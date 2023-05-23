#-------------------------------------------------
#
# Project created by QtCreator 2016-09-16T11:01:18
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network widgets

TARGET = MqtControl
TEMPLATE = app

win32:RC_ICONS += ../minos.ico
mac:ICON=../minos.ico

SOURCES += main.cpp\
        ControlMain.cpp \
    windowMonitor.cpp \
    CommonMonitor.cpp \
    controlport.cpp \
    HidControl.cpp \
    K8055Container.cpp \
    portconf.cpp \
    PiGPIO.cpp

HEADERS  += ControlMain.h \
    windowMonitor.h \
    CommonMonitor.h \
    controlport.h \
    K8055Container.h \
    portconf.h \
    PiGPIO.h

FORMS    += ControlMain.ui \
    windowMonitor.ui

