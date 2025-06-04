#ifndef PCCWKEYER_H
#define PCCWKEYER_H

#include <QObject>
#include <QSerialPort>
#include <QAudioOutput>
#include <QIODevice>
#include <QTimer>
#include <QQueue>
#include <QMap>
#include <QByteArray>
#include <QBuffer>

class PcCwKeyer : public QObject {
    Q_OBJECT

public:
    PcCwKeyer(const QString &portName, int wpm = 20, int farnsworthWpm = -1, bool sidetone = false, QObject *parent = nullptr);
    ~PcCwKeyer();

    void sendText(const QString &text);
    void setWPM(int charWpm, int wordWpm = -1);  // wordWpm = -1 disables Farnsworth
    bool isBusy() const;

private slots:
    void processQueue();

private:
    void enqueueSymbolSequence(const QString &morse);
    void enqueueDelay(int ms);
    void key(bool on);
    void playTone(bool on);
    QByteArray generateTone(int frequency, int durationMs, int sampleRate = 44100);

    QSerialPort serial;
    QTimer timer;
    QQueue<std::function<void()>> actions;

    int charDot;  // ms per dot at character speed
    int spaceDot; // ms per dot at word spacing speed (Farnsworth)
    bool useFarnsworth;




    // Sidetone
    QAudioOutput *audioOut = nullptr;
    QBuffer toneBuffer;
    QByteArray toneData;

    const QMap<QChar, QString> morseTable = {
        {'A', ".-"},    {'B', "-..."},  {'C', "-.-."},  {'D', "-.."},
        {'E', "."},     {'F', "..-."},  {'G', "--."},   {'H', "...."},
        {'I', ".."},    {'J', ".---"},  {'K', "-.-"},   {'L', ".-.."},
        {'M', "--"},    {'N', "-."},    {'O', "---"},   {'P', ".--."},
        {'Q', "--.-"},  {'R', ".-."},   {'S', "..."},   {'T', "-"},
        {'U', "..-"},   {'V', "...-"},  {'W', ".--"},   {'X', "-..-"},
        {'Y', "-.--"},  {'Z', "--.."},  {'1', ".----"}, {'2', "..---"},
        {'3', "...--"}, {'4', "....-"}, {'5', "....."}, {'6', "-...."},
        {'7', "--..."}, {'8', "---.."}, {'9', "----."}, {'0', "-----"},
        {' ', " "}
    };
};

#endif // PCCWKEYER_H




