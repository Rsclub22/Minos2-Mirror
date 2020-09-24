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


SpotData::SpotData(qint64 _rxTime, QString _spotTime, QString _spotDate, QDateTime _spotDateTime,
                   QString _dxFreq, QString _dxBandStr, QString _dxBandMask,
                   QString _dxModeStr, QString _dxModeMaskStr,
                   QString _dxCall, bool _dxCallWorked,
                   QString _dxLocator, bool _dxLocatorWorked,
                   QString distance, QString bearing,
                   QString _spotterCall, QString _spotterLocator,
                   QString _dxPropMode, QString comment)
{


    rxTime = _rxTime;
    spotTime = _spotTime;
    spotDate = _spotDate;
    spotDateTime = _spotDateTime;
    dxFreq = _dxFreq;
    dxBandStr = _dxBandStr;
    dxBandMask = _dxBandMask;
    dxModeStr = _dxModeStr;
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



void SpotData::clear(){

    rxTime = 0;
    spotTime.clear();
    spotDate.clear();
    spotDateTime = QDateTime::currentDateTime();
    dxCall.clear();
    dxFreq.clear();
    dxBandStr.clear();
    dxBandMask.clear();
    dxModeStr.clear();
    dxModeMaskStr.clear();
    spotterCall.clear();
    spotComment.clear();
    dxLocator.clear();
    dxDist.clear();
    dxBrg.clear();
    dxCallWorked = false;
    dxLocatorWorked = false;
    dxLocatorFromQrz = false;
    sentToMemory = false;
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
    dxModeMaskStr = spd.dxModeMaskStr;
    dxCall = spd.dxCall;
    dxLocator = spd.dxLocator;
    dxDist = spd.dxDist;
    dxBrg = spd.dxBrg;
    dxCallWorked = spd.dxCallWorked;
    dxLocatorWorked = spd.dxLocatorWorked;
    sentToMemory = spd.sentToMemory;
    spotterCall = spd.spotterCall;
    spotterLocator = spd.spotterLocator;
    dxPropMode = spd.dxPropMode;
    spotComment = spd.spotComment;
    askQrz = spd.askQrz;
    dxLocatorFromQrz = spd.dxLocatorFromQrz;
}


