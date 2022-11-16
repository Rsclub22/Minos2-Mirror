/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#include  <QtGlobal>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif
#include <QtEndian>
#include <QtMath>
#include <numeric>
#include <QtCore>

#include "fileutils.h"
#include "MTrace.h"
#include "rrsoundsys.h"
#include "riff.h"
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
#define BUFFSIZE RINGBUFFERSIZE * FRAMESAMPLES * 2

static QWaitCondition bufferNotEmpty;
static QWaitCondition bufferNotFull;
static QMutex mutex;

static int16_t inBuff[BUFFSIZE];
static int recIndex = 0;
static int inFrame = 0;
static int recIndex2 = 0;
static int inFrame2 = 0;
static int writeIndex = 0;

bool closeFlag = false;

RiffWriter::RiffWriter(RtAudioSoundSystem *parent) : QThread(parent), ss(parent), terminated(false)
{
}
RiffWriter::~RiffWriter(){}

void RiffWriter::run()
{
    for (;;)
    {
        mutex.lock();
        if (writeIndex == recIndex || writeIndex == recIndex2)
            bufferNotEmpty.wait(&mutex);
        mutex.unlock();

        if (terminated)
            break;

        int f = std::min(recIndex, recIndex2);
        while (f > writeIndex)
        {
            ss->writeDataToFile(&inBuff[writeIndex%RINGBUFFERSIZE * FRAMESAMPLES * 2], FRAMESAMPLES);
            mutex.lock();
            writeIndex += 1;

            bufferNotFull.wakeAll();
            mutex.unlock();
#ifdef Q_OS_UNIX
            sync();     // make sure it goes to disk
#endif
        }

        if (closeFlag)
        {
            ss->outWave->Close();
#ifdef Q_OS_UNIX
            sync();     // make sure it goes to disk
#endif
        }

    }
}

//==============================================================================
int audioCallback( void */*outputBuffer*/, void *inputBuffer,
                                unsigned int nFrames,
                                double streamTime,
                                RtAudioStreamStatus status,
                                void *userData )
{
    RtAudioSoundSystem *qss = static_cast<RtAudioSoundSystem *>(userData);
    return qss->audioCallback(inputBuffer, nFrames, streamTime, status, 1);
}
int audioCallback2( void */*outputBuffer*/, void *inputBuffer,
                                unsigned int nFrames,
                                double streamTime,
                                RtAudioStreamStatus status,
                                void *userData )
{
    RtAudioSoundSystem *qss = static_cast<RtAudioSoundSystem *>(userData);
    return qss->audioCallback(inputBuffer, nFrames, streamTime, status, 2);
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
       unsigned int devices = audio->getDeviceCount();
       RtAudio::DeviceInfo info;
       for ( unsigned int i=0; i<devices; i++ )
       {
         info = audio->getDeviceInfo( i );
         if ( info.probed == true )
         {
           trace( "device = "  + QString::number(i) +  " " + info.name.c_str());
           trace( "Maximum output channels = " + QString::number(info.outputChannels) + " Maximum input channels = " + QString::number(info.inputChannels));

           QString buff("Sample rates: ");
           for (auto r:info.sampleRates)
           {
               QString pref;
               if (r == info.preferredSampleRate)
               {
                   pref = "**";
               }
               buff += pref + QString::number(r) + pref + " ";
           }
           trace(buff);
         }
         if (i == defInput)
         {
             inChannels = info.inputChannels;
         }
         if (info.inputChannels)
         {
             inputDevices.append(info.name.c_str());
         }
         deviceIds[QString(info.name.c_str())] = i;
       }
       trace( "Default input channels = " + QString::number(inChannels));
    }
    catch (RtAudioError &error)
    {
       // Handle the exception here
       trace(error.getMessage().c_str());
       audio = nullptr;
    }
}
void RtAudioSoundSystem::stop()
{
    stopDMA();

    wThread->terminated = true;
    bufferNotEmpty.wakeAll();
    wThread->wait();
    try
    {
        if (audio->isStreamRunning())
        {
           // Stop the stream.
           audio->stopStream();
        }
    }
    catch ( RtAudioError& error )
    {
        trace(error.getMessage().c_str());
    }
}
void RtAudioSoundSystem::closedown()
{
    if (audio2)
    {
        stop();

        delete audio2;
        audio2 = nullptr;

        delete outWave;
        outWave = nullptr;
    }
    if (audio)
    {
        stop();

        delete audio;
        audio = nullptr;

        delete outWave;
        outWave = nullptr;
    }
}

RtAudioSoundSystem::~RtAudioSoundSystem()
{
   closedown();
}
void RtAudioSoundSystem::setVUCallBack( VUCallBack cb )
{
   WinVUCallback = cb;
}
void RtAudioSoundSystem::setVUCallBack2( VUCallBack cb )
{
   WinVUCallback2 = cb;
}

