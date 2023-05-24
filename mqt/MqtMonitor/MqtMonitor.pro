#-------------------------------------------------
#
# Project created by QtCreator 2017-07-30T11:35:33
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network widgets

equals(INC_MAP, 1): {
QT += qml
QT += quick
}

TARGET = MqtMonitor
TEMPLATE = app

win32:RC_ICONS += ../MinosMonitor.ico
mac:ICON=../MinosMonitor.icns

SOURCES += \
        main.cpp \
        MonitorMain.cpp \
    MonitoringFrame.cpp \
    MonitorParameters.cpp

HEADERS += \
        MonitorMain.h \
    MonitoringFrame.h \
    MonitorParameters.h

FORMS += \
        MonitorMain.ui \
    MonitoringFrame.ui
