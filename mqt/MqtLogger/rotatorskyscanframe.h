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
#include "minospanel.h"
#include "rotatorcommon.h"

#include "MinosLoggerEvents.h"


namespace Ui {
class RotatorSkyScanFrame;
}





class RotatorSkyScanFrame : public MinosPanel
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
    void setSkyScanVisible(bool state);
    void setSkyScanStartBearing(int startBearing);
    void setSkyScanEndBearing(int endBearing);

signals:

    void sendSkyScanButtonState(SkyScanButtonState state);

private slots:
    void setSkyScanPauseButtonColour(QString style);
    void onSkyScanStartButtonClicked();
    void onSkyScanStopButtonClicked();
private:

    Ui::RotatorSkyScanFrame *ui;
    void traceMsg(QString msg);

    int rotatorBearing = 0;
    int currentBearing = 0;

    int startScanBearing = 0;
    int endScanBearing = 0;

    QString nextStepBearing;
    QString countDownTime;

    SkyScanButtonState buttonStateFromRotControl;
    SkyScanButtonState buttonStateToRotControl;

    bool skyScanVisible = false;
    bool skyScanReverse = false;


    void setSkyScanStartButtonColour(QString style);
    void setSkyScanStopButtonColour(QString style);
    void setSkyScanCWIndicatorOnOff(bool state);
    void setSkyScanCCWIndicatorOnOff(bool state);

    void handleSkyScanButtonStateFromRotator();
};

#endif // ROTATORSKYSCANFRAME_H
