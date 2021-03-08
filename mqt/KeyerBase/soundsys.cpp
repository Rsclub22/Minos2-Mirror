/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "mqtUtils_pch.h"

#include <QtEndian>
#include <QtMath>
#include <numeric>
#include <QtCore>

#include "soundsys.h"
#include "keyerlog.h"
#include "riff.h"

#define FRAMES 16
#define FRAMESAMPLES 256
#define RINGBUFFERSIZE 1024

static QWaitCondition bufferNotEmpty;
static QWaitCondition bufferNotFull;
static QMutex mutex;

class InBuff
{
public:
    unsigned int frameCount;
    int16_t buff[FRAMESAMPLES * 2];
};

static InBuff inBuffs[RINGBUFFERSIZE];
static int recIndex = 0;
static int writeIndex = 0;

RiffWriter::RiffWriter(RtAudioSoundSystem *parent) : QThread(parent), ss(parent), terminated(false)
{
}
RiffWriter::~RiffWriter(){}

void RiffWriter::run()
{
    for (;;)
    {
        mutex.lock();
        if (writeIndex == recIndex)
            bufferNotEmpty.wait(&mutex);
        mutex.unlock();

        if (terminated)
            break;

        if (inBuffs[writeIndex%RINGBUFFERSIZE].frameCount > 0)
            ss->writeDataToFile(inBuffs[writeIndex%RINGBUFFERSIZE].buff, inBuffs[writeIndex%RINGBUFFERSIZE].frameCount);
        else
        {
            ss->outWave.Close();
#ifdef Q_OS_UNIX
            sync();     // make sure it goes to disk
#endif
        }

        mutex.lock();
        ++writeIndex;

        bufferNotFull.wakeAll();
        mutex.unlock();
    }
}

/*static*/
//==============================================================================
RtAudioSoundSystem *RtAudioSoundSystem::createSoundSystem()
{
   return new RtAudioSoundSystem();
}
void LPFilter::initialise (int channels, double corner, double sampleRate)
{
    mNumChannels = channels;
    mZx[0] = mZx[1] = mZx[2] = mZx[3] = 0.0;
    mZy[0] = mZy[1] = mZy[2] = mZy[3] = 0.0;

    double mQuality = 0.707;

    double theta = 2. * M_PI * (corner/sampleRate);
    double d = 0.5 * (1. / mQuality) * sin(theta);
    double beta = 0.5 * ( (1. - d) / (1. + d) );
    double gamma = (0.5 + beta) * cos(theta);

    a0 = 0.5 * (0.5 + beta - gamma);
    a1 = 0.5 + beta - gamma;

    a2 = a0;
    b1 = -2. * gamma;
    b2 = 2. * beta;
}

