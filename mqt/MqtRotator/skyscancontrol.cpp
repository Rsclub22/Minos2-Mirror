/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
/////////////////////////////////////////////////////////////////////////////




#include "skyscancontrol.h"
#include <QTimer>
#include <QtMath>
#include <QMessageBox>
#include "MTrace.h"
#include "rotatormainwindow.h"




SkyScanControl::SkyScanControl(RotatorMainWindow *rotatorMainWindow, QObject *parent)
    : QObject(parent),
    m_rotatorMainWindow(rotatorMainWindow)
{
    skyScanIntervalTimer = new QTimer(this);
    connect(skyScanIntervalTimer, &QTimer::timeout, this, &SkyScanControl::skyScanIntervalTimerTimeOut);
    connect(m_rotatorMainWindow, &RotatorMainWindow::sendRotationStatusToSkyScan, this, &SkyScanControl::handleRotationBearings);
}

SkyScanControl::~SkyScanControl()
{

}


void SkyScanControl::initSkyScan(int currentBearing_, enum southStop southType_, enum endStop endStopType_, int minRot, int maxRot, int start, int end, int step, int interval)
{
    currentBearing = currentBearing_;
    southType = southType_;
    endStopType = endStopType_;
    minRotation = minRot;
    maxRotation = maxRot;
    startScanBearing = start;
    endScanBearing = end;
    stepDegrees = step;
    scanPauseTimeInterval = interval;  // Mins
    //currentBearing = initialBearing;


}




void SkyScanControl::startSkyscan()
{

    traceMessage(QString("Start skyscan current bearing = %1, startScanBearing = %2, endScanBearing = %3, minRotation = %4, maxRotation = %5")
                     .arg(currentBearing).arg(startScanBearing).arg(endScanBearing).arg(minRotation).arg(maxRotation));

    skyScanStateFlags.setSkyScanRunning(true);

    // move rotator to startScan or endScan which ever is closest

    targetBearing = closestBearing(currentBearing, startScanBearing, endScanBearing, minRotation, maxRotation);

    if ( currentBearing != startScanBearing && currentBearing != endScanBearing)
    {
        traceMessage(QString("not at startScan or endScanBearing moving to bearing = %1").arg(targetBearing));

        // Set flag to indicate movement to start position
        skyScanStateFlags.setMovingToStartPosition(true);

        sendNextStepBearingToDisplay();

        // Emit signal to command the calling program to rotate
        emit rotateTo(targetBearing);
    }
    else
    {
        // If already at start position, begin sky scan immediately

        determinePath();

        // which direction will we start
        if (startScanBearing < endScanBearing)
        {
            QString("startSkyScan - startScanBearing < endScanBearing ");

            if (targetBearing == startScanBearing)
            {

                traceMessage("startSkyScan - targetBearing == startScanBearing, rotate forwardPath");
                skyScanStateFlags.setReverseScan(false);
            }
            else
            {
              traceMessage("startSkyScan - targetBearing == endScanBearing, rotate revervsePath");
              skyScanStateFlags.setReverseScan(true);
            }
        }
        else
        {
            QString("startSkyScan - startScanBearing > endScanBearing ");
            if (targetBearing == startScanBearing)
            {
                traceMessage("startSkyScan - targetBearing == startScanBearing, rotate revervsePath");
                skyScanStateFlags.setReverseScan(true);
            }
            else
            {
                traceMessage("startSkyScan - targetBearing == endScanBearing, rotate forwardPath");
                skyScanStateFlags.setReverseScan(false);
            }
        }

        if (skyScanStateFlags.getReverseScan())
        {
            m_rotatorMainWindow->setSkyScanCCWIndicatorOnOff(true);
            m_rotatorMainWindow->setSkyScanCWIndicatorOnOff(false);
        }
        else
        {

            m_rotatorMainWindow->setSkyScanCCWIndicatorOnOff(false);
            m_rotatorMainWindow->setSkyScanCWIndicatorOnOff(true);
        }

        rotateToNextPosition();
    }



}

