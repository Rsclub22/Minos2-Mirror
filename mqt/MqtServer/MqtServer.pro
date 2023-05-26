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
macx:ICON=../MinosServer.icns

ios {
    ios_icon.files += $$files(../ControlFiles/ios/MqtServer/*.png)
    QMAKE_BUNDLE_DATA += ios_icon
    QMAKE_INFO_PLIST = ../ControlFiles/ios/Info.plist
}

win32:LIBS += -lws2_32

SOURCES += main.cpp\
    MinosLink.cpp \
    MServer.cpp \
    MServerPubSub.cpp \
    MServerZConf.cpp \
    PubSubServer.cpp \
    serverThread.cpp \
    clientThread.cpp \
    servermain.cpp \
    minoslistener.cpp \
    ServerDetails.cpp

HEADERS  += \
    MinosLink.h \
    MServer.h \
    MServerPubSub.h \
    MServerZConf.h \
    PubSubServer.h \
    serverThread.h \
    clientThread.h \
    servermain.h \
    minoslistener.h \
    ServerDetails.h

FORMS    += \
    servermain.ui \
    ServerDetails.ui

