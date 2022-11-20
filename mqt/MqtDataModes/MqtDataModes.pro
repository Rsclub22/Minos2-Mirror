include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT += core gui network
QT += widgets

TARGET = MqtDataModes
TEMPLATE = app

win32:RC_ICONS += ../minos.ico

SOURCES += \
    main.cpp \
    dmmainwindow.cpp

HEADERS += \
    dmmainwindow.h

FORMS += \
    dmmainwindow.ui

