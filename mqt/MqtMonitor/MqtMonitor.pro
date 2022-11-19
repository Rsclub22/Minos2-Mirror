#-------------------------------------------------
#
# Project created by QtCreator 2017-07-30T11:35:33
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network widgets

win32: {
lessThan(QT_MAJOR_VERSION, 6) {
    QT += qml
    QT += quick
}
}
TARGET = MqtMonitor
TEMPLATE = app

win32:RC_ICONS += ../MinosMonitor.ico

SOURCES += \
    MonitorTreeModel.cpp \
        main.cpp \
        MonitorMain.cpp \
    MonitoringFrame.cpp \
    MonitoredLog.cpp \
    MonitoredContestLog.cpp \
    MonitorParameters.cpp

HEADERS += \
        MonitorMain.h \
    MonitorTreeModel.h \
    MonitoringFrame.h \
    MonitoredLog.h \
    MonitoredContestLog.h \
    MonitorParameters.h

FORMS += \
        MonitorMain.ui \
    MonitoringFrame.ui
