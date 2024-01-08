#-------------------------------------------------
#
# Project created by QtCreator 2017-07-29T13:06:40
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)
include($$PWD/../mqthamlib.pri)

QT       += core gui
QT       += widgets
QT       += network
QT  += serialport

TARGET = MqtRigControl
TEMPLATE = app

win32:RC_ICONS += ../MinosRig.ico
macx:ICON=../MinosRig.icns

#message(rig control)
win32-g++*{
    DEFINES += WIN32
    QT += axcontainer

#message(g++ dumpcpp)
    TYPELIBS = $$system(dumpcpp OmniRig.tlb)
}
win32-msvc{
        DEFINES += WIN32
        QT += axcontainer
        #message(msvc dumpcpp)
        TYPELIBS = $$system(dumpcpp OmniRig.tlb)
}
win32{
LIBS *= -lsetupapi -lhid
}
#message($$INCLUDEPATH)
SOURCES += main.cpp\
    hamlibrigcontrol.cpp \
    rigbase.cpp \
    rigcapabilities.cpp \
    rigcontrolmainwindow.cpp \
    rigcontrolrpc.cpp \
    rigfactory.cpp \
    rigsetupform.cpp \
    transvertsetupform.cpp \
    addtransverterdialog.cpp \
    addradiodialog.cpp \
    rigsetupdialog.cpp \
    smeterbar.cpp \
    rigctldclient.cpp

HEADERS  += rigcontrolmainwindow.h \
    hamlibrigcontrol.h \
    rigbase.h \
    rigcapabilities.h \
    rigcontrolrpc.h \
    rigfactory.h \
    rigsetupform.h \
    transvertsetupform.h \
    addtransverterdialog.h \
    addradiodialog.h \
    rigsetupdialog.h \
    smeterbar.h \
    rigctldclient.h

win32:{HEADERS +=  OmniWrapper.h \
                   omnirigcontrol.h
       SOURCES += OmniWrapper.cpp \
       omnirigcontrol.cpp
       }

FORMS    += rigcontrolmainwindow.ui \
    rigsetupdialog.ui \
    rigsetupform.ui \
    transvertsetupform.ui \
    addtransverterdialog.ui \
    addradiodialog.ui

equals(INC_TUNE, 1): {
    SOURCES += \
    flexcontrolframe.cpp \
    configureknobs.cpp \
    powermateframe.cpp

    HEADERS += \
    flexcontrolframe.h \
    configureknobs.h \
    powermateframe.h

    FORMS += \
    flexcontrolframe.ui \
    configureknobs.ui \
    powermateframe.ui
}
