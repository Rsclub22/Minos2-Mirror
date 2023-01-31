#ifndef INBUFF_H
#define INBUFF_H

#include <QDataStream>

class BuffHeader
{
public:
    qint64 sequence = 0;
    qint64 tnow = 0;
    qint32 rms = 0;
    qint16 frameCount = 0;
    bool ptt = false;

    friend QDataStream& operator>> (QDataStream& in, BuffHeader& ff);
    friend QDataStream& operator<< (QDataStream & out, const BuffHeader & base);
};
class InBuff
{
public:
    InBuff();
    ~InBuff();

    BuffHeader bh;
    int16_t *buff;
};

#endif // INBUFF_H
