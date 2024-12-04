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

class skyScanControl : public QObject
{
    Q_OBJECT
public:
    explicit skyScanControl(QObject *parent = nullptr);

    enum StopPosition { North = 0, South = 180 };

    void setStartScanBrg(int startScanBrg_){startScanBrg = startScanBrg_;}
    int getStartScanBrg(){return startScanBrg;}

    void setEndtScanBrg(int endScanBrg_){endScanBrg = endScanBrg_;}
    int getEndScanBrg(){return endScanBrg;}

    void setStepDegrees(int stepDegrees_){stepDegrees = stepDegrees_;}
    int getStepDegrees(){return stepDegrees;}

    void setScanPauseTime(int scanPauseTimeMs_){scanPauseTimeMs = scanPauseTimeMs_;}
    int getScanPauseTime(){return scanPauseTimeMs;}

    void startSkyscan();

private:

    int range;              // rotation range (e.g. 360, 450, 540)
    int startScanBrg = 0;
    int endScanBrg = 0;
    int stepDegrees = 0;
    int currentBrg = 0;
    int wrapPoint = 0;      // Maximum bearing before wrap around
    StopPosition stopPosition = StopPosition::North;
    int scanPauseTimeMs = 0;

    QList<int> rotationPath;

    void determinePath();

    void rotateToNextPosition();
    QString determineDirection(int current, int target);


};

#endif // SKYSCANCONTROL_H
