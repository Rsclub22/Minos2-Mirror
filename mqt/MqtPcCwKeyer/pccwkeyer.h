/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Pc Serial Port DTR CW Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef PCCWKEYER_H
#define PCCWKEYER_H

#include <QObject>
#include <QSerialPort>
#include <QHash>
#include <QChar>

#include "CwWorker.h"

class PcCwKeyer : public QObject
{
    Q_OBJECT

public:
    explicit PcCwKeyer(QObject *parent = nullptr);
    ~PcCwKeyer();

    void openComPort(const QString portName);
    void closeComport(const QString portName);
    void setWPM(int wpm);
    void sendText(const QString &text);

    void abortTransmission();
    void close();

    void pttOn(bool on);
    void setPttPendingFlag(bool state){pttPendingOff = state;}
    bool getPttPendingFlag(){return pttPendingOff;}

    void setPostTxDelayMs(int delay){postTxDelayMs = delay;}

    static QString convertCharToMorse(QChar c);


    bool isSerialOpen();
signals:
    void requestKey(bool on);
    void nextStringRequested();
    void serialPortOpen(bool success);
    void serialPortError(const QString &error);

    void startTxMessage();
    void finished();  // Emitted when current CW transmission ends


private slots:
    void key(bool on);
    void handleSerialPortError(QSerialPort::SerialPortError error);
    void onWorkerFinished();

private:
    QSerialPort serial;
    qreal charDot = 60.0; // default
    CwWorker *worker = nullptr;

    bool pttPendingOff = false;

    int postTxDelayMs = 0;



    void enqueueMorseText(const QString &text);


};

#endif // PCCWKEYER_H
