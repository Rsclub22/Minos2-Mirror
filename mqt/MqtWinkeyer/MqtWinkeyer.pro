include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui
QT       += widgets
QT       += network
QT  += serialport

RC_ICONS += ../minos.ico

TARGET = MqtWinkeyer
TEMPLATE = app

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    rxthread.cpp \
    txthread.cpp \
    winKeyerCommon.cpp \
    winkeyerControl.cpp \
    winkeyerlineinput.cpp \
    winkeyermainwindow.cpp \
    winkeyerrpc.cpp \
    winkeyersetupdialog.cpp

HEADERS += \
    rxthread.h \
    txthread.h \
    winKeyerCommon.h \
    winkeyerControl.h \
    winkeyerlineinput.h \
    winkeyermainwindow.h \
    winkeyerrpc.h \
    winkeyersetupdialog.h

FORMS += \
    winkeyermainwindow.ui \
    winkeyersetupdialog.ui

