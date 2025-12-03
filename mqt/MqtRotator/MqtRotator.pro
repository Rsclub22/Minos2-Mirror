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
macx:ICON=../MinosRotator.icns

SOURCES += \
    hamlibRotcontrol.cpp \
    pstconfigdialog.cpp \
    rotatorbase.cpp \
    rotatorfactory.cpp \
    rotcapabilities.cpp \
    rotatormainwindow.cpp \
    main.cpp \
    logdialog.cpp \
    rotatorlog.cpp \
    rotatorRpc.cpp \
    rotsetupform.cpp \
    addantennadialog.cpp \
    rotsetupdialog.cpp \
    skyscancontrol.cpp \
    skyscanpresetsdialog.cpp

HEADERS  += \
    hamlibRotcontrol.h \
    pstconfigdialog.h \
    rotatorbase.h \
    rotatorfactory.h \
    rotcapabilities.h \
    rotatormainwindow.h \
    logdialog.h \
    rotatorlog.h \
    rotatorRpc.h \
    rotsetupform.h \
    addantennadialog.h \
    rotsetupdialog.h \
    skyscancontrol.h \
    skyscanpresetsdialog.h

FORMS    += \
    pstconfigdialog.ui \
    logdialog.ui \
    rotatormainwindow.ui \
    rotsetupform.ui \
    addantennadialog.ui \
    rotsetupdialog.ui \
    skyscanpresetsdialog.ui

win32:{HEADERS +=     pstRotControl.h
SOURCES += pstRotControl.cpp
}
