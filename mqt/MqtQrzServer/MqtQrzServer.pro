
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network widgets
QT       += sql

TARGET = MqtQrzServer
TEMPLATE = app

win32:RC_ICONS += ../MinosQRZ.ico
mac:ICON=../MinosQRZ.ico

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    qrzconfiguredialog.cpp \
    qrzdb.cpp \
    qrzservermainwindow.cpp \
    qrzserverminosparameters.cpp \
    qrzserverrpc.cpp

HEADERS += \
    qrzconfiguredialog.h \
    qrzdb.h \
    qrzservermainwindow.h \
    qrzserverminosparameters.h \
    qrzserverrpc.h

FORMS += \
    qrzconfiguredialog.ui \
    qrzservermainwindow.ui

