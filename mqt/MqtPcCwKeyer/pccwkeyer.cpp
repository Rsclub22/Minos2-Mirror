#include "pccwkeyer.h"
#include <QtMath>
#include <QDebug>
#include <QTime>




PcCwKeyer::PcCwKeyer(const QString &portName, int wpm, int farnsworthWpm, bool sidetone, bool dtrRts, QObject *parent)
    : QObject(parent), useSidetone(sidetone), useDtrRts(dtrRts) {
    serial.setPortName(portName);
    serial.setBaudRate(QSerialPort::Baud9600);
    if (!serial.open(QIODevice::ReadWrite)) {
        qFatal("Failed to open port %s", qPrintable(portName));

    }

    key(false);
    setWPM(wpm, farnsworthWpm);

    connect(&timer, &QTimer::timeout, this, &PcCwKeyer::processQueue);
    timer.setSingleShot(true);

    if (useSidetone) {
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
}

PcCwKeyer::~PcCwKeyer() {
    key(false);
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
    for (const QChar ch : upper) {
        if (!morseTable.contains(ch)) continue;
        QString code = morseTable[ch];

        if (code == " ") {
            enqueueAction([] {}, spaceDot * 7);  // word space
        } else {
            enqueueSymbolSequence(code);
            enqueueAction([] {}, spaceDot * 3);  // inter-char space
        }
    }

    if (!timer.isActive()) {
        timer.start(1);
    }
}

void PcCwKeyer::enqueueSymbolSequence(const QString &morse) {
    for (int i = 0; i < morse.length(); ++i) {
        int toneLen = (morse[i] == '.') ? charDot : charDot * 3;

        enqueueAction([this, toneLen] {
            key(true);
            if (useSidetone) playToneFor(toneLen);
        }, toneLen);

        enqueueAction([this] {
            key(false);
        }, spaceDot);
    }
}

void PcCwKeyer::enqueueAction(std::function<void()> func, int delayMs) {
    timedActions.enqueue({func, delayMs});
}

void PcCwKeyer::processQueue() {
    if (timedActions.isEmpty()) {
        key(false);
        emit nextStringRequested();
        return;
    }

    auto next = timedActions.dequeue();
    next.func();

    if (!timedActions.isEmpty()) {
        timer.start(next.delayMs);
    } else {
        timer.stop();
        emit nextStringRequested();
    }
}

void PcCwKeyer::key(bool on) {

    if (useDtrRts)
    {
       serial.setDataTerminalReady(on);
    }
    else
    {
        serial.setReadBufferSize(on);
    }

}

void PcCwKeyer::playToneFor(int durationMs) {
    if (!audioOut) return;

    QByteArray data = generateTone(600, durationMs);
    QBuffer* buffer = new QBuffer(this);
    buffer->setData(data);
    buffer->open(QIODevice::ReadOnly);

    connect(audioOut, &QAudioOutput::stateChanged, buffer, [buffer](QAudio::State state) {
        if (state == QAudio::IdleState || state == QAudio::StoppedState)
            buffer->deleteLater();
    });

    audioOut->start(buffer);
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

bool PcCwKeyer::isBusy() const {
    return !timedActions.isEmpty();
}




void PcCwKeyer::setUseSideTone(bool useSideTone_)
{

    if (useSidetone == useSideTone_)
        return;  // no change

    useSidetone = useSideTone_;

    if (useSidetone)
    {
        // Create new audioOut if not already created
        if (!audioOut)
        {
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
    }
    else
    {
        if (audioOut)
        {
            audioOut->stop();
            delete audioOut;
            audioOut = nullptr;
        }
    }
}




void PcCwKeyer::abortTransmission()
{
    // Stop any pending CW actions
    timer.stop();
    timedActions.clear();

    // Unkey the transmitter
    key(false);

    // Stop sidetone if active
    if (useSidetone && audioOut) {
        audioOut->stop();
    }

    qDebug() << "CW transmission aborted.";
}


void PcCwKeyer::close()
{
    if (serial.isOpen())
    {
        serial.close();
    }
}
