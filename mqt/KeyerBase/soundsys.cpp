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
#include "keyerlog.h"
#include "inbuff.h"
#include "riffwriter.h"
#include "databuffer.h"
#include "ipsystem.h"

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

#include "soundsys.h"

#define FRAMES 16
#define FRAMESAMPLES 256

/*static*/
//==============================================================================
RtAudioSoundSystem *RtAudioSoundSystem::createSoundSystem()
{
   return new RtAudioSoundSystem();
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

         if (i == defInput)
         {
             trace("(Default input)");
             defaultInput = info.name.c_str();
         }
         if (i == defOutput)
         {
             trace("(Default output)");
             defaultOutput = info.name.c_str();
         }
         if (info.inputChannels)
         {
             inputDevices.append(info.name.c_str());
         }
         if (info.outputChannels)
         {
             outputDevices.append(info.name.c_str());
         }
         deviceIds[QString(info.name.c_str())] = i;
         inChannels[info.name.c_str()] = info.inputChannels;
         outChannels[info.name.c_str()] = info.outputChannels;
         trace( QString(info.name.c_str()) + " output channels = "
               + QString::number(outChannels[info.name.c_str()])
               + " input channels = " + QString::number(inChannels[info.name.c_str()]));
       }
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
   closedown();

   if (micfilter1)
   {
       free_bw_band_pass(micfilter1);
       micfilter1 = nullptr;
       free_bw_band_pass(micfilter2);
       micfilter2 = nullptr;
       free_bw_band_pass(replayfilter1);
       replayfilter1 = nullptr;
       free_bw_band_pass(replayfilter2);
       replayfilter2 = nullptr;
    }
}
void errorCallback( RtAudioError::Type /*type*/, const std::string &errorText)
{
    trace(QString("RTAudio error callback: ") + errorText.c_str());
}
bool RtAudioSoundSystem::initialise( QString ind, QString outd, QString host, QString port  )
{
    if (!audio)
    {
        audio = new RtAudio();
    }

    curInDev = ind;
    curOutDev = outd;

    bool oip = false;
    RtAudio::StreamParameters outParams;
    RtAudio::StreamParameters inParams;
    RtAudio::StreamOptions soptions;

    unsigned int bufferFrames = FRAMESAMPLES;

    if (deviceIds.contains(outd))
    {
        outParams.deviceId = deviceIds[outd];
    }
    else
    {
        // IP device
        oip = true;
    }

    outParams.firstChannel = 0;
    outParams.nChannels = outChannels[outd];

    bool iip = false;
    if (deviceIds.contains(ind))
    {
        inParams.deviceId = deviceIds[ind];
    }
    else
    {
        iip = true;
    }
    inParams.firstChannel = 0;
    inParams.nChannels = inChannels[ind];

    soptions.flags = 0;
    soptions.numberOfBuffers = RTAUDIO_MINIMIZE_LATENCY;
    soptions.priority = 0;
    soptions.streamName = "";

    try
    {
        audio->openStream(oip?nullptr:&outParams,
                      iip?nullptr:&inParams,
                      RTAUDIO_SINT16, sampleRate,
                      &bufferFrames, ::audioCallback,
                      static_cast<void *>(this),
                      &soptions,
                      &errorCallback
                      );
    } catch (RtAudioError &error)
    {
        trace(error.getMessage().c_str());
        return false;
    }

    trace(QString("Audio stream opened OK. Buffers = %1 bufferFrames %2").arg(soptions.numberOfBuffers).arg(bufferFrames));

    if (!iip)
    {
        micCompressor.setSampleRate(sampleRate);
        micCompressor.setWindow(10);       // milliseconds
        micCompressor.setThresh( -10 );
        micCompressor.setRatio( 0.1 );
        micCompressor.setAttack( 1.0 );     // 1ms seems like a good look-ahead to me
        micCompressor.setRelease( 10.0 ); // 10ms release is good
        micCompressor.initRuntime();

        replayCompressor.setSampleRate(sampleRate);
        replayCompressor.setWindow(10);       // milliseconds
        replayCompressor.setThresh( -10 );
        replayCompressor.setRatio( 0.1 );
        replayCompressor.setAttack( 1.0 );     // 1ms seems like a good look-ahead to me
        replayCompressor.setRelease( 10.0 ); // 10ms release is good
        replayCompressor.initRuntime();

        micfilter1 = create_bw_band_pass_filter(4, 48000, 100, 3000);   // order, sampling freq, lower half power, upper half power
        micfilter2 = create_bw_band_pass_filter(4, 48000, 100, 3000);   // order, sampling freq, lower half power, upper half power

        replayfilter1 = create_bw_band_pass_filter(4, 48000, 100, 3000);   // order, sampling freq, lower half power, upper half power
        replayfilter2 = create_bw_band_pass_filter(4, 48000, 100, 3000);   // order, sampling freq, lower half power, upper half power


    }
    if (!dataBuffer)
    {
        dataBuffer = new IPADataBuffer(this);
        dataBuffer->setBuffers(bufferFrames);
        dataBuffer->startInput();
    }

    if (!ipSystem)
    {
        ipSystem = IPSystem::createIPSystem();
        connect(ipSystem, &IPSystem::sequenceCount, this, &RtAudioSoundSystem::sequenceCount);
        connect(this, &RtAudioSoundSystem::soundAvailable, this, &RtAudioSoundSystem::onSoundAvailable
                , static_cast<Qt::ConnectionType>(Qt::UniqueConnection|Qt::QueuedConnection));

        // iip also means data receiver

        ipSystem->initialise(!iip, dataBuffer, QHostAddress(host), port.toInt());
        ipSystem->doStart();
    }
    if (!wThread)
    {
        wThread = new RiffWriter(this, bufferFrames);
        wThread->start();
    }

    audio->startStream();
    return true;
}
void RtAudioSoundSystem::onSoundAvailable()
{
    if (ipSystem)
    {
        while(ipSystem->tryOutput()){}
    }
}

