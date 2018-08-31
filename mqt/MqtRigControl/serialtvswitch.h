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
    QStringList comportErrMsgs = { "No Error", "Device Not Found", "Permission Error"
                                   ,"Open Error", "Parity Error", "Framing Error"
                                   ,"Break Condition", "Write Error", "Read Error"
                                   ,"Resource Error", "Unsupported Operation Error"
                                   ,"Unknown Error", "Timeout Error", "Not Open Error"
                                  };



};

#endif // SERIALTVSWITCH_H
