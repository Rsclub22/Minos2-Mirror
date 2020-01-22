/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef SERIALTVSWITCH_H
#define SERIALTVSWITCH_H


#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include "serialcomms.h"

class SerialTVSwitch: public QObject
{
    Q_OBJECT
public:
    SerialTVSwitch(QObject *parent = nullptr);




    void sendTVSwMessage(const QByteArray &msg);
    void closeComport();

    bool openComport(QString comport);
    QString error();


    bool getOpenFlag();
private slots:

    //void handleError(QSerialPort::SerialPortError error);



private:

    QSerialPort *sComPort = nullptr;
    QByteArray m_msg;
    bool openFlag = false;
    QStringList comportErrMsgs = { tr("No Error"), tr("Device Not Found"), tr("Permission Error")
                                   ,tr("Open Error"), tr("Parity Error"), tr("Framing Error")
                                   ,tr("Break Condition"), tr("Write Error"), tr("Read Error")
                                   ,tr("Resource Error"), tr("Unsupported Operation Error")
                                   ,tr("Unknown Error"), tr("Timeout Error"), tr("Not Open Error")
                                  };



};

#endif // SERIALTVSWITCH_H