void RtAudioSoundSystem::passPTT(bool b)
{
    pttState = b;
}
void RtAudioSoundSystem::stop()
{
    stopDMA();

    if (wThread)
    {
        wThread->terminated = true;
        wThread->wakeAll();
        wThread->wait();
    }
    if (audio->isStreamRunning())
    {
       // Stop the stream.
       audio->stopStream();
    }
}
void RtAudioSoundSystem::closedown()
{
    if (audio)
    {
        stop();

        delete audio;
        audio = nullptr;

        delete outWave;
        outWave = nullptr;

        delete wThread;
        wThread = nullptr;

        delete dataBuffer;
        dataBuffer = nullptr;

        delete ipSystem;
        ipSystem = nullptr;
    }
}

unsigned int RtAudioSoundSystem::setRate(unsigned int rate)
{
    dropped = 0;
    missed = 0;
    callbacks = 0;
    stime = 0;
    cbacks = 0.0;

   sampleRate = rate;
   return sampleRate;
}

void RtAudioSoundSystem::setVolumeMults(qreal record, qreal replay, qreal passThrough, const CompressorParams &comp, bool df, bool dc)
{
    // input levels are dB, so the actual multiplier is 10**(level/10)
    // BUT level is already * 10, so we need /100
    recordMult = qPow(10, record/100);
    replayMult = qPow(10, replay/100);
    passThroughMult = qPow(10, passThrough/100);

    compression = comp;

    micCompressor.setWindow(comp.window);
    micCompressor.setAttack(comp.attack);
    micCompressor.setRelease(comp.release);
    micCompressor.setThresh(comp.threshold);
    micCompressor.setRatio(comp.ratio);

    //micCompressor.initRuntime();

    replayCompressor.setWindow(comp.window);
    replayCompressor.setAttack(comp.attack);
    replayCompressor.setRelease(comp.release);
    replayCompressor.setThresh(comp.threshold);
    replayCompressor.setRatio(comp.ratio);

    //replayCompressor.initRuntime();

    makeUpGain = comp.makeUpGain;

    doBWFilter = df;
    doCompression = dc;
}

