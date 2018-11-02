#ifndef SPOTDATA_H
#define SPOTDATA_H

#include <QString>
#include "clustercommon.h"

class SpotData
{
public:
    SpotData();
    SpotData(QString _spotTime, QString _dxFreq, QString _dxCall, QString _dxLocator, QString _spotterCall, QString _spotterLocator, QString comment);

    void operator = (const SpotData& spd);

    QString spotTime;
    QString dxFreq;
    QString dxCall;
    QString dxLocator;
    QString dxCallWorked;
    QString dxLocatorWorked;
    QString spotterCall;
    QString spotterLocator;
    QString spotComment;









};

#endif // SPOTDATA_H




