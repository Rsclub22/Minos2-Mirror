#-------------------------------------------------
#
# Project created by QtCreator 2015-11-27T23:58:51
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqthamlib.pri)


QT       += core gui
QT       += widgets
QT       += network
QT       += help

equals(INC_MAP, 1): {
QT += qml
QT += quick
}

TARGET = MqtBase
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += ../MqtUtils
INCLUDEPATH += ../TinyXML
INCLUDEPATH += ../XMPPLib

SOURCES += \
    AntennaDetail.cpp \
    AntennaState.cpp \
    BandList.cpp \
    CacheSelection.cpp \
    ConfigurationOption.cpp \
    MonitorTreeModel.cpp \
    MonitoredContestLog.cpp \
    MonitoredLog.cpp \
    StartConfigManager.cpp \
    StatisticsDisplay.cpp \
    bandmapmarkerdetails.cpp \
    baseloglist.cpp \
    calcs.cpp \
    Calendar.cpp \
    CalendarList.cpp \
    ChatFrame.cpp \
    ChatServer.cpp \
    ConfigElementFrame.cpp \
    callsign.cpp \
    checkmodeagainstfreq.cpp \
    checkoperatingfreq.cpp \
    contacts.cpp \
    contest.cpp \
    cutils.cpp \
    DisplayContestContact.cpp \
    dtg.cpp \
    enqdlg.cpp \
    freqmodebandplan.cpp \
    gjvparams.cpp \
    htmldelegate.cpp \
    indicatorpushbutton.cpp \
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
    locator.cpp \
    minosNetUtils.cpp \
    minositem.cpp \
    minoskeyboard.cpp \
    MinosLines.cpp \
    MinosLoggerEvents.cpp \
    MinosParameters.cpp \
    MinosTableView.cpp \
    minosqlabel.cpp \
    minostablewidget.cpp \
    MinosTestImport.cpp \
    monitoredlogs.cpp \
    monitoredstation.cpp \
    mults.cpp \
    MultsImpl.cpp \
    presetbutton.cpp \
    profiles.cpp \
    qlogtabwidget.cpp \
    qmlcpplink.cpp \
    qsomapframe.cpp \
    RigCache.cpp \
    remotelogs.cpp \
    rigcommon.cpp \
    RigDetails.cpp \
    RigState.cpp \
    rigcontrolcommonconstants.cpp \
    rigutils.cpp \
    RotatorCache.cpp \
    rotatorcommon.cpp \
    rotpresetdialog.cpp \
    ScreenContact.cpp \
    StartConfig.cpp \
    TreeUtils.cpp \
    serialdata.cpp \
    serialtvswitch.cpp \
    spotbasedata.cpp \
    validators.cpp \
    dxspotdatamodel.cpp \
    qttelnet.cpp \
    clusterClientServer.cpp \
    helpbrowser.cpp \
    helptextbrowser.cpp \
    clustercommon.cpp \
    CallsignLineEdit.cpp \
    locatorlineedit.cpp \
    bearinglineedit.cpp \
    callsigninputdialog.cpp \
    locatorinputdialog.cpp \
    WsjtxNetworkMessage.cpp \
    WsjtxMessageServer.cpp \
    Wsjtx_qt_helpers.cpp

HEADERS += \
    AntennaDetail.h \
    AntennaState.h \
    BandList.h \
    ConfigurationOption.h \
    MonitorTreeModel.h \
    MonitoredContestLog.h \
    MonitoredLog.h \
    StartConfigManager.h \
    StatisticsDisplay.h \
    bandmapmarkerdetails.h \
    CacheSelection.h \
    baseloglist.h \
    calcs.h \
    Calendar.h \
    CalendarList.h \
    ChatFrame.h \
    ChatServer.h \
    ConfigElementFrame.h \
    callsign.h \
    checkmodeagainstfreq.h \
    checkoperatingfreq.h \
    contacts.h \
    contest.h \
    cutils.h \
    DisplayContestContact.h \
    dtg.h \
    enqdlg.h \
    focuswatcher.h \
    freqmodebandplan.h \
    gjvparams.h \
    htmldelegate.h \
    indicatorpushbutton.h \
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
    locator.h \
    minosNetUtils.h \
    minositem.h \
    minoskeyboard.h \
    MinosLines.h \
    MinosLoggerEvents.h \
    MinosParameters.h \
    MinosTableView.h \
    minosqlabel.h \
    minostablewidget.h \
    MinosTestImport.h \
    monitoredlogs.h \
    monitoredstation.h \
    mults.h \
    MultsImpl.h \
    ProfileEnums.h \
    presetbutton.h \
    profiles.h \
    qlogtabwidget.h \
    qmlcpplink.h \
    qsomapframe.h \
    RigCache.h \
    qrzServerCommon.h \
    remotelogs.h \
    rigRotSetupCommon.h \
    rigcommon.h \
    rigcontrolcommonconstants.h \
    RigDetails.h \
    RigState.h \
    rigutils.h \
    RotatorCache.h \
    rotatorcommon.h \
    rotpresetdialog.h \
    ScreenContact.h \
    StartConfig.h \
    TreeUtils.h \
    serialdata.h \
    serialtvswitch.h \
    spotbasedata.h \
    validators.h \
    waitcursor.h \
    dxspotdatamodel.h \
    qttelnet.h \
    clustercommon.h \
    clusterClientServer.h \
    rigmemcommondata.h \
    helpbrowser.h \
    helptextbrowser.h \
    CallsignLineEdit.h \
    locatorlineedit.h \
    bearinglineedit.h \
    callsigninputdialog.h \
    locatorinputdialog.h \
    bandmapcommon.h \
    WsjtxNetworkMessage.hpp \
    WsjtxMessageServer.hpp \
    Wsjtx_pimpl_h.hpp \
    Wsjtx_pimpl_impl.hpp \
    Wsjtx_qt_helpers.hpp


FORMS += \
    ChatFrame.ui \
    ConfigElementFrame.ui \
    MatchTreeFrame.ui \
    StartConfigManager.ui \
    StatisticsDisplay.ui \
    minoskeyboard.ui \
    monitoredlogs.ui \
    rotpresetdialog.ui \
    StartConfig.ui \
    helpbrowser.ui \
    callsigninputdialog.ui \
    locatorinputdialog.ui \
    qsomapframe.ui

equals(INC_MAP, 1): {
RESOURCES += QSOView/qml.qrc
}

