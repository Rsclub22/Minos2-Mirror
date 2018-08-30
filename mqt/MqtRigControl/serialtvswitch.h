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

    QSerialPort::SerialPortError error();
private slots:

    //void handleError(QSerialPort::SerialPortError error);



private:

    QSerialPort *sComPort = nullptr;
    QByteArray m_msg;



};

#endif // SERIALTVSWITCH_H
