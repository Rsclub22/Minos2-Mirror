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

    void setPathStart(int start){pathStart = start;}
    int getPathStart(){return pathStart;}

    void setPathEnd(int end){pathEnd = end;}
    int getPathEnd(){return pathEnd;}
    QString getListOfSteps()
    {
        QStringList stepList;
        for (int value : rotationPath)
        {
          stepList.append(QString::number(value));
        }

        QString result = stepList.join(',');

        return result;
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

    void initSkyScan(enum southStop southType_, enum endStop endStopType_, int minRot, int maxRot, int start, int end, int step, int interval);
    void startSkyscan();
    void stopSkyscan();
    void pauseSkyscan();


    bool getMovingToStartPositionFlag(){return movingToStepPosition;}
    bool getMovingToStepPositionFlag(){return movingToStepPosition;}
    bool getSkyScanPauseIntervalFlag(){return skyScanPauseInterval;}


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



    bool skyScanRunning = false;
    bool skyScanPauseed = false;
    bool movingToStartPosition = false;
    bool movingToStepPosition = false;
    bool skyScanPauseInterval = false;
    bool reverseScan = false;


    ScanPath forwardRotationPath;
    ScanPath reverseRotationPath;

    QList<int> rotationPath;
    //QList<int> reverseRotationPath;

    int scanPauseTimeInterval = 0;
    int scanPauseTimeCount = 0;   // seconds
    QTimer *skyScanIntervalTimer = nullptr;

    void determinePath();

    void rotateToNextPosition();
    QString determineDirection(int current, int target);
    int toStandardBearing(int bearing);
    int toStopTypeBearing(int bearing);
    void traceMessage(QString msg);

    void calcForwardPath(int startBearing, int endBearing);
    void calcReversePath(int startBearing, int endBearing);
    void dumpRotationPathToTraceLog(ScanPath &scanPath, const QString &pathname);
};

#endif // SKYSCANCONTROL_H
