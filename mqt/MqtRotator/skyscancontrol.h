/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef SKYSCANCONTROL_H
#define SKYSCANCONTROL_H

#include <QObject>
#include <QList>
#include <QTimer>
#include "rotatorcommon.h"



class RotatorMainWindow;



class ScanState
{
public:
    explicit ScanState(){clear();}

    void clear()
    {
        movingToStartPosition = false;
        movingToStepPosition = false;
        reachedStartPosition = false;
        reacheadEndPosition = false;
        skyScanPauseInterval = false;

        skyScanRunning = false;
        skyScanPaused = false;

        reverseScan = false;
    };


    void setReachedStartPosition(bool reachedStartPosition_){reachedStartPosition = reachedStartPosition_;}
    bool getReachedStartPosition(){return reachedStartPosition;}

    void setReachedEndPosition(bool reacheadEndPosition_){reacheadEndPosition = reacheadEndPosition_;}
    bool getReachedEndPosition(){return reacheadEndPosition;}

    void setSkyScanRunning(bool skyScanRunning_){skyScanRunning = skyScanRunning_;}
    bool getSkyScanRunning(){return skyScanRunning;}

    void setSkyScanPaused(bool skyScanPaused_){skyScanPaused = skyScanPaused_;}
    bool getSkyScanPaused(){return skyScanPaused;}

    void setReverseScan(bool reverseScan_){reverseScan = reverseScan_;}
    bool getReverseScan(){return reverseScan;}

    void setMovingToStartPosition(bool movingToStartPosition_){movingToStartPosition = movingToStartPosition_;}
    bool getMovingToStartPosition(){return movingToStartPosition;}

    void setMovingToStepPosition(bool movingToStepPosition_){movingToStepPosition = movingToStepPosition_;}
    bool getMovingToStepPosition(){return movingToStepPosition;}

    void setSkyScanPauseInterval(bool skyScanPauseInterval_){skyScanPauseInterval = skyScanPauseInterval_;}
    bool getSkyScanPauseInterval(){return skyScanPauseInterval;}

private:


    bool movingToStartPosition = false;
    bool movingToStepPosition = false;
    bool reachedStartPosition = false;
    bool reacheadEndPosition = false;
    bool skyScanPauseInterval = false;

    bool skyScanRunning = false;
    bool skyScanPaused = false;

    bool reverseScan = false;

};

class ScanPath
{
public:

    explicit ScanPath(){}

    void clearScanPath()
    {
        pathStart = 0;
        pathEnd = 0;
        rotationPath.clear();

    }
    void clearRotationPath(){rotationPath.clear();}
    void appendToScanPath(int bearing){rotationPath.append(bearing);}
    bool isEmpty(){return rotationPath.isEmpty();}
    int numScanSteps(){return rotationPath.count();}
    int takeFirst(){return rotationPath.takeFirst();}
    int getNextStep()
    {
        if (isEmpty())
        {
          return -999;  // error!
        }

        return rotationPath.first();

    }

    void setPathStart(int start){pathStart = start;}
    int getPathStart(){return pathStart;}

    void setPathEnd(int end){pathEnd = end;}
    int getPathEnd(){return pathEnd;}
    QString getListOfSteps()
    {
        QStringList stepList;
        for (auto value : rotationPath)
        {
          stepList.append(QString::number(value));
        }

        QString result = stepList.join(',');

        return result;
    }

    void convertNegBearings()
    {
        for (auto& bearing : rotationPath)
        {
            if (bearing < 0)
            {
                bearing = COMPASS_MAX360 + bearing;
            }
        }
    }



private:

    int pathStart = 0;
    int pathEnd = 0;
    QList<int> rotationPath;

};



class SkyScanControl : public QObject
{
    Q_OBJECT
public:


    explicit SkyScanControl(RotatorMainWindow *rotatorMainWindow, QObject *parent);

    ~SkyScanControl();

    void initSkyScan(int currentBearing_, enum southStop southType_, enum endStop endStopType_, int minRot, int maxRot, int start, int end, int step, int interval);
    void startSkyscan();
    void stopSkyscan();
    void pauseSkyscan(bool pauseState); // true starts pause, false cancels pause




signals:

    void rotateTo(int bearing);
    void displaySkyScanPauseIntervalTime(int time);
    void displaySkyScanNextStepBearing(int degrees);

private slots:
    void skyScanIntervalTimerTimeOut();
    void handleRotationBearings(int bearing, skyScanBearingStates brgState);
private:


    RotatorMainWindow *m_rotatorMainWindow;

    int minRotation = 0;
    int maxRotation = 0;
    int startScanBearing = 0;
    int endScanBearing = 0;
    int stepDegrees = 0;
    int currentBearing = 0;
    int targetBearing = 0;      // these are intermediate bearings calculated between starScan
                                // bearing and end scan bearing
    southStop southType = southStop::S_STOPOFF;
    endStop endStopType = ROT_0_360;

    ScanState skyScanStateFlags;
    ScanPath forwardRotationPath;
    ScanPath reverseRotationPath;



    int scanPauseTimeInterval = 0;
    int scanPauseTimeCount = 0;   // seconds
    QTimer *skyScanIntervalTimer = nullptr;

    bool testPauseTimerActive = false;

    void determinePath();

    void rotateToNextPosition();
    QString determineDirection(int current, int target);
    int toStandardBearing(int bearing);
    int toStopTypeBearing(int bearing);
    void traceMessage(QString msg);

    void calcForwardPath(int startBearing, int endBearing);
    void calcReversePath(int startBearing, int endBearing);
    void dumpRotationPathToTraceLog(ScanPath &scanPath, const QString &pathname);
    void sendNextStepBearingToDisplay();

    int closestBearing(int currentBearing, int start, int end, int minRange, int maxRange);
    int normaliseBearing(int bearing, int minRange, int maxRange);
    int angularDistance(int from, int to, int minRange, int maxRange);

    void determinePathCompassSensor();
    void calcForwardPathCompassSensor(int startBearing_, int endBearing_);
    void calcReversePathCompassSensor(int startBearing_, int endBearing_);
    void getTestMode();
};

#endif // SKYSCANCONTROL_H
