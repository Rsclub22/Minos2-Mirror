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

#include "mqtUtils_pch.h"

#include "riff.h"
#include "SimpleComp.h"


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

#include "SimpleComp.h"

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
class LPFilter
{
    int mNumChannels;
    // past data
    double mZx[4];
    double mZy[4];
    // filter coefficients
    double a0;
    double a1;
    double a2;
    double b1;
    double b2;
public:
    LPFilter(){}
    void initialise(int channels, double corner, double sampleRate);
    inline double filterSample (const double inSample, const int channel);
};
class RtAudioSoundSystem: public QObject
{
    Q_OBJECT

private slots:

signals:
    void interruptOK();
    void setActionTime1();
    void actionQueueFinished();
    void setVU(unsigned int a, unsigned int b, unsigned int c);

protected:
    void readFromFile(void *outputBuffer, unsigned int nFrames, int16_t &maxvol, qreal &rmsval);

public:
    RtAudioSoundSystem();
    virtual ~RtAudioSoundSystem();

    bool initialise( QString &errmess );

    unsigned int setRate(unsigned int rate);
    void setFilter(int cf);

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

    void setVolumeMults(qreal record, qreal replay, qreal passThrough);

    void setData(int16_t *data, unsigned int len);
    void setPipData(int16_t *data, unsigned int len, unsigned int delayLen);

    int16_t *dataptr = nullptr;
    unsigned long samples = 0;

    WaveFile outWave;
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

    chunkware_simple::SimpleCompRms compressor;
    LPFilter lpFilter;

    int filterCorner = 0;

    // internal values
    unsigned int sampleRate = 0;

    bool tone = false;

    bool playingFile = false;
    bool recordingFile = false;
    bool passThrough = false;

    bool inputEnabled = false;
    bool outputEnabled = false;
    bool passThroughEnabled = false;

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
