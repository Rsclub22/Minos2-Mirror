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


#include "LoggerContest.h"
#include "rotatorskyscanframe.h"
#include "rotatorcommon.h"
#include "MTrace.h"
#include "ui_rotatorskyscanframe.h"


RotatorSkyScanFrame::RotatorSkyScanFrame(QWidget *parent):
    QFrame(parent),
    ct(nullptr),
    ui(new Ui::RotatorSkyScanFrame)
{

    ui->setupUi(this);
    skyScanEnabled = false;
    ui->skyScanGroupBox->setEnabled(skyScanEnabled);

}


RotatorSkyScanFrame::~RotatorSkyScanFrame()
{
    delete ui;

}



void RotatorSkyScanFrame::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);

}

void RotatorSkyScanFrame::on_ContestPageChanged()
{




}

void RotatorSkyScanFrame::setSkyScanEnabled(bool state)
{
    traceMsg(QString("enable msg = %1").arg(state ? "True" : "False"));
    if (skyScanEnabled != state)
    {
        skyScanEnabled = state;
        ui->skyScanGroupBox->setEnabled(skyScanEnabled);
    }
}

void RotatorSkyScanFrame::setSkyScanStartBearing(int startBearing)
{
    traceMsg(QString("start bearing = %1").arg(startBearing));
    if (skyScanEnabled)
    {
        if (startScanBearing != startBearing)
        {
            startScanBearing = startBearing;
            ui->startBearingDisplay->setText(QString::number(startScanBearing));
        }
    }
}

void RotatorSkyScanFrame::setSkyScanEndBearing(int endBearing)
{
    traceMsg(QString("end bearing = %1").arg(endBearing));
    if (skyScanEnabled)
    {
        if (endScanBearing != endBearing)
        {
            endScanBearing = endBearing;
            ui->endBearingDisplay->setText(QString::number(endScanBearing));
        }
    }
}

void RotatorSkyScanFrame::setSkyScanNextStep(QString nextStep)
{
    traceMsg(QString("nextStep bearing = %1").arg(nextStep));
    if (skyScanEnabled)
    {
        if (nextStepBearing != nextStep)
        {
            nextStepBearing = nextStep;
            ui->nextStepDisplayLbl->setText(nextStepBearing);
        }
    }
}


void RotatorSkyScanFrame::setSkyScanCountDown(QString countDown)
{
    traceMsg(QString("count down = %1").arg(countDown));
    if (skyScanEnabled)
    {
        if (countDownTime != countDown)
        {
            countDownTime = countDown;
            ui->pauseCountDisplay->setText(countDownTime);
        }
    }
}

void RotatorSkyScanFrame::setSkyScanButtonState(int state)
{
    traceMsg(QString("button state = %1").arg(QString::number(state)));
    rpcConstants::SkyScanButtonState bState = static_cast<rpcConstants::SkyScanButtonState>(state);

    if (skyScanEnabled)
    {
        if (bState != buttonState)
        {
            buttonState = bState;
        }
    }
}

void RotatorSkyScanFrame::setSkyScanReverseScan(bool state)
{

}


void RotatorSkyScanFrame::setRotatorBearing(QString bearing)
{
    traceMsg(QString("Bearings from rotator control = %1").arg(bearing));
    displayCompassBearingWithOverlap(bearing, rotatorBearing, currentBearing, ui->currentBearingDisplay);

}


void RotatorSkyScanFrame::setSkyScanStartButtonColour(QString style)
{
    ui->skyScanStartPb->setStyleSheet(style);
}

void RotatorSkyScanFrame::setSkyScanStopButtonColour(QString style)
{
    ui->skyScanStopPb->setStyleSheet(style);
}

void RotatorSkyScanFrame::setSkyScanCWIndicatorOnOff(bool state)
{
    setSkyScanDirectionIndOnOff(ui->forwardScanTb, state);
}

void RotatorSkyScanFrame::setSkyScanCCWIndicatorOnOff(bool state)
{
    setSkyScanDirectionIndOnOff(ui->reverseScanTb, state);
}



void RotatorSkyScanFrame::traceMsg(QString msg)
{
    trace(QString("[SkyScanFrame] %1").arg( msg));
}

