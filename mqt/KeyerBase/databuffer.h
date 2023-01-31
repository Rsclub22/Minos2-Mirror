#ifndef DATABUFFER_H
#define DATABUFFER_H

#include <QObject>
#include <QMutex>

class InBuff;

class IPADataBuffer : public QObject
{
    Q_OBJECT

    QMutex mutex;

    InBuff *inBuffs = nullptr;
    int recIndex = -1;
    int writeIndex = -1;


public:
    IPADataBuffer(QObject *parent) ;
    virtual ~IPADataBuffer() override;

    void setBuffers(int bufferFrames);

    void startInput();
    void finishInput();

    void unlockNextInput();
    InBuff *getNextInputBuffer();

    bool isOutputAvailable();

    void unlockNextOutput();
    InBuff *getNextOutputBuffer();
};

#endif // DATABUFFER_H
