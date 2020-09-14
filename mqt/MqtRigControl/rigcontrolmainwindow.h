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
    enum rigCtldTraceCodes {NONE, BUG, ERR, WARN, VERBOSE, TRACE};
    const QStringList  rigCtldTraceStr = {"", "-v", "-vv", "-vvv", "-vvvv", "-vvvvv"};
}

//#define RIGCONTROL_TEST

void delay(int sec);
void sleepFor(qint64 milliseconds);



class RigControlMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RigControlMainWindow(QWidget *parent = nullptr);
    ~RigControlMainWindow();

    bool freqPresetChanged = false;

    const QString version = "2.20";


private:

    Ui::RigControlMainWindow *ui;
    StdInReader stdinReader;
    bool closeApp = false;
    RigControlRpc *msg = nullptr;

    RigSetupDialog *setupRadio;
    RigBase  *radio;
    RigFactory* rigFactory;
    RigCapabilities rigCap;
    QString appName = "";
    QLabel *status;
    int radioIndex;
    QTimer *pollTimer;
    class QTimer LogTimer;
    int pollTime;
    bool rigErrorFlag;
    bool cmdLockFlag;
    bool traceCommsFlag;
    // data from rigctld
    QProcess *rigCtldProcess;
    QString rigctld_radioNumber;
    int irigctld_radioNumber = 0;
    QString rigctld_radioName;
    QString rigctld_radioMfg;
    QTimer *RigCtldStatusTimer;
    int rigCtldConnectDelay;

    // data from logger
    QString logger_freq;
    QString slogMode;
    QString selRadioMode;   // onSelectRadio mode from logger at startup
    QString selRadioFreq;
    //rmode_t logMode;
    QString selTvBand;      // selected band from radio
    QString transVertSwNum;
    bool logRitOn;
    bool supVolume;     // radio supports volume
    bool supSignalStrength;
    const int PASSBAND_NOCHANGE = -1;

    QVector<QSharedPointer<BandInfo>  > bands;
    QStringList presetFreq;


    // data from radio
    Frequency rfrequency;       // read frequency
    QString sfreq;          // read freq converted to string
    MODE rmode;          // read radio mode
    //pbwidth_t rwidth;        // read radio rx bw
    VFO curVfo;
    bool supportGetVfo = false;
    bool supportSetVfo = false;
    Frequency curVfoFrq;
    Frequency curTransVertFrq;
    MODE curMode;
    QString sCurMode;
    bool mgmModeFlag;
    QStringList  mgmModes;
    ShortFreq rRitFreq;
    int ritMaxKHzFreq;
    bool ritKHzFlag;
    int curVol;
    int curSignalStrength = 0;

    // rit functions supported by current radio

    bool radioSupGetRit;
    bool radioSupSetRit;
    bool radioSupGetRitState;
    bool radioSupSetRitState;
    bool radioRitOn;

    bool ritEnable;         // flag to enable rit

    bool radioCommsOK;

    bool ritTestEnabled = false; // for test....

    SerialTVSwitch *serialTVSw = nullptr;

    QString geoStr;         // geometry registry location


    QVector<QPushButton*> supRadioInd;
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
    void setFreq(QString, VFO vfo);
    void displayFreqVfo(double);

    void displayModeVfo(QString);


    void displayTransVertVfo(double frequency);




    void readTraceLogFlag();


    void closeEvent(QCloseEvent *event);


    //void sendBandListLogger();
    void sendStatusLogger(const QString &message);
    void sendStatusToLogDisConnected();
    void sendStatusToLogConnected();
    void sendStatusToLogError(QString);
    void sendTransVertOffsetToLogger(int tvNum);
    void sendTransVertSwitchToLogger(const QString &swNum);
    void sendFreqToLog(Frequency freq);
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
    void setRitFreq(VFO vfo, ShortFreq ritFreq);
    void cmdLockOn();
    void cmdLockOff();
    int getMinosModeIndex(QString mode);


    void setTransVertDisplayVisible(bool visible);
    void writeWindowTitle(QString appName);
    void sendTransVertStatusToLog(bool status);

    void refreshRadio();

    QString getBand(Frequency freq);

    void testBoxesVisible(bool visible);

    void upDateRadio();
    //void loadBands();

    void sendTransVertSwitchToComPort(const QString &swNum);
    void sendRitFreqLogger(int ritFreq);

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
    void supRadioIndToggle(int offset, displayIndicator::indicatorType type);
    void updateSupportedRadioIndicators();
    void turnOffAllsupRadioIndicators();
    void showActiveTransVertIndicator(QString cb);

    void clearSupportRitFlags();




    void getRitSupportStatus();
    void setRitGetSetFreqIndicatorVisible(bool state);
    void ritSetFreqIndicatorToggle(bool state);
    void ritGetFreqIndicatorToggle(bool state);
    void saveRitEnableChk(bool state);
    bool readRitEnableChk();

    void getRigctldNames(QString address, quint16 port);
    void clrRigctldNames();

    //bool findSupRadioBand(const QString band);
    //void buildSupportedRadioBands(int radioModelNumber);
    //void buildSupBandList(int radioModelNumber);
    //bool findSupTransBand(const QString band);

    void clearTransVertSupport();

    bool readTestStandAloneFlag();
    void buildSupBandList(int radioIdx, int radioModelNumber, QStringList &bandList);
    void buildSupportedRadioBands(int radioIdx, int radioModelNumber, QStringList& supBandList);
    bool findSupRadioBand(const QString band, const QStringList& supBandsList);
    bool findSupTransBand(const QString band, const int radioIdx);
    //void sendBandListLogger(const int radioIdx, const QStringList &supBandList);

    void initCacheData();

    void addVolStatusToRigCache(const int radIdx, bool status);
    void sendTransVertEnabled(bool status);


    void runRigCtlDaemon(const QString manufacturer, const QString model, const QString comport, const QString baudRate, const QString dataBits, const QString civ, const QString netAdd, const QString portNum, const QString stopBits, const QString parity, const QString handshake, const QString rtsState, const QString dtrState, rigCtldTrace::rigCtldTraceCodes diagnostics);


    int openRigCtldRadio(bool localRigCtld);
    void setRigCltdIndicatorVisible(bool visible);
    void setRigCtldIndicator(RIGCTLD_INDICATOR_ID idNum);


    bool rigCtldKill();

    int getTXStatus(VFO vfo);


    void getRigCtldConnectDelay();



    MODE mapQStrMode(QString mode);
    void updateCurrentRadioFromAvailRadios(int ridx);
    MODE convertQStringToMode(QString modeStr);
    QString convertModeToQString(MODE m);



    void setSmeterVisible(bool visible);

    void addBandListToRigCache(const int radioIdx, const QStringList &supBandList);



    int getRxFreq(VFO vfo);
    void processRxFrequencyForDisplay();

    int getAndSendVfo();
    void sendMaxRitFreqLogger();

    void showRitTestControl(bool state);



