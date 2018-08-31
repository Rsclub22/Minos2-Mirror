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


#include "serialtvswitch.h"
#include <QSerialPortInfo>

SerialTVSwitch::SerialTVSwitch(QObject *parent) : QObject(parent)
{

}


void SerialTVSwitch::sendTVSwMessage(const QByteArray &msg)
{
    m_msg = msg;
    sComPort->write(m_msg);
}

bool SerialTVSwitch::openComport(const QString comport)
{
    sComPort = new QSerialPort;
    sComPort->setPortName(comport);
    //sComPort->setBaudRate(QSerialPort::Baud9600);

    openFlag = sComPort->open(QIODevice::WriteOnly);
    return openFlag;

}

void SerialTVSwitch::closeComport()
{
    if (sComPort)
    {
        sComPort->close();
        delete sComPort;
    }
}

bool SerialTVSwitch::getOpenFlag()
{
    return openFlag;
}

QString SerialTVSwitch::error()
{
    QSerialPort::SerialPortError err = sComPort->error();
    return comportErrMsgs[err];
}
