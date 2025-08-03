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

    void setRotatorCompassBearing(const QString &s);


    void setSkyScanVisible(bool state);

    void setRotatorSouthStopOffset(QString data);
    void setSkyScanRotatorStartBearing(int rotatorStartBearing);
    void setSkyScanRotatorEndBearing(int rotatorEndBearing);
signals:
    void sendCompassDial(int);
    void updateSkyScanRotatorStartBearing(int);
    void updateSkyScanRotatorEndBearing(int);
    void updateEndStopType(int);
    void updateSouthStopType(int);
    void updateAntennaOffset(int);

protected:
    QWidget *getRotatorControlGroupBoxObject() override;
    QComboBox *getAntennaSelectObject() override;
    QToolButton* getRotateButtonObject() override;
    BearingLineEdit* getBrgLineEditObject() override;
    QLabel *getRotConnectStateLabelObject() override;
    QLabel *getRotatorStatMsgLabelObject() override;
    QLabel* getRotBrgDisplayObject() override;
    QToolButton *getRotateLeftObject() override;
    QToolButton *getRotateRightObject() override;
    QToolButton *getnudgeLeftObject() override;
    QToolButton *getnudgeRightObject() override;




private slots:
    void  compassClicked(int);
    void stop_rotation();
private:
    Ui::RotatorCompassFrame *ui;

    bool skyScanVisible = false;






};

#endif // ROTATORCOMPASSFRAME_H
