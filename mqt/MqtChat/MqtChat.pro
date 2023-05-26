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
macx:ICON = ../MinosChat.icns

ios {
    ios_icon.files += $$files(../ControlFiles/ios/MqtChat/*.png)
    QMAKE_BUNDLE_DATA += ios_icon
    QMAKE_INFO_PLIST = ../ControlFiles/ios/Info.plist
}

SOURCES += main.cpp \
    chatmain.cpp

HEADERS  += \
    chatmain.h

FORMS    += \
    chatmain.ui
