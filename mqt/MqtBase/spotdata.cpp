#include "spotdata.h"

SpotData::SpotData()
{

}

SpotData::SpotData(QString _spotTime, QString _dxFreq, QString _dxCall, QString _dxLocator, QString _spotterCall, QString _spotterLocator, QString comment)
{
    spotTime = _spotTime;
    dxFreq = _dxFreq;
    dxCall = _dxCall;
    dxLocator = _dxLocator;
    dxCallWorked = BOOL_NO;
    dxLocatorWorked = BOOL_NO;
    spotterCall = _spotterCall;
    spotterLocator = _spotterLocator;
    spotComment = comment;
}

void SpotData::operator = (const SpotData& spd)
{
    spotTime = spd.spotTime;
    dxFreq = spd.dxFreq;
    dxCall = spd.dxCall;
    dxLocator = spd.dxLocator;
    dxCallWorked = spd.dxCallWorked;
    dxLocatorWorked = spd.dxLocatorWorked;
    spotterCall = spd.spotterCall;
    spotterLocator = spd.spotterLocator;
    spotComment = spd.spotComment;
}