double LPFilter::filterSample (const double inSample, const int channel)
{
    // Derived from
    // http://creatingsound.com/2014/02/dsp-audio-programming-series-part-2/

    double outSample;
    int idx0 = mNumChannels * channel;
    int idx1 = idx0 + 1;

    outSample = (a0 * inSample)
                + (a1 * mZx[idx0])
                + (a2 * mZx[idx1])
                - (b1 * mZy[idx0])
                - (b2 * mZy[idx1]);

    mZx[idx1] = mZx[idx0];
    mZx[idx0] = inSample;
    mZy[idx1] = mZy[idx0];
    mZy[idx0] = outSample;

    return outSample;
}
//==============================================================================
int audioCallback( void *outputBuffer, void *inputBuffer,
                                unsigned int nFrames,
                                double streamTime,
                                RtAudioStreamStatus status,
                                void *userData )
{
    RtAudioSoundSystem *qss = static_cast<RtAudioSoundSystem *>(userData);
    return qss->audioCallback(outputBuffer, inputBuffer, nFrames, streamTime, status);
}
//==============================================================================
RtAudioSoundSystem::RtAudioSoundSystem()
{
    try
    {
       audio = new RtAudio();

       wThread = new RiffWriter(this);
       wThread->start();

       unsigned int defInput = audio->getDefaultInputDevice();
       unsigned int defOutput = audio->getDefaultOutputDevice();
       unsigned int devices = audio->getDeviceCount();
       RtAudio::DeviceInfo info;
       for ( unsigned int i=0; i<devices; i++ ) {
         info = audio->getDeviceInfo( i );
         if ( info.probed == true ) {
           trace( "device = "  + QString::number(i) +  " " + info.name.c_str());
           trace( "Maximum output channels = " + QString::number(info.outputChannels) + " Maximum input channels = " + QString::number(info.inputChannels));
         }
         if (i == defInput)
         {
             inChannels = info.inputChannels;
         }
         if (i == defOutput)
         {
             outChannels = info.outputChannels;
         }
       }
       trace( "Default output channels = " + QString::number(outChannels) + " Default input channels = " + QString::number(inChannels));
    }
    catch (RtAudioError &error)
    {
       // Handle the exception here
       trace(error.getMessage().c_str());
       audio = nullptr;
    }
}
RtAudioSoundSystem::~RtAudioSoundSystem()
{
   passThroughEnabled = false;
   stopDMA();

   wThread->terminated = true;
   bufferNotEmpty.wakeAll();
   wThread->wait();
   try
   {
      // Stop the stream.
      audio->stopStream();
   }
   catch ( RtAudioError& error )
   {
       trace(error.getMessage().c_str());
   }
   delete audio;
}
bool RtAudioSoundSystem::initialise( QString &/*errmess*/ )
{

    compressor.setSampleRate(sampleRate);
    compressor.setWindow(10);       // milliseconds
    compressor.setThresh( -10 );
    compressor.setRatio( 0.1 );
    compressor.setAttack( 1.0 );     // 1ms seems like a good look-ahead to me
    compressor.setRelease( 10.0 ); // 10ms release is good
    compressor.initRuntime();

    lpFilter.initialise(2, filterCorner, sampleRate);

    try
    {
        RtAudio::StreamParameters outParams;
        RtAudio::StreamParameters inParams;
        RtAudio::StreamOptions soptions;

        unsigned int bufferFrames = FRAMESAMPLES;

        outParams.deviceId = audio->getDefaultOutputDevice();
        outParams.firstChannel = 0;
        outParams.nChannels = outChannels;

        inParams.deviceId = audio->getDefaultInputDevice();
        inParams.firstChannel = 0;
        inParams.nChannels = inChannels;

        soptions.flags = 0;
        soptions.numberOfBuffers = FRAMES;
        soptions.priority = 0;
        soptions.streamName = "";

        audio->openStream(&outParams,
                          &inParams,
                          RTAUDIO_SINT16, sampleRate,
                          &bufferFrames, ::audioCallback,
                          static_cast<void *>(this),
                          &soptions
                          );
        trace("Audio stream opened OK");

        audio->startStream();
    }
    catch (RtAudioError &error)
    {
        trace(error.getMessage().c_str());
    }

    return true;
}
unsigned int RtAudioSoundSystem::setRate(unsigned int rate)
{
   sampleRate = rate;
   return sampleRate;
}
void RtAudioSoundSystem::setFilter(int fc)
{
   filterCorner = fc;
}

void RtAudioSoundSystem::setVolumeMults(qreal record, qreal replay, qreal passThrough)
{
    // input levels are dB, so the actual multiplier is 10**(level/10)
    // BUT level is already * 10, so we need /100
    recordMult = qPow(10, record/100);
    replayMult = qPow(10, replay/100);
    passThroughMult = qPow(10, passThrough/100);
}


