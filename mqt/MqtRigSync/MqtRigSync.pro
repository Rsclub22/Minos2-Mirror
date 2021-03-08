include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui
QT       += network
QT       += widgets

TARGET = MqtRigSync
TEMPLATE = app

win32:RC_ICONS += ../minos.ico

SOURCES += \
    main.cpp \
    RSMainWindow.cpp \
    n1mmlink.cpp \
    wsjtxlink.cpp

HEADERS += \
    RSMainWindow.h \
    n1mmlink.h \
    qs_defines.h \
    wsjtxlink.h

FORMS += \
    RSMainWindow.ui
