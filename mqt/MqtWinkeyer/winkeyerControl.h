/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
//                  Based on example code from K1EL.
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////
#ifndef WinkeyerControl_H
#define WinkeyerControl_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QThread>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QByteArray>
#include "rxthread.h"
#include "txthread.h"
#include "winKeyerCommon.h"



class WinkeyerControl : public QObject {
    Q_OBJECT

public:
    explicit WinkeyerControl(QObject *parent = nullptr);
    ~WinkeyerControl();

   // bool openSerialPort(const QString &portName, int baudRate);
   // void closeSerialPort();

    void enqueueData(const QByteArray &data);

    void openWinKeyer();
    void closeWinKeyer();

    int initComport(QString comport);
    void start();
    void stop();

    bool isSerialPortOpen();
    void wk_st_freq(quint8 value);

    void SetWEchoBack(quint8 value);
    quint8 getWEchoBack();

    void setWkStatus1(quint8 value);
    quint8 getWkStatus1();

    void setWkSpeedPot(quint8 value);
    quint8 getWkSpeedPot();

    void setDoGetPot(bool state);
    bool getDoGetPot();

    void setDoBlock(bool state);
    bool getDoBlock();


    void setDumpPot(bool state);
    bool getDumpPot();

    void setPotLock(bool state);
    bool getPotLock();

    void setDumpPdl(bool state);
    bool getDumpPdl();

    void setDumpDebug(bool state);
    bool getDumpDebug();

    void setDumpState(bool state);
    bool getDumpState();

    void setBreakinClear(bool state);
    bool getBreakinClear();

    void setNewVolume(bool state);
    bool getNewVolume();

    void setNewX2mode(bool state);
    bool getNewX2mode();

    void setNewRTTY(bool state);
    bool getNewRTTY();

    void setNewClear(bool state);
    bool getNewClear();

    void setCurPause(bool state);
    bool getCurPause();

    void setNewPause(bool state);
    bool getNewPause();

    void setcurTune(bool state);
    bool getcurTune();

    void setNewTune(bool state);
    bool getNewTune();

    void setCurDirect(quint8 direct_);
    quint8 getCurDirect();

    void setNewDirect(quint8 direct_);
    quint8 getNewDirect();

    void setHscwValue(quint8 value);
    quint8 getHscwValue();

    void setX2modeValue(quint8 value);
    quint8 getX2modeValue();

    void setVolumeValue(quint8 value);
    quint8 getVolumeValue();

    void setRttyValue1(quint8 value);
    quint8 getRttyValue1();

    void setRttyValue2(quint8 value);
    quint8 getRttyValue2();

    void setDoSoftReset(bool state);
    bool getDoSoftReset();

    void setDoBackSpace(bool state);
    bool getDoBackSpace();

    void setCloseTXFlag(bool state);
    bool getCloseTxFlag();

    void setSetHscwFlag(bool state);
    bool getSetHscwFlag();

    void setWK1Flag(bool state);
    bool getWK1Flag();

    void setWK2Flag(bool state);
    bool getWK2Flag();

    void setWK3Flag(bool state);
    bool getWK3Flag();

    QSharedPointer<WinkeyerStateStorage> getCurrentWinkeyStateStoragePtr();
    QSharedPointer<WinkeyerStateStorage> getNewWinkeyStateStoragePtr();

