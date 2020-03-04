/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
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
#include "rigcontrol.h"
#include "BandList.h"
#include "serialtvswitch.h"
#include "smeterbar.h"


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
    RigControlRpc *msg = nullptr;

    RigSetupDialog *setupRadio;
    RigControl  *radio;
    QString appName = "";
    QLabel *status;
    int radioIndex;
    QTimer *pollTimer;
    class QTimer LogTimer;
    int pollTime;
    bool rigErrorFlag;
    bool cmdLockFlag;
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
    //rmode_t logMode;
    QString selTvBand;      // selected band from radio
    QString transVertSwNum;
    bool logRitOn;
    bool supVolume;     // radio supports volume
    bool supSignalStrength;
    const int PASSBAND_NOCHANGE = -1;

    QVector<BandDetail> bands;
    QStringList presetFreq;
    bool ignorePresetFreq;           // on contest start
    bool ignorePreviousFreq;        // on contest swap

    int *test_mem;
    // data from radio
    freq_t rfrequency;       // read frequency
    QString sfreq;          // read freq converted to string
    rmode_t rmode;          // read radio mode
    pbwidth_t rwidth;        // read radio rx bw
    double curVfoFrq;
    double curTransVertFrq;
    rmode_t curMode;
    QString sCurMode;
    bool mgmModeFlag;
    QStringList  mgmModes;
    int rRitFreq;        // converted from hamlib long ritFreq
    int curVol;
    int curSignalStrength = 0;

    // rit functions supported by current radio

    bool radioSupGetRit;
    bool radioSupSetRit;
    bool radioSupGetRitState;
    bool radioSupRitOnOff;
    bool radioRitOn;

    bool ritEnable;         // flag to enable rit

    bool radioCommsOK;

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
    int getAndSendFrequency(vfo_t vfo);
    int getAndSendMode(vfo_t vfo);
    //QString convertFreqString(double);

    void setPolltime(int);
    int getPolltime();
    void showStatusMessage(const QString &);
    void hamlibError(int errorCode, QString cmd);
//    void frequency_updated(double frequency);
//    void mode_updated(QString);
    void setFreq(QString, vfo_t vfo);
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
    void sendFreqToLog(freq_t freq);
    void sendModeToLog(QString mode);
    void sendRitEnableStatus(bool status);
    void sendRitEnableStatusLogger();
    void sendVolToLog(int level);
    //void sendRxPbFlagToLog();

    void setMode(QString mode, vfo_t vfo);
    void displayPassband(pbwidth_t width);


    void chkRadioMgmModeChanged();
    void dumpRadioToTraceLog();
    void setRitFreqDisplayVisible(bool state);
    int getRitFreq(vfo_t vfo);
    int setRitFreq(vfo_t vfo, shortfreq_t ritFreq);
    void cmdLockOn();
    void cmdLockOff();
    int getMinosModeIndex(QString mode);


    void setTransVertDisplayVisible(bool visible);
    void writeWindowTitle(QString appName);
    void sendTransVertStatusToLog(bool status);

    void refreshRadio();

    QString getBand(freq_t freq);

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
    int  getRitRadioStatus(vfo_t vfo, bool *status);
    void sendRadioRitStatusLogger(bool status);

    int getVolume(vfo_t vfo);
    int setVolume(vfo_t vfo, int level);
    //void sendVolStatusToLog(bool status);

    int getSignalStrength(vfo_t vfo);
    void displaySignalStrength(int level);


    void initialiseSupportedRadioDisplay();
    void supRadioIndToggle(int offset, displayIndicator::indicatorType type);
    void updateSupportedRadioIndicators();
    void turnOffAllsupRadioIndicators();
    void showActiveTransVertIndicator(QString cb);

    void clearSupportRitFlags();




    void getRitSupportStatus(int modelNumber);
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
    void buildSupportedRadioBands(int radioModelNumber, QStringList& supBandList);
    bool findSupRadioBand(const QString band, const QStringList& supBandsList);
    bool findSupTransBand(const QString band, const int radioIdx);
    //void sendBandListLogger(const int radioIdx, const QStringList &supBandList);

    void initCacheData();

    void addVolStatusToRigCache(const int radIdx, bool status);
    void sendTransVertEnabled(bool status);


    void runRigCtlDaemon(const QString manufacturer, const QString model, const QString comport, const QString baudRate, const QString dataBits, const QString civ, const QString netAdd, const QString portNum, const QString stopBits, const int &parity, const QString handshake, const QString rtsState, const QString dtrState, rigCtldTrace::rigCtldTraceCodes diagnostics);


    int openRigCtldRadio();
    void setRigCltdIndicatorVisible(bool visible);
    void rigCtldIndicatorToggle(bool state);


    bool rigCtldKill();

    int getTXStatus(vfo_t vfo);


    void getRigCtldConnectDelay();


    //void sendIgnorePresetFreqToLog(bool status);
    //void sendIgnorePreviousFreqToLog(bool status);
    void addIgnorePresetFreqToRigCache(bool status);
    void addIgnorePreviousFreqToRigCache(bool status);
    bool readIgnorePresetFreqFlag();
    bool readIgnorePreviousFreqFlag();
    void saveIgnorePresetFreqFlag(bool state);
    void saveIgnorePreviousFreqFlag(bool state);
    void addBandListToRigCache(const int radioIdx, const QStringList &supBandList);


private slots:

    void onStdInRead(QString);
    void saveTraceLogFlag(bool);

    void getRadioInfo();
    void logMessage(QString s);
    void about();
    void LogTimerTimer();

    void loggerSetFreq(QString freq);
    void loggerSetMode(QString mode);
    void loggerSetVolume(int level);
    void currentRadioSettingChanged(QString radioName);
    void updateSelectRadioBox();
    void aboutRigConfig();

    void onSelectRadio(PubSubName, QString mode);
    void selFreqClicked();
    void setupBandFreq();
    void selectRadio();
    void onLaunchSetup();

    void setRitFreq(int ritFreq);
    void setRitLogStatus(bool status);

    void sendRadioListLogger();

    void ritEnableChecked(int state);
    void updateRigDetailsCache();

#ifdef RIGCONTROL_TEST

    void incRit();   // for test.... *************************
    void ritbuttontoggle();  // for test.... *************************
signals:
#endif



    void rigCtldMessage();
    void rigCtldErrorMessage();
    void rigCtldStarted();
    void rigCtldStatusTimeout();
    void onIgnorePresetFreq();
    void onIgnorePreviousFreq();
};

#endif // RIGCONTROLMAINWINDOW_H
