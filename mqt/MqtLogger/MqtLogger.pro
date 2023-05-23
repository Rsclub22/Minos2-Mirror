#-------------------------------------------------
#
# Project created by QtCreator 2015-11-30T19:20:34
#
#-------------------------------------------------
include($$PWD/../mqt.pri)
include($$PWD/../keyerbase.pri)
include($$PWD/../mqtapplibs.pri)

QT       += core gui
QT       += widgets
QT       += network
QT       += help
QT       += charts
QT       += serialport

equals(INC_MAP, 1): {
QT += qml
QT += quick
}


TARGET = MqtLogger
TEMPLATE = app

win32:RC_ICONS += ../MinosLogger.ico
mac:ICON=../MinosLogger.ico

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
    BandsSelect.cpp \
    BandSwitchFrame.cpp \
    ContestApp.cpp \
    ContestDetailsTransferObject.cpp \
    ContestPage.cpp \
    ContestPageControl.cpp \
    DisplayOptions.cpp \
    ExternalMqtKeyer.cpp \
    FilterFrame.cpp \
    InternalVoiceMemoryKeyer.cpp \
    LocCalcFrame.cpp \
    LoggerContacts.cpp \
    LoggerContest.cpp \
    MinosTestExport.cpp \
    OptionsDialog.cpp \
    PrintFile.cpp \
    QSOTextEditFrame.cpp \
    RigMemoryFrame.cpp \
    RotPresets.cpp \
    ScreenConfig.cpp \
    ScreenConfigElement.cpp \
    ScreenConfigFile.cpp \
    ScreenConfigManager.cpp \
    ScreenConfigRow.cpp \
    ScreenConfigScreen.cpp \
    SendRPCDM.cpp \
    StackedInfoFrame.cpp \
    TClockFrame.cpp \
    TSessionManager.cpp \
    WsjtxConfigureCQ.cpp \
    bandmapclientfilterdialog.cpp \
    bandmapdatamodel.cpp \
    bandmapgraphicspanel.cpp \
    bandmapspotmarker.cpp \
    bandselbuttons.cpp \
    cabrillo.cpp \
    checkupdates.cpp \
    contestdetails.cpp \
    defdirsdlg.cpp \
    districtframe.cpp \
    dmbuttonframe.cpp \
    dxccframe.cpp \
    freqlineedit.cpp \
    locTreeFrame.cpp \
    locframe.cpp \
    main.cpp \
    n1mmbroadcast.cpp \
    n1mmbroadcastconfig.cpp \
    qrzdisplayframe.cpp \
    qsologframe.cpp \
    radiosettingdialog.cpp \
    reg1test.cpp \
    rigcontrolcwmessagekeyer.cpp \
    rigcontrolframe.cpp \
    rigcontrolvoicememorykeyer.cpp \
    rigmemdialog.cpp \
    ritlineedit.cpp \
    rotcontrolframe.cpp \
    runbuttondialog.cpp \
    runbuttonsframe.cpp \
    taboutbox.cpp \
    tbundleframe.cpp \
    tcalendarform.cpp \
    tclockdlg.cpp \
    tentryoptionsform.cpp \
    tforcelogdlg.cpp \
    tloccalcform.cpp \
    tlogcontainer.cpp \
    tmanagelistsdlg.cpp \
    tqsoeditdlg.cpp \
    tsettingseditdlg.cpp \
    tsinglelogframe.cpp \
    tstatsdispframe.cpp \
    clusterclientfilterdialog.cpp \
    clusterclientframe.cpp \
    txVmExternalButtonDialog.cpp \
    txVmInternalSetupDialog.cpp \
    txvmbuttonsframe.cpp \
    txvminternalbuttondialog.cpp \
    txvmrigbuttondialog.cpp \
    txvmrigsetupdialog.cpp \
    voicekeyerbase.cpp \
    voicekeyerfactory.cpp \
    volumeslider.cpp \
    tminosbshelpform.cpp \
    radiodetails.cpp \
    WsjtxFrame.cpp \
    WsjtxServer.cpp \
    WsjtxConfigure.cpp \
    WsjtxDecode.cpp \
    WsjtxDecodesModel.cpp \
    bandmapclientframe.cpp \
    bandmapcallsignmarker.cpp \
    bandmapfreqdial.cpp	\
    bandmapview.cpp \
    Clusterbandmapconfigure.cpp




