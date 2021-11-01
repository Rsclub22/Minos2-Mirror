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
class RtAudioSoundSystem;

class RiffWriter : public QThread
{
    Q_OBJECT

     RtAudioSoundSystem *ss;
public:
     bool terminated;
    RiffWriter(RtAudioSoundSystem *parent = nullptr) ;
    virtual ~RiffWriter() override;

    virtual void run() Q_DECL_OVERRIDE;

};

class RtAudioSoundSystem: public QObject
{
    Q_OBJECT

public:
    RtAudioSoundSystem();
    virtual ~RtAudioSoundSystem();

    virtual bool initialise(QString ind);
    void stop();
    void closedown();

    QStringList inputDevices;

    virtual unsigned int setRate(unsigned int rate);

    virtual bool startDMA(const QString &fname , int ct);
    virtual void stopDMA();

    void startInput();
    void stopInput();
    bool startInput(QString fn , int cycleTime, bool continuation);

    WaveFile *outWave = nullptr;
    void writeDataToFile(void *inp, unsigned int nFrames);
    RiffWriter *wThread = nullptr;

    int audioCallback( void *outputBuffer, void *inputBuffer,
                                    unsigned int nFrames,
                                    double streamTime,
                                    RtAudioStreamStatus status );

    void setVUCallBack(VUCallBack cb);
    void setRecordLevel(int l);
    void setMono(bool);
private:

    RtAudio *audio = nullptr;
    VUCallBack WinVUCallback;

    unsigned int inChannels = 0;
    unsigned int outChannels = 0;

    QMap<QString, int> deviceIds;


    // internal values
    unsigned int sampleRate = 0;
    QDateTime swapTime;
    QString baseName;
    int cycleTime = 10;
    double recordMult = 1.0;

    bool inputEnabled = false;
    bool mono = false;
};

#endif
