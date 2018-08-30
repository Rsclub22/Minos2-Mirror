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
    SerialTVSwitch(QString comport, QObject *parent = nullptr);

    ~SerialTVSwitch();


    void sendTVSwMessage(QByteArray msg);
    void closeComport();

private:

    QSerialPort *sComPort = nullptr;
    SerialComms *tVSwPort = nullptr;


};

#endif // SERIALTVSWITCH_H
