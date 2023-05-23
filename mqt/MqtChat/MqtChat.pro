#-------------------------------------------------
#
# Project created by QtCreator 2016-01-01T15:22:33
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui
QT       += network
QT       += widgets

TARGET = MqtChat
TEMPLATE = app

win32:RC_ICONS += ../MinosChat.ico
mac:ICON=../MinosChat.ico

SOURCES += main.cpp \
    chatmain.cpp

HEADERS  += \
    chatmain.h

FORMS    += \
    chatmain.ui