bool RtAudioSoundSystem::initialise( QString ind, QString ind2)
{
    try
    {
        if (!audio)
        {
            audio = new RtAudio();
        }
        RtAudio::StreamParameters inParams;
        RtAudio::StreamOptions soptions;

        unsigned int bufferFrames = FRAMESAMPLES;

        inParams.deviceId = deviceIds[ind];
        inParams.firstChannel = 0;
        inParams.nChannels = inChannels;

        soptions.flags = 0;
        soptions.numberOfBuffers = FRAMES;
        soptions.priority = 0;
        soptions.streamName = "";

        audio->openStream(nullptr,
                          &inParams,
                          RTAUDIO_SINT16, sampleRate,
                          &bufferFrames, ::audioCallback,
                          static_cast<void *>(this),
                          &soptions
                          );
        trace("Audio stream opened OK");

        audio->startStream();

        if (!ind2.isEmpty())
        {
            if (!audio2)
            {
                audio2 = new RtAudio();
            }
            RtAudio::StreamParameters inParams;
            RtAudio::StreamOptions soptions;

            unsigned int bufferFrames = FRAMESAMPLES;

            inParams.deviceId = deviceIds[ind2];
            inParams.firstChannel = 0;
            inParams.nChannels = inChannels;

            soptions.flags = 0;
            soptions.numberOfBuffers = FRAMES;
            soptions.priority = 0;
            soptions.streamName = "";

            audio2->openStream(nullptr,
                              &inParams,
                              RTAUDIO_SINT16, sampleRate,
                              &bufferFrames, ::audioCallback2,
                              static_cast<void *>(this),
                              &soptions
                              );
            trace("Audio 2 stream opened OK");

            audio2->startStream();
        }
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

void RtAudioSoundSystem::setRecordLevel(int l)
{
    // input levels are dB, so the actual multiplier is 10**(level/10)
    // BUT level is already * 10, so we need /100
    recordMult = qPow(10, l/100.0);
}
void RtAudioSoundSystem::setRecordLevel2(int l)
{
    // input levels are dB, so the actual multiplier is 10**(level/10)
    // BUT level is already * 10, so we need /100
    recordMult2 = qPow(10, l/100.0);
}

void RtAudioSoundSystem::setMono(bool s)
{
    mono = s;
}
void RtAudioSoundSystem::setMono2(bool s)
{
    mono2 = s;
}
int RtAudioSoundSystem::audioCallback( void *inputBuffer,
                                unsigned int nFrames,
                                double /*streamTime*/,
                                RtAudioStreamStatus status,
                                int instance
                                       )
{


    if (inputBuffer == nullptr || nFrames == 0)
    {
        return 0;   // no data
    }

    int temp = 0;
    double recmult = recordMult;
    if (instance == 2)
    {
        recmult = recordMult2;
    }

    if ( status == RTAUDIO_INPUT_OVERFLOW)
    {
        trace("Stream input underflow detected.");
    }
    if (status == RTAUDIO_OUTPUT_UNDERFLOW)
    {
        // shouldn't happen as we don't have an output device
        trace("Stream output overflow detected.");
    }

#if defined (_MSC_VER)
    int16_t *inStageBuffer = new int16_t[nFrames * 2];
#else
    int16_t inStageBuffer[nFrames * 2];
#endif

    int16_t * q = reinterpret_cast<  int16_t * > ( inputBuffer );
    int16_t * p = &inStageBuffer[0];
    qreal sqaccum = 0.0;
    int16_t maxvol = 0;

    for (unsigned int i = 0; i < nFrames ; i++)
    {
        // copy to staging buffer
        int16_t s1 = q[i * inChannels];
        int16_t s2 = (inChannels > 1)?q[i * inChannels + 1]:q[i * inChannels];

        qreal val1 = s1 * recmult;
        qreal val2 = s2 * recmult;

        if (mono)
        {
            val1 = (val1 + val2)/2;
            val2 = val1;
        }

        if (val1 > 32767.0)
            val1 = 32767.0;
        if (val1 < -32767.0)
            val1 = -32767.0;

        if (val2 > 32767.0)
            val2 = 32767.0;
        if (val2 < -32767.0)
            val2 = -32767.0;

        p[i * inChannels] = static_cast<qint16>(val1);
        p[i * inChannels + 1] = static_cast<qint16>(val2);
        int16_t sample = static_cast<int16_t>(std::max( std::abs(val1), std::abs(val2) ));

        if ( sample > maxvol )
           maxvol = sample;
        sqaccum += sample * sample;
    }

    qreal rmsval = sqrt(sqaccum/nFrames);
    trace(QString("VU %1 %2 %3").arg(maxvol).arg(rmsval).arg(nFrames));
    WinVUCallback( instance, static_cast<unsigned int>(maxvol),
                  static_cast<unsigned int>(rmsval),
                  nFrames );

    if (inputEnabled )
    {
        mutex.lock();
        if (recIndex - writeIndex >= RINGBUFFERSIZE - 1)
            bufferNotFull.wait(&mutex);
        mutex.unlock();

        // this is where we have to do the clever channel merging
        // do we have to assume that nFrames is the same for both?
        // or do we have to have a "simple" much larger ring of frames?

        if (instance == 1)
        {
            for ( unsigned int i = 0; i < nFrames; i++)
            {
                int16_t mix = (inStageBuffer[i * 2] + inStageBuffer[i * 2 + 1])/2;
                int offset = (recIndex % RINGBUFFERSIZE) * FRAMESAMPLES * 2 + inFrame;
                inBuff[offset] = mix;
                inFrame += 2;
            }
        }
        if (audio2)
        {
            if (instance == 2)
            {
                for ( unsigned int i = 0; i < nFrames; i++)
                {
                    int16_t mix = (inStageBuffer[i * 2] + inStageBuffer[i * 2 + 1])/2;
                    int offset = (recIndex2 % RINGBUFFERSIZE) * FRAMESAMPLES * 2 + inFrame2 + 1;
                    inBuff[offset] = mix;
                    inFrame2 += 2;
                }
            }
        }
        else
        {
            inFrame2 = inFrame;
        }

        mutex.lock();
        if (instance == 1)
        {
            if (inFrame >= FRAMESAMPLES)
            {
                ++recIndex;
                inFrame = 0;
            }
        }

        if (recIndex2 >= 0)
        {
            if (inFrame2 >= FRAMESAMPLES)
            {
                ++recIndex2;
                inFrame2 = 0;
            }
        }
        bufferNotEmpty.wakeAll();
        mutex.unlock();
    }
#if defined (_MSC_VER)
    delete [] inStageBuffer;
#else
    // nothing needed
#endif

    /*
   To continue normal stream operation, the RtAudioCallback function
   should return a value of zero.  To stop the stream and drain the
   output buffer, the function should return a value of one.  To abort
   the stream immediately, the client should return a value of two.      */

    return 0;
}
void RtAudioSoundSystem::startInput()
{
    inputEnabled = true;
}

void RtAudioSoundSystem::stopInput()
{
    inputEnabled = false;
     mutex.lock();
     if (recIndex - writeIndex >= RINGBUFFERSIZE - 1)     // not correct... we want "caught up"
         bufferNotFull.wait(&mutex);
     mutex.unlock();

     closeFlag = true;;  // mark to close

     mutex.lock();
     ++recIndex;

     bufferNotEmpty.wakeAll();
     mutex.unlock();
}
bool RtAudioSoundSystem::startInput( QString fname , int ct, bool continuation)
{
    // open fname, assign a text(?)
    // startInput() will also be called later

    // Should we do this in the writer thread?
    closeFlag = false;
    if (!continuation)
    {
        recIndex = 0;
        recIndex2 = 0;
        writeIndex = 0;
        if (!outWave)
        {
            outWave = new WaveFile;
        }
    }


    QDateTime tnow = QDateTime::currentDateTime();
    swapTime = tnow.addSecs(ct);

    if (fname.isEmpty())
    {
        return false;
    }
    QString baseName = ExtractFileName(fname);
    QString path = ExtractFileDir(fname);
    QString suffix = ExtractFileExt(fname);
    if (suffix.isEmpty() || suffix == ".")
    {
        suffix = ".wav";
    }
    else
    {
        int suff = baseName.indexOf(suffix);
        if (suff > 0)
        {
            baseName = baseName.left(suff);
        }
    }

    int dash = baseName.indexOf("_");
    if (dash >= 0)
    {
        baseName = baseName.left(dash);
    }

    QString now = tnow.toString("_yyyy-MM-dd hh-mm-ss");
    baseName = baseName + now + suffix;

    QString basePath = path + "/" + tnow.toString("yyyy-MM-dd");
    CreateDir(basePath);

    fname= basePath + "/" + baseName;

    DDCRET ret = outWave->OpenForWrite( fname.toLatin1(), sampleRate, 16, 2 );
    if ( ret == DDC_SUCCESS )
       return true;

    return false;
}

void RtAudioSoundSystem::writeDataToFile(void *inp, unsigned int nFrames)
{
    // data arrives here; we need to write it to the (already open) file,

    if (outWave && inp && nFrames)
    {
        const int16_t *q = reinterpret_cast< const int16_t * > ( inp );
        trace(QString("writeDataToFile %1 from %2 limit %3").arg(nFrames * 2).arg(q - inBuff).arg(RINGBUFFERSIZE * FRAMESAMPLES * 2));

        DDCRET ret = outWave->WriteData ( q, nFrames * 2 );   // size is numdata
        if ( ret != DDC_SUCCESS )
        {
            return;
        }
        QDateTime tnow = QDateTime::currentDateTime();
        QString secsPart = tnow.toString("ss");
//        QString minsPart = tnow.toString("mm");

        if (secsPart == "00" && /*minsPart[1] == "0" &&*/ tnow.msecsTo(swapTime) <= 0)
        {
            trace("Cycling wav file");
            // time to swap...
            outWave->Close();
            startInput(baseName, cycleTime, true);  // start continuation file
        }
    }
}

//==============================================================================
bool RtAudioSoundSystem::startDMA( const QString &fname, int ct )
{
    cycleTime = ct;
    baseName = fname;

    trace( "(StartDMA) Starting input" );

    if ( !startInput( fname, cycleTime, false ) )
        return false;

    startInput();
    return true;
}
void RtAudioSoundSystem::stopDMA()
{
    if (inputEnabled)
    {
        trace( "stopDMA" );

        stopInput();
    }
}
