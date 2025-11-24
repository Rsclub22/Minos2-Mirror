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

#include <QObject>
#include <QDateTime>
#include <QMap>

class SoundPlayer;
class RtAudio;
class spFile;

class SoundPlayer: public QObject
{
    Q_OBJECT

protected:
    bool readFromFile(void *outputBuffer, unsigned int nFrames);

public:
    SoundPlayer();
    virtual ~SoundPlayer();

    bool initialise();
    void stop();
    void closedown();

    static void playSound(QString fname, int volume);

    QStringList outputDevices;

    QString defaultOutput;

    unsigned int setRate(unsigned int rate);

    bool startDMA( );
    void stopDMA();

    static SoundPlayer *createSoundPlayer();

    void startOutput();
    void stopOutput();

    void setVolumeMults(qreal replay);

    void setData(int16_t *data, unsigned int len);

    spFile *spf = nullptr;
    int16_t *dataptr = nullptr;
    int numChannels = 0;
    uint32_t samples = 0;   /* fsample for current file  */

    int audioCallback( void *outputBuffer, void *inputBuffer,
                                    unsigned int nFrames,
                                    double streamTime,
                                    unsigned int status );
private:

    RtAudio *audio = nullptr;
    static SoundPlayer *soundPlayer;

    QMap<QString, int> deviceIds;
    QMap<QString, unsigned int> outChannels;

    QString curOutDev;

    // internal values
    unsigned int sampleRate = 0;

    bool playingFile = false;

    bool outputEnabled = false;

    qreal replayMult = 0.0;

    qint64 m_pos = 0;
    qint64 p_pos = 0;
    QByteArray m_buffer;
    void doPlaySound(QString fname, int volume);
signals:
    void draining();
private slots:
    void onDraining();
};

#endif
