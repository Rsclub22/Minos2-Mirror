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


#include "spotdata.h"

SpotData::SpotData()
{

}

SpotData::SpotData(qint64 _rxTime, QString _spotTime, QString _spotDate,
                   QString _dxFreq, QString _dxBandStr, QString _dxBandMask,
                   QString _dxMode, QString _dxModeMaskStr,
                   QString _dxCall, bool _dxCallWorked,
                   QString _dxLocator, bool _dxLocatorWorked,
                   QString distance, QString bearing,
                   QString _spotterCall, QString _spotterLocator,
                   QString _dxPropMode, QString comment)
{

    rxTime = _rxTime;
    spotTime = _spotTime;
    spotDate = _spotDate;
    dxFreq = _dxFreq;
    dxBandStr = _dxBandStr;
    dxBandMask = _dxBandMask;
    dxMode = _dxMode;
    dxModeMaskStr = _dxModeMaskStr;
    dxCall = _dxCall;
    dxLocator = _dxLocator;
    dxDist = distance;
    dxBrg = bearing;
    dxCallWorked = _dxCallWorked;
    dxLocatorWorked = _dxLocatorWorked;
    sentToMemory = BOOL_NO;
    spotterCall = _spotterCall;
    spotterLocator = _spotterLocator;
    dxPropMode = _dxPropMode;
    spotComment = comment;
}

void SpotData::operator = (const SpotData& spd)
{
    rxTime = spd.rxTime;
    spotTime = spd.spotTime;
    spotDate = spd.spotDate;
    dxFreq = spd.dxFreq;
    dxBandStr = spd.dxBandStr;
    dxBandMask = spd.dxBandMask;
    dxMode = spd.dxMode;
    dxModeMaskStr = spd.dxModeMaskStr;
    dxCall = spd.dxCall;
    dxLocator = spd.dxLocator;
    dxDist = spd.dxDist;
    dxBrg = spd.dxDist;
    dxCallWorked = spd.dxCallWorked;
    dxLocatorWorked = spd.dxLocatorWorked;
    sentToMemory = spd.sentToMemory;
    spotterCall = spd.spotterCall;
    spotterLocator = spd.spotterLocator;
    dxPropMode = spd.dxPropMode;
    spotComment = spd.spotComment;
}