void SkyScanControl::stopSkyscan()
{
    traceMessage("Stopping skyscan...");
    skyScanStateFlags.clear();
    forwardRotationPath.clearScanPath();
    reverseRotationPath.clearScanPath();
    skyScanIntervalTimer->stop(); // Stop any pending timer events
    m_rotatorMainWindow->setSkyScanCCWIndicatorOnOff(false);
    m_rotatorMainWindow->setSkyScanCWIndicatorOnOff(false);
}

void SkyScanControl::pauseSkyscan(bool pauseState)
{
    traceMessage(QString("SkyScan Pause - state = %1").arg(pauseState ? "Start Pause" : "Cancel Pause"));
    if (pauseState)
    {
        skyScanIntervalTimer->stop(); // Stop any pending timer events
    }
    else
    {

    }



}

void SkyScanControl::skyScanIntervalTimerTimeOut()
{
    if (skyScanStateFlags.getSkyScanPauseInterval())
    {
        scanPauseTimeCount--;
        emit displaySkyScanPauseIntervalTime(scanPauseTimeCount);
        traceMessage(QString("Pause Interval Count = %1").arg(scanPauseTimeCount));

        if (scanPauseTimeCount <= 0)
        {
            traceMessage(QString("Pause Interval Finished, count = %1").arg(scanPauseTimeCount));
            skyScanIntervalTimer->stop();
            skyScanStateFlags.setSkyScanPauseInterval(false);
            rotateToNextPosition();
        }

    }

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
        if (skyScanStateFlags.getMovingToStartPosition())
        {
            traceMessage(QString("Reached StartScan Position = %1").arg(currentBearing));
            skyScanStateFlags.setMovingToStartPosition(false);
            startSkyscan();
        }
        else if (skyScanStateFlags.getMovingToStepPosition())
        {

            traceMessage(QString("Reached step Position = %1").arg(currentBearing));
            sendNextStepBearingToDisplay();

            skyScanStateFlags.setMovingToStepPosition(false);



            // set the endScanBearing based upon direction of scan
            int endBearing;

            if (!skyScanStateFlags.getReverseScan())
            {
                endBearing = forwardRotationPath.getPathEnd();
            }
            else
            {
                endBearing = reverseRotationPath.getPathEnd();
            }


            if (currentBearing == endBearing)
            {
                traceMessage(QString("Reached end of Scan Position = %1").arg(currentBearing));
                if (!skyScanStateFlags.getReverseScan())
                {
                    traceMessage("We are going to reverse scan");
                    skyScanStateFlags.setReverseScan(true);
                    m_rotatorMainWindow->setSkyScanCCWIndicatorOnOff(true);
                    m_rotatorMainWindow->setSkyScanCWIndicatorOnOff(false);


                }
                else
                {
                    traceMessage("We are going to forward scan");
                    skyScanStateFlags.setReverseScan(false);
                    m_rotatorMainWindow->setSkyScanCCWIndicatorOnOff(false);
                    m_rotatorMainWindow->setSkyScanCWIndicatorOnOff(true);


                }

                determinePath();
            }
            // Schedule the next rotation
            traceMessage(QString("reverseScan flag = %1").arg(skyScanStateFlags.getReverseScan() ? "reverseScan" : "forwardScan"));
            skyScanStateFlags.setSkyScanPauseInterval(true);
            scanPauseTimeCount = scanPauseTimeInterval * 60;
            traceMessage(QString("start interval timer %1 secs").arg(scanPauseTimeCount));
            emit displaySkyScanPauseIntervalTime(scanPauseTimeCount);

            skyScanIntervalTimer->start(1000);

        }
    }
    else if (brgState == skyScanBearingStates::ROT_STOPPED_MOVING)
    {
        traceMessage(QString("Error Rotator has timedout and stopped moving"));
    }

}

