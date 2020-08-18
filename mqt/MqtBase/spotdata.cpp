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
    clear();
}


SpotData::SpotData(const SpotData &spd)
{
    rxTime = spd.rxTime;
    spotTime = spd.spotTime;
    spotDate = spd.spotDate;
    spotDateTime = spd.spotDateTime;
    dxFreq = spd.dxFreq;
    dxBandStr = spd.dxBandStr;
    dxBandMask = spd.dxBandMask;
    dxModeStr = spd.dxModeStr;
    dxModeMask = spd.dxModeMask;
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
    askQrz = spd.askQrz;
}




void SpotData::clear(){

    dxCall.clear();
    dxFreq.clear();
    dxBandStr.clear();
    dxBandMask.clear();
    dxModeStr.clear();
    dxModeMask.clear();
    spotterCall.clear();
    spotComment.clear();
    spotTime.clear();
    spotDateTime = QDateTime::currentDateTimeUtc();
    spotDate.clear();
    dxLocator.clear();
    spotterLocator.clear();
    dxPropMode.clear();
    askQrz = false;
}



void SpotData::operator = (const SpotData& spd)
{
    rxTime = spd.rxTime;
    spotTime = spd.spotTime;
    spotDate = spd.spotDate;
    spotDateTime = spd.spotDateTime;
    dxFreq = spd.dxFreq;
    dxBandStr = spd.dxBandStr;
    dxBandMask = spd.dxBandMask;
    dxModeStr = spd.dxModeStr;
    dxModeMask = spd.dxModeMask;
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
    askQrz = spd.askQrz;
}


