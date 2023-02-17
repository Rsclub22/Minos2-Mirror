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


#include "serialPTT.h"
#include <QtSerialPort/QSerialPortInfo>

SerialPTT::SerialPTT(QObject *parent) : QObject(parent)
{

}


void SerialPTT::setPTT(bool p)
{
    if (dtr)
    {
        sComPort->setDataTerminalReady(p);
    }
    if (rts)
    {
        sComPort->setRequestToSend(p);
    }
}

bool SerialPTT::openComport(const QString comport, bool r, bool d)
{
    if (sComPort && sComPort->isOpen())
    {
        return true;
    }
    if (comport.isEmpty())
    {
        openFlag = false;
        return false;
    }
    dtr = d;
    rts = r;
    if (!sComPort)
    {
        sComPort = new QSerialPort;
    }
    sComPort->setPortName(comport);
    //sComPort->setBaudRate(QSerialPort::Baud9600);

    openFlag = sComPort->open(QIODevice::WriteOnly);
    return openFlag;

}

void SerialPTT::closeComport()
{

    if (openFlag)
    {
        sComPort->close();
        delete sComPort;
        sComPort = nullptr;
        openFlag = false;
    }

}

void SerialPTT::setRtsDtr(bool r, bool d)
{
    rts = r;
    dtr = d;
}

bool SerialPTT::isOpen()
{
    return openFlag;
}

QString SerialPTT::error()
{
    if (sComPort)
    {
        QSerialPort::SerialPortError err = sComPort->error();
        return comportErrMsgs[err];
    }
    else
    {
        return tr("Serial port not open");
    }
}