void SkyScanControl::sendNextStepBearingToDisplay()
{

    int nextStep = 0;

    if (skyScanStateFlags.getMovingToStepPosition())
    {

        if (!skyScanStateFlags.getReverseScan())
        {
            nextStep = forwardRotationPath.getNextStep();
            if (nextStep != -999)
            {
                emit displaySkyScanNextStepBearing(nextStep);
            }



        }
        else
        {
            nextStep = reverseRotationPath.getNextStep();
            if (nextStep != -999)
            {

                emit displaySkyScanNextStepBearing(reverseRotationPath.getNextStep());
            }

        }
    }
    else if (skyScanStateFlags.getMovingToStartPosition())
    {


        emit displaySkyScanNextStepBearing(targetBearing);

    }

}

void SkyScanControl::determinePath()
{
    forwardRotationPath.clearRotationPath();
    reverseRotationPath.clearRotationPath();

    int bearing = startScanBearing;

    // Convert to standard range if needed
    if (southType == S_STOPINV)
    {
        bearing = toStandardBearing(bearing);
        endScanBearing = toStandardBearing(endScanBearing);
    }


    if (startScanBearing > endScanBearing)
    {

        calcForwardPath(endScanBearing, startScanBearing);
        calcReversePath(startScanBearing, endScanBearing);
    }
    else
    {

        calcForwardPath(startScanBearing, endScanBearing);
        calcReversePath(endScanBearing, startScanBearing);
    }

    dumpRotationPathToTraceLog(forwardRotationPath, QString("forward rotation path"));
    dumpRotationPathToTraceLog(reverseRotationPath, QString("reverse rotation path"));

    /*
    // Generate the forward rotation path

    bearing = bearing + stepDegrees; // first step

    while (true)
    {
        rotationPath.append(bearing);

        // Break if we reach the end angle
        if (bearing == endScanBearing) break;

        // Update the bearing
        bearing += stepDegrees;

        // Handle wrap-around logic
        if (bearing > maxRotation)
        {

            bearing = minRotation + (bearing - maxRotation - 1);
        }

        if (bearing < minRotation)
        {

            bearing = maxRotation - (minRotation - bearing - 1);
        }

        // Prevent infinite loops
        if (bearing == startScanBearing)
        {

            break;
        }
    }

    // Generate the reverse rotation path
    bearing = endScanBearing;
    while (true)
    {
        bearing -= stepDegrees; // Step backward
        if (bearing < minRotation)
        {

            bearing = maxRotation - (minRotation - bearing - 1);
        }

        if (bearing > maxRotation)
        {

            bearing = minRotation + (bearing - maxRotation - 1);
        }
        reverseRotationPath.append(bearing);

        // Break if we reach the start angle
        if (bearing == startScanBearing)
        {
            break;
        }
        // Prevent infinite loops
        if (bearing == endScanBearing)
        {

            break;
        }
    }
*/
}

