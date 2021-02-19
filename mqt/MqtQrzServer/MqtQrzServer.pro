
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network widgets

TARGET = MqtQrzServer
TEMPLATE = app

win32:RC_ICONS += ../MinosRig.ico



CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    qrzconfiguredialog.cpp \
    qrzservermainwindow.cpp

HEADERS += \
    qrzconfiguredialog.h \
    qrzservermainwindow.h

FORMS += \
    qrzconfiguredialog.ui \
    qrzservermainwindow.ui

