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


RotatorCompassFrame::RotatorCompassFrame(QWidget *parent)
    : RotControlFrameBase(parent)
    , ui(new Ui::RotatorCompassFrame)
{
    ui->setupUi(this);

    rotFrameData.setFrameName("rotCompassControl");

    connect(this, &RotatorCompassFrame::sendCompassDial, ui->compassDialDisplay, &MinosCompass::compassDialUpdate);
    connect(this, &RotatorCompassFrame::updateEndStopType, ui->compassDialDisplay, &MinosCompass::updateEndStopType);
    connect(this, &RotatorCompassFrame::updateSouthStopType, ui->compassDialDisplay, &MinosCompass::updateSouthStopType);
    connect(this, &RotatorCompassFrame::updateAntennaOffset, ui->compassDialDisplay, &MinosCompass::updateAntennaOffset);
    connect(this, &RotatorCompassFrame::updateSkyScanRotatorStartBearing, ui->compassDialDisplay, &MinosCompass::updateSkyScanStartBearing);
    connect(this, &RotatorCompassFrame::updateSkyScanRotatorEndBearing, ui->compassDialDisplay, &MinosCompass::updateSkyScanEndBearing);


    // click on compass rose

    connect(ui->compassDialDisplay, &MinosCompass::sendClickBearing, this, &RotatorCompassFrame::compassClicked);
    connect(ui->compassDialDisplay, &MinosCompass::sendStop, this, &RotatorCompassFrame::stop_rotation);




/*
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
*/
}

RotatorCompassFrame::~RotatorCompassFrame()
{
    delete ui;
}



QGroupBox* RotatorCompassFrame::getRotatorControlGroupBoxObject()
{
    return ui->rotatoCompassFrameGroupBox;
}

QComboBox* RotatorCompassFrame::getAntennaSelectObject()
{
    return ui->antennaNameSel;
}

QToolButton* RotatorCompassFrame::getRotateButtonObject()
{
    return ui->Rotate;
}

BearingLineEdit* RotatorCompassFrame::getBrgLineEditObject()
{
    return ui->BrgLineEdit;
}


QLabel* RotatorCompassFrame::getRotConnectStateLabelObject()
{
    return ui->rotConnectState;
}
QLabel* RotatorCompassFrame::getRotatorStatMsgLabelObject()
{
    return ui->rotatorStatMsg;
}
QLabel* RotatorCompassFrame::getRotBrgDisplayObject()
{
    return ui->RotBrgDisplay;
}


QToolButton* RotatorCompassFrame::RotatorCompassFrame::getRotateLeftObject()
{
    return ui->RotateLeft;
}
QToolButton* RotatorCompassFrame::getRotateRightObject()
{
    return ui->RotateRight;
}
QToolButton* RotatorCompassFrame::getnudgeLeftObject()
{
    return ui->nudgeLeft;
}
QToolButton* RotatorCompassFrame::getnudgeRightObject()
{
    return ui->nudgeRight;
}


void  RotatorCompassFrame::setRotatorCompassBearing(const QString &s)
{
    traceMsg(QString("Bearings from rotator control = %1").arg(s));

    // send to text display

    int bearing = 0;
    bool ok = true;

    QStringList sl = s.split(":");
    if (sl.count() == 3)
    {
        bearing = sl[0].toInt(&ok);
        if (ok)
        {
           emit sendCompassDial(bearing);
        }
    }

    ok = false;

    if (!ok)
    {
        traceMsg("Bearing from rotator control error!");
    }


}

void  RotatorCompassFrame::compassClicked(int bearing)
{
    traceMsg(QString("Compass Dial Click, Send Bearing %1 to Rotator Control").arg(QString::number(bearing)));
    emit sendRotator(rpcConstants::eRotateDirect, bearing);

    ui->BrgLineEdit->setText(QString::number(bearing));
    ui->BrgLineEdit->selectAll();
}


void  RotatorCompassFrame::stop_rotation()
{
    traceMsg(QString("Compass Dial Clicked"));
    emit sendRotator(rpcConstants::eRotateStop, 0);
}


// the compass uses rotatorBearings to draw skyscan annulus
// also southStop and rotator type
void RotatorCompassFrame::setSkyScanRotatorStartBearing(int rotatorStartBearing)
{
    if (skyScanVisible)
    {
        emit updateSkyScanRotatorStartBearing(rotatorStartBearing);
    }

}

void RotatorCompassFrame::setSkyScanRotatorEndBearing(int rotatorEndBearing)
{
    if (skyScanVisible)
    {
        emit updateSkyScanRotatorEndBearing(rotatorEndBearing);
    }

}


void RotatorCompassFrame::setSkyScanVisible(bool state)
{
    skyScanVisible = state;
}

void RotatorCompassFrame::setRotatorSouthStopOffset(QString data)
{
    if (data.count(':') != 2)
    {
        return;
    }

    int rotType;
    bool endStopOk = false;

    int south_Stop;
    bool southStopOk = false;

    int antennaOffset;
    bool antennaOffsetOk = false;


    QStringList dl = data.split(":", Qt::KeepEmptyParts);

    if (dl.count() != 3)
    {
        return;
    }

    rotType = dl[0].toInt(&endStopOk);

    if (endStopOk)
    {
        emit updateEndStopType(rotType);
    }

    south_Stop = dl[1].toInt(&southStopOk);

    if (southStopOk)
    {
        emit updateSouthStopType(south_Stop);
    }

    antennaOffset = dl[2].toInt(&antennaOffsetOk);

    if (antennaOffsetOk)
    {
        emit  updateAntennaOffset(antennaOffset);
    }








}
