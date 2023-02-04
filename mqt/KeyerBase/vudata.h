#ifndef VUDATA_H
#define VUDATA_H

#include <QObject>
class vudata
{
public:
    vudata();

    qint16 peak = 0;
    qreal rms = 0.0;
    int blocks = 0;
    qint64 delay = 0;
    int callbacks = 0;
    int dropped = 0;
    int missed = 0;
};

#endif // VUDATA_H
