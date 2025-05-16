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

#include "qshortcut.h"
#include "rotatorcommon.h"
#include <QShortcut>
#include "RPCCommandConstants.h"
#include "rigmemcommondata.h"
#include "MinosLoggerEvents.h"

namespace Ui {
class RotatorCompassFrame;
}

class BaseContestLog;

class RotatorCompassFrame : public QFrame
{
    Q_OBJECT

public:
    explicit RotatorCompassFrame(QWidget *parent = nullptr);
    ~RotatorCompassFrame();

    void setContest(BaseContestLog *c);

private:
    Ui::RotatorCompassFrame *ui;
    BaseContestLog *ct = nullptr;

    QShortcut *nudgeRight1;
    QShortcut *nudgeRight2;
    QShortcut *nudgeLeft;
    QShortcut *rotateLeft;
    QShortcut *rotateRight;
    QShortcut *turnToBearing;
    QShortcut *rotateStop;

    int maxAzimuth = 0;
    int minAzimuth = 0;
    int currentBearing = 0;
    int rotatorBearing = 0;
    bool moving = false;
    bool movingCW = false;
    bool movingCCW = false;
    bool rot_left_button_status;
    bool rot_right_button_status;
    bool rotConnected = false;
    bool supportCwCcwCmd = true;
    bool supportStopCommand = true;
    bool rotError = false;

    QString antennaName;

    QString lastConnectStat;
    QString lastStatus;

    int compassDialBearing = 0;
    int antennaOffset = 0;
    int skyScanStartBearing = 0;
    int skyScanEndBearing = 0;
    endStop endStopType = ROT_0_360;
    southStop southStopType = southStop::S_STOPOFF;
};

#endif // ROTATORCOMPASSFRAME_H
