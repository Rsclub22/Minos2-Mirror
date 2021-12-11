/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017 - 2020
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
// Hamlib Library
//
/////////////////////////////////////////////////////////////////////////////



#ifndef RIGCONTROLMAINWINDOW_H
#define RIGCONTROLMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QVector>
#include <QProcess>

#include "mqtUtils_pch.h"
#include "BandList.h"
#include "serialtvswitch.h"
#include "smeterbar.h"
#include "rigcommon.h"
#include "rigbase.h"
#include "rigfactory.h"
#include "rigcapabilities.h"
#include "serialCommonData.h"


class QLabel;
class QComboBox;
class QBitArray;
class RigSetupDialog;
class RigControl;
class RigControlRpc;






const bool RIGCTLD_ON = true;
const bool RIGCTLD_OFF = false;


namespace Ui {
class RigControlMainWindow;
}

namespace displayIndicator {
    enum indicatorType { OFF, RADIO, TRANSVERT, TRANSVERT_ON};
}

namespace rigCtldTrace {
    enum rigCtldTraceCodes {rctNONE, rctBUG, rctERR, rctWARN, rctVERBOSE, rctTRACE};
    const QStringList  rigCtldTraceStr = {"", "-v", "-vv", "-vvv", "-vvvv", "-vvvvv"};
}

//#define RIGCONTROL_TEST

void delay(int sec);
void sleepFor(qint64 milliseconds);


class LoggerRequests
{

public:

    LoggerRequests(){clear();}

    void clear()
    {
        logger_freq = Frequency(0);
        selRadioFreq =  Frequency(0);
        logRitOn = false;
    }

    Frequency logger_freq;
    Frequency selRadioFreq;
    QString selBand;
    QString slogMode;
    QString selRadioMode;   // onSelectRadio mode from logger at startup
    bool logRitOn;
};

class RigCtldDetails
{
public:
    RigCtldDetails(){clear();}

    void clear()
    {
        irigctld_radioNumber = 0;
        rigCtldConnectDelay = 0;
    }


    QString rigctld_radioNumber;
    int irigctld_radioNumber;
    QString rigctld_radioName;
    QString rigctld_radioMfg;

    int rigCtldConnectDelay;
};

class RigSupCapabilities
{
public:
    RigSupCapabilities(){clear();}


    void clear()
    {
        supVolume = false;
        supSignalStrength = false;
        radioSupGetRit = false;
        radioSupSetRit = false;
        radioSupGetRitState = false;
        radioSupSetRitState = false;
        supportGetVfo = false;
        supportSetVfo = false;
        supportGetPtt = false;
        supportSetPtt = false;
        supportVoiceMemory = false;
        suportSendMorse = false;
        supportStopMorse = false;
        supportWaitMorse = false;

    }

    bool supVolume;     // radio supports volume
    bool supSignalStrength;
    bool radioSupGetRit;
    bool radioSupSetRit;
    bool radioSupGetRitState;
    bool radioSupSetRitState;
    bool supportGetVfo;
    bool supportSetVfo;
    bool supportGetPtt;
    bool supportSetPtt;
    bool supportVoiceMemory;
    bool suportSendMorse;
    bool supportStopMorse;
    bool supportWaitMorse;

};


class RigStateDetails
{

public:

  RigStateDetails(){clear();}

  void clear()
  {
     radioIndex = 0;
     pollTime = 1000;
     rfrequency = Frequency(0);
     curVfoFreq = Frequency(0);
     curOtherFreq = Frequency(0);
     selTransverterNum = NO_TRANSVERTER_NUM;
     mgmModeFlag = false;
     radioRitOn = false;
     ritEnable = false;
     ritMaxKHzFreq = MAX_RITFREQ;
     ritKHzFlag = false;
     radioCommsOK = false;
     rigErrorFlag = false;
     traceCommsFlag = false;
     curVol = 0;
     curSignalStrength = 0;
     curPttStatus = false;

  }