int RtAudioSoundSystem::audioCallback(void *outputBuffer, void *inputBuffer,
                                unsigned int nFrames,
                                double /*streamTime*/,
                                unsigned int status )
{
    qint64 tnow = QDateTime::currentMSecsSinceEpoch();
    if (stime == 0)
    {
        stime = tnow;
    }
    unsigned int inCh = inChannels[curInDev];
    unsigned int outCh = outChannels[curOutDev];

    cbacks += 1;
    qreal msecsPerCallback = (tnow - stime)/cbacks;
    qreal actual = (256 * 1000)/msecsPerCallback;
   // return 0;

    if (inputBuffer == nullptr && !ipSystem->receiving)
    {
        vudata v;
        v.actual = actual;

        emit setVU( v );
        return 0;
    }
#if defined (_MSC_VER)
    int16_t *inStageBuffer = new int16_t[nFrames * 2];
#else
    int16_t inStageBuffer[nFrames * 2];
#endif

    if (outputBuffer == nullptr && inputBuffer == nullptr)
    {
        return 0;   // no data
    }
    callbacks++;

    if ( status == RTAUDIO_INPUT_OVERFLOW)
    {
        trace("Stream input overflow detected.");
    }
    if (status == RTAUDIO_OUTPUT_UNDERFLOW)
    {
        trace("Stream output underflow detected.");
    }

    // If no outputBuffer, we may be sending IP; if so we need to get the outputBuffer
    // from the IPDataBuffer to be filled

    // NB if there is no outputBuffer and we are reading a file then no InterruptOK
    // gets sent - so the play gets killed quite quickly

    InBuff *replayBuff = nullptr;   // output from ringbuffer - send to audio. inputBuffer should be null
    InBuff *recordBuff = nullptr;   //audio to record into ringbuffer. outputBuffer should be null

    if (outputBuffer == nullptr)
    {
        recordBuff = dataBuffer->getNextInputBuffer();
        if (recordBuff)
        {
            recordBuff->bh.ptt = pttState;
            recordBuff->bh.frameCount = nFrames;

            outputBuffer = recordBuff->buff;
        }

    }
    if (inputBuffer == nullptr)
    {
        replayBuff = dataBuffer->getNextOutputBuffer();
        while (dataBuffer->buffered() > 10)
        {
            dropped++;
            dataBuffer->unlockNextOutput();
            // pick up the latest frame
            replayBuff = dataBuffer->getNextOutputBuffer();
        }
        if (replayBuff)
        {

            inputBuffer = replayBuff->buff;
            bool ptts = replayBuff->bh.ptt;
            if (ptts != pttState)
            {
                pttState = ptts;
                emit ptt(ptts);
            }
        }
        else
        {
            missed++;
        }

    }

    if (outputBuffer != nullptr && nFrames > 0)
    {
        memset(outputBuffer, 0, nFrames * 2 * outCh);   // 2 bytes, 2 channels
    }

    // if we are reading from IP compression etc was done earlier
    // If we are reading from a file, replayCompressor is applied in readFromFile()
    if (!replayBuff && inputBuffer && !outputEnabled && nFrames && outputBuffer)
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
            double initi1 = q[i * inCh];
            double initi2 = (inCh > 1)?q[i * inCh + 1]:q[i * inCh];

            double s1 = initi1;
            double s2 = initi2;

            if (passThroughEnabled)
            {
                // don't compress while recording
                // then passthrough and replay will be compressed similarly

                double ds1 = s1/32768.0;
                double ds2 = s2/32768.0;

                if (doBWFilter)
                {
                    ds1 =  bw_band_pass(micfilter1, ds1);
                    ds2 =  bw_band_pass(micfilter2, ds2);
                }
                if (doCompression)
                {
                    micCompressor.process(ds1, ds2);
                    ds1 *= chunkware_simple::dB2lin(makeUpGain);
                    ds2 *= chunkware_simple::dB2lin(makeUpGain);
                }

                s1 = ds1 * 32768.0;
                s2 = ds2 * 32768.0;
            }

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

            if (passThroughEnabled)
            {
                m[i * outCh] = static_cast<qint16>(val1);
                m[i * outCh + 1] = static_cast<qint16>(val2);

            }
            if (inputEnabled)
            {
                p[i * outCh] = static_cast<qint16>(val1);
                if (inCh > 1)
                    p[i * outCh + 1] = static_cast<qint16>(val2);
                else
                    p[i * outCh + 1] = static_cast<qint16>(val1);
            }

            int16_t sample = static_cast<int16_t>(std::max( val1, val2 ));
            if ( sample > maxvol )
               maxvol = sample;

            sqaccum += sample * sample;
        }
        qreal rmsval = sqrt(sqaccum/nFrames);
        if (inputEnabled || passThroughEnabled)
        {
            vudata v;
            v.blocks = nFrames;
            v.peak = maxvol;
            v.rms = rmsval;
            v.actual = actual;

            emit setVU( v );
        }
        if (recordBuff != nullptr)
        {
            recordBuff->bh.rms = rmsval;
            recordBuff->bh.peak = maxvol;
        }
        if (inputEnabled)
        {
            wThread->copyBuffer(inStageBuffer, nFrames);
        }
    }
    if (outputBuffer != nullptr && nFrames != 0 && outputEnabled )
    {
        int16_t maxvol = 0;
        qreal rmsval = 0.0;
        readFromFile(outputBuffer, nFrames, maxvol, rmsval);

        vudata v;
        v.blocks = nFrames;
        v.peak = maxvol;
        v.rms = rmsval;
        v.actual = actual;

        emit setVU( v );
    }
    if (replayBuff != nullptr)  // playing from IP
    {
        memcpy(outputBuffer, replayBuff->buff, replayBuff->bh.frameCount * 2 * outCh);

        qint64 delay = QDateTime::currentMSecsSinceEpoch() - replayBuff->bh.tnow;
        //int buffered = dataBuffer->buffered();

        dataBuffer->unlockNextOutput();
        vudata v;
        v.blocks = nFrames;
        v.peak = replayBuff->bh.peak;
        v.rms = replayBuff->bh.rms;
        v.delay = delay;
        v.dropped = dropped;
        v.callbacks = callbacks;
        v.missed = missed;
        v.actual = actual;

        emit setVU( v );
    }
    if (recordBuff != nullptr)
    {
        dataBuffer->unlockNextInput();
        emit soundAvailable();  // to IP; try to send it
    }

    /*
   To continue normal stream operation, the RtAudioCallback function
   should return a value of zero.  To stop the stream and drain the
   output buffer, the function should return a value of one.  To abort
   the stream immediately, the client should return a value of two.
   */

    return 0;
}


