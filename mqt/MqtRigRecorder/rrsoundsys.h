/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef soundsysH
#define soundsysH

#include <QObject>
#include <QThread>
#include <QMap>
#include <QDateTime>
#include <QWaitCondition>
#include <QMutex>

#include "vucallback.h"


#if !defined (_MSC_VER)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
// as we don't want to change rtaudio.h...
#include "RtAudio.h"
#if !defined (_MSC_VER)
#pragma GCC diagnostic pop
#endif

#include "riff.h"

class RtAudio;
class RRRtAudioSoundSystem;

class RRRiffWriter : public QThread
{
    Q_OBJECT

     RRRtAudioSoundSystem *ss;

public:
     bool terminated;
    RRRiffWriter(RRRtAudioSoundSystem *parent = nullptr) ;
    virtual ~RRRiffWriter() override;

    virtual void run() Q_DECL_OVERRIDE;

};

class RRRtAudioSoundSystem: public QObject
{
    Q_OBJECT

public:
    RRRtAudioSoundSystem();
    virtual ~RRRtAudioSoundSystem();

    bool initialise(QString ind, QString ind2);
    void stop();
    void closedown();

    static QWaitCondition bufferNotEmpty;
    static QWaitCondition bufferNotFull;
    static QMutex mutex;

    QStringList inputDevices;

    unsigned int setRate(unsigned int rate);

    bool startDMA(const QString &fname , int ct);
    void stopDMA();

    void startInput();
    void stopInput();
    bool startInput(QString fn , int cycleTime, bool continuation);

    WaveFile *outWave = nullptr;
    void writeDataToFile(void *inp, unsigned int nFrames);
    RRRiffWriter *wThread = nullptr;

    int audioCallback( void *inputBuffer,
                        unsigned int nFrames,
                        double streamTime,
                        RtAudioStreamStatus status,
                        int instance);

    void setVUCallBack(VUCallBack cb);
    void setRecordLevel(int l);
    void setRecordLevel2(int l);
    void setMono(bool);
private:

    RtAudio *audio = nullptr;
    RtAudio *audio2 = nullptr;
    VUCallBack WinVUCallback = nullptr;

    QMap<QString, unsigned int> inChannels;
    QMap<QString, int> deviceIds;

    QString curInDev1;
    QString curInDev2;

    // internal values
    unsigned int sampleRate = 0;
    QDateTime swapTime;
    QString baseName;
    int cycleTime = 10;
    double recordMult = 1.0;
    double recordMult2 = 1.0;

    bool inputEnabled = false;
    bool mono = false;
};

#endif
