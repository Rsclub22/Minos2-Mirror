#ifndef RIFFWRITER_H
#define RIFFWRITER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

class RtAudioSoundSystem;
class InBuff;

class RiffWriter : public QThread
{
    Q_OBJECT

     RtAudioSoundSystem *ss;

     QWaitCondition bufferNotEmpty;
     QWaitCondition bufferNotFull;
     QMutex mutex;

     int bufferFrames ;
     InBuff *inBuffs;
     int recIndex = -1;
     int writeIndex = -1;


public:
     bool terminated;
    RiffWriter(RtAudioSoundSystem *parent, int bufferFrames) ;
    virtual ~RiffWriter() override;

    virtual void run() Q_DECL_OVERRIDE;

    void startInput();
    void wakeAll();

    void copyBuffer(int16_t *inStageBuffer, int nFrames);
    void finishInput();

};

#endif // RIFFWRITER_H
