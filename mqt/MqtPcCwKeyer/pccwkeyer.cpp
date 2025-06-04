#include "pccwkeyer.h"
#include <QtMath>
#include <QDebug>
#include <QTime>




PcCwKeyer::PcCwKeyer(const QString &portName, int wpm, int farnsworthWpm, bool sidetone, QObject *parent)
    : QObject(parent)
{
    serial.setPortName(portName);
    serial.setBaudRate(QSerialPort::Baud9600);
    if (!serial.open(QIODevice::ReadWrite)) {
        qFatal("Failed to open port %s", qPrintable(portName));
    }

    key(false);
    setWPM(wpm, farnsworthWpm);

    connect(&timer, &QTimer::timeout, this, &PcCwKeyer::processQueue);
/*
    if (sidetone) {
        toneData = generateTone(600, 100);  // 600 Hz, 100ms segment
        toneBuffer.setData(toneData);
        toneBuffer.open(QIODevice::ReadOnly);

        QAudioFormat format;
        format.setSampleRate(44100);
        format.setChannelCount(1);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        format.setSampleFormat(QAudioFormat::Int16);
#else
        format.setSampleSize(16);
        format.setSampleType(QAudioFormat::SignedInt);
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setCodec("audio/pcm");
#endif

        audioOut = new QAudioOutput(format, this);
    }
*/
}

PcCwKeyer::~PcCwKeyer() {
    key(false);
    playTone(false);
    serial.close();
}

void PcCwKeyer::setWPM(int charWpm, int wordWpm) {
    charDot = 1200 / charWpm;
    if (wordWpm > 0 && wordWpm < charWpm) {
        spaceDot = 1200 / wordWpm;
        useFarnsworth = true;
    } else {
        spaceDot = charDot;
        useFarnsworth = false;
    }
}

void PcCwKeyer::sendText(const QString &text) {
    QString upper = text.toUpper();
    for (QChar ch : upper) {
        if (!morseTable.contains(ch)) continue;
        QString code = morseTable[ch];

        if (code == " ") {
            enqueueDelay(spaceDot * 7);  // word space
        } else {
            enqueueSymbolSequence(code);
            enqueueDelay(spaceDot * 3);  // inter-char space
        }
    }

    if (!timer.isActive())
        timer.start(1);  // <-- fine resolution timer
}

void PcCwKeyer::enqueueSymbolSequence(const QString &morse) {
    for (int i = 0; i < morse.length(); ++i) {
        int toneLen = (morse[i] == '.') ? charDot : charDot * 3;

        qDebug() << "Symbol:" << morse[i] << "duration:" << toneLen;

        actions.enqueue([this] { key(true); playTone(true); });
        enqueueDelay(toneLen);
        actions.enqueue([this] { key(false); playTone(false); });

        if (i != morse.length() - 1)
            enqueueDelay(spaceDot);  // Inter-element space
    }
}

void PcCwKeyer::enqueueDelay(int ms) {
    for (int i = 0; i < ms; ++i)
        actions.enqueue([] {});
}


void PcCwKeyer::key(bool on) {
    qDebug() << "DTR" << (on ? "ON" : "OFF") << "at" << QTime::currentTime().toString("hh:mm:ss.zzz");

    serial.setDataTerminalReady(on);
}

void PcCwKeyer::playTone(bool on) {

    return;

    if (!audioOut) return;
    if (on) {
        toneBuffer.seek(0);
        audioOut->start(&toneBuffer);
    } else {
        audioOut->stop();
    }
}

QByteArray PcCwKeyer::generateTone(int frequency, int durationMs, int sampleRate) {
    const int samples = sampleRate * durationMs / 1000;
    QByteArray data;
    data.resize(samples * sizeof(qint16));
    qint16 *buf = reinterpret_cast<qint16*>(data.data());

    for (int i = 0; i < samples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        buf[i] = static_cast<qint16>(32767 * qSin(2 * M_PI * frequency * t));
    }

    return data;
}

void PcCwKeyer::processQueue() {
    if (!actions.isEmpty()) {
        auto act = actions.dequeue();
        act();
    } else {
        timer.stop();
        key(false);
        playTone(false);
    }
}

bool PcCwKeyer::isBusy() const {
    return !actions.isEmpty();
}


