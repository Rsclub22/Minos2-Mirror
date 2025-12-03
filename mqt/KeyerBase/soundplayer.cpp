/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "MTrace.h"
#include <QDateTime>
#include <QtEndian>
#include <QtMath>

#include "riff.h"

#include "delayedaction.h"

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

#include "soundplayer.h"

#define FRAMES 16
#define FRAMESAMPLES 256

class spFile
{
public:
    QString fileName;
    bool loaded = false;
    unsigned int sampleRate = 0;       // system required sample rate
    unsigned long fsample = 0;        // number of bytes for each sound files
    int16_t *fptr = nullptr;          // data area for each sound file
    unsigned int rate = 0;
    int BitsPerSample = 0;
    int NumChannels = 0;

    bool LoadFile( QString &errmess )
    {
        trace( "Trying to open file " + fileName );

        loaded = false;
        delete[] fptr;
        fptr = nullptr;
        WaveFile inWave;
        int ret = inWave.OpenForRead( fileName );
        if ( ret != DDC_SUCCESS )
        {
            errmess = "Invalid WAV file " + fileName + "\n";
            trace( errmess );
            return false;
        }
        else
        {
            rate = inWave.SamplingRate();
            BitsPerSample = inWave.BitsPerSample();
            NumChannels = inWave.NumChannels();
            fsample = inWave.NumSamples();

            if ( /*rate == static_cast<unsigned int>(sampleRate) &&*/ BitsPerSample == 16  )
            {
                if (NumChannels == 1)
                {
                    fptr = new int16_t[ fsample * 1 ];
                    if ( inWave.ReadData( fptr, fsample * 1 ) == DDC_SUCCESS )
                    {
                        trace( "File " + fileName + " opened samples = " + QString::number( fsample ) );
                        loaded = true;
                    }
                    else
                    {
                        loaded = false;
                    }
                }
                else
                {
                    fptr = new int16_t[ fsample * 2 ];
                    if ( inWave.ReadData( fptr, fsample * 2 ) == DDC_SUCCESS )
                    {
                        trace( "File " + fileName + " opened samples = " + QString::number( fsample ) );
                        loaded = true;
                    }
                    else
                    {
                        loaded = false;
                    }
                }
                if (loaded)
                {
                    trace(QString("%1 opened rate %2 channels %3 bitspersample %4 numsamples %5")
                              .arg(fileName).arg(sampleRate).arg(NumChannels).arg(BitsPerSample).arg(fsample));
                }
            }
            else
            {
                trace( "File " + fileName + " wrong data format" );
                loaded = false;
            }

        }
        return loaded;
    }
    spFile()
    {}
    ~spFile()
    {
        delete[] fptr;
        fptr = nullptr;
    }
};


SoundPlayer *SoundPlayer::soundPlayer = nullptr;
/*static*/
//==============================================================================
SoundPlayer *SoundPlayer::createSoundPlayer()
{
   return new SoundPlayer();
}
//==============================================================================
int audioCallback( void *outputBuffer, void *inputBuffer,
                                unsigned int nFrames,
                                double streamTime,
                                RtAudioStreamStatus status,
                                void *userData )
{
    SoundPlayer *qss = static_cast<SoundPlayer *>(userData);
    return qss->audioCallback(outputBuffer, inputBuffer, nFrames, streamTime, status);
}
void errorCallback( RtAudioErrorType /*type*/, const std::string &errorText)
{
    trace(QString("RTAudio error callback: ") + errorText.c_str());
}
//==============================================================================
void SoundPlayer::playSound(QString fname, int volume)
{
    if (soundPlayer)
    {
        trace("playSound already running");
    }
    else
    {
        soundPlayer = createSoundPlayer();
        soundPlayer->doPlaySound(fname, volume);
    }
}
void SoundPlayer::doPlaySound(QString fname, int volume)
{
    delete spf;
    spf = new spFile();
    spf->fileName = fname;
    QString err;
    if (!spf->LoadFile(err))
    {
        trace(err);
        delete spf;
        spf = nullptr;
        return;
    }
    samples = static_cast<uint32_t>(spf ->fsample * spf->NumChannels);
    dataptr = spf ->fptr;
    numChannels = spf->NumChannels;
    setRate(spf->rate);
    setVolumeMults(volume/100.0);

    initialise();
    startDMA();
}
void SoundPlayer::onDraining()
{
    delayedAction(this, [=]()
    {
        soundPlayer->deleteLater();
    }, 1000 );
}

