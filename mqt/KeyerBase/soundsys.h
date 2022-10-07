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

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "riff.h"
#include "SimpleComp.h"
#include "CompressorParams.h"
#include "adis_filter.h"


class RtAudioSoundSystem;
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

#define FRAMES 16
#define FRAMESAMPLES 256
#define RINGBUFFERSIZE 1024

class InBuff
{
public:
    unsigned int frameCount;
    int16_t buff[FRAMESAMPLES * 2];
};

class RiffWriter : public QThread
{
    Q_OBJECT

     RtAudioSoundSystem *ss;

     QWaitCondition bufferNotEmpty;
     QWaitCondition bufferNotFull;
     QMutex mutex;

     InBuff inBuffs[RINGBUFFERSIZE];
     int recIndex = -1;
     int writeIndex = -1;


public:
     bool terminated;
    RiffWriter(RtAudioSoundSystem *parent = nullptr) ;
    virtual ~RiffWriter() override;

    virtual void run() Q_DECL_OVERRIDE;

    void startInput();
    void wakeAll();

    void copyBuffer(int16_t *inStageBuffer, int nFrames);
    void finishInput();

};
class RtAudioSoundSystem: public QObject
{
    Q_OBJECT

private slots:

signals:
    void interruptOK();
    void ssOutputFinished();
    void actionQueueFinished();
    void setVU(unsigned int a, unsigned int b, unsigned int c);

protected:
    void readFromFile(void *outputBuffer, unsigned int nFrames, int16_t &maxvol, qreal &rmsval);

public:
    RtAudioSoundSystem();
    virtual ~RtAudioSoundSystem();

    bool doBWFilter = true;
    bool doCompression = true;

    bool initialise(QString ind , QString outd);
    void stop();
    void closedown();

    QStringList inputDevices;
    QStringList outputDevices;

    unsigned int setRate(unsigned int rate);

    bool startDMA( bool play, const QString &fname, int pipSamples, int16_t *pipptr, int pipStartDelaySamples );
    void stopDMA();

    static RtAudioSoundSystem *createSoundSystem();

    void startOutput();
    void stopOutput();
    void startInput();
    void stopInput();
    bool startInput( QString fn );

    bool startMicPassThrough();
    bool stopMicPassThrough();

    void setVolumeMults(qreal record, qreal replay, qreal passThrough, const CompressorParams &comp, bool df, bool dc);

    void setData(int16_t *data, unsigned int len);
    void setPipData(int16_t *data, unsigned int len, unsigned int delayLen);

    int16_t *dataptr = nullptr;
    unsigned long samples = 0;

    WaveFile *outWave = nullptr;
    void writeDataToFile(void *inp, unsigned int nFrames);
    RiffWriter *wThread = nullptr;

    int audioCallback( void *outputBuffer, void *inputBuffer,
                                    unsigned int nFrames,
                                    double streamTime,
                                    unsigned int status );
private:

    RtAudio *audio = nullptr;

    unsigned int inChannels = 0;
    unsigned int outChannels = 0;

    QMap<QString, int> deviceIds;

    chunkware_simple::SimpleCompRms micCompressor;
    chunkware_simple::SimpleCompRms replayCompressor;

    double makeUpGain = 0.0;

    BWBandPass* micfilter1;
    BWBandPass* micfilter2;

    BWBandPass* replayfilter1;
    BWBandPass* replayfilter2;

    // internal values
    unsigned int sampleRate = 0;

    bool tone = false;

    bool playingFile = false;
    bool recordingFile = false;
    bool passThrough = false;

    bool inputEnabled = false;
    bool outputEnabled = false;
    bool passThroughEnabled = false;

    CompressorParams compression;

    qreal recordMult = 0.0;
    qreal replayMult = 0.0;
    qreal passThroughMult = 0.0;

    qint64 m_pos = 0;
    qint64 p_pos = 0;
    QByteArray m_buffer;
    QByteArray p_buffer;
    unsigned long pipDelayBytes = 0;

};

#endif
