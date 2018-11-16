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

SpotData::SpotData(QString _spotTime, QString _dxFreq,
                   QString _dxFreqMaskStr, QString _dxModeMaskStr,
                   QString _dxCall, bool _dxCallWorked,
                   QString _dxLocator, bool _dxLocatorWorked,
                   QString distance, QString bearing,
                   QString _spotterCall, QString _spotterLocator,
                   QString comment)
{
    spotTime = _spotTime;
    dxFreq = _dxFreq;
    dxFreqMaskStr = _dxFreqMaskStr;
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
    spotComment = comment;
}

void SpotData::operator = (const SpotData& spd)
{
    spotTime = spd.spotTime;
    dxFreq = spd.dxFreq;
    dxFreqMaskStr = spd.dxFreqMaskStr;
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
    spotComment = spd.spotComment;
}


