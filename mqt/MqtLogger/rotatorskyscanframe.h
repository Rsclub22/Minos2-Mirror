/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator SkyScan Frame
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef ROTATORSKYSCANFRAME_H
#define ROTATORSKYSCANFRAME_H

#include <QFrame>
#include <QObject>
#include <QToolButton>
#include "RPCCommandConstants.h"
#include "MinosLoggerEvents.h"


namespace Ui {
class RotatorSkyScanFrame;
}





class RotatorSkyScanFrame : public QFrame
{
    Q_OBJECT
    LoggerContestLog *ct = nullptr;
public:
    explicit RotatorSkyScanFrame(QWidget *parent);
    ~RotatorSkyScanFrame();



    void setContest(BaseContestLog *c);
    void on_ContestPageChanged();
    void setSkyScanNextStep(QString nextStep);
    void setSkyScanCountDown(QString countDown);
    void setSkyScanButtonState(int state);
    void setSkyScanReverseScan(bool state);
    void setRotatorBearing(QString bearing);
    void setSkyScanEnabled(bool state);
    void setSkyScanStartBearing(int startBearing);
    void setSkyScanEndBearing(int endBearing);

private:

    Ui::RotatorSkyScanFrame *ui;
    void traceMsg(QString msg);

    int rotatorBearing = 0;
    int currentBearing = 0;

    int startScanBearing = 0;
    int endScanBearing = 0;

    QString nextStepBearing;
    QString countDownTime;

    rpcConstants::SkyScanButtonState buttonState = rpcConstants::eSkyScanStop;

    bool skyScanEnabled = false;
    bool skyScanReverse = false;


    void setSkyScanStartButtonColour(QString style);
    void setSkyScanStopButtonColour(QString style);
    void setSkyScanCWIndicatorOnOff(bool state);
    void setSkyScanCCWIndicatorOnOff(bool state);

};

#endif // ROTATORSKYSCANFRAME_H
