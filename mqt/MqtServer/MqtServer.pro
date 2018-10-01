#-------------------------------------------------
#
# Project created by QtCreator 2015-12-30T14:59:17
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtminlibs.pri)

QT       += core gui
QT       += widgets
QT       += network


TARGET = MqtServer
TEMPLATE = app

win32:RC_ICONS += ../MinosServer.ico

win32:LIBS += -lws2_32

PRECOMPILED_HEADER = minos_pch.h

SOURCES += main.cpp\
    MinosLink.cpp \
    MServer.cpp \
    MServerPubSub.cpp \
    MServerZConf.cpp \
    PubSubServer.cpp \
    serverThread.cpp \
    clientThread.cpp \
    servermain.cpp \
    minoslistener.cpp

HEADERS  += \
    minos_pch.h \
    MinosLink.h \
    MServer.h \
    MServerPubSub.h \
    MServerZConf.h \
    PubSubServer.h \
    serverThread.h \
    clientThread.h \
    servermain.h \
    minoslistener.h

FORMS    += \
    servermain.ui

