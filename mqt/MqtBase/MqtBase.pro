#-------------------------------------------------
#
# Project created by QtCreator 2015-11-27T23:58:51
#
#-------------------------------------------------
include($$PWD/../mqt.pri)

QT       += core gui
QT       += widgets
QT       += network

TARGET = MqtBase
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += ../MqtUtils
INCLUDEPATH += ../TinyXML
INCLUDEPATH += ../XMPPLib
INCLUDEPATH += ../qtcsv/src/include

SOURCES += \
    BandList.cpp \
    calcs.cpp \
    contacts.cpp \
    contest.cpp \
    cutils.cpp \
    latlong.cpp \
    list.cpp \
    ListContact.cpp \
    MatchContact.cpp \
    MinosLoggerEvents.cpp \
    MinosParameters.cpp \
    MinosTestImport.cpp \
    mults.cpp \
    MultsImpl.cpp \
    mwin.cpp \
    ScreenContact.cpp \
    validators.cpp \
    DisplayContestContact.cpp \
    TreeUtils.cpp \
    minossplitter.cpp \
    htmldelegate.cpp \
    CalendarList.cpp \
    qlogtabwidget.cpp \
    minoskeyboard.cpp \
    MinosLines.cpp \
    Calendar.cpp \
    tconfigframe.cpp \
    StartConfig.cpp \
    MinosTableView.cpp \
    ConfigElementFrame.cpp \
    MatchCollection.cpp \
    rigutils.cpp \
    minostablewidget.cpp

HEADERS += \
    BandList.h \
    base_pch.h \
    calcs.h \
    contacts.h \
    contest.h \
    latlong.h \
    list.h \
    ListContact.h \
    MatchContact.h \
    MinosLoggerEvents.h \
    MinosParameters.h \
    MinosTestImport.h \
    mults.h \
    MultsImpl.h \
    mwin.h \
    ProfileEnums.h \
    ScreenContact.h \
    validators.h \
    DisplayContestContact.h \
    TreeUtils.h \
    minossplitter.h \
    htmldelegate.h \
    CalendarList.h \
    cutils.h \
    qlogtabwidget.h \
    minoskeyboard.h \
    MinosLines.h \
    Calendar.h \
    waitcursor.h \
    tconfigframe.h \
    StartConfig.h \
    MinosTableView.h \
    ConfigElementFrame.h \
    rigcontrolcommonconstants.h \
    MatchCollection.h \
    rigutils.h \
    minostablewidget.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    MqtBase.pro.autosave

FORMS += \
    minoskeyboard.ui \
    tconfigframe.ui \
    StartConfig.ui \
    ConfigElementFrame.ui
