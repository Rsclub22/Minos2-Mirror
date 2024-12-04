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


skyScanControl::skyScanControl(QObject *parent)
    : QObject{parent}
{

}


void skyScanControl::startSkyscan()
{
    determinePath(); // Calculate the rotation path
    rotateToNextPosition(); // Start the rotation process
}

void skyScanControl::determinePath()
{
    rotationPath.clear();
    int angle = startScanBrg;

    // Generate the rotation path
    while (true)
    {
        angle += stepDegrees;
        if (angle >= wrapPoint) angle -= wrapPoint; // Wrap around based on range

        // Avoid crossing through the stop position
        if ((stopPosition == North && (angle == 0 || (startScanBrg < endScanBrg && angle > endScanBrg))) ||
            (stopPosition == South && (angle == 180 || (startScanBrg < endScanBrg && angle > endScanBrg))))
        {
            break;
        }

        if (angle == startScanBrg)
        {
            break; // End the loop if we complete a full circle
        }


        rotationPath.append(angle);
    }

    // Ensure endAngle is added if reachable
    if (endScanBrg > startScanBrg)
    {
        rotationPath.append(endScanBrg);
    }
}

void skyScanControl::rotateToNextPosition()
{
    if (rotationPath.isEmpty())
    {
        trace("Skyscan complete.");
        return;
    }

    int targetBrg = rotationPath.takeFirst();
    QString direction = determineDirection(currentBrg, targetBrg);

    trace(QString("Rotating %1 to %2 degrees.").arg(direction, targetBrg));

    currentBrg = targetBrg;

    // Schedule the next rotation
    QTimer::singleShot(scanPauseTimeMs, this, &skyScanControl::rotateToNextPosition);
}

QString skyScanControl::determineDirection(int current, int target)
{
    int cwDistance = (target - current + wrapPoint) % wrapPoint;
    int ccwDistance = (current - target + wrapPoint) % wrapPoint;

    if (cwDistance < ccwDistance)
    {
        return "CW";
    } else
    {
        return "CCW";
    }
}
