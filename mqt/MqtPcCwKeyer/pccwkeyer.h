#ifndef PCCWKEYER_H
#define PCCWKEYER_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QQueue>
#include <QAudioOutput>
#include <QBuffer>
#include <QMap>
#include <functional>

class PcCwKeyer : public QObject
{
    Q_OBJECT
public:
    explicit PcCwKeyer(const QString &portName, int wpm, int farnsworthWpm = 0, bool sidetone = false, bool dtrRts = false, QObject *parent = nullptr);
    ~PcCwKeyer();

    void setWPM(int charWpm, int wordWpm = 0);
    void sendText(const QString &text);
    bool isBusy() const;

    void setUseSideTone(bool useSideTone_);
    void abortTransmission();
    void close();
private slots:
    void processQueue();

private:
    struct TimedAction {
        std::function<void()> func;
        int delayMs;
    };

    QSerialPort serial;
    QTimer timer;
    QQueue<TimedAction> timedActions;

    int charDot = 60;
    int spaceDot = 60;
    bool useFarnsworth = false;
    bool useSidetone = false;
    bool useDtrRts = false;

    QAudioOutput *audioOut = nullptr;

    void key(bool on);
    void playToneFor(int durationMs);
    void enqueueAction(std::function<void()> func, int delayMs);
    void enqueueSymbolSequence(const QString &morse);
    QByteArray generateTone(int frequency, int durationMs, int sampleRate = 44100);



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
