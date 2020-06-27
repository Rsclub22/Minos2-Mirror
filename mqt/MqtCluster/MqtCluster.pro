#-------------------------------------------------
#
# Project created by QtCreator 2017-09-10T22:05:15
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network sql
QT += widgets

TARGET = MqtCluster
TEMPLATE = app

win32:RC_ICONS += ../MinosDXC.ico

win32:LIBS += -lWs2_32

SOURCES += main.cpp\
        clustermainwindow.cpp \
    sentSpotdata.cpp \
    sentSpotdatamodel.cpp \
    setupdialog.cpp \
    cluster.cpp \
    clusterrpc.cpp \
    userclustercommanddialog.cpp



HEADERS  += clustermainwindow.h \
    sentSpotdata.h \
    sentSpotdatamodel.h \
    setupdialog.h \
    cluster.h \
    clusterrpc.h \
    userclustercommanddialog.h



FORMS    += clustermainwindow.ui \
    setupdialog.ui \
    userclustercommanddialog.ui