SoundPlayer::SoundPlayer()
{
    connect(this, &SoundPlayer::draining, this, &SoundPlayer::onDraining, Qt::QueuedConnection);
    audio = new RtAudio(RtAudio::UNSPECIFIED, &errorCallback);


    std::vector<unsigned int> devices = audio->getDeviceIds();
    //trace(QString("Found %1 device(s)").arg(devices.size()));

    unsigned int defOutput = audio->getDefaultOutputDevice();

    for (unsigned int i=0; i<devices.size(); i++)
    {
        RtAudio::DeviceInfo info = audio->getDeviceInfo( devices[i] );
        // trace( "device = "  + QString::number(i) +  " " + info.name.c_str());
        // trace( "Maximum output channels = " + QString::number(info.outputChannels) + " Maximum input channels = " + QString::number(info.inputChannels));
        // QString buff("Sample rates: ");
        // for (auto r:info.sampleRates)
        // {
        //     QString pref;
        //     if (r == info.preferredSampleRate)
        //     {
        //         pref = "**";
        //     }
        //     buff += pref + QString::number(r) + pref + " ";
        // }
        // trace(buff);

        if (devices[i] == defOutput)
        {
//            trace("(Default output)");
            defaultOutput = info.name.c_str();
        }
        if (info.outputChannels)
        {
            outputDevices.append(info.name.c_str());
        }
        deviceIds[QString(info.name.c_str())] = devices[i];
        outChannels[info.name.c_str()] = info.outputChannels;
//        trace( QString(info.name.c_str()) + " output channels = "
//              + QString::number(outChannels[info.name.c_str()]));
    }
}
SoundPlayer::~SoundPlayer()
{
    closedown();
    soundPlayer = nullptr;
}

bool SoundPlayer::initialise( )
{
    if (!audio)
    {
        audio = new RtAudio(RtAudio::UNSPECIFIED, &errorCallback);
    }

    curOutDev = defaultOutput;

    trace(QString("SoundPlayer::initialise outd %1").arg(curOutDev));

    RtAudio::StreamParameters outParams;
    RtAudio::StreamOptions soptions;

    unsigned int bufferFrames = FRAMESAMPLES;

    if (deviceIds.contains(curOutDev))
    {
        outParams.deviceId = deviceIds[curOutDev];
    }

    outParams.firstChannel = 0;
    outParams.nChannels = outChannels[curOutDev];


    soptions.flags = 0;
    soptions.numberOfBuffers = RTAUDIO_MINIMIZE_LATENCY;
    soptions.priority = 0;
    soptions.streamName = "";

    RtAudioErrorType err = audio->openStream(&outParams,
                  nullptr,
                  RTAUDIO_SINT16, sampleRate,
                  &bufferFrames, ::audioCallback,
                  static_cast<void *>(this),
                  &soptions
                  );
    if(err != RTAUDIO_NO_ERROR)
    {
        trace(QString(audio->getErrorText().c_str()));
        return false;
    }

    trace(QString("Audio stream opened OK. Buffers = %1 bufferFrames %2").arg(soptions.numberOfBuffers).arg(bufferFrames));

    audio->startStream();
    return true;
}
void SoundPlayer::stop()
{
    stopDMA();

    if (audio->isStreamRunning())
    {
       // Stop the stream.
       audio->stopStream();
    }
}
void SoundPlayer::closedown()
{
    if (audio)
    {
        stop();

        delete audio;
        audio = nullptr;

        delete spf;
    }
}

unsigned int SoundPlayer::setRate(unsigned int rate)
{
   sampleRate = rate;
   return sampleRate;
}

void SoundPlayer::setVolumeMults( qreal replay)
{

    replayMult = replay;
}

