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
    dxFreq = 0;
}

BandmapData::BandmapData(qint64 _rxTime, QString _spotTime, QString _dxFreqStr,
                   qint64 _dxFreq, QString _dxMode, QString _dxFreqMaskStr, QString _dxModeMaskStr,
                   QString _dxCall, bool _dxCallWorked,
                   QString _dxLocator, bool _dxLocatorWorked,
                   QString distance, QString bearing,
                   QString _spotterCall, QString _spotterLocator, QString _dxPropMode,
                   QString comment, bandmapSpotType::SPOT_TYPE _spotType)
{

    rxTime = _rxTime;
    spotTime = _spotTime;
    dxFreqStr = _dxFreqStr;
    dxFreqMaskStr = _dxFreqMaskStr;
    dxFreq = _dxFreq;
    dxMode = _dxMode;
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
    dxPropMode = _dxPropMode;
    spotComment = comment;
    spotType = _spotType;
    isSelected = false;
}

void BandmapData::operator = (const BandmapData& bmd)
{
    rxTime = bmd.rxTime;
    spotTime = bmd.spotTime;
    dxFreqStr = bmd.dxFreqStr;
    dxFreqMaskStr = bmd.dxFreqMaskStr;
    dxFreq = bmd.dxFreq;
    dxMode = bmd.dxMode;
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
    dxPropMode = bmd.dxPropMode;
    spotComment = bmd.spotComment;
    spotType = bmd.spotType;
    isSelected = bmd.isSelected;
}
