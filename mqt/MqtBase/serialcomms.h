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

#ifndef SERIALCOMMS_H
#define SERIALCOMMS_H

#include <QObject>
#include <QByteArray>
#include <QSerialPort>
#include <QTextStream>
#include <QTimer>

class SerialComms : public QObject
{
    Q_OBJECT

public:
    explicit SerialComms(QSerialPort *serialPort, QObject *parent = nullptr);
    virtual ~SerialComms() override
    {}

     void write(QByteArray writeData);
     void setMsgTermChar(char c);

     void close();
     void open();

signals:
     void serialPortError(QString portName, QString errorString);
     void noDataRxTimeout(QString portName);
     void failedToWriteData(QString portName, QString errorString);
     void failToWriteAllData(QString portName, QString errorString);
     void writeTimeout(QString portName, QString errorString);
     void dataSuccesfullySent(QString portName);
     void msgReceived();

private slots:

    void handleBytesWritten(qint64 bytes);
    void handleTimeoutWrite();
    void handleError(QSerialPort::SerialPortError error);

    void handleReadyRead();
    void handleTimeoutRead();


private:
    QSerialPort *m_serialPort = nullptr;
    QByteArray m_writeData;
    QByteArray m_readData;
    char msgTermChar = '\n';
    QTextStream m_standardOutput;
    qint64 m_bytesWritten = 0;
    QTimer wrt_timer;
    QTimer rd_timer;



};

#endif // SERIALCOMMS_H



