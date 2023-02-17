#include "inbuff.h"


InBuff::InBuff()
{
}

InBuff::~InBuff()
{
    delete [] buff;
}
QDataStream & operator>>(QDataStream &in, BuffHeader &ff)
{
    in  >> ff.sequence;
    in >> ff.tnow;
    in >> ff.rms;
    in >> ff.peak;
    in >> ff.ptt;
    in >> ff.frameCount;
    return in;
}

QDataStream & operator<<(QDataStream &out, const BuffHeader &base)
{
    out << base.sequence;
    out << base.tnow;
    out << base.rms;
    out << base.peak;
    out << base.ptt;
    out << base.frameCount;
    return out;
}
