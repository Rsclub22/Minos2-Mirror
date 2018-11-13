////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Client
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef SPOTDATA_H
#define SPOTDATA_H

#include <QString>
#include "clustercommon.h"

class SpotData
{
public:
    SpotData();
    SpotData(QString _spotTime, QString _dxFreq, QString _dxFreqMaskStr, QString _dxModeMaskStr, QString _dxCall, QString _dxLocator, QString _spotterCall, QString _spotterLocator, QString comment);

    void operator = (const SpotData& spd);

    QString spotTime;
    QString dxFreq;
    QString dxFreqMaskStr;
    QString dxModeMaskStr;
    QString dxCall;
    QString dxLocator;
    bool dxCallWorked;
    bool dxLocatorWorked;
    bool sentToMemory;
    QString spotterCall;
    QString spotterLocator;
    QString spotComment;









};

#endif // SPOTDATA_H




