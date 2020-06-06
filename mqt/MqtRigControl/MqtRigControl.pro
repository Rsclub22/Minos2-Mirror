#-------------------------------------------------
#
# Project created by QtCreator 2017-07-29T13:06:40
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)
include($$PWD/../mqthamlib.pri)

QT       += core gui serialport
QT       += widgets
QT       += network



TARGET = MqtRigControl
TEMPLATE = app

win32:RC_ICONS += ../MinosRig.ico

win32 {
DEFINES += WIN32
QT += axcontainer
#TYPELIBS = $$system(dumpcpp -getfile {4FE359C5-A58F-459D-BE95-CA559FB4F270})
TYPELIBS = $$system(dumpcpp OmniRig.tlb)
}

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
    freqpresetdialog.cpp \
    addradiodialog.cpp \
    rigsetupdialog.cpp \
    serialtvswitch.cpp \
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
    freqpresetdialog.h \
    addradiodialog.h \
    rigsetupdialog.h \
    serialtvswitch.h \
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
    freqpresetdialog.ui \
    addradiodialog.ui
