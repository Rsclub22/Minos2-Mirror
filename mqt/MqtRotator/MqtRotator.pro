#-------------------------------------------------
#
# Project created by QtCreator 2016-03-26T21:27:38
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqthamlib.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui network widgets
QT       += serialport

TARGET = MqtRotator
TEMPLATE = app

win32:RC_ICONS += ../MinosRotator.ico

SOURCES += \
    hamlibRotcontrol.cpp \
    rotatorbase.cpp \
    rotatorfactory.cpp \
    rotcapabilities.cpp \
    skyscandialog.cpp \
    rotatormainwindow.cpp \
    minoscompass.cpp \
    main.cpp \
    logdialog.cpp \
    rotatorlog.cpp \
    rotatorRpc.cpp \
    rotsetupform.cpp \
    addantennadialog.cpp \
    rotsetupdialog.cpp

HEADERS  += \
    hamlibRotcontrol.h \
    rotatorbase.h \
    rotatorfactory.h \
    rotcapabilities.h \
    skyscandialog.h \
    rotatormainwindow.h \
    minoscompass.h \
    logdialog.h \
    rotatorlog.h \
    rotatorRpc.h \
    rotsetupform.h \
    addantennadialog.h \
    rotsetupdialog.h

FORMS    += \
    skyscandialog.ui \
    logdialog.ui \
    rotatormainwindow.ui \
    rotsetupform.ui \
    addantennadialog.ui \
    rotsetupdialog.ui

win32:{HEADERS +=     pstRotControl.h
SOURCES += pstRotControl.cpp
}