  RigCapabilities rigCap;
  int radioIndex;
  int pollTime;
  Frequency rfrequency;
  MODE rmode;
  VFO curVfo;
  Frequency curVfoFreq;
  Frequency curTransVertFreq;
  Frequency curOtherFreq;
  QString curBand;
  QString curTransvertBand;
  int selTransverterNum = 0;
  QString selTvBand;
  QString transVertSwNum;
  QString selTransVertBandIndicator;
  MODE curMode;
  QString curModeStr;
  bool mgmModeFlag;
  QStringList mgmModes;
  bool radioRitOn;
  ShortFreq rRitFreq;
  bool ritEnable;
  int ritMaxKHzFreq;
  bool ritKHzFlag;
  bool radioCommsOK;
  int curVol;
  int curSignalStrength;
  bool rigErrorFlag;
  bool curPttStatus;
  bool traceCommsFlag;

};

class SupIndicatorDetails
{

public:

    SupIndicatorDetails(){
        supIndicator = nullptr;
        supIndicatorLabel = nullptr;
    };


QPushButton *supIndicator;
QLabel *supIndicatorLabel;
QString bandType;


};

class RigControlMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RigControlMainWindow(QWidget *parent = nullptr);
    ~RigControlMainWindow();

    //bool freqPresetChanged = false;

    const QString version = "2.20";


private:

    Ui::RigControlMainWindow *ui;
    StdInReader stdinReader;
    bool closeApp = false;
    RigControlRpc *msg = nullptr;


    RigBase  *radio;
    RigFactory* rigFactory;

    RigStateDetails *rigStateDetails;
    QTimer *RigCtldStatusTimer = nullptr;
    QProcess *rigCtldProcess = nullptr;

    LoggerRequests *loggerRequests;
    RigCtldDetails *rigCtldDetails;
    RigSupCapabilities *selectedRigSupCap;

    //RigCapabilities rigCap;
    QString appName = "";
    QLabel *status;
    int radioIndex;
    QTimer *pollTimer;
    class QTimer LogTimer;

    bool cmdLockFlag;

    int selTransverterNum;
    QString transVertSwNum;
    SerialTVSwitch *serialTVSw = nullptr;



    const int PASSBAND_NOCHANGE = -1;

    QVector<QSharedPointer<BandInfo>  > bands;

    scatParams currentRadio;
    QString currentRadioName;
    //QStringList availRadios;



    bool radioCommsOK;

    bool ritTestEnabled = false; // for test....


    //SerialTVSwitch *serialTVSw = nullptr;

    QString geoStr;         // geometry registry location


    QVector<QPushButton*> allSupRadioInd;
    QVector<QLabel*> allSupRadioIndLabels;

    QMap<QString, QSharedPointer<SupIndicatorDetails> > allBandSupRadioInd;


    QString selTransVertBandIndicator = "";

    void initActionsConnections();
    void initSelectRadioBox();
    void setSelectRadioBoxVisible(bool visible);
    void setRadioNameLabelVisible(bool visible);
    int openRadio();
    void closeRadio();
    int getAndSendFrequency(VFO vfo);
    int getAndSendMode(VFO vfo);
    //QString convertFreqString(double);

    void setPolltime(int);
    int getPolltime();
    void showStatusMessage(const QString &);
    void radioError(int errorCode, QString cmd);
