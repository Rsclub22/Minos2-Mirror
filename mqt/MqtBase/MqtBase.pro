#-------------------------------------------------
#
# Project created by QtCreator 2015-11-27T23:58:51
#
#-------------------------------------------------
include($$PWD/../mqt.pri)

QT       += core gui
QT       += widgets
QT       += network
QT       += serialport
QT       += help

TARGET = MqtBase
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += ../MqtUtils
INCLUDEPATH += ../TinyXML
INCLUDEPATH += ../XMPPLib

PRECOMPILED_HEADER = ../MqtBase/base_pch.h

SOURCES += \
    AntennaDetail.cpp \
    AntennaState.cpp \
    BandList.cpp \
    CacheSelection.cpp \
    bandmapmarkerdetails.cpp \
    calcs.cpp \
    Calendar.cpp \
    CalendarList.cpp \
    ChatFrame.cpp \
    ChatServer.cpp \
    ConfigElementFrame.cpp \
    checkmodeagainstfreq.cpp \
    checkoperatingfreq.cpp \
    contacts.cpp \
    contest.cpp \
    cutils.cpp \
    DisplayContestContact.cpp \
    freqmodebandplan.cpp \
    htmldelegate.cpp \
    KeyerState.cpp \
    latlong.cpp \
    list.cpp \
    ListContact.cpp \
    MatchCollection.cpp \
    MatchContact.cpp \
    MatchArchiveFrame.cpp \
    MatchOtherFrame.cpp \
    MatchThisFrame.cpp \
    MatchThread.cpp \
    MatchTreeFrame.cpp \
    minosNetUtils.cpp \
    minoskeyboard.cpp \
    MinosLines.cpp \
    MinosLoggerEvents.cpp \
    MinosParameters.cpp \
    MinosTableView.cpp \
    minostablewidget.cpp \
    MinosTestImport.cpp \
    mults.cpp \
    MultsImpl.cpp \
    mwin.cpp \
    profiles.cpp \
    qlogtabwidget.cpp \
    RigCache.cpp \
    rigcommon.cpp \
    RigDetails.cpp \
    RigState.cpp \
    rigutils.cpp \
    RotatorCache.cpp \
    rotatorcommon.cpp \
    rotpresetbutton.cpp \
    rotpresetdialog.cpp \
    ScreenContact.cpp \
    serialcomms.cpp \
    StartConfig.cpp \
    TreeUtils.cpp \
    validators.cpp \
    dxspotdatamodel.cpp \
    qttelnet.cpp \
    clusterClientServer.cpp \
    spotdata.cpp \
    helpbrowser.cpp \
    helptextbrowser.cpp \
    clustercommon.cpp \
    CallsignLineEdit.cpp \
    locatorlineedit.cpp \
    bearinglineedit.cpp \
    callsigninputdialog.cpp \
    locatorinputdialog.cpp \
    bandmapdata.cpp \
    bandmapdatamodel.cpp

HEADERS += \
    AntennaDetail.h \
    AntennaState.h \
    BandList.h \
    bandmapmarkerdetails.h \
    base_pch.h \
    CacheSelection.h \
    calcs.h \
    Calendar.h \
    CalendarList.h \
    ChatFrame.h \
    ChatServer.h \
    ConfigElementFrame.h \
    checkmodeagainstfreq.h \
    checkoperatingfreq.h \
    contacts.h \
    contest.h \
    cutils.h \
    DisplayContestContact.h \
    focuswatcher.h \
    freqmodebandplan.h \
    htmldelegate.h \
    KeyerState.h \
    latlong.h \
    list.h \
    ListContact.h \
    MatchCollection.h \
    MatchContact.h \
    MatchArchiveFrame.h \
    MatchOtherFrame.h \
    MatchThisFrame.h \
    MatchThread.h \
    MatchTreeFrame.h \
    minosNetUtils.h \
    minoskeyboard.h \
    MinosLines.h \
    MinosLoggerEvents.h \
    MinosParameters.h \
    MinosTableView.h \
    minostablewidget.h \
    MinosTestImport.h \
    mults.h \
    MultsImpl.h \
    mwin.h \
    ProfileEnums.h \
    profiles.h \
    qlogtabwidget.h \
    RigCache.h \
    rigcommon.h \
    rigcontrolcommonconstants.h \
    RigDetails.h \
    RigState.h \
    rigutils.h \
    RotatorCache.h \
    rotatorcommon.h \
    rotpresetbutton.h \
    rotpresetdialog.h \
    ScreenContact.h \
    serialcomms.h \
    StartConfig.h \
    TreeUtils.h \
    validators.h \
    waitcursor.h \
    dxspotdatamodel.h \
    qttelnet.h \
    clustercommon.h \
    clusterClientServer.h \
    spotdata.h \
    rigmemcommondata.h \
    helpbrowser.h \
    helptextbrowser.h \
    CallsignLineEdit.h \
    locatorlineedit.h \
    bearinglineedit.h \
    callsigninputdialog.h \
    locatorinputdialog.h \
    bandmapdata.h \
    bandmapdatamodel.h \
    bandmapcommon.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    MqtBase.pro.autosave

FORMS += \
    ChatFrame.ui \
    ConfigElementFrame.ui \
    MatchTreeFrame.ui \
    minoskeyboard.ui \
    rotpresetdialog.ui \
    StartConfig.ui \
    helpbrowser.ui \
    callsigninputdialog.ui \
    locatorinputdialog.ui
