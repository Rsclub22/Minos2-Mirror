/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Pc Serial Port DTR CW Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
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

#include "cwworker.h"


class PcCwKeyer : public QObject
{
    Q_OBJECT

public:
    explicit PcCwKeyer(QObject *parent = nullptr);
    ~PcCwKeyer();

    void setWPM(int wpm);
    void sendText(const QString &text);
    void abortTransmission();

    void openComPort(const QString portName);
    void closeComport(const QString portName);
    void close();

    bool isSerialOpen();
    void setPttPendingFlag(bool on) { pttPending = on; };
    void setPostTxDelayMs(int ms) { postTxDelayMs = ms; }
    void pttOn(bool on);

signals:
    void serialPortOpen(bool);
    void serialPortError(const QString &message);
    void startTxMessage();
    void nextStringRequested();

private slots:
    void handleSerialPortError(QSerialPort::SerialPortError error);
    void onWorkerFinished();

private:
    void enqueueMorseText(const QString &text);
    void enqueueOnOff(bool on, int delayMs);
    QString convertCharToMorse(QChar c);

    QSerialPort serial;
    CwWorker *worker = nullptr;
    double charDot = 60.0; // default for ~20 WPM
    int postTxDelayMs = 250;

    bool pttPending = false;


    bool getPttPendingFlag() const { return pttPending; }
};

#endif // PCCWKEYER_H

