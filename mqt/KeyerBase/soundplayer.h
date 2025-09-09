/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef SoundPlayerH
#define SoundPlayerH

#include <QDateTime>
#include "riff.h"

class SoundPlayer;
class RtAudio;
class dvkFile;

class SoundPlayer: public QObject
{
    Q_OBJECT

signals:
    void interruptOK();
    void ssOutputFinished();
    void actionQueueFinished();
    void soundAvailable();


protected:
    void readFromFile(void *outputBuffer, unsigned int nFrames);

public:
    SoundPlayer();
    virtual ~SoundPlayer();

    bool initialise();
    void stop();
    void closedown();

    static void playSound(QString fname);

    QStringList inputDevices;
    QStringList outputDevices;

    QString defaultInput;
    QString defaultOutput;

    unsigned int setRate(unsigned int rate);

    bool startDMA( );
    void stopDMA();

    static SoundPlayer *createSoundPlayer();

    void startOutput();
    void stopOutput();

    void setVolumeMults(qreal replay);

    void setData(int16_t *data, unsigned int len);

    dvkFile *dvkf = nullptr;
    int16_t *dataptr = nullptr;
    int numChannels = 0;
    uint32_t samples = 0;   /* fsample for current file  */

    WaveFile *outWave = nullptr;
    void writeDataToFile(void *inp, unsigned int nFrames);

    int audioCallback( void *outputBuffer, void *inputBuffer,
                                    unsigned int nFrames,
                                    double streamTime,
                                    unsigned int status );
private:

    RtAudio *audio = nullptr;
    static SoundPlayer *soundPlayer;

    QMap<QString, int> deviceIds;
    QMap<QString, unsigned int> inChannels;
    QMap<QString, unsigned int> outChannels;

    QString curInDev;
    QString curOutDev;

    // internal values
    unsigned int sampleRate = 0;

    bool playingFile = false;

    bool outputEnabled = false;

    qreal replayMult = 0.0;

    qint64 m_pos = 0;
    qint64 p_pos = 0;
    QByteArray m_buffer;
    void doPlaySound(QString fname);
};

#endif
