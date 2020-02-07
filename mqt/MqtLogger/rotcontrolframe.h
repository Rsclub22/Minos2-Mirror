/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef ROTCONTROLFRAME_H
#define ROTCONTROLFRAME_H

#include "base_pch.h"
#include "MinosLoggerEvents.h"
#include <QShortcut>
#include "rotatorcommon.h"
#include "presetbutton.h"
#include "rigmemcommondata.h"
#include "rotpresetdialog.h"


namespace Ui {
class RotControlFrame;
}
class RotControlFrame;




class RotControlFrame : public QFrame
{
    Q_OBJECT

    LoggerContestLog *ct = nullptr;
public:
    explicit RotControlFrame(QWidget *parent);
    ~RotControlFrame();

    Ui::RotControlFrame *ui;

    void setContest(BaseContestLog *);

    void setRotatorLoaded();

    void setRotatorList();
    void setRotatorState(const QString &s);
    void setRotatorBearing(const QString &s);
    void setRotatorAntennaName(const QString &s);
    void setRotatorMaxAzimuth(const int s);
    void setRotatorMinAzimuth(const int s);
    int getAngle(QString);
    int getCurrentBearing();

    QString getBrgTxtFrmFrame();

    void getRotDetails(memoryData::memData &m);

    void on_ContestPageChanged();
    void presetTurn(QString);

    void setCwCcwCmdEnable(bool s);

    void checkConnection();
    void closeContest();
private:

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
    bool rotError = false;

    QString lastConnectStat;
    QString lastStatus;

    void clearRotatorFlags();

    QString antennaName;
    bool rotatorLoaded;
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

    void keyPressEvent(QKeyEvent *event);
    void setCwCcW_Items_Visible(bool visible);




    void setTurnDisplayText(QString brg);
    QString convertBearingForDisplay(QString bearing);


    //bool validateBearingEntry(const QString brg);

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
    void getBrgFrmQSOLog(QString);

    void on_nudgeLeft_clicked();
    void on_nudgeRight_clicked();
    void on_antennaName_activated(const QString &arg1);


    void setBrgFromSpot(QString brg);
    void setBrgFromFrmMemory(QString Brg);
    void setBrgFromMatchFrame(QString brg);
    //void on_BearingStTextChange(const QString);
};

#endif // ROTCONTROLFRAME_H
