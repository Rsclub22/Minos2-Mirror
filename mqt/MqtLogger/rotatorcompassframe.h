/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef ROTATORCOMPASSFRAME_H
#define ROTATORCOMPASSFRAME_H

#include <QFrame>

#include <QShortcut>
#include "rotControlFrameBase.h"

namespace Ui {
class RotatorCompassFrame;
}

class BaseContestLog;

class RotatorCompassFrame : public RotControlFrameBase
{
    Q_OBJECT

public:
    explicit RotatorCompassFrame(QWidget *parent = nullptr);
    ~RotatorCompassFrame();



protected:
    QGroupBox *getRotatorControlGroupBoxObject();
    QComboBox *getAntennaSelectObject();
    QLabel *getRotConnectStateLabelObject();
    QLabel *getRotatorStatMsgLabelObject();
    QToolButton *getRotateLeftObject();
    QToolButton *getRotateRightObject();
    QToolButton *getnudgeLeftObject();
    QToolButton *getnudgeRightObject();
private:
    Ui::RotatorCompassFrame *ui;





};

#endif // ROTATORCOMPASSFRAME_H
