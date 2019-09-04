#-------------------------------------------------
#
# Project created by QtCreator 2015-11-30T19:20:34
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui
QT       += widgets
QT       += network
QT       += help

TARGET = MqtLogger
TEMPLATE = app

win32:RC_ICONS += ../MinosLogger.ico

android {
    CONFIG_INSTALL_PATH=/assets/Configuration

    config.path = $$CONFIG_INSTALL_PATH
    config.files += android/Configuration/*
    config.depends += FORCE

    INSTALLS += config
}

INCLUDEPATH += $$PWD/../MqtRotator
INCLUDEPATH += $$PWD/../MqtRigControl
INCLUDEPATH += $$PWD/../MqtBandMap

SOURCES += \
    AdifImport.cpp \
    ContestApp.cpp \
    ContestDetailsTransferObject.cpp \
    FilterFrame.cpp \
    LocCalcFrame.cpp \
    LoggerContacts.cpp \
    LoggerContest.cpp \
    MinosTestExport.cpp \
    PrintFile.cpp \
    RigMemoryFrame.cpp \
    RotPresets.cpp \
    ScreenConfig.cpp \
    ScreenConfigElement.cpp \
    ScreenConfigFile.cpp \
    ScreenConfigManager.cpp \
    ScreenConfigRow.cpp \
    SendRPCDM.cpp \
    StackedInfoFrame.cpp \
    TClockFrame.cpp \
    TSessionManager.cpp \
    bandmapclientfilterdialog.cpp \
    bandmapgraphicspanel.cpp \
    bandmapspotmarker.cpp \
    contestdetails.cpp \
    districtframe.cpp \
    dxccframe.cpp \
    enqdlg.cpp \
    freqlineedit.cpp \
    locTreeFrame.cpp \
    locframe.cpp \
    main.cpp \
    minoscontestloaddialog.cpp \
    qsologframe.cpp \
    reg1test.cpp \
    rigcontrolframe.cpp \
    rigmemdialog.cpp \
    ritlineedit.cpp \
    rotcontrolframe.cpp \
    runbuttondialog.cpp \
    taboutbox.cpp \
    tbundleframe.cpp \
    tcalendarform.cpp \
    tclockdlg.cpp \
    tentryoptionsform.cpp \
    tforcelogdlg.cpp \
    tloccalcform.cpp \
    tlogcontainer.cpp \
    tmanagelistsdlg.cpp \
    tminosbandchooser.cpp \
    tqsoeditdlg.cpp \
    tsettingseditdlg.cpp \
    tsinglelogframe.cpp \
    tstatsdispframe.cpp \
    clusterclientfilterdialog.cpp \
    clusterclientframe.cpp \
    volumeslider.cpp \
    tminosbshelpform.cpp \
    radiodetails.cpp \
    WsjtxFrame.cpp \
    WsjtxServer.cpp \
    WsjtxConfigure.cpp \
    WsjtxDecode.cpp \
    WsjtxDecodesModel.cpp \
    WsjtxNetworkMessage.cpp \
    WsjtxMessageServer.cpp \
    WsjtxRadio.cpp \
    WsjtxRadioMetaType.cpp \
    Wsjtx_qt_helpers.cpp \
	bandmapclientframe.cpp \
    bandmapcallsignmarker.cpp \
    bandmapfreqdial.cpp	\
    bandmapview.cpp




HEADERS  += \
    AdifImport.h \
    ContestApp.h \
    ContestDetailsTransferObject.h \
    FilterFrame.h \
    LocCalcFrame.h \
    LoggerContacts.h \
    LoggerContest.h \
    MinosTestExport.h \
    RigMemoryFrame.h \
    RotPresets.h \
    ScreenConfig.h \
    ScreenConfigElement.h \
    ScreenConfigFile.h \
    ScreenConfigManager.h \
    ScreenConfigRow.h \
    SendRPCDM.h \
    StackedInfoFrame.h \
    TClockFrame.h \
    TSessionManager.h \
    bandmapclientfilterdialog.h \
    bandmapgraphicspanel.h \
    bandmapspotmarker.h \
    contestdetails.h \
    districtframe.h \
    dxccframe.h \
    enqdlg.h \
    freqlineedit.h \
    locTreeFrame.h \
    locframe.h \
    minoscontestloaddialog.h \
    printfile.h \
    qsologframe.h \
    reg1test.h \
    rigcontrolframe.h \
    rigmemdialog.h \
    ritlineedit.h \
    rotcontrolframe.h \
    runbuttondialog.h \
    taboutbox.h \
    tbundleframe.h \
    tcalendarform.h \
    tclockdlg.h \
    tentryoptionsform.h \
    tforcelogdlg.h \
    tloccalcform.h \
    tlogcontainer.h \
    tmanagelistsdlg.h \
    tminosbandchooser.h \
    tqsoeditdlg.h \
    tsettingseditdlg.h \
    tsinglelogframe.h \
    tstatsdispframe.h \
    clusterclientfilterdialog.h \
    clusterclientframe.h \
    volumeslider.h \
    tminosbshelpform.h \
    radiodetails.h \
    WsjtxFrame.h \
    WsjtxServer.h \
    WsjtxConfigure.h \
    WsjtxDecode.h \
    WsjtxDecodesModel.hpp \
    WsjtxNetworkMessage.hpp \
    WsjtxMessageServer.hpp \
    Wsjtx_pimpl_h.hpp \
    Wsjtx_pimpl_impl.hpp \
    Wsjtx_qt_helpers.hpp \
    WsjtxRadio.hpp \
    bandmapclientframe.h \
    bandmapcallsignmarker.h \
    bandmapfreqdial.h	\
    bandmapview.h

FORMS    += \
    bandmapclientfilterdialog.ui \
    contestdetails.ui \
    districtframe.ui \
    dxccframe.ui \
    FilterFrame.ui \
    LocCalcFrame.ui \
    locframe.ui \
    locTreeFrame.ui \
    minoscontestloaddialog.ui \
    presetbuttondialog.ui \
    qsologframe.ui \
    rigcontrolframe.ui \
    rigmemdialog.ui \
    RigMemoryFrame.ui \
    rotcontrolframe.ui \
    RotPresets.ui \
    runbuttondialog.ui \
    ScreenConfig.ui \
    ScreenConfigElement.ui \
    ScreenConfigManager.ui \
    ScreenConfigRow.ui \
    StackedInfoFrame.ui \
    taboutbox.ui \
    tbundleframe.ui \
    tcalendarform.ui \
    tclockdlg.ui \
    TClockFrame.ui \
    tentryoptionsform.ui \
    tforcelogdlg.ui \
    tloccalcform.ui \
    tlogcontainer.ui \
    tmanagelistsdlg.ui \
    tminosbandchooser.ui \
    tqsoeditdlg.ui \
    TSessionManager.ui \
    tsettingseditdlg.ui \
    tsinglelogframe.ui \
    clusterclientfilterdialog.ui \
    clusterclientframe.ui \
    tstatsdispframe.ui \
    tminosbshelpform.ui \
    WsjtxFrame.ui \
    WsjtxConfigure.ui \
    bandmapclientframe.ui


DISTFILES += \
    AndroidTemplate/AndroidManifest.xml \
    AndroidTemplate/gradle/wrapper/gradle-wrapper.jar \
    AndroidTemplate/gradlew \
    AndroidTemplate/res/values/libs.xml \
    AndroidTemplate/build.gradle \
    AndroidTemplate/gradle/wrapper/gradle-wrapper.properties \
    AndroidTemplate/gradlew.bat \
    android/MinosLogger.ini

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/AndroidTemplate
