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


#include "sentSpotdata.h"

SentSpotData::SentSpotData()
{

}

SentSpotData::SentSpotData(qint64 _rxTime, QString _spotTime,
                   QString _dxFreq, QString _dxCall,
                   QString _dxLocator, QString comment, bool _sentOk, QString _reason)
{

    rxTime = _rxTime;
    spotTime = _spotTime;
    dxFreq = _dxFreq;
    dxCall = _dxCall;
    dxLocator = _dxLocator;
    spotComment = comment;
    sentOk  = _sentOk;
    reason = _reason;

}

void SentSpotData::operator = (const SentSpotData& sspd)
{
    rxTime = sspd.rxTime;
    spotTime = sspd.spotTime;
    dxFreq = sspd.dxFreq;
    dxCall = sspd.dxCall;
    dxLocator = sspd.dxLocator;
    spotComment = sspd.spotComment;
    sentOk  = sspd.sentOk;
    reason = sspd.reason;
}


