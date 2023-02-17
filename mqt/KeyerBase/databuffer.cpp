#include <QDataStream>
#include "inbuff.h"
#include "databuffer.h"

#define RINGBUFFERSIZE 1024

IPADataBuffer::IPADataBuffer(QObject *parent) : QObject(parent)
{

}
IPADataBuffer::~IPADataBuffer()
{
    delete [] inBuffs;
}

void IPADataBuffer::setBuffers(int bufferFrames)
{
    delete [] inBuffs;

    inBuffs = new InBuff[RINGBUFFERSIZE];

    for(int i = 0; i < RINGBUFFERSIZE; i++)
    {
        inBuffs[i].buff = new int16_t[bufferFrames * 2];
    }
}

bool IPADataBuffer::isOutputAvailable()
{
    bool ret = false;
    mutex.lock();
    if (writeIndex != recIndex)
    {
        ret = true;
    }
    mutex.unlock();
    return ret;
}

InBuff *IPADataBuffer::getNextOutputBuffer()
{
    mutex.lock();
    if (writeIndex < recIndex)
    {
        mutex.unlock();
        return &inBuffs[writeIndex%RINGBUFFERSIZE];
    }
    mutex.unlock();
    return nullptr;
}
void IPADataBuffer::unlockNextOutput()
{
    mutex.lock();
    ++writeIndex;
    mutex.unlock();
}

void IPADataBuffer::unlockNextInput()
{
    mutex.lock();
    ++recIndex;
    mutex.unlock();
}
InBuff *IPADataBuffer::getNextInputBuffer()
{
    mutex.lock();
    if (recIndex >= 0 && writeIndex >= 0 && recIndex - writeIndex < RINGBUFFERSIZE - 1)     //both only ever increase and are used %RINGBUFFERSIZE
    {
        mutex.unlock();
        return &inBuffs[recIndex % RINGBUFFERSIZE];
    }
    mutex.unlock();
    return nullptr;
}

void IPADataBuffer::startInput()
{
    mutex.lock();
    recIndex = 0;
    writeIndex = 0;
    mutex.unlock();
}


