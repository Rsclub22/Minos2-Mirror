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

#ifndef ROTCONTROLFRAME_H
#define ROTCONTROLFRAME_H

#include <QFrame>
#include <QShortcut>
#include <QComboBox>
#include "RPCCommandConstants.h"
#include "rigmemcommondata.h"
#include "MinosLoggerEvents.h"
#include "rotControlFrameBase.h"


namespace Ui {
class RotControlFrame;
}





class RotControlFrame : public RotControlFrameBase
{
    Q_OBJECT


public:
    explicit RotControlFrame(QWidget *parent);
    ~RotControlFrame();

    Ui::RotControlFrame *ui;



protected:

    //QWidget* getCompassObject() override;
    QGroupBox* getRotatorControlGroupBoxObject() override;
    QComboBox* getAntennaSelectObject() override;
    QLabel* getRotConnectStateLabelObject() override;
    QLabel* getRotatorStatMsgLabelObject() override;
    BearingLineEdit* getBrgLineEditObject() override;
    QToolButton* getRotateButtonObject() override;
    QToolButton* getStopRotateObject() override;
    QToolButton* getRotateLeftObject() override;
    QToolButton* getRotateRightObject() override;
    QToolButton* getnudgeLeftObject() override;
    QToolButton* getnudgeRightObject() override;
    QLabel* getRotBrgDisplayObject() override;


/*
    QShortcut* getNudgeRightShortCutObject() override;
    QShortcut* getNudgeLeftShortCutObject() override;
    QShortcut* getRotateLeftShortCutObject() override;
    QShortcut* getRotateRightShortCutObject() override;
    QShortcut* getTurnToBearingShortCutObject() override;
    QShortcut* getRotateStopShortCutObject() override;
*/
private:















};

#endif // ROTCONTROLFRAME_H
