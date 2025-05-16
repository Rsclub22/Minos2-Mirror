/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017 - 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "cutils.h"
#include "tlogcontainer.h"

#include "rotcontrolframe.h"
#include "ui_rotcontrolframe.h"


RotControlFrame::RotControlFrame(QWidget *parent):
    RotControlFrameBase(parent)
    , ui(new Ui::RotControlFrame)
{
    ui->setupUi(this);

    clearBearingLineEdit();

/*
    nudgeRight1 = new QShortcut(QKeySequence("Ctrl++"), parent);   // Ctrl +
    connect(nudgeRight1, &QShortcut::activated, ui->nudgeRight, &QToolButton::click);

    nudgeRight2 = new QShortcut(QKeySequence("Ctrl+="), parent);   // Ctrl +
    connect(nudgeRight2, &QShortcut::activated, ui->nudgeRight, &QToolButton::click);

    nudgeLeft = new QShortcut(QKeySequence::ZoomOut, parent);   // Ctrl -
    connect(nudgeLeft, &QShortcut::activated, ui->nudgeLeft, &QToolButton::click);

    rotateLeft = new QShortcut(QKeySequence(ROTATE_CCW_KEY), parent);
    connect(rotateLeft, &QShortcut::activated, this, &RotControlFrame::on_RotateLeft_clicked);


    rotateRight = new QShortcut(QKeySequence(ROTATE_CW_KEY), parent);
    connect(rotateRight, &QShortcut::activated, this, &RotControlFrame::on_RotateRight_clicked);

    turnToBearing = new QShortcut(QKeySequence(ROTATE_TURN_KEY), parent);
    connect(turnToBearing, &QShortcut::activated, this, &RotControlFrame::on_Rotate_clicked);

    rotateStop = new QShortcut(QKeySequence(ROTATE_STOP_KEY), parent);
    connect(rotateStop, &QShortcut::activated, this, &RotControlFrame::on_StopRotate_clicked);

*/
    rot_left_button_off();
    rot_right_button_off();
    showTurnButOff();
    setCwCcW_Items_Visible(rotFrameData.getSupportCwCcwCmd());   // init visible
    traceMsg("RotControlFrame Started");

}



RotControlFrame::~RotControlFrame()
{
    delete ui;
}


QGroupBox* RotControlFrame::getRotatorControlGroupBoxObject()
{
    return ui->rotatorControlGroupBox;
}

QComboBox* RotControlFrame::getAntennaSelectObject()
{
    return ui->antennaNameSel;
}

QLabel* RotControlFrame::getRotConnectStateLabelObject()
{
    return ui->rotConnectState;
}
QLabel* RotControlFrame::getRotatorStatMsgLabelObject()
{
    return ui->rotatorStatMsg;
}
BearingLineEdit* RotControlFrame::getBrgLineEditObject()
{
    return ui->BrgLineEdit;
}
QToolButton* RotControlFrame::getRotateButtonObject()
{
    return ui->Rotate;
}
QToolButton* RotControlFrame::getStopRotateObject()
{
    return ui->StopRotate;
}
QToolButton* RotControlFrame::getRotateLeftObject()
{
    return ui->RotateLeft;
}
QToolButton* RotControlFrame::getRotateRightObject()
{
    return ui->RotateRight;
}
QToolButton* RotControlFrame::getnudgeLeftObject()
{
    return ui->nudgeLeft;
}
QToolButton* RotControlFrame::getnudgeRightObject()
{
    return ui->nudgeRight;
}
QLabel* RotControlFrame::getRotBrgDisplayObject()
{
    return ui->RotBrgDisplay;
}
/*
QShortcut* RotControlFrame::getNudgeRightShortCutObject()
{
    return ui.
}
QShortcut* RotControlFrame::getNudgeLeftShortCutObject()
{

}
QShortcut* RotControlFrame::getRotateLeftShortCutObject()
{

}
QShortcut* RotControlFrame::getRotateRightShortCutObject()
{

}
QShortcut* RotControlFrame::getTurnToBearingShortCutObject()
{

}
QShortcut* RotControlFrame::getRotateStopShortCutObject()
{

}

*/