HEADERS  += \
    AdifImport.h \
    BandsSelect.h \
    BandSwitchFrame.h \
    ContestApp.h \
    ContestDetailsTransferObject.h \
    ContestPage.h \
    ContestPageControl.h \
    DisplayOptions.h \
    ExternalMqtKeyer.h \
    FilterFrame.h \
    InternalVoiceMemoryKeyer.h \
    LocCalcFrame.h \
    LoggerContacts.h \
    LoggerContest.h \
    MinosTestExport.h \
    OptionsDialog.h \
    QSOTextEditFrame.h \
    RigMemoryFrame.h \
    RotPresets.h \
    ScreenConfig.h \
    ScreenConfigElement.h \
    ScreenConfigFile.h \
    ScreenConfigManager.h \
    ScreenConfigRow.h \
    ScreenConfigScreen.h \
    SendRPCDM.h \
    StackedInfoFrame.h \
    TClockFrame.h \
    TSessionManager.h \
    WsjtxConfigureCQ.h \
    bandmapclientfilterdialog.h \
    bandmapdatamodel.h \
    bandmapgraphicspanel.h \
    bandmapspotmarker.h \
    bandselbuttons.h \
    cabrillo.h \
    checkupdates.h \
    contestdetails.h \
    defdirsdlg.h \
    districtframe.h \
    dmbuttonframe.h \
    dxccframe.h \
    freqlineedit.h \
    locTreeFrame.h \
    locframe.h \
    n1mmbroadcast.h \
    n1mmbroadcastconfig.h \
    printfile.h \
    qrzdisplayframe.h \
    qsologframe.h \
    radiosettingdialog.h \
    reg1test.h \
    rigcontrolcwmessagekeyer.h \
    rigcontrolframe.h \
    rigcontrolvoicememorykeyer.h \
    rigmemdialog.h \
    ritlineedit.h \
    rotcontrolframe.h \
    runbuttondialog.h \
    runbuttonsframe.h \
    taboutbox.h \
    tbundleframe.h \
    tcalendarform.h \
    tclockdlg.h \
    tentryoptionsform.h \
    tforcelogdlg.h \
    tloccalcform.h \
    tlogcontainer.h \
    tmanagelistsdlg.h \
    tqsoeditdlg.h \
    tsettingseditdlg.h \
    tsinglelogframe.h \
    tstatsdispframe.h \
    clusterclientfilterdialog.h \
    clusterclientframe.h \
    txVmExternalButtonDialog.h \
    txVmInternalSetupDialog.h \
    txvmbuttonsframe.h \
    txvminternalbuttondialog.h \
    txvmrigbuttondialog.h \
    txvmrigsetupdialog.h \
    voicekeyerbase.h \
    voicekeyerfactory.h \
    volumeslider.h \
    tminosbshelpform.h \
    radiodetails.h \
    WsjtxFrame.h \
    WsjtxServer.h \
    WsjtxConfigure.h \
    WsjtxDecode.h \
    WsjtxDecodesModel.hpp \
    bandmapclientframe.h \
    bandmapcallsignmarker.h \
    bandmapfreqdial.h	\
    bandmapview.h \
    Clusterbandmapconfigure.h

FORMS    += \
    BandsSelect.ui \
    BandSwitchFrame.ui \
    ContestPage.ui \
    ContestPageControl.ui \
    DisplayOptions.ui \
    OptionsDialog.ui \
    ScreenConfigScreen.ui \
    WsjtxConfigureCQ.ui \
    bandmapclientfilterdialog.ui \
    checkupdates.ui \
    contestdetails.ui \
    defdirsdlg.ui \
    districtframe.ui \
    dmbuttonframe.ui \
    dxccframe.ui \
    FilterFrame.ui \
    LocCalcFrame.ui \
    locframe.ui \
    locTreeFrame.ui \
    n1mmbroadcastconfig.ui \
    qrzdisplayframe.ui \
    qsologframe.ui \
    radiosettingdialog.ui \
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
    runbuttonsframe.ui \
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
    bandmapclientframe.ui \
    Clusterbandmapconfigure.ui \
    txVmExternalButtonDialog.ui \
    txVmInternalSetupDialog.ui \
    txvmbuttonsframe.ui \
    txvminternalbuttondialog.ui \
    txvmrigbuttondialog.ui \
    txvmrigsetupdialog.ui

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

win32{ LIBS += -lUser32 -lole32 -luuid -lshlwapi}

