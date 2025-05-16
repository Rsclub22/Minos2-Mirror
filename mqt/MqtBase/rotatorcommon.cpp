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

/*
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

*/





// bearing >= 0
QString convertBearingToString(int bearing)
{
    QString bearingStr = QString::number(bearing);

    if (bearing >= 0)
    {
        if (bearing < 10)
        {
            bearingStr = bearingStr.prepend("00");
        }
        else if (bearing < 100)
        {
            bearingStr = bearingStr.prepend("0");
        }
    }
    else
    {
        if (bearing > -10)
        {
            bearingStr = bearingStr.insert(1, "00");
        }
        else if (bearing > -100)
        {
            bearingStr = bearingStr.insert(1, "0");
        }
    }


    return bearingStr;
}


void displayCompassBearingWithOverlap(const QString newBearing, RotFrameData &rotFrameData, QLabel* displayLabel)
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
    rotFrameData.setRotatorBearing(sl[1].toInt(&ok, 10));

    if (!ok)
    {
        trace("Error converting rotatorBearing to int");
        return;
    }

    int iBearing = sl[0].toInt(&ok, 10);
    rotFrameData.setAntennaBearing(iBearing);

    if (!ok)
    {
        trace("Error converting displayBearing to int");
        return;
    }


    QString brg;
    QChar degsym = QChar(DEGREE_SYMBOL);

    brg = QString("%1%2").arg(iBearing, 3, 10, QChar('0')).arg(degsym);

    brg.append("</font>");

    if (rotFrameData.getRotatorBearing() > COMPASS_MAX360 && sl[2] == "1")
    {
        brg.prepend("<font color='Red'>");
        displayLabel->setText(brg);
    }
    else if (rotFrameData.getRotatorBearing() < COMPASS_MIN0 && sl[2] == "1")
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



SkyScanButtonState::SkyScanButtonState() : state(0)
{

}

void SkyScanButtonState::clear()
{
    state = 0;
}

int SkyScanButtonState::getState()
{
    return state;
}


void SkyScanButtonState::setState(int state_)
{
    state = state_;
}


QString SkyScanButtonState::getButtonStateToString()
{
    QString startBut = isStart() ? "On" : "Off";
    QString pauseBut = isPause() ? "On" : "Off";
    QString stopBut = isStop() ? "On" : "Off";
    QString fwdBut = isForward()  ? "On" : "Off";
    QString revBut = isReverse()  ? "On" : "Off";

    return QString("Start = %1, Pause = %2, Stop = %3, Fwd = %4, Rev = %5").arg(startBut).arg(pauseBut).arg(stopBut).arg(fwdBut).arg(revBut);
}

void SkyScanButtonState::setButtonState(int bit, bool value)
{
    if (value)
        state |= (1 << bit); // Set the bit
    else
        state &= ~(1 << bit); // Clear the bit
}

bool SkyScanButtonState::getButtonState(int bit) const
{
    return state & (1 << bit); // Check the bit
}

void SkyScanButtonState::setStart(bool value)
{
    setButtonState(START_BIT, value);
}

void SkyScanButtonState::setPause(bool value)
{
    setButtonState(PAUSE_BIT, value);
}

void SkyScanButtonState::setStop(bool value)
{
    setButtonState(STOP_BIT, value);
}

void SkyScanButtonState::setForward(bool value)
{
    setButtonState(FORWARD_BIT, value);
}

void SkyScanButtonState::setReverse(bool value)
{
    setButtonState(REVERSE_BIT, value);
}

bool SkyScanButtonState::isStart() const
{
    return getButtonState(START_BIT);
}

bool SkyScanButtonState::isPause() const
{
    return getButtonState(PAUSE_BIT);
}

bool SkyScanButtonState::isStop() const
{
    return getButtonState(STOP_BIT);
}

bool SkyScanButtonState::isForward() const
{
    return getButtonState(FORWARD_BIT);
}

bool SkyScanButtonState::isReverse() const
{
    return getButtonState(REVERSE_BIT);
}


bool isEasternBearing(int bearing)
{
    if (bearing > COMPASS_MAX360)
    {
        return true;
    }
    if (bearing >= COMPASS_MIN0 && bearing <= COMPASS_HALF)
    {
        return true;
    }

    return false;
}


bool isWesternBearing(int bearing)
{
    if (bearing > COMPASS_HALF && bearing <= COMPASS_MAX360 )
    {
        return true;
    }

    return false;
}





void skyScanDisplayRotatorMinAzMaxAz(int minAz, int maxAz, int antennaOffset, enum southStop southStopType, enum endStop endStopType, skyScanRotatorDisplayLabels displayLabels)
{
    Q_UNUSED(endStopType)

    QString minAzStr = QString::number(minAz).rightJustified(3, '0'); //convertBearingToString(minAz);
    QString maxAzStr = QString::number(maxAz).rightJustified(3, '0'); //convertBearingToString(maxAz);

    displayLabels.minAzimuthLabel->setText(minAzStr);
    displayLabels.maxAzimuthLabel->setText(maxAzStr);
    displayLabels.antennaOffsetLabel->setText(QString::number(antennaOffset));

    QString sStopDisplay = "South ";

    if (southStopType == S_STOPOFF)
    {
        displayLabels.southStopDisplayLabel->setVisible(false);
        displayLabels.rotatorStopLabel->setVisible(false);


    }
    else if (southStopType == S_STOPINV)
    {
        sStopDisplay.append("180 - 180");
        displayLabels.southStopDisplayLabel->setText(sStopDisplay);
        displayLabels.southStopDisplayLabel->setVisible(true);
        displayLabels.rotatorStopLabel->setVisible(true);

    }
    else if (southStopType == S_STOP_COMPASS_SENSOR)
    {

        sStopDisplay.append("-180 - 180 Compass Sensor");
        displayLabels.southStopDisplayLabel->setText(sStopDisplay);
        displayLabels.southStopDisplayLabel->setVisible(true);
        displayLabels.rotatorStopLabel->setVisible(true);
    }

}


