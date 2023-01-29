#include "soundsys.h"
#include "riffwriter.h"

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

        if (terminated)
        {
            mutex.unlock();
            break;
        }

        if (writeIndex == -1 && recIndex == -1)
        {
            mutex.unlock();
            continue;
        }
        mutex.unlock();

        if (inBuffs[writeIndex%RINGBUFFERSIZE].frameCount > 0)
        {
            ss->writeDataToFile(inBuffs[writeIndex%RINGBUFFERSIZE].buff, inBuffs[writeIndex%RINGBUFFERSIZE].frameCount);
            mutex.lock();
            ++writeIndex;

            bufferNotFull.wakeAll();
            mutex.unlock();
        }
        else
        {
            ss->outWave->Close();
            mutex.lock();
            writeIndex = -1;
            recIndex = -1;
            bufferNotFull.wakeAll();
            bufferNotEmpty.wakeAll();
            mutex.unlock();
#ifdef Q_OS_UNIX
            sync();     // make sure it goes to disk
#endif
        }

    }
}

void RiffWriter::startInput()
{
    finishInput();
    mutex.lock();
    recIndex = 0;
    writeIndex = 0;
    mutex.unlock();
}

void RiffWriter::wakeAll()
{
    bufferNotEmpty.wakeAll();
}

void RiffWriter::copyBuffer(int16_t *inStageBuffer, int nFrames)
{
    mutex.lock();
    if (recIndex - writeIndex >= RINGBUFFERSIZE - 1)     //both only ever increase and are used %RINGBUFFERSIZE
    {
        // wait for writer to create a gap
        // NB writer waits when recIndex == writeIndex
        // after writing it signals bufferNotFull
        bufferNotFull.wait(&mutex);
    }
    mutex.unlock();

    if (writeIndex == -1 && recIndex == -1)
    {
        return;
    }

    inBuffs[recIndex % RINGBUFFERSIZE].frameCount = nFrames;
    memcpy(inBuffs[recIndex % RINGBUFFERSIZE].buff, inStageBuffer, nFrames * 4);

    mutex.lock();
    ++recIndex;
    bufferNotEmpty.wakeAll();
    mutex.unlock();
}

void RiffWriter::finishInput()
{
    // we want to wait for all buffers to be written

    mutex.lock();
    if (writeIndex == -1 && recIndex == -1)
    {
        mutex.unlock();
        return;
    }

    if (recIndex - writeIndex >= RINGBUFFERSIZE - 1)     // both only ever increase and are used %RINGBUFFERSIZE
    {
        // wait for writer to create a gap so that we can insert a "close" frame
        // NB writer waits when recIndex == writeIndex
        // after writing it signals bufferNotFull
        bufferNotFull.wait(&mutex);
    }

    mutex.unlock();

    inBuffs[recIndex%RINGBUFFERSIZE].frameCount = 0;  // mark to close

    mutex.lock();
    ++recIndex;

    bufferNotEmpty.wakeAll();   // say there is room for input
    mutex.unlock();
}
