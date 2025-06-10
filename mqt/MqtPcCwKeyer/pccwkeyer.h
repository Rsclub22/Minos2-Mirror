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

#ifndef PCCWKEYER_H
#define PCCWKEYER_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QQueue>
#include <QMap>
#include <functional>

class PcCwKeyer : public QObject
{
    Q_OBJECT
public:
    explicit PcCwKeyer(int wpm, bool dtrRts = false, QObject *parent = nullptr);
    ~PcCwKeyer();

    void openComPort(const QString portName);
    void setWPM(int charWpm);
    void sendText(const QString &text);
    bool isBusy() const;


    void abortTransmission();
    void close();

signals:
    void nextStringRequested();
    void serialPortOpen(bool);
    void serialPortError(QString);

private slots:
    void processQueue();
    void handleSerialPortError(QSerialPort::SerialPortError error);

private:
    struct TimedAction {
        std::function<void()> func;
        int delayMs;
    };

    QSerialPort serial;
    Qt::TimerType timerType = Qt::PreciseTimer;
    QTimer timer;   // CW timer

    QQueue<TimedAction> timedActions;

    qreal charDot = 60;
    int spaceDot = 60;

    bool useDtrRts = false;

    void key(bool on);

    void enqueueAction(std::function<void()> func, int delayMs);
    void enqueueSymbolSequence(const QString &morse);



    static inline const QMap<QChar, QString> morseTable = {
        { 'A', ".-" }, { 'B', "-..." }, { 'C', "-.-." }, { 'D', "-.." },
        { 'E', "." }, { 'F', "..-." }, { 'G', "--." }, { 'H', "...." },
        { 'I', ".." }, { 'J', ".---" }, { 'K', "-.-" }, { 'L', ".-.." },
        { 'M', "--" }, { 'N', "-." }, { 'O', "---" }, { 'P', ".--." },
        { 'Q', "--.-" }, { 'R', ".-." }, { 'S', "..." }, { 'T', "-" },
        { 'U', "..-" }, { 'V', "...-" }, { 'W', ".--" }, { 'X', "-..-" },
        { 'Y', "-.--" }, { 'Z', "--.." },
        { '0', "-----" }, { '1', ".----" }, { '2', "..---" }, { '3', "...--" },
        { '4', "....-" }, { '5', "....." }, { '6', "-...." },
         { '7', "--..." }, { '8', "---.." }, { '9', "----." },
          { ' ', " " }
          };

};

#endif // PCCWKEYER_H
