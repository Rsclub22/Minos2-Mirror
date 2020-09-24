////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Client
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef SENTSPOTDATA_H
#define SENTSPOTDATA_H

#include <QString>
#include "clustercommon.h"

class SentSpotData
{
public:
    SentSpotData();
    SentSpotData(qint64 _rxTime, QString _spotTime,
             Frequency _dxFreq, QString _dxCall,
             QString _dxLocator, QString comment, bool _sentOk, QString _reason);

    void operator = (const SentSpotData& spd);

    qint64 rxTime;
    QString spotTime;
    Frequency dxFreq;
    QString dxCall;
    QString dxLocator;
    QString spotComment;
    bool sentOk;
    QString reason;



};

#endif // SENDSPOTDATA_H




