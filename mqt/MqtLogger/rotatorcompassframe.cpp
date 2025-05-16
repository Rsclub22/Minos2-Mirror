/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control Logger Compass
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
/////////////////////////////////////////////////////////////////////////////
#include "rotatorcompassframe.h"
#include "ui_rotatorcompassframe.h"

#include "ContestApp.h"
#include "LoggerContest.h"
#include "cutils.h"
#include "tlogcontainer.h"
#include "SendRPCDM.h"
#include "rotatorcommon.h"
#include "MTrace.h"

RotatorCompassFrame::RotatorCompassFrame(QWidget *parent)
    : QFrame(parent)
    , ct(nullptr)
    , ui(new Ui::RotatorCompassFrame)
{
    ui->setupUi(this);


    nudgeRight1 = new QShortcut(QKeySequence("Ctrl++"), parent);   // Ctrl +
    connect(nudgeRight1, &QShortcut::activated, ui->nudgeRight, &QToolButton::click);

    nudgeRight2 = new QShortcut(QKeySequence("Ctrl+="), parent);   // Ctrl +
    connect(nudgeRight2, &QShortcut::activated, ui->nudgeRight, &QToolButton::click);

    nudgeLeft = new QShortcut(QKeySequence::ZoomOut, parent);   // Ctrl -
    connect(nudgeLeft, &QShortcut::activated, ui->nudgeLeft, &QToolButton::click);

    rotateLeft = new QShortcut(QKeySequence(ROTATE_CCW_KEY), parent);
    connect(rotateLeft, &QShortcut::activated, this, &RotatorCompassFrame::on_RotateLeft_clicked);


    rotateRight = new QShortcut(QKeySequence(ROTATE_CW_KEY), parent);
    connect(rotateRight, &QShortcut::activated, this, &RotatorCompassFrame::on_RotateRight_clicked);

    rotateStop = new QShortcut(QKeySequence(ROTATE_STOP_KEY), parent);
    connect(rotateStop, &QShortcut::activated, this, &RotatorCompassFrame::on_StopRotate_clicked);


    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::BrgStrToRot, this, &RotatorCompassFrame::setBrgFromQSOLog);

    // from match frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MatchBrgStrToRot, this, &RotatorCompassFrame::setBrgFromMatchFrame);

    // from cluster frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::SpotBrgStrToRot, this, &RotatorCompassFrame::setBrgFromSpot);

    // from memory frame
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::MemBrgStrToRot, this, &RotatorCompassFrame::setBrgFromFrmMemory);

}

RotatorCompassFrame::~RotatorCompassFrame()
{
    delete ui;
}


void RotatorCompassFrame::setContest(BaseContestLog( *c))
{
    ct = c;
}