int RtAudioSoundSystem::audioCallback(void *outputBuffer, void *inputBuffer,
                                unsigned int nFrames,
                                double /*streamTime*/,
                                unsigned int status )
{
#if defined (_MSC_VER)
    int16_t *inStageBuffer = new int16_t[nFrames * 2];
#else
    int16_t inStageBuffer[nFrames * 2];
#endif

    if (outputBuffer == nullptr && inputBuffer == nullptr)
    {
        return 0;   // no data
    }

    if ( status == RTAUDIO_INPUT_OVERFLOW)
    {
        trace("Stream input underflow detected.");
    }
    if (status == RTAUDIO_OUTPUT_UNDERFLOW)
    {
        trace("Stream output overflow detected.");
    }


    if (outputBuffer != nullptr && nFrames > 0)
    {
        memset(outputBuffer, 0, nFrames * 2 * outChannels);   // 2 bytes, 2 channels
    }

    if (inputBuffer && nFrames)
    {
        // ALWAYS apply compressor to input, so it continues to adapt
        int16_t * q = reinterpret_cast<  int16_t * > ( inputBuffer );
        int16_t * m = reinterpret_cast< int16_t * > ( outputBuffer );
        int16_t * p = &inStageBuffer[0];
        int16_t maxvol = 0;
        qreal sqaccum = 0.0;
        qreal volmult = (inputEnabled?recordMult:passThroughMult);

        for (unsigned int i = 0; i < nFrames ; i++)
        {
            double initi1 = q[i * inChannels];
            double initi2 = (inChannels > 1)?q[i * inChannels + 1]:q[i * inChannels];

            double s1 = initi1;
            double s2 = initi2;

            s1 /= 32768.0;
            s2 /= 32768.0;

            compressor.process(s1, s2);

            s1 *= 32768.0;
            s2 *= 32768.0;

            qreal val1 = s1 * volmult;
            qreal val2 = s2 * volmult;

            if (val1 > 32767.0)
                val1 = 32767.0;
            if (val1 < -32767.0)
                val1 = -32767.0;

            if (val2 > 32767.0)
                val2 = 32767.0;
            if (val2 < -32767.0)
                val2 = -32767.0;

            if (filterCorner > 0)
            {
                val1 = lpFilter.filterSample(val1, 0);
                val2 = lpFilter.filterSample(val2, 1);
            }

            if (passThroughEnabled)
            {
                m[i * outChannels] = static_cast<qint16>(val1);
                m[i * outChannels + 1] = static_cast<qint16>(val2);

            }
            if (inputEnabled)
            {
                p[i * outChannels] = static_cast<qint16>(val1);
                if (inChannels > 1)
                    p[i * outChannels + 1] = static_cast<qint16>(val2);
                else
                    p[i * outChannels + 1] = static_cast<qint16>(val1);
            }

            int16_t sample = static_cast<int16_t>(std::max( val1, val2 ));
            if ( sample > maxvol )
               maxvol = sample;

            sqaccum += sample * sample;
        }
        if (inputEnabled || passThroughEnabled)
        {
            qreal rmsval = sqrt(sqaccum/nFrames);
            emit setVU( static_cast<unsigned int>(maxvol),
                          static_cast<unsigned int>(rmsval),
                          nFrames );
        }

        if (inputEnabled)
        {
            mutex.lock();
            if (recIndex - writeIndex >= RINGBUFFERSIZE - 1)
                bufferNotFull.wait(&mutex);
            mutex.unlock();

            inBuffs[recIndex % RINGBUFFERSIZE].frameCount = nFrames;
            memcpy(inBuffs[recIndex % RINGBUFFERSIZE].buff, inStageBuffer, nFrames * 4);

            mutex.lock();
            ++recIndex;
            bufferNotEmpty.wakeAll();
            mutex.unlock();
        }
    }
    if (outputBuffer != nullptr && nFrames != 0 && outputEnabled )
    {
        int16_t maxvol = 0;
        qreal rmsval = 0.0;
        readFromFile(outputBuffer, nFrames, maxvol, rmsval);

        emit setVU( static_cast<unsigned int>(maxvol),
                      static_cast<unsigned int>(rmsval),
                      nFrames );
    }

    /*
   To continue normal stream operation, the RtAudioCallback function
   should return a value of zero.  To stop the stream and drain the
   output buffer, the function should return a value of one.  To abort
   the stream immediately, the client should return a value of two.      */
    return 0;
}


void RtAudioSoundSystem::startOutput()
{
    trace("startOutput");
    outputEnabled = true;
}

void RtAudioSoundSystem::stopOutput()
{
    outputEnabled = false;
    emit setActionTime1();
    emit setVU(0, 0, 0);
}
void RtAudioSoundSystem::startInput()
{
    inputEnabled = true;
}

void RtAudioSoundSystem::stopInput()
{
    inputEnabled = false;
    emit actionQueueFinished();
     mutex.lock();
     if (recIndex - writeIndex >= RINGBUFFERSIZE - 1)     // not correct... we want "caught up"
         bufferNotFull.wait(&mutex);
     mutex.unlock();

     inBuffs[recIndex%RINGBUFFERSIZE].frameCount = 0;  // mark to close

     mutex.lock();
     ++recIndex;

     bufferNotEmpty.wakeAll();
     mutex.unlock();
}
bool RtAudioSoundSystem::startInput( QString fn )
{
    // open fname, assign a text(?)
    // startInput() will also be called later

    // Should we do this in the writer thread?
    recIndex = 0;
    writeIndex = 0;

    if ( outWave.OpenForWrite( fn.toLatin1(), sampleRate, 16, 2 ) == DDC_SUCCESS )
       return true;

    return false;
}

void RtAudioSoundSystem::setData(int16_t *data, unsigned int len)
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
void RtAudioSoundSystem::setPipData(int16_t *data, unsigned int len, unsigned int delayLen)
{
    pipDelayBytes = delayLen * sizeof(uint16_t);
    p_buffer.resize(static_cast<int>(len) * static_cast<int>(sizeof(uint16_t)));
    unsigned char *ptr = reinterpret_cast<unsigned char *>(p_buffer.data());

    for (unsigned int i = 0; i < len; i++)
    {
        uint16_t value = static_cast<uint16_t>(data[i]);
        // This may or may not be neccesary... on a big endian system
        qToLittleEndian<uint16_t>(value, ptr);
        ptr += 2;
    }
    p_pos = 0;
}