//    void frequency_updated(double frequency);
//    void mode_updated(QString);
    void setFreq(Frequency, VFO vfo);
    void displayFreqVfo(Frequency);

    void displayModeVfo(QString);

    void displayTransVertVfo(Frequency frequency);

    void readTraceLogFlag();

    void closeEvent(QCloseEvent *event);

    //void sendBandListLogger();
    void sendStatusLogger(const QString &message);
    void sendStatusToLogDisConnected();
    void sendStatusToLogConnected();
    void sendStatusToLogError(QString);
    //void sendTransVertOffsetToLogger(QString transvertName);
    void sendTransVertSwitchToLogger(const QString &swNum);
    void sendFreqToLog(const Frequency &freq);
    void sendModeToLog(QString mode);
    void sendRitEnableStatus(bool status);
    void sendRitEnableStatusLogger();
    void sendVolToLog(int level);
    //void sendRxPbFlagToLog();

    void setMode(QString mode, VFO vfo);
    //void displayPassband(pbwidth_t width);


    void chkRadioMgmModeChanged();
    void dumpRadioToTraceLog();
    void setRitFreqDisplayVisible(bool state);
    int getRitFreq(VFO vfo);
    void setRitFreq(VFO vfo, const ShortFreq &ritFreq);
    void cmdLockOn();
    void cmdLockOff();

    void setTransVertDisplayVisible(bool visible);
    void writeWindowTitle(QString appName);
    void sendTransVertStatusToLog(bool status);

    void refreshRadio();

    QString getBand(const Frequency &freq);

    void testBoxesVisible(bool visible);

    void upDateRadio(QString radioName);
    //void loadBands();

    void sendTransVertSwitchToComPort(const QString &swNum);
    void sendRitFreqLogger(const ShortFreq &ritFreq);

    void setRitEnableDisplayVisible(bool s);
    void setRitOnOffDisplayVisible(bool s);
    void setRitOnOffDisplay(bool s);
    void setRitEnableDisplay(bool s);
    void ritIndicatorToggle(bool state);
    //void setRitStatusIndicatorsVisible(bool state);
    int  getRitRadioStatus(VFO vfo, bool *status);
    void sendRadioRitStatusLogger(bool status);

    int getVolume(VFO vfo);
    int setVolume(VFO vfo, int level);
    //void sendVolStatusToLog(bool status);

    int getSignalStrength(VFO vfo);
    void displaySignalStrength(int level);


    void initialiseSupportedRadioDisplay();
    //void supRadioIndToggle(int offset, displayIndicator::indicatorType type);
    void supRadioIndToggle(QString band, displayIndicator::indicatorType type);
    void updateSupportedRadioIndicators();
    void turnOffAllsupRadioIndicators();
    void showActiveTransVertIndicator(QString cb);

    void clearSupportRitFlags();




    void getRitSupportStatus();
    void setRitGetSetFreqIndicatorVisible(bool state);
    void ritSetFreqIndicatorToggle(bool state);
    void ritGetFreqIndicatorToggle(bool state);
    //void saveRitEnableChk(bool state);
    //bool readRitEnableChk();

    void getRigctldNames(QString address, quint16 port);
    void clrRigctldNames();

    //bool findSupRadioBand(const QString band);
    //void buildSupportedRadioBands(int radioModelNumber);
    //void buildSupBandList(int radioModelNumber);
    //bool findSupTransBand(const QString band);

    void clearTransVertSupport();

    bool readTestStandAloneFlag();
    //void buildSupBandList(int radioIdx, int radioModelNumber, QStringList &bandList);
    void buildSupportedRadioBands(int radioIdx, int radioModelNumber, QStringList& supBandList);
    bool findSupRadioBand(const QString band, const QStringList& supBandsList);
    //bool findSupTransBand(const QString band, const int radioIdx);
    //void sendBandListLogger(const int radioIdx, const QStringList &supBandList);

    //void initCacheData();

    void addVolStatusToRigCache(bool status);
    void sendTransVertEnabled(bool status);


    void runRigCtlDaemon(const QString manufacturer, const QString model, const QString comport, const QString baudRate, const QString dataBits, const QString civ, const QString netAdd, const QString portNum, const QString stopBits, const QString parity, const QString handshake, const QString rtsState, const QString dtrState, rigCtldTrace::rigCtldTraceCodes diagnostics);


    int openRigCtldRadio(bool localRigCtld);
    void setRigCltdIndicatorVisible(bool visible);
    void setRigCtldIndicator(RIGCTLD_INDICATOR_ID idNum);


    bool rigCtldKill();

    int getTXStatus(VFO vfo);


    void getRigCtldConnectDelay();

    void updateCurrentRadioFromAvailRadios(QString radioName);

    void setSmeterVisible(bool visible);

    //void addBandListToRigCache(const int radioIdx, const QStringList &supBandList);



    int getRxFreq(VFO vfo);
    void processRxFrequencyForDisplay();

    int getAndSendVfo();
    void sendMaxRitFreqLogger();

    void showRitTestControl(bool state);




    void getAndSendTransVertSwNum(QString transvertName);
    bool findTransverter(QString &transVerterBand, QString band);

    void setPttIndOnOff(bool state);
    void setpttIndVisible(bool visible);
    void setCwMemIndOnOff(bool state);

    void setVoiceMemIndOnOff(bool state);


    void setPttGroupItemsVisible(bool visible);
    void setMemoryGroupVisible(bool visible);
    void setVoiceMemIndVisible(bool visible);
    void setCwMemIndVisible(bool visible);
    void setTxRxIndOnOff(bool state);
    void sendPttTypeLogger();
    void sendPttEnabledLogger();
    void sendPttStateLogger();


    void setIndicatorVisible(const QString bandType, const bool visible);
    void sendRadioSwitchCompleteToLogger();
    bool selectTransverter(QString &band, Frequency &f);
    int setTxState(VFO vfo, bool txState);


    void checkSupportVolume();
    void checkSupportSMeter();
    void checkSupportRit();
    bool checkSupportVoiceMemory();
    bool checkSupportCwKeyerMemory();
    void checkSupportPtt();


    void checkSupportPollRadio();
    void setupTransVerter();
    void addVoiceMemStatusToRigCache(bool status);
    void addCwKeyerMemoryStatusToRigCache(int cwMemType);
    void addPTTEnabledStatusToRigCache(bool status);
    QString getRigCtldExePath();
    QString getRigCtldExeName();
    void setRigCtldExePath(const QString &);
    void setRigCtldExeName(const QString &);

    void readTranVerterSetting(scatParams *radio, QString transvertName, QSettings &config);
    void getRadioConfigData(scatParams *radio, QString radioName);
    void checkIniFileVersion();
    void updateAvailRadiosToVersion2(QSettings &settings, QStringList &availRadios, int numAvailRadios);

    void getAvailRadiosList(QStringList &availRadios);
    void readCurrentRadio(QString &currentRadioName);

    void saveCurrentRadio(const QString currentRadioName);
    void buildSupportedRadioBands(scatParams *radioData, QStringList &supBandList);
    bool findSupTransBand(const QString band, const scatParams *radioData);
    void buildSupBandList(scatParams *radioData, QStringList &bandList);
    void initCacheData(QStringList &availRadios);
    void addBandListToRigCache(const QString radioName, const QStringList &supBandList);
    void checkSupportCatFeatures();
    bool availRadiosContains(const QString radioName);
    void setRigControlPttState(bool state);
    void handleIcomCwMessage(QString cwMsg);
    void handleYaesuCwMessage(QString msg);
