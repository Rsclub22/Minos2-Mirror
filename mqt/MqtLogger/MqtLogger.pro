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
#DEPENDPATH += $$PWD/../MqtRotator
INCLUDEPATH += $$PWD/../MqtRigControl
#DEPENDPATH += $$PWD/../MqtRigControl
INCLUDEPATH += $$PWD/../MqtBandMap
#DEPENDPATH += $$PWD/../MqtBandMap

SOURCES += \
    AdifImport.cpp \
    ContestApp.cpp \
    ContestDetailsTransferObject.cpp \
    FilterFrame.cpp \
    LocCalcFrame.cpp \
    LoggerContacts.cpp \
    LoggerContest.cpp \
    MatchArchiveFrame.cpp \
    MatchOtherFrame.cpp \
    MatchThisFrame.cpp \
    MatchThread.cpp \
    MatchTreeFrame.cpp \
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
    contestdetails.cpp \
    districtframe.cpp \
    dxccframe.cpp \
    enqdlg.cpp \
    freqlineedit.cpp \
    locTreeFrame.cpp \
    locframe.cpp \
    main.cpp \
    profiles.cpp \
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
    tminoshelpform.cpp \
    tqsoeditdlg.cpp \
    tsettingseditdlg.cpp \
    tsinglelogframe.cpp \
    tstatsdispframe.cpp \
    volumeslider.cpp \
    clusterframe.cpp

HEADERS  += \
    AdifImport.h \
    ContestApp.h \
    ContestDetailsTransferObject.h \
    FilterFrame.h \
    LocCalcFrame.h \
    LoggerContacts.h \
    LoggerContest.h \
    MatchArchiveFrame.h \
    MatchOtherFrame.h \
    MatchThisFrame.h \
    MatchThread.h \
    MatchTreeFrame.h \
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
    contestdetails.h \
    districtframe.h \
    dxccframe.h \
    enqdlg.h \
    freqlineedit.h \
    locTreeFrame.h \
    locframe.h \
    printfile.h \
    profiles.h \
    qsologframe.h \
    reg1test.h \
    rigcontrolframe.h \
    rigmemcommondata.h \
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
    tminoshelpform.h \
    tqsoeditdlg.h \
    tsettingseditdlg.h \
    tsinglelogframe.h \
    tstatsdispframe.h \
    volumeslider.h \
    clusterframe.h

FORMS    += \
    contestdetails.ui \
    districtframe.ui \
    dxccframe.ui \
    FilterFrame.ui \
    LocCalcFrame.ui \
    locframe.ui \
    locTreeFrame.ui \
    MatchTreeFrame.ui \
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
    tminoshelpform.ui \
    tqsoeditdlg.ui \
    TSessionManager.ui \
    tsettingseditdlg.ui \
    tsinglelogframe.ui \
    tstatsdispframe.ui


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
