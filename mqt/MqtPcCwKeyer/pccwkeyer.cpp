/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Pc Serial Port DTR CW Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
//
//
/////////////////////////////////////////////////////////////////////////////




#include "PcCwKeyer.h"
#include "MTrace.h"
#include <QTimer>
#include <QDebug>

PcCwKeyer::PcCwKeyer(QObject *parent)
    : QObject(parent)
{
    worker = new CwWorker(this);
    connect(worker, &CwWorker::finished, this, &PcCwKeyer::onWorkerFinished);
}

PcCwKeyer::~PcCwKeyer()
{
    abortTransmission();
    delete worker;
    close();
}

void PcCwKeyer::setWPM(int wpm)
{
    charDot = 1200.0 / wpm;
    qDebug() << "WPM set to" << wpm << ", dot duration:" << charDot << "ms";
}

void PcCwKeyer::openComPort(const QString portName)
{
    serial.setPortName(portName);
    serial.setBaudRate(QSerialPort::Baud9600);
    if (!serial.open(QIODevice::ReadWrite))
    {

        pttOn(false);
        emit serialPortOpen(false);
        return;
    }

    connect(&serial, &QSerialPort::errorOccurred, this, &PcCwKeyer::handleSerialPortError);

    worker->setSerialPort(&serial);

    emit serialPortOpen(true);
}

void PcCwKeyer::closeComport(const QString portName)
{
    if (serial.isOpen()) {
        qDebug() << "Closing port" << portName;
        serial.close();
    }
}

void PcCwKeyer::close()
{
    if (serial.isOpen())
        serial.close();
}

bool PcCwKeyer::isSerialOpen()
{
    return serial.isOpen();
}

void PcCwKeyer::sendText(const QString &text)
{
    enqueueMorseText(text);
}

void PcCwKeyer::enqueueMorseText(const QString &text)
{
    emit startTxMessage();

    QString upperText = text.toUpper();

    for (int i = 0; i < upperText.length(); ++i)
    {
        QChar c = upperText[i];

        if (c == ' ')
        {
            int gapDuration = static_cast<int>(charDot * 6); // Word gap
            worker->enqueueAction([] {}, gapDuration);
            continue;
        }

        QString morse = convertCharToMorse(c);
        if (morse.isEmpty())
            continue;

        for (int j = 0; j < morse.length(); ++j)
        {
            const bool isDot = morse[j] == '.';
            int onMs = static_cast<int>(charDot * (isDot ? 1 : 3));

            enqueueOnOff(true, onMs);  // Key down
            enqueueOnOff(false, static_cast<int>(charDot));  // Symbol gap
        }

        // Inter-character gap
        if (i + 1 < upperText.length() && upperText[i + 1] != ' ')
        {
            int gapDuration = static_cast<int>(charDot * 2);
            worker->enqueueAction([] {}, gapDuration);
        }
    }

    worker->start();
}

void PcCwKeyer::enqueueOnOff(bool on, int delayMs)
{
    worker->enqueueKey(on, delayMs);
}

QString PcCwKeyer::convertCharToMorse(QChar c)
{
    static const QMap<QChar, QString> morseMap {
        { 'A', ".-" },    { 'B', "-..." },  { 'C', "-.-." }, { 'D', "-.." },
        { 'E', "." },     { 'F', "..-." },  { 'G', "--." },  { 'H', "...." },
        { 'I', ".." },    { 'J', ".---" },  { 'K', "-.-" },  { 'L', ".-.." },
        { 'M', "--" },    { 'N', "-." },    { 'O', "---" },  { 'P', ".--." },
        { 'Q', "--.-" },  { 'R', ".-." },   { 'S', "..." },  { 'T', "-" },
        { 'U', "..-" },   { 'V', "...-" },  { 'W', ".--" },  { 'X', "-..-" },
        { 'Y', "-.--" },  { 'Z', "--.." },

        { '0', "-----" }, { '1', ".----" }, { '2', "..---" }, { '3', "...--" },
        { '4', "....-" }, { '5', "....." }, { '6', "-...." }, { '7', "--..." },
        { '8', "---.." }, { '9', "----." },

        { '"', ".-..-."}, {'\'', "...-..-"}, {'(', "-.--."}, {')', "-.--.-"},
        {'[', "-.--."}, {']', "-.--.-"}, {'+', ".-.-."}, {',', "--..--"},
        {'-', "-....-"}, {'.', ".-.-.-"}, {'/', "-..-."}, {':', "---..."},
        {';', "-.-.-."}, {'=', "-...-"}, {'?', "..--.."}, {'@', ".--.-."},
        {'_', "..--.-"}, {'!', "---."}
    };

    return morseMap.value(c, QString());
}

void PcCwKeyer::abortTransmission()
{
    worker->clear();
    pttOn(false);
    trace("CW transmission aborted.");
}

void PcCwKeyer::pttOn(bool on)
{
    serial.setRequestToSend(on);
}

void PcCwKeyer::handleSerialPortError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        qDebug() << "Serial port error:" << serial.errorString();
        emit serialPortError(serial.errorString());
    }
}

void PcCwKeyer::onWorkerFinished()
{
    if (getPttPendingFlag()) {
        QTimer::singleShot(postTxDelayMs, this, [this]() {
            pttOn(false);
            emit nextStringRequested();
        });
        setPttPendingFlag(false);
    }
    else
    {
        emit nextStringRequested();
    }
}