private slots:

    void onStdInRead(QString);
    void saveTraceLogFlag(bool);

    void getRadioInfo(bool pubNow);
    //void getRadioInfo();
    void logMessage(QString s);
    void about();
    void LogTimerTimer();

    void loggerSetFreq(Frequency freq);
    void loggerSetMode(QString mode);
    void loggerSetVolume(int level);
    void currentRadioSettingChanged(QString radioName);
    void updateSelectRadioBox();
    void aboutRigConfig();

    void onSelectRadio(PubSubName, QString, Frequency freq, QString mode);
    void selFreqClicked();
    //void setupBandFreq();
    void selectRadio(int index);
    void onLaunchSetup();
    void onConfigureRigctld();

    void setRitFreq(ShortFreq ritFreq);
    void setRitLogStatus(bool status);

    void sendRadioListLogger(const QStringList &availRadios);

    //void ritEnableChecked(int state);
    void updateRigDetailsCache();



    void testIncRit(int value);  // for test.... *************************
    void ritbuttontoggle();  // for test.... *************************


    void rigCtldMessage();
    void rigCtldErrorMessage();
    void rigCtldStarted();
    void rigCtldStatusTimeout();

    void onNewRxFreq(Frequency);
    void onNewMode();
    void onRigStatus(int status, QString cmd);

    void onRitOn();
    void onRitOff();
    void onRitOffset();
    void onRit0();
    void onNewVfo(QString omniRigVfo);





    void pollRadioInfo();


    void onSetVoiceMessageNum(QString msgNum);
    void onCwKeyerPbClicked();
    void onCwKeyerStopPbClicked();

    void loggerSetBand(QString band);
    void onTxPttTestPbClicked();
    void onSetPttOnOff(bool pttOnOff);
    void onPttState(bool state);
    void onSetCwTxMessage(QString cwMsg);
    void on_reconnectButton_clicked();

};

#endif // RIGCONTROLMAINWINDOW_H
