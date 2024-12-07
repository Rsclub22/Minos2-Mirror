/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
//
/////////////////////////////////////////////////////////////////////////////




#include "skyscancontrol.h"
#include <QTimer>
#include "MTrace.h"




SkyScanControl::SkyScanControl(QObject *parent)
    : QObject(parent)
{
    movingToStartPosition = false;
    skyScanIntervalTimer = new QTimer(this);
    connect(skyScanIntervalTimer, &QTimer::timeout, this, &SkyScanControl::skyScanIntervalTimerTimeOut);
}

SkyScanControl::~SkyScanControl()
{

}

void SkyScanControl::initSkyScan(enum southStop southType_, int minRot, int maxRot, int start, int end, int step, int interval)
{
    southType = southType_;
    minRotation = minRot;
    maxRotation = maxRot;
    startScanBearing = start;
    endScanBearing = end;
    stepDegrees = step;
    scanPauseTimeMs = interval * 1000;  // mSecs
    //currentBearing = initialBearing;
    movingToStartPosition = false;
}


void SkyScanControl::startSkyscan()
{

    // If current bearing is not at the start position, move the rotator there first
    if (currentBearing != startScanBearing)
    {
        trace("Moving rotator to start position.");

        // Set flag to indicate movement to start position
        movingToStartPosition = true;

        // Emit signal to command the calling program to rotate
        emit rotateTo(startScanBearing);
    }
    else
    {
        // If already at start position, begin sky scan immediately
        rotateToNextPosition();
    }



}

void SkyScanControl::stopSkyscan()
{
    trace("Stopping skyscan...");
    rotationPath.clear();
    skyScanIntervalTimer->stop(); // Stop any pending timer events
}

void SkyScanControl::pauseSkyscan()
{
    trace("Pausing skyscan...");
    skyScanIntervalTimer->stop(); // Stop any pending timer events
}

void SkyScanControl::skyScanIntervalTimerTimeOut()
{
    rotateToNextPosition();
}

void SkyScanControl::setCurrentBearing(int newBearing)
{
    if (movingToStartPosition)
    {
        // If still moving to start position, check if it has reached
        if (newBearing == startScanBearing)
        {
            movingToStartPosition = false; // Clear flag
            startSkyscan(); // Start the sky scan
        }
    }

    currentBearing = newBearing;

    trace(QString("skyScan - Updated current bearing to %1 degrees.").arg(currentBearing));
}

void SkyScanControl::determinePath()
{
    rotationPath.clear();
    int bearing = startScanBearing;

    // Convert to standard range if needed
    if (southType == S_STOPINV)
    {
        bearing = toStandardBearing(bearing);
        endScanBearing = toStandardBearing(endScanBearing);
    }

    // Generate the rotation path
    while (true)
    {
        bearing += stepDegrees;

        // Handle wrap-around logic
        if (bearing > maxRotation) bearing = minRotation + (bearing - maxRotation - 1);
        if (bearing < minRotation) bearing = maxRotation - (minRotation - bearing - 1);

        rotationPath.append(bearing);

        // Break if we reach the end angle
        if (bearing == endScanBearing) break;

        // Prevent infinite loops
        if (bearing == startScanBearing) break;
    }
}

void SkyScanControl::rotateToNextPosition()
{
    if (rotationPath.isEmpty())
    {
        trace(QString("Skyscan complete."));
        return;
    }

    int targetBearing = rotationPath.takeFirst();

    // Convert back to stop type range if needed
    if (southType == S_STOPINV)
    {
        targetBearing = toStopTypeBearing(targetBearing);
    }

    QString command = determineDirection(currentBearing, targetBearing);

    traceMessage(QString("Rotating %1 to %2 degrees.").arg(command).arg(targetBearing));

    emit rotateTo(targetBearing);

    currentBearing = targetBearing;

    // Schedule the next rotation
    skyScanIntervalTimer->start(scanPauseTimeMs);
}

QString SkyScanControl::determineDirection(int current, int target)
{
    int range = maxRotation - minRotation + 1;

    // Convert angles to standard range for calculation
    if (southType == S_STOPINV)
    {
        current = toStandardBearing(current);
        target = toStandardBearing(target);
    }

    int cwDistance = (target - current + range) % range;
    int ccwDistance = (current - target + range) % range;

    return cwDistance < ccwDistance ? ROT_STATUS_ROTATE_CCW : ROT_STATUS_ROTATE_CW;
}

int SkyScanControl::toStandardBearing(int bearing)
{
    if (southType == S_STOPINV)
    {
        // South (0°) -> -180°, North (180°) stays 180°, South again (360°) -> 540°
        if (bearing >= 0 && bearing <= 180)
        {
            return bearing - 180;
        }
        else if (bearing > 180 && bearing <= 360)
        {
            return bearing - 180;
        }
    }
    return bearing;
}

int SkyScanControl::toStopTypeBearing(int bearing)
{
    if (southType == S_STOPINV)
    {
        // Standard -180° -> 0°, North (180°) stays 180°, 540° -> 360°
        if (bearing >= -180 && bearing < 0)
        {
            return bearing + 180;
        }
        else if (bearing >= 0 && bearing <= 360)
        {
            return bearing + 180;
        }
    }
    return bearing;
}

void SkyScanControl::traceMessage(QString msg)
{
    trace(QString("skyScan - %1").arg(msg));
}
