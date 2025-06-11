/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Pc Serial Port DTR CW Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////





#include "PcCwKeyer.h"
#include <QtMath>
#include <QTimer>
#include <QDebug>


PcCwKeyer::PcCwKeyer(int wpm, QObject *parent)
    : QObject(parent)
{
    setWPM(wpm);
    key(false);

    worker = new CwWorker(this);
    connect(worker, &CwWorker::finished, this, &PcCwKeyer::onWorkerFinished);

    connect(this, &PcCwKeyer::requestKey, this, &PcCwKeyer::key);


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
    qDebug() << "WPM set to" << wpm << ", dot duration:" << charDot;
}

void PcCwKeyer::openComPort(const QString portName)
{
    serial.setPortName(portName);
    serial.setBaudRate(QSerialPort::Baud9600);
    if (!serial.open(QIODevice::ReadWrite)) {
        qFatal("Failed to open port %s", qPrintable(portName));
        emit serialPortOpen(false);
    } else {
        connect(&serial, &QSerialPort::errorOccurred, this, &PcCwKeyer::handleSerialPortError);
        emit serialPortOpen(true);
    }
}

void PcCwKeyer::sendText(const QString &text)
{
    enqueueMorseText(text);
}

void PcCwKeyer::enqueueMorseText(const QString &text)
{
    QString upperText = text.toUpper();

    for (int i = 0; i < upperText.length(); ++i) {
        QChar c = upperText[i];

        if (c == ' ') {
            // Word gap = 7 dot units, minus the 1 unit already added after last symbol
            qint32 gapDuration = static_cast<int>(charDot * 6);
            qDebug() << "Word gap: OFF for" << gapDuration << "ms";
            worker->enqueueAction([] {}, gapDuration);
            continue;
        }

        QString morse = convertCharToMorse(c);
        if (morse.isEmpty())
            continue;  // Skip unknown characters

        for (int j = 0; j < morse.length(); ++j) {
            QChar symbol = morse[j];
            qreal onMs = (symbol == '.') ? charDot : charDot * 3;

            // Key down
            worker->enqueueAction([this, symbol, onMs] {
                qDebug().nospace() << "Emit key ON: symbol '" << symbol << "' duration " << onMs << " ms";
                emit requestKey(true);
            }, static_cast<int>(onMs));

            // Key up and symbol gap (always 1 unit)
            worker->enqueueAction([this] {
                qDebug() << "Emit key OFF: symbol gap duration" << charDot << "ms";
                emit requestKey(false);
            }, static_cast<int>(charDot));
        }

        // Inter-character gap = 3 dot units total (1 already added), so add 2 more
        if (i + 1 < upperText.length() && upperText[i + 1] != ' ') {
            qint32 gapDuration = static_cast<int>(charDot * 2);
            qDebug() << "Inter-character gap: OFF for" << gapDuration << "ms";
            worker->enqueueAction([] {}, gapDuration);
        }
    }

    worker->start();
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

        { '"', ".-..-."}, {'\'', "...-..-"}, {'(', "-.--.-"}, {')', "-.--.-"},
        {'[', "-.--."}, {']', "-.--.-"}, {'+', ".-.-."}, {',', "--..--"},
        {'-', "-....-"}, {'.', ".-.-.-"}, {'/', "-..-."}, {':', "---..."},
        {';', "-.-.-."}, {'=', "-...-"}, {'?', "..--."}, {'@', ".--.-."},
        {'_', "..--.-"}, {'!', "---."}
    };

    return morseMap.value(c, QString());
}

void PcCwKeyer::abortTransmission()
{
    worker->clear();
    emit requestKey(false);
    qDebug() << "CW transmission aborted.";
}

void PcCwKeyer::close()
{
    if (serial.isOpen()) {
        serial.close();
    }
}

void PcCwKeyer::key(bool on)
{
    serial.setDataTerminalReady(on);

}

void PcCwKeyer::pttOn(bool on)
{
    qDebug() << "PTT " << (on ? "On" : "Off");
    serial.setRequestToSend(on);
}

void PcCwKeyer::handleSerialPortError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        qDebug() << "Serial port disconnected";
        emit serialPortError(serial.errorString());
    }
}

void PcCwKeyer::onWorkerFinished()
{
    if (getPttPendingFlag())
    {
        QTimer::singleShot(postTxDelayMs, this, [this]() {
            pttOn(false);  // Turn off transmitter
            emit nextStringRequested();
        });
        setPttPendingFlag(false);
    }
    else
    {
        emit nextStringRequested();
    }


}