void RtAudioSoundSystem::startOutput()
{
    trace("startOutput");
    outputEnabled = true;
}

void RtAudioSoundSystem::stopOutput()
{
    trace("stopOutput");
    outputEnabled = false;
    emit ssOutputFinished();
    emit setVU(vudata());
}
void RtAudioSoundSystem::startInput()
{
    inputEnabled = true;

}

void RtAudioSoundSystem::stopInput()
{
    inputEnabled = false;
    emit actionQueueFinished();
    wThread->finishInput();
}
bool RtAudioSoundSystem::startInput( QString fn )
{
    // open fname, assign a text(?)
    // startInput() will also be called later

    // Should we do this in the writer thread?
    wThread->startInput();
    if (!outWave)
    {
        outWave = new WaveFile;
    }

    if ( outWave->OpenForWrite( fn.toLatin1(), sampleRate, 16, 2 ) == DDC_SUCCESS )
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
        DDCRET ret = outWave->WriteData ( q, nFrames * 2 );   // size is numdata
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

                for (int i = 0; i < total/(2 * 2); i += 1)
                {
                    // if NOT pip and NOT tone apply the compressor

                    qreal val = *m++;
                    qreal val2 = *m++;
                    if (!tone)
                    {

                        double ds1 = val/32768.0;
                        double ds2 = val2/32768.0;

                        if (doBWFilter)
                        {
                            ds1 =  bw_band_pass(replayfilter1, ds1);
                            ds2 =  bw_band_pass(replayfilter2, ds2);
                        }
                        if (doCompression)
                        {
                            replayCompressor.process(ds1, ds2);
                            ds1 *= chunkware_simple::dB2lin(makeUpGain);
                            ds2 *= chunkware_simple::dB2lin(makeUpGain);
                        }

                        val = ds1 * 32768.0;
                        val2 = ds2 * 32768.0;


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
    emit setVU( vudata() );
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
    emit setVU( vudata() );
    return true;
}