int SoundPlayer::audioCallback(void *outputBuffer, void */*inputBuffer*/,
                                unsigned int nFrames,
                                double /*streamTime*/,
                                unsigned int status )
{
    unsigned int outCh = outChannels[curOutDev];

    if (outputBuffer == nullptr)
    {
        return 0;   // no data
    }

    if ( status == RTAUDIO_INPUT_OVERFLOW)
    {
        trace("Stream input overflow detected.");
    }
    if (status == RTAUDIO_OUTPUT_UNDERFLOW)
    {
        trace("Stream output underflow detected.");
    }

    if (outputBuffer != nullptr && nFrames > 0)
    {
        memset(outputBuffer, 0, nFrames * 2 * outCh);   // 2 bytes, 2 channels
    }
    if (outputBuffer != nullptr && nFrames != 0 && outputEnabled )
    {
        if (!readFromFile(outputBuffer, nFrames))
        {
            emit draining();
            return 1;   // stop the stream, drain buffer
        }

    }

    /*
   To continue normal stream operation, the RtAudioCallback function
   should return a value of zero.  To stop the stream and drain the
   output buffer, the function should return a value of one.  To abort
   the stream immediately, the client should return a value of two.
   */

    return 0;
}

void SoundPlayer::setData(int16_t *data, unsigned int len)
{
    m_buffer.resize(static_cast<int>(len) * static_cast<int>(sizeof(uint16_t)));
    unsigned char *ptr = reinterpret_cast<unsigned char *>(m_buffer.data());

    for (unsigned int i = 0; i < len; i++)
    {
        uint16_t value = static_cast<uint16_t>(data[i]);
        // This may or may not be neccesary... on a big endian system
        qToLittleEndian<uint16_t>(value, ptr);
        ptr += 2;
    }
    m_pos = 0;
}
bool SoundPlayer::readFromFile(void *outputBuffer, unsigned int nFrames)
{
    if (outputBuffer && nFrames)
    {
        int16_t *q = reinterpret_cast< int16_t * > ( outputBuffer );

        qint64 len;
        if (numChannels == 1)
        {
            len = nFrames * 2 * 1;
        }
        else
        {
            len = nFrames * 2 * 2;
        }

        qint64 total = 0;
        if (m_pos >= m_buffer.size())
        {
            stopOutput();
            return false;
        }
        else
        {
            if (!m_buffer.isEmpty())
            {
                total = qMin((m_buffer.size() - m_pos), len);

                const int16_t *m = reinterpret_cast< const int16_t * > ( &m_buffer.constData()[m_pos] );

                qreal mult = replayMult;

                for (int i = 0; i < total/(2 * numChannels); i += 1)
                {

                    qreal val = *m++;
                    qreal val2;
                    if (numChannels == 1)
                    {
                        val2 = val;
                    }
                    else
                    {
                        val2 = *m++;
                    }

                    val *= mult;
                    if (val > 32767.0)
                        val = 32767.0;
                    if (val < -32767.0)
                        val = -32767.0;

                    val2 *= mult;
                    if (val2 > 32767.0)
                        val2 = 32767.0;
                    if (val2 < -32767.0)
                        val2 = -32767.0;
                    *q++ = static_cast<qint16>(val);
                    *q++ = static_cast<qint16>(val2);
                }
                m_pos += total;
                return true;
            }
            else
            {
                stopOutput();
                trace("m_buffer empty");
            }
        }
    }
    return false;
}

//==============================================================================
bool SoundPlayer::startDMA()
{
   // start input / output

    trace( "(StartDMA) Starting output" );

    playingFile = true;

    setData(dataptr, samples);
    startOutput();

    return true;
}
void SoundPlayer::stopDMA()
{
//    Here we need to stop input/output
    trace( "stopDMA" );

    if (playingFile)
        stopOutput();

    m_buffer.clear();
    m_pos = 0;

    playingFile = false;
}
void SoundPlayer::startOutput()
{
    trace("startOutput");
    outputEnabled = true;
}

void SoundPlayer::stopOutput()
{
    trace("stopOutput");
    outputEnabled = false;
}
