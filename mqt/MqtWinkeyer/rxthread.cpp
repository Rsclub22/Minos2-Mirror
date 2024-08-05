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
#include "RxThread.h"
#include <QSerialPort>
#include <QDebug>

RxThread::RxThread(QSerialPort *serialPort, QObject *parent)
    : QThread(parent), serialPort(serialPort) {}



void RxThread::run()
{
    while (true) {
        if (serialPort->waitForReadyRead(100)) {
            QByteArray data = serialPort->readAll();
            emit dataReceived(data);
        }
    }
}

/*
void RxThread::run()
{
    QByteArray rbuf;
    while (runRxThread)
    {
        if (serial->waitForReadyRead(10)) // 10 ms timeout
         {
            rbuf = serial->readAll();
            for (char value : rbuf)
            {
                if (activeWinKeyerOpenCmd != ADMIN_NONE)
                {
                    if (activeWinKeyerOpenCmd == ADMIN_ECHO)
                    {
                        if (value == '\x55')
                        {
                            emit winKeyerPhysicallyPresent();
                        }
                    }
                    else if (activeWinKeyerOpenCmd == ADMIN_OPEN)
                    {
                        emit winKeyerRevisionReceived(value);
                    }
                    else if (activeWinKeyerOpenCmd == ADMIN_GETMINOR)
                    {
                        emit winKeyerGetMinorReceived(value);
                    }
                    else if (activeWinKeyerOpenCmd == ADMIN_GETTYPE)
                    {
                        emit winKeyerGetTypeReceived(value);
                    }
                }
                else
                {

                    if ((value & 0xC0) == 0xC0)
                    {
                        emit statusChanged(value & 0x3f);
                    }
                    else if (value & 0x80)
                    {
                        emit speedPotChanged((value & 0x7F));
                    }
                    else
                    {
                        emit echoBackReceived(value);
                    }

                }





            }
        }
        msleep(1);
    }
}

void RxThread::stop()
{
    runRxThread = false;
}


void RxThread::setWinKeyerOpenFlag(bool state)
{
    winKeyerOpenFlag = state;
}
bool RxThread::getWinKeyerOpenFlag()
{
    return winKeyerOpenFlag;
}

void RxThread::setActiveWinKeyerOpenCmd(quint8 cmdNum)
{
    activeWinKeyerOpenCmd = cmdNum;
}
quint8 RxThread::getActiveWinKeyerOpenCmd()
{
    return activeWinKeyerOpenCmd;
}
*/
