/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                       Bandmap Data
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////

#include "bandmapdata.h"

BandmapData::BandmapData()
{
    rxTime = 0;
    sentToMemory = false;
}

BandmapData::BandmapData(qint64 _rxTime, QString _spotTime, QString _dxFreq,
                   QString _dxFreqMaskStr, QString _dxModeMaskStr,
                   QString _dxCall, bool _dxCallWorked,
                   QString _dxLocator, bool _dxLocatorWorked,
                   QString distance, QString bearing,
                   QString _spotterCall, QString _spotterLocator,
                   QString comment)
{

    rxTime = _rxTime;
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
    sentToMemory = false;
    spotterCall = _spotterCall;
    spotterLocator = _spotterLocator;
    spotComment = comment;
}

void BandmapData::operator = (const BandmapData& bmd)
{
    rxTime = bmd.rxTime;
    spotTime = bmd.spotTime;
    dxFreq = bmd.dxFreq;
    dxFreqMaskStr = bmd.dxFreqMaskStr;
    dxModeMaskStr = bmd.dxModeMaskStr;
    dxCall = bmd.dxCall;
    dxLocator = bmd.dxLocator;
    dxDist = bmd.dxDist;
    dxBrg = bmd.dxDist;
    dxCallWorked = bmd.dxCallWorked;
    dxLocatorWorked = bmd.dxLocatorWorked;
    sentToMemory = bmd.sentToMemory;
    spotterCall = bmd.spotterCall;
    spotterLocator = bmd.spotterLocator;
    spotComment = bmd.spotComment;
}