void SkyScanControl::rotateToNextPosition()
{
    if (!skyScanStateFlags.getReverseScan() && forwardRotationPath.isEmpty())
    {
        traceMessage(QString("forward scan complete."));
        return;
    }
    else if (skyScanStateFlags.getReverseScan() && reverseRotationPath.isEmpty())
    {
        traceMessage(QString("reverse scan complete"));
    }

    if (!skyScanStateFlags.getReverseScan())
    {
        targetBearing = forwardRotationPath.takeFirst();
        traceMessage(QString("forward scan, next target bearing = %1").arg(targetBearing));
    }
    else
    {
        targetBearing = reverseRotationPath.takeFirst();
        traceMessage(QString("reverse scan, next target bearing = %1").arg(targetBearing));
    }



    // Convert back to stop type range if needed
    if (southType == S_STOPINV)
    {
        targetBearing = toStopTypeBearing(targetBearing);
    }

    QString command = determineDirection(currentBearing, targetBearing);

    traceMessage(QString("Rotating %1 to %2 degrees.").arg(command).arg(targetBearing));

    skyScanStateFlags.setMovingToStepPosition(true);

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

void SkyScanControl::calcForwardPath(int startBearing, int endBearing)
{
    // Generate the forward rotation path

    forwardRotationPath.setPathStart(startBearing);
    traceMessage(QString("forwardPath Start Bearing = %1").arg(startBearing));
    forwardRotationPath.setPathEnd(endBearing);
    traceMessage(QString("forwardPath End Bearing = %1").arg(endBearing));

    int nextStepBearing = startBearing + stepDegrees; // first step

    while (true)
    {
        forwardRotationPath.appendToScanPath(nextStepBearing);
        traceMessage(QString("forwardPath append nextStep Bearing = %1").arg(nextStepBearing));

        // Break if we reach the end angle
        if (nextStepBearing == endBearing) break;

        // Update the bearing
        nextStepBearing += stepDegrees;

        // Handle wrap-around logic
        if (nextStepBearing > maxRotation)
        {

            nextStepBearing = minRotation + (nextStepBearing - maxRotation - 1);
        }

        if (nextStepBearing < minRotation)
        {

            nextStepBearing = maxRotation - (minRotation - nextStepBearing - 1);
        }

        // Prevent infinite loops
        if (nextStepBearing == startBearing)
        {

            break;
        }
    }
}


void SkyScanControl::calcReversePath(int startBearing, int endBearing)
{

    reverseRotationPath.setPathStart(startBearing);
    reverseRotationPath.setPathEnd(endBearing);

    // Generate the reverse rotation path
    int nextStepBearing = startBearing;

    while (true)
    {
        nextStepBearing -= stepDegrees; // Step backward
        if (nextStepBearing < minRotation)
        {

            nextStepBearing = maxRotation - (minRotation - nextStepBearing - 1);
        }

        if (nextStepBearing > maxRotation)
        {

            nextStepBearing = minRotation + (nextStepBearing - maxRotation - 1);
        }

        reverseRotationPath.appendToScanPath(nextStepBearing);

        // Break if we reach the start angle
        if (nextStepBearing == endBearing)
        {
            break;
        }
        // Prevent infinite loops
        //if (nextStepBearing == endBearing)
       // {

       //    break;
       // }
    }

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





// Function to normalize a bearing to the desired range

int SkyScanControl::normaliseBearing(int bearing, int minRange, int maxRange)
{
    int range = maxRange - minRange;
    while (bearing < minRange)
    {
        bearing += range;
    }
    while (bearing >= maxRange)
    {
        bearing -= range;
    }
    return bearing;
}

// Function to calculate the shortest angular distance between two bearings

int SkyScanControl::angularDistance(int from, int to, int minRange, int maxRange)
{
    int range = maxRange - minRange;
    int diff = (to - from + range) % range;
    return qMin(diff, range - diff);
}

// Function to find the closest bearing

int SkyScanControl::closestBearing(int currentBearing, int start, int end, int minRange, int maxRange)
{
    // Normalise bearings to the given range
    currentBearing = normaliseBearing(currentBearing, minRange, maxRange);
    start = normaliseBearing(start, minRange, maxRange);
    end = normaliseBearing(end, minRange, maxRange);

    // Calculate distances to start and end
    int distanceToStart = angularDistance(currentBearing, start, minRange, maxRange);
    int distanceToEnd = angularDistance(currentBearing, end, minRange, maxRange);

    // Return the closer bearing
    return (distanceToStart <= distanceToEnd) ? start : end;
}

void SkyScanControl::dumpRotationPathToTraceLog(ScanPath &scanPath, const QString &pathname)
{
    QString msg;

    msg.append(pathname);
    msg.append(QString(" - Start Scan Bearing: %1").arg(scanPath.getPathStart()));
    msg.append(QString(" - End Scan Bearing: %1").arg(scanPath.getPathEnd()));
    msg.append(QString(" - Steps: "));
    msg.append(scanPath.getListOfSteps());

    traceMessage(msg);
}

void SkyScanControl::traceMessage(QString msg)
{
    trace(QString("[skyScan] - %1").arg(msg));
}
