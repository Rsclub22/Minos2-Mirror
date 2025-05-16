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

    LoggerContestLog *ct = nullptr;
public:
    explicit RotControlFrame(QWidget *parent);
    ~RotControlFrame();

    Ui::RotControlFrame *ui;



    void getRotDetails(memoryData::memData &m);







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

    QString lastConnectStat;
    QString lastStatus;

    void clearRotatorFlags();

    QString antennaName;
    bool isRotatorLoaded();


    void rot_left_button_on();
    void rot_left_button_off();
    void rot_right_button_on();
    void rot_right_button_off();

    void turnTo(int angle);

    void showRotLeftButOn();
    void showRotLeftButOff();
    void showRotRightButOn();
    void showRotRightButOff();
    void traceMsg(QString msg);
    void showTurnButOn();
    void showTurnButOff();


    void setCwCcW_Items_Visible(bool visible);





    QString convertBearingForDisplay(QString bearing);




signals:
    void selectRotator(QString);
    void sendRotator(rpcConstants::RotateDirection direction, int angle );

    void turnBearingReturn();
    void bearingEditReturn();

    void rotatorConnected(bool);




private slots:

    void on_Rotate_clicked();
    void on_RotateLeft_clicked();
    void on_RotateRight_clicked();
    void on_StopRotate_clicked();

    void on_nudgeLeft_clicked();
    void on_nudgeRight_clicked();
    void on_antennaNameSel_activated(int arg1);


    void setBrgFromQSOLog(QString);
    void setBrgFromSpot(QString brg);
    void setBrgFromFrmMemory(QString Brg);
    void setBrgFromMatchFrame(QString brg);
};

#endif // ROTCONTROLFRAME_H
