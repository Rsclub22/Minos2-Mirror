#-------------------------------------------------
#
# Project created by QtCreator 2015-11-27T19:15:23
#
#-------------------------------------------------
include($$PWD/../mqt.pri)

QT       += core gui
QT       += widgets
QT       += network

TARGET = XMPPLib
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += ../TinyXML
INCLUDEPATH += ../MqtUtils

SOURCES += \
    Dispatcher.cpp \
    PubSubClient.cpp \
    PubSubName.cpp \
    RPCPubSub.cpp \
    ServerEvent.cpp \
    XMPPEvents.cpp \
    XMPPRPCObj.cpp \
    XMPPRPCParams.cpp \
    XMPPStanzas.cpp \
    MinosConnection.cpp \
    MinosRPC.cpp \
    AnalysePubSubNotify.cpp \
    PublishState.cpp

HEADERS += \
    Dispatcher.h \
    MapWrapper.h \
    PubSubClient.h \
    PubSubName.h \
    RPCPubSub.h \
    ServerEvent.h \
    XMPPEvents.h \
    XMPPRPCObj.h \
    XMPPRPCParams.h \
    XMPPStanzas.h \
    MinosConnection.h \
    MinosRPC.h \
    RPCCommandConstants.h \
    AnalysePubSubNotify.h \
    PublishState.h
    
