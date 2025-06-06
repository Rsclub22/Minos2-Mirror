include($$PWD/../mqt.pri)
# Temporary don't look for translation files
TRANSLATIONS =
include($$PWD/../mqtapplibs.pri)

QT       += core gui widgets network serialport multimedia

TARGET = MqtPcCwKeyer
TEMPLATE = app

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    pccwkeyer.cpp \
    pccwkeyermainwindow.cpp \
    pccwkeyerrpc.cpp



HEADERS += \
    pccwkeyer.h \
    pccwkeyermainwindow.h \
    pccwkeyerrpc.h




FORMS += \
    pccwkeyermainwindow.ui

