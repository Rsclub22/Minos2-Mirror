/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "rotatorcommon.h"
#include "AppStartup.h"

const char * RotPresetData::presetButtonLabels[4] = {QT_TR_NOOP("&Read"),
                                        QT_TR_NOOP("&New"),
                                        QT_TR_NOOP("&Edit"),
                                        QT_TR_NOOP("&Clear")};

RotPresetData::RotPresetData(int _number, QString _name, QString _bearing)
{
    number = _number;
    name = _name;
    bearing = _bearing;
}


RotPresetData::RotPresetData()
{

}



QString ANTENNA_PATH_LOCAL()
{
    return getDirectoryLocation(dlConfiguration) + "/Antenna/";
}

QString ANTENNA_PATH_LOGGER()
{
    return getDirectoryLocation(dlConfiguration) + "/Antenna/";
}

QString PST_CONFIG_FILE()
{
    return getDirectoryLocation(dlConfiguration) + "/PSTConfig.ini";
}

QString CONFIGURATION_FILEPATH_LOCAL()
{
    return getDirectoryLocation(dlConfiguration);
}

QString CONFIGURATION_FILEPATH_LOGGER()
{
    return getDirectoryLocation(dlConfiguration);
}

QString getBearingStateTxt(skyScanBearingStates state)
{
    return bearingStateTxt[state];
}


int adjustOverlapBearingToCompassBearing(int value)
{
    int bearing = value;

    if (bearing < 0)
    {
        bearing = bearing + COMPASS_MAX360;
    }
    else if (bearing > 360)
    {
        bearing = bearing - COMPASS_MAX360;
    }

    return bearing;
}







// bearing >= 0
QString convertBearingToString(int bearing)
{
    QString bearingStr = QString::number(bearing);

    if (bearing < 10)
    {
        bearingStr = bearingStr.prepend("00");
    }
    else if (bearing < 100)
    {
        bearingStr = bearingStr.prepend("0");
    }

    return bearingStr;
}
