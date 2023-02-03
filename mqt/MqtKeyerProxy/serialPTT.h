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


#ifndef SERIALPTT_H
#define SERIALPTT_H


#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QByteArray>
#include <QStringList>

class SerialPTT: public QObject
{
    Q_OBJECT
public:
    SerialPTT(QObject *parent = nullptr);
    void setPTT(bool b);

    void closeComport();

    void setRtsDtr(bool r, bool d);
    bool openComport(QString comport, bool r, bool d);
    QString error();


    bool isOpen();

private:

    QSerialPort *sComPort = nullptr;
    bool openFlag = false;
    QStringList comportErrMsgs = { tr("No Error"), tr("Device Not Found"), tr("Permission Error")
                                   ,tr("Open Error"), tr("Parity Error"), tr("Framing Error")
                                   ,tr("Break Condition"), tr("Write Error"), tr("Read Error")
                                   ,tr("Resource Error"), tr("Unsupported Operation Error")
                                   ,tr("Unknown Error"), tr("Timeout Error"), tr("Not Open Error")
                                  };

    bool rts = false;
    bool dtr = false;

};

#endif // SERIALPTT_H
