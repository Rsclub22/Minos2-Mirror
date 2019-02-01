#include "bandmapcallsignmarker.h"

BandmapCallsignMarker::BandmapCallsignMarker()
{

}

BandmapCallsignMarker::BandmapCallsignMarker(QString callsign, QString logtime, int type, int bearing)
{
    callSign = callsign;
    logTime = logtime;
    spotType = type;
    callBearing = bearing;
}


void BandmapCallsignMarker::setcallSign(QString callsign)
{
    callSign = callsign;
}

QString BandmapCallsignMarker::getcallSign()
{
    return callSign;
}

void BandmapCallsignMarker::setmarkerTime(QString logtime)
{
    logTime = logtime;
}

QString BandmapCallsignMarker::getmarkerTime()
{
    return logTime;
}

void BandmapCallsignMarker::setspotType(int spot_type)
{
    spotType = spot_type;
}

int BandmapCallsignMarker::getspotType()
{
    return spotType;
}


void BandmapCallsignMarker::setcallBearing(int bearing)
{
    callBearing = bearing;
}

int BandmapCallsignMarker::getcallBearing()
{
    return callBearing;
}