private slots:

    void onStdInRead(QString);
    void saveTraceLogFlag(bool);

    void getRadioInfo(bool pubNow);
    //void getRadioInfo();
    void logMessage(QString s);
    void about();
    void LogTimerTimer();

    void loggerSetFreq(QString freq);
    void loggerSetMode(QString mode);
    void loggerSetVolume(int level);
    void currentRadioSettingChanged(QString radioName);
    void updateSelectRadioBox();
    void aboutRigConfig();

    void onSelectRadio(PubSubName, QString freq, QString mode);
    void selFreqClicked();
    void setupBandFreq();
    void selectRadio();
    void onLaunchSetup();

    void setRitFreq(int ritFreq);
    void setRitLogStatus(bool status);

    void sendRadioListLogger();

    void ritEnableChecked(int state);
    void updateRigDetailsCache();



    void testIncRit(int value);  // for test.... *************************
    void ritbuttontoggle();  // for test.... *************************


    void rigCtldMessage();
    void rigCtldErrorMessage();
    void rigCtldStarted();
    void rigCtldStatusTimeout();

    void onNewRxFreq(quint64);
    void onNewMode();
    void onRigStatus(int status, QString cmd);

    void onRitOn();
    void onRitOff();
    void onRitOffset();
    void onRit0();
    void onNewVfo(QString omniRigVfo);





    void pollRadioInfo();

};

#endif // RIGCONTROLMAINWINDOW_H
