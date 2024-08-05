/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef RXTHREAD_H
#define RXTHREAD_H

#include <QThread>
#include <QSerialPort>

class WinkeyerControl;

class RxThread : public QThread
{
    Q_OBJECT

public:
    explicit RxThread(QSerialPort *serialPort, QObject *parent = nullptr);
    void run() override;
    //void stop();

    //void setWinKeyerOpenFlag(bool state);
    //bool getWinKeyerOpenFlag();

    //void setActiveWinKeyerOpenCmd(quint8 cmdNum);
    //quint8 getActiveWinKeyerOpenCmd();


signals:
    void dataReceived(const QByteArray &data);

    //void statusChanged(int status);
    //void speedPotChanged(int speedPot);
    //void echoBackReceived(char echoBack);
    //void winKeyerPhysicallyPresent();
    //void winKeyerRevisionReceived(quint8 value);
    //void winKeyerGetMinorReceived(quint8 value);
    //void winKeyerGetTypeReceived(quint8 value);

private:
    QSerialPort *serialPort;


    //bool runRxThread;
    //bool winKeyerOpenFlag = false;
    //quint8 activeWinKeyerOpenCmd = ADMIN_NONE;
};

#endif // RXTHREAD_H