    int wkSendBufferedChar(quint8 ch);
    int wkGetXoffStatus();
    int wkSetDitDahRation(quint8 value);
    int wkGetEcho();
    int wkGetSpeedPot();
    int wkGetStatus();
    int wkDirectKey(quint8 value);
    int wkKeySample(quint8 value);
    int wkSetKComp(quint8 value);
    int wkSetMode(quint8 value);
    int wkSetFarns(quint8 speed);
    int wkKey(bool state);
    int wkSetRTTY(quint8 WKrtty, quint8 RYmode);
    int wkSetStVolume(quint8 volume);
    int wkSetX2Mode(quint8 x2mode);
    int wkPincfg();
    int wkPincfg(quint8 value);
    int wkBackspace();
    int wkPauseCw(bool state);
    int wkSetupSpeedPot(quint8 minwpm, quint8 wpmrange);
    int wkSetPttLeadTail(quint8 lead, quint8 tail);
    int wkSetWeight(quint8 value);
    int wkSetWpmSpeed(quint8 value);
    int wkSetSidetoneFreq(quint8 value);
    int wkSetBuffPtt(bool state);
    int wkSetBufKey(bool state);
    int wkStartBufferedWait(quint8 sec);
    int wkMergeChar(quint8 c1, quint8 c2);
    int wkBufferedSpeedChange(quint8 wpm);
    int wkBufferedHscw(quint8 wpm);
    int wkCancelBufferedWpm();
    int wkBufferedNull();


signals:
    void dataReceived(const QByteArray &data);
    void writeData(const QByteArray &data);

    void winKeyerOpenStatus(bool open);


private slots:
    void handleDataReceived(const QByteArray &data);


    void handleSerialTimeout();

private:
    QSerialPort *serialPort;
    TxThread *txThread;
    RxThread *rxThread;

    QMutex mutex;
    QQueue<QByteArray> txQueue;
    QWaitCondition txCondition;

    friend class TxThread;

    QTimer *serialTimeoutTimer;
    int serialTimeoutInterval = 1000;

    bool winKeyerOpenFlag = false;

    // These values are received when opening Winkeyer
    // except if ignoreExtraDataOnOpening is true, then
    // we only send is present cmd and open cmd

    bool ignoreExtraDataOnOpening = true;

    quint8 activeSerialCmd = 0;


    quint8 wkVersion;
    quint8 wkMinor;
    quint8 chipType;
    bool wkIsOpen = false;
    bool RTTY_OK = false;
    quint8 showMsg = 0;
    quint8 wEchoBack = 0;
    quint8 wkSpeedPot = 0;
    quint8 tune;
    quint8 pause;
    bool potLock = false;
    int isWK2 = 0;
    int isWK23 = 0;
    int isWK3 = 0;
    quint8 wkStatus1;
    bool ESC = false;


    bool newTune = false;
    bool curTune = false;
    bool newPause = false;
    bool curPause = false;
    bool newClear = false;
    bool newRTTY = false;
    bool newX2mode = false;
    bool newVolume = false;
    bool breakinClear = false;
    bool dumpState = false;
    bool dumpDebug = false;
    bool dumpPdl = false;
    bool dumpPot = false;
    bool doBlock = false;
    bool doGetPot = false;
    bool doSoftReset = false;
    bool doBackSpace = false;
    bool closeTx = false;
    //int validCallBack = 0;
    quint8 curDirect = 0;
    quint8 newDirect = 0;
    bool setHscw = false;
    bool setWK1 = false;
    bool setWK2 = false;
    bool setWK3 = false;
    quint8 hscwValue;
    quint8 x2modeValue;
    quint8 volumeValue;
    quint8 rttyValue1 = 0;
    quint8 rttyValue2 = 0;



    QSharedPointer<WinkeyerState> currentWinkeyerStatePtr;
    QSharedPointer<WinkeyerState> newWinkeyerStatePtr;
    QSharedPointer<WinkeyerStateStorage> currentWinkeyStateStoragePtr;
    QSharedPointer<WinkeyerStateStorage> newWinkeyStateStoragePtr;


    // cmds sent as part of open keyer sequence
    void testWinKeyerConnected();
    void sendOpenWinKeyerCmd();
    void getWinKeyerMinor();
    void getWinKeyerType();
    void completeOpenCmd();
    void statusChanged(quint8 status);
    void speedPotChanged(quint8 speedPot);
    void echoBackReceived(quint8 echoBack);
    void startSerialTimeout(quint8 currentCmd);
    void stopSerialTimeout();


    QString getAdminCmdText(quint8 cmdNum);

    void sendSerialData(QByteArray &data);
};

#endif // WinkeyerControl_H