void RtAudioSoundSystem::writeDataToFile(void *inp, unsigned int nFrames)
{
    // data arrives here; we need to write it to the (already open) file,

    if (inp && nFrames)
    {
        const int16_t *q = reinterpret_cast< const int16_t * > ( inp );
        DDCRET ret = outWave.WriteData ( q, nFrames * 2 );   // size is numdata
        if ( ret != DDC_SUCCESS )
        {
            return;
        }
        emit interruptOK();
    }
}
void RtAudioSoundSystem::readFromFile(void *outputBuffer, unsigned int nFrames, int16_t &maxvol, qreal &rmsval)
{
    if (outputBuffer && nFrames)
    {
        int16_t *q = reinterpret_cast< int16_t * > ( outputBuffer );

        qint64 len = nFrames * 2 * 2;

        // we have to add in the pip here as well...
        qint64 total = 0;
        if (m_pos >= m_buffer.size())
        {
            // add in pip delay and pip
            if (p_pos >= p_buffer.size())
            {
                stopOutput();
                return;
            }
            if (pipDelayBytes > 0)
            {
                qint64 ps = pipDelayBytes;
                total = qMin(ps, len);
//                q += total/2;
                pipDelayBytes -= total;
            }
            else
            {
                if (!p_buffer.isEmpty())
                {
                    total = qMin((p_buffer.size() - p_pos), len);
                    memcpy(q, p_buffer.constData() + p_pos, static_cast<size_t>(total));
//                    q += total/2;
                    p_pos += total;
                }
                else
                {
                    p_buffer.clear();
                    stopOutput();
                    trace("p_buffer empty");
                }
            }
        }
        else
        {
            if (!m_buffer.isEmpty())
            {
                total = qMin((m_buffer.size() - m_pos), len);

                const int16_t *m = reinterpret_cast< const int16_t * > ( &m_buffer.constData()[m_pos] );

                qreal mult = replayMult;
                if (tone)
                    mult = 1.0;

                for (int i = 0; i < total/2; i++)
                {
                    qreal val =*m++ * mult;
                    if (val > 32767.0)
                        val = 32767.0;
                    if (val < -32767.0)
                        val = -32767.0;
                    *q++ = static_cast<qint16>(val);
                }
                m_pos += total;
            }
            else
            {
                stopOutput();
                trace("m_buffer empty");
            }
        }
        emit interruptOK();

        // should already have any gain multiplication done
         q = reinterpret_cast< int16_t * > ( outputBuffer );

         // determine max for VU meter
        qreal sqaccum = 0.0;
        for ( unsigned int i = 0; i < nFrames; i++ )
        {
           int16_t sample = static_cast<int16_t>(abs( *q++ ));
           if ( sample > maxvol )
              maxvol = sample;

           sqaccum += sample * sample;
        }

        rmsval = sqrt(sqaccum/nFrames);
    }
}

//==============================================================================
bool RtAudioSoundSystem::startDMA(bool play, const QString &fname, int pipSamples, int16_t *pipptr , int pipStartDelaySamples)
{
   // start input / output

   if ( play )
   {
        if ( sblog )
        {
            trace( "(StartDMA) Starting output" );
        }

        playingFile = true;
        recordingFile = false;
        passThrough = false;

        tone = fname.isEmpty();

        setData(dataptr, samples);

        if (pipSamples > 0)
        {
            unsigned long psamples = pipSamples;
            setPipData(pipptr, psamples, pipStartDelaySamples);
        }
        startOutput();
   }
   else
   {
       if ( sblog )
       {
          trace( "(StartDMA) Starting input" );
       }

         if ( !startInput( fname ) )
            return false;

         playingFile = false;
         recordingFile = true;
         passThrough = false;

         startInput();
   }
   return true;
}
void RtAudioSoundSystem::stopDMA()
{
//    Here we need to stop input/output
    trace( "stopDMA" );

    if (playingFile)
        stopOutput();

    if (recordingFile)
        stopInput();

    m_buffer.clear();
    p_buffer.clear();
    m_pos = 0;
    p_pos = 0;

    playingFile = false;
    recordingFile = false;
    passThrough = true;
    emit setVU( 0, 0, 0 );
}
bool RtAudioSoundSystem::startMicPassThrough()
{
    trace("startMicPassThrough");

    passThroughEnabled = true;
   return true;
}
bool RtAudioSoundSystem::stopMicPassThrough()
{
    trace("stopMicPassThrough");

    passThroughEnabled = false;
    emit setVU( 0, 0, 0 );
    return true;
}
