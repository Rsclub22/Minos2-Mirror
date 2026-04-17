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
#include "MTrace.h"
#include "ui_rotatorskyscanframe.h"


RotatorSkyScanFrame::RotatorSkyScanFrame(QWidget *parent):
    MinosPanel(parent),
    ct(nullptr),
    ui(new Ui::RotatorSkyScanFrame)
{

    ui->setupUi(this);
    ui->skyScanPausePb->setVisible(false);      // not using pause at the moment
    skyScanVisible = false;
    ui->skyScanGroupBox->setEnabled(skyScanVisible);

    ui->forwardScanTb->setFocusPolicy(Qt::NoFocus);                       // Prevent focus rect
    ui->forwardScanTb->setAttribute(Qt::WA_TransparentForMouseEvents);    // Prevent interaction if it's just an indicator
    ui->forwardScanTb->setStyleSheet(SKYSCAN_DIRECTION_INDICATOR_OFF);    // Initial OFF state

    ui->reverseScanTb->setFocusPolicy(Qt::NoFocus);                       // Prevent focus rect
    ui->reverseScanTb->setAttribute(Qt::WA_TransparentForMouseEvents);    // Prevent interaction if it's just an indicator
    ui->reverseScanTb->setStyleSheet(SKYSCAN_DIRECTION_INDICATOR_OFF);    // Initial OFF state


    connect(ui->skyScanStartPb, &QPushButton::clicked, this, &RotatorSkyScanFrame::onSkyScanStartButtonClicked);
    connect(ui->skyScanStopPb, &QPushButton::clicked, this, &RotatorSkyScanFrame::onSkyScanStopButtonClicked);


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

void RotatorSkyScanFrame::setSkyScanVisible(bool state)
{
    traceMsg(QString("enable msg = %1").arg(state ? "True" : "False"));
    if (skyScanVisible != state)
    {
        skyScanVisible = state;
        ui->skyScanGroupBox->setEnabled(skyScanVisible);
    }
}

void RotatorSkyScanFrame::setSkyScanStartBearing(int startBearing)
{
    traceMsg(QString("start bearing = %1").arg(startBearing));
    if (skyScanVisible)
    {
        if (startScanBearing != startBearing)
        {
            startScanBearing = startBearing;
            ui->startBearingDisplay->setText(QString("%1%2").arg(QString::number(startScanBearing).rightJustified(3, '0')).arg(QChar(DEGREE_SYMBOL)));
        }
    }
}

void RotatorSkyScanFrame::setSkyScanEndBearing(int endBearing)
{
    traceMsg(QString("end bearing = %1").arg(endBearing));

    if (skyScanVisible)
    {
        if (endScanBearing != endBearing)
        {
            endScanBearing = endBearing;
            ui->endBearingDisplay->setText(QString("%1%2").arg(QString::number(endScanBearing).rightJustified(3, '0')).arg(QChar(DEGREE_SYMBOL)));
        }
    }
}

void RotatorSkyScanFrame::setSkyScanNextStep(QString nextStep)
{
    traceMsg(QString("nextStep bearing = %1").arg(nextStep));
    if (skyScanVisible)
    {
        if (nextStepBearing != nextStep)
        {
            nextStepBearing = nextStep;
            ui->nextStepDisplayLbl->setText(QString("%1%2").arg(nextStepBearing).arg(QChar(DEGREE_SYMBOL)));
        }
    }
}


void RotatorSkyScanFrame::setSkyScanCountDown(QString countDown)
{
    traceMsg(QString("count down = %1").arg(countDown));
    if (skyScanVisible)
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
    traceMsg(QString("button state from rotator %1").arg(buttonStateFromRotControl.getButtonStateToString()));

    if (skyScanVisible)
    {
        if (state != buttonStateFromRotControl.getState())
        {
            buttonStateFromRotControl.setState(state);
            traceMsg(QString("Update button state change from rotator %1").arg(buttonStateFromRotControl.getButtonStateToString()));
            handleSkyScanButtonStateFromRotator();
        }
    }
}

void RotatorSkyScanFrame::setSkyScanReverseScan(bool state)
{

}


void RotatorSkyScanFrame::setRotatorBearing(QString bearing)
{
    traceMsg(QString("Bearings from rotator control = %1").arg(bearing));

    RotFrameData rotFrameData;
    rotFrameData.setRotatorBearing(rotatorBearing);
    rotFrameData.setAntennaBearing(currentBearing);

    displayCompassBearingWithOverlap(bearing, rotFrameData, ui->currentBearingDisplay);

    rotatorBearing = rotFrameData.getRotatorBearing();
    currentBearing = rotFrameData.getAntennaBearing();


}


void RotatorSkyScanFrame::handleSkyScanButtonStateFromRotator()
{
    if (buttonStateFromRotControl.isStart())
    {
        setSkyScanStartButtonColour(BUTTON_ON_STYLE);
        setSkyScanPauseButtonColour("");
        setSkyScanStopButtonColour("");

    }

    if (buttonStateFromRotControl.isStop())
    {
        setSkyScanStartButtonColour("");
        setSkyScanPauseButtonColour("");
        setSkyScanStopButtonColour(BUTTON_ON_STYLE);

        setSkyScanCWIndicatorOnOff(false);
        setSkyScanCCWIndicatorOnOff(false);
    }

    //if (buttonStateFromRotControl.isPause())
   // {
   //     setSkyScanStartButtonColour("");
   //     setSkyScanPauseButtonColour(BUTTON_ON_STYLE);
   //     setSkyScanStopButtonColour("");
   // }

    if (buttonStateFromRotControl.isForward())
    {
        setSkyScanCWIndicatorOnOff(true);

    }

    if (buttonStateFromRotControl.isReverse())
    {

        setSkyScanCCWIndicatorOnOff(true);
    }
}


void RotatorSkyScanFrame::setSkyScanStartButtonColour(QString style)
{
    ui->skyScanStartPb->setStyleSheet(style);
}

void RotatorSkyScanFrame::setSkyScanPauseButtonColour(QString style)
{
    ui->skyScanPausePb->setStyleSheet(style);
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

void RotatorSkyScanFrame::onSkyScanStartButtonClicked()
{
    if (skyScanVisible)
    {

        traceMsg(QString("start button clicked"));
        if (!buttonStateFromRotControl.isStart())
        {
            buttonStateToRotControl.setStart(true);
            buttonStateToRotControl.setStop(false);
            traceMsg(QString("send start button state to rotator = %1").arg(buttonStateToRotControl.getButtonStateToString()));

            emit sendSkyScanButtonState(buttonStateToRotControl);
        }
    }
}

void RotatorSkyScanFrame::onSkyScanStopButtonClicked()
{
    if (skyScanVisible)
    {
        traceMsg(QString("stop button clicked"));
        if (buttonStateFromRotControl.isStart())
        {
            buttonStateToRotControl.setStart(false);
            buttonStateToRotControl.setStop(true);
            traceMsg(QString("send stop button state to rotator = %1").arg(buttonStateToRotControl.getButtonStateToString()));

            emit sendSkyScanButtonState(buttonStateToRotControl);
        }
    }
}

void RotatorSkyScanFrame::traceMsg(QString msg)
{
    trace(QString("[SkyScanFrame] %1").arg( msg));
}

