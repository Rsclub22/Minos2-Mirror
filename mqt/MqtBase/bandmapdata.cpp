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
    rotConnected = false;
    dxCallWorked = false;
    dxLocatorWorked = false;
    sentToMemory = false;
    spotType = bandmapSpotType::NONE;
    isSelected = false;

}

BandmapData::BandmapData(qint64 _rxTime, QString _spotTime, QString _spotDate,
                   Frequency _dxFreq, QString _dxBand, QString _dxBandMask,
                   QString _dxMode,  QString _dxModeMask,
                   QString _dxCall, bool _dxCallWorked,
                   QString _dxLocator, bool _dxLocatorIsFromNode, bool _dxLocatorWorked,
                   QString _dxDistrict, bool _dxDistrictWorked,
                   QString _distance, QString _bearing, QString _rotBrg, bool _rotConnected,
                   QString _spotterCall, QString _spotterLocator, QString _dxPropMode,
                   bool _runModeOn, bool _offRunFreq,
                   QString comment, bandmapSpotType::SPOT_TYPE _spotType)
{

    rxTime = _rxTime;
    spotTime = _spotTime;
    spotDate = _spotDate;
    spotTime =  _spotTime;
    dxFreq = _dxFreq;
    dxBand = _dxBand;
    dxBandMask = _dxBandMask;
    dxMode = _dxMode;
    dxModeMask = _dxModeMask;
    dxCall = _dxCall;
    dxLocator = _dxLocator;
    dxLocatorIsFromNode = _dxLocatorIsFromNode;
    dxDist = _distance;
    dxDistrict = _dxDistrict;
    dxDistrictWorked = _dxDistrictWorked;
    dxBrg = _bearing;
    rotBrg = _rotBrg;
    rotConnected = _rotConnected;
    dxCallWorked = _dxCallWorked;
    dxLocatorWorked = _dxLocatorWorked;
    sentToMemory = false;
    spotterCall = _spotterCall;
    spotterLocator = _spotterLocator;
    dxPropMode = _dxPropMode;
    spotComment = comment;
    runModeOn = _runModeOn;
    offRunFreq = _offRunFreq;
    spotType = _spotType;
    isSelected = false;
}

void BandmapData::operator = (const BandmapData& bmd)
{
    rxTime = bmd.rxTime;
    spotTime = bmd.spotTime;
    spotDate = bmd.spotDate;
    spotDateTime = bmd.spotDateTime;
    dxFreq = bmd.dxFreq;
    dxBand = bmd.dxBand;
    dxBandMask = bmd.dxBandMask;
    dxMode = bmd.dxMode;
    dxModeMask = bmd.dxModeMask;
    dxCall = bmd.dxCall;
    dxLocator = bmd.dxLocator;
    dxLocatorIsFromNode = bmd.dxLocatorIsFromNode;
    dxDist = bmd.dxDist;
    dxBrg = bmd.dxDist;
    rotBrg = bmd.rotBrg;
    rotConnected = bmd.rotConnected;
    dxDistrict = bmd.dxDistrict;
    dxDistrictWorked = bmd.dxDistrictWorked;
    dxCallWorked = bmd.dxCallWorked;
    dxLocatorWorked = bmd.dxLocatorWorked;
    sentToMemory = bmd.sentToMemory;
    spotterCall = bmd.spotterCall;
    spotterLocator = bmd.spotterLocator;
    dxPropMode = bmd.dxPropMode;
    runModeOn = bmd.runModeOn;
    offRunFreq = bmd.offRunFreq;
    spotComment = bmd.spotComment;
    spotType = bmd.spotType;
    isSelected = bmd.isSelected;
}

void BandmapData::operator = (const BandmapData* bmd)
{
    rxTime = bmd->rxTime;
    spotTime = bmd->spotTime;
    spotDate = bmd->spotDate;
    spotDateTime = bmd->spotDateTime;
    dxFreq = bmd->dxFreq;
    dxBand = bmd->dxBand;
    dxBandMask = bmd->dxBandMask;
    dxMode = bmd->dxMode;
    dxModeMask = bmd->dxModeMask;
    dxCall = bmd->dxCall;
    dxLocator = bmd->dxLocator;
    dxLocatorIsFromNode = bmd->dxLocatorIsFromNode;
    dxDist = bmd->dxDist;
    dxBrg = bmd->dxDist;
    rotBrg = bmd->rotBrg;
    rotConnected = bmd->rotConnected;
    dxDistrict = bmd->dxDistrict;
    dxDistrictWorked = bmd->dxDistrictWorked;
    dxCallWorked = bmd->dxCallWorked;
    dxLocatorWorked = bmd->dxLocatorWorked;
    sentToMemory = bmd->sentToMemory;
    spotterCall = bmd->spotterCall;
    spotterLocator = bmd->spotterLocator;
    dxPropMode = bmd->dxPropMode;
    runModeOn = bmd->runModeOn;
    offRunFreq = bmd->offRunFreq;
    spotComment = bmd->spotComment;
    spotType = bmd->spotType;
    isSelected = bmd->isSelected;
}




void BandmapData::clear()
{

    rxTime = 0;
    spotTime.clear();
    spotDate.clear();
    spotDateTime.currentDateTimeUtc();
    dxFreq.clear();
    dxBand.clear();
    dxBandMask.clear();
    dxMode.clear();
    dxModeMask.clear();
    dxCall.clear();
    dxLocator.clear();
    dxLocatorIsFromNode = false;
    dxDist.clear();
    dxBrg.clear();
    rotBrg.clear();
    rotConnected = false;
    dxDistrict.clear();
    dxDistrictWorked = false;
    dxCallWorked = false;
    dxLocatorWorked = false;
    sentToMemory = false;
    spotterCall.clear();
    spotterLocator.clear();
    dxPropMode.clear();
    runModeOn = false;
    offRunFreq = false;
    spotComment.clear();
    spotType = bandmapSpotType::NONE;
    isSelected = false;

}
