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
#include "rotatormainwindow.h"




SkyScanControl::SkyScanControl(RotatorMainWindow *rotatorMainWindow, QObject *parent)
    : QObject(parent),
    m_rotatorMainWindow(rotatorMainWindow)
{
    movingToStartPosition = false;
    skyScanIntervalTimer = new QTimer(this);
    connect(skyScanIntervalTimer, &QTimer::timeout, this, &SkyScanControl::skyScanIntervalTimerTimeOut);
    connect(m_rotatorMainWindow, &RotatorMainWindow::sendRotationStatusToSkyScan, this, &SkyScanControl::handleRotationBearings);
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

    skyScanRunning = true;

    // If current bearing is not at the start position, move the rotator there first
    if (currentBearing != startScanBearing)
    {
        traceMessage("Moving rotator to start position.");

        // Set flag to indicate movement to start position
        movingToStartPosition = true;
        targetBearing = startScanBearing;

        // Emit signal to command the calling program to rotate
        emit rotateTo(targetBearing);
    }
    else
    {
        // If already at start position, begin sky scan immediately
        determinePath();
        rotateToNextPosition();
    }



}

void SkyScanControl::stopSkyscan()
{
    traceMessage("Stopping skyscan...");
    skyScanRunning = false;
    movingToStartPosition = false;
    skyScanPauseed = false;
    movingToStepPosition = false;
    skyScanPauseInterval = false;
    rotationPath.clear();
    skyScanIntervalTimer->stop(); // Stop any pending timer events
}

void SkyScanControl::pauseSkyscan()
{
    traceMessage("Pausing skyscan...");
    skyScanIntervalTimer->stop(); // Stop any pending timer events
}

void SkyScanControl::skyScanIntervalTimerTimeOut()
{
    skyScanIntervalTimer->stop();
    skyScanPauseInterval = false;
    rotateToNextPosition();
}


void SkyScanControl::handleRotationBearings(int bearing, skyScanBearingStates brgState)
{

    traceMessage(QString("new bearing from rotator = %1, bearing state = %2").arg(bearing).arg(getBearingStateTxt(brgState)));
    if (currentBearing != bearing)
    {

        currentBearing = bearing;
    }



    if (brgState == skyScanBearingStates::ROT_STOPPED)
    {

        traceMessage(QString("Rotator stopped current bearing is %1 degrees.").arg(currentBearing));
    }
    else if (brgState == skyScanBearingStates::ROT_REACHED_TARGET || brgState == skyScanBearingStates::ROT_NEAR_TARGET)
    {
        if (movingToStartPosition)
        {
            traceMessage(QString("Reached StartScan Position = %1").arg(currentBearing));
            movingToStartPosition = false;
            startSkyscan();
        }
        else if (movingToStepPosition)
        {

            traceMessage(QString("Reached step Position = %1").arg(currentBearing));
            movingToStepPosition = false;

            if (currentBearing == endScanBearing)
            {
                traceMessage(QString("Reached end of Scan Position = %1").arg(currentBearing));
                if (!reverseScan)
                {
                    traceMessage("We are going to reverse scan");
                    // we want to reverse direction
                    reverseScan = true;
                    determinePath();
                    reversePath(rotationPath);
                }
                else
                {
                    traceMessage("We are going to forward scan");
                    reverseScan = false;
                    determinePath();
                }
            }
            // Schedule the next rotation
            traceMessage(QString("reverseScan flag = %1").arg(reverseScan ? "reverseScan" : "forwardScan"));
            traceMessage(QString("start interval timer %1 ms").arg(scanPauseTimeMs));
            skyScanPauseInterval = true;
            skyScanIntervalTimer->start(scanPauseTimeMs);

        }
    }
    else if (brgState == skyScanBearingStates::ROT_STOPPED_MOVING)
    {
        traceMessage(QString("Error Rotator has timedout and stopped moving"));
    }



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

void SkyScanControl::reversePath(QList<int> &rotationPath)
{
    QList<int> reversedPath;
    for (int i = rotationPath.size() - 1; i >= 0; --i)
    {
        reversedPath.append(rotationPath[i]);
    }
    rotationPath = reversedPath;
}

void SkyScanControl::rotateToNextPosition()
{
    if (rotationPath.isEmpty())
    {
        traceMessage(QString("scan complete."));
        return;
    }

    targetBearing = rotationPath.takeFirst();

    // Convert back to stop type range if needed
    if (southType == S_STOPINV)
    {
        targetBearing = toStopTypeBearing(targetBearing);
    }

    QString command = determineDirection(currentBearing, targetBearing);

    traceMessage(QString("Rotating %1 to %2 degrees.").arg(command).arg(targetBearing));

    movingToStepPosition = true;
    emit rotateTo(targetBearing);


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
