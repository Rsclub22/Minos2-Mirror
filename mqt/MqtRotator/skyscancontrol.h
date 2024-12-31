/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
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

class SkyScanControl : public QObject
{
    Q_OBJECT
public:


    explicit SkyScanControl(RotatorMainWindow *rotatorMainWindow, QObject *parent);

    ~SkyScanControl();

    void initSkyScan(enum southStop southType_, int minRot, int maxRot, int start, int end, int step, int interval);
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



    bool skyScanRunning = false;
    bool skyScanPauseed = false;
    bool movingToStartPosition = false;
    bool movingToStepPosition = false;
    bool skyScanPauseInterval = false;
    bool reverseScan = false;


    QList<int> rotationPath;
    QList<int> reverseRotationPath;

    int scanPauseTimeInterval = 0;
    int scanPauseTimeCount = 0;   // seconds
    QTimer *skyScanIntervalTimer = nullptr;

    void determinePath();

    void rotateToNextPosition();
    QString determineDirection(int current, int target);
    int toStandardBearing(int bearing);
    int toStopTypeBearing(int bearing);
    void traceMessage(QString msg);

    int calcNumberOfStepsAvailable(int startBearing, int stepSize, int maxAzimuth);
};

#endif // SKYSCANCONTROL_H
