/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018 - 2025
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "rotatorcommon.h"
#include "AppStartup.h"
#include "MTrace.h"
#include <QToolButton>


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


void displayCompassBearingWithOverlap(const QString newBearing, int &rotatorBearing, int &currentBearing, QLabel* displayLabel)
{
    // extract displayBearing:rotatorBearing:overlapstatus
    QStringList sl = newBearing.split(':');
    if (sl.size() < 3)
        return;

    trace("Display Bearing = " + sl[0]);
    trace("Rotator Bearing = " + sl[1]);
    trace("OverlapStatus = " + sl[2]);

    // save rotatorBearing
    bool ok;
    rotatorBearing = sl[1].toInt(&ok, 10);

    if (!ok)
    {
        trace("Error converting rotatorBearing to int");
        return;
    }

    int iBearing = sl[0].toInt(&ok, 10);
    currentBearing = iBearing;

    if (!ok)
    {
        trace("Error converting displayBearing to int");
        return;
    }


    QString brg;
    QChar degsym = QChar(DEGREE_SYMBOL);

    brg = QString("%1%2").arg(iBearing, 3, 10, QChar('0')).arg(degsym);

    brg.append("</font>");

    if (rotatorBearing > COMPASS_MAX360 && sl[2] == "1")
    {
        brg.prepend("<font color='Red'>");
        displayLabel->setText(brg);
    }
    else if (rotatorBearing < COMPASS_MIN0 && sl[2] == "1")
    {
        brg.prepend("<font color='Blue'>");
        displayLabel->setText(brg);
    }
    else
    {
        brg.prepend("<font color='Black'>");
        displayLabel->setText(brg);
    }

}


void setSkyScanDirectionIndOnOff(QToolButton *indicator, bool state)
{
    if (state)
    {
        indicator->setStyleSheet(SKYSCAN_DIRECTION_INDICATOR_ON);
    }
    else
    {
        indicator->setStyleSheet(SKYSCAN_DIRECTION_INDICATOR_OFF);
    }
}
