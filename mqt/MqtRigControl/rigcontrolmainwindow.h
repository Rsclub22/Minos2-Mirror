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


void delay(int sec);



namespace Ui {
class RigControlMainWindow;
}

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
    int radioIndex = 0;
    QTimer *pollTimer;
    class QTimer LogTimer;
    int pollTime;
    bool rigErrorFlag = false;
    bool cmdLockFlag = false;
    // data from logger
    QString logger_freq;
    QString slogMode;
    QString selRadioMode;   // onSelectRadio mode from logger at startup
    rmode_t logMode;
    QString selTvBand;      // selected band from radio
    QString transVertSwNum;
    bool logRitOn;
    bool supVolume = false;     // radio supports volume
    bool supSignalStrength = false;
    const int PASSBAND_NOCHANGE = -1;

    QVector<BandDetail*> bands;
    QStringList presetFreq;

    // data from radio
    freq_t rfrequency;       // read frequency
    QString sfreq;          // read freq converted to string
    rmode_t rmode;          // read radio mode
    pbwidth_t rwidth;        // read radio rx bw
    double curVfoFrq = 0;
    double curTransVertFrq = 0;
    rmode_t curMode;
    QString sCurMode;
    bool mgmModeFlag = false;
    shortfreq_t rRitFreq = 0;
    QString sRitFreq;
    int curVol = 0;
    int curSignalStrength = 0;


    SerialTVSwitch *serialTVSw = nullptr;

    int tpm = 1;

    QString geoStr;         // geometry registry location



    void initActionsConnections();
    void initSelectRadioBox();
    void setSelectRadioBoxVisible(bool visible);
    void setRadioNameLabelVisible(bool visible);
    void openRadio();
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

    void sendRadioListLogger();
    void sendBandListLogger();
    void sendStatusLogger(const QString &message);
    void sendStatusToLogDisConnected();
    void sendStatusToLogConnected();
    void sendStatusToLogError();
    void sendTransVertOffsetToLogger(int tvNum);
    void sendTransVertSwitchToLogger(const QString &swNum);
    void sendErrorMessageToLogger(QString errMsg);
    void sendFreqToLog(freq_t freq);
    void sendModeToLog(QString mode);
    void sendRitEnableStatus(bool status);
    void sendRitEnableStatusLogger();
    void sendTpm(int tpm);
    void sendVolToLog(int level);
    //void sendRxPbFlagToLog();

    void setMode(QString mode, vfo_t vfo);
    void displayPassband(pbwidth_t width);


    void chkRadioMgmModeChanged();
    void dumpRadioToTraceLog();
    void setRitDisplayVisible(bool state);
    int getRitFreq(vfo_t vfo);
    int setRitFreq(vfo_t vfo, shortfreq_t ritFreq);
    void cmdLockOn();
    void cmdLockOff();
    int getMinosModeIndex(QString mode);


    void setTransVertDisplayVisible(bool visible);
    void writeWindowTitle(QString appName);
    void sendTransVertStatus(bool status);

    void refreshRadio();

    QString getBand(freq_t freq);

    void testBoxesVisible(bool visible);

    void upDateRadio();
    void loadBands();

    void sendTransVertSwitchToComPort(const QString &swNum);
    void sendRitFreqLogger(double ritFreq);

    void setRitEnableDisplayVisible(bool s);
    void setRitOnOffDisplayVisible(bool s);
    void setRitOnOffDisplay(bool s);
    void setRitEnableDisplay(bool s);
    int getVolume(vfo_t vfo);
    int setVolume(vfo_t vfo, int level);

    int getSignalStrength(vfo_t vfo);
    void displaySignalStrength(int level);
    void sendVolStatusToLog(bool status);

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
    void setTpm(int tpm, QString f);

    void setRitFreqStr(QString ritFreq);
    void setRitLogStatus(bool status);



signals:


};

#endif // RIGCONTROLMAINWINDOW_H
