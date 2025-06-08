/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control Frame Base for Logger
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017 - 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef ROTCONTROLFRAMEBASE_H
#define ROTCONTROLFRAMEBASE_H

#include <QFrame>
#include <QShortcut>
#include <QComboBox>
#include <QGroupBox>
#include "RPCCommandConstants.h"
#include "rigmemcommondata.h"
#include "MinosLoggerEvents.h"
#include "bearinglineedit.h"
#include "ContestApp.h"






class RotControlFrameBase : public QFrame
{
    Q_OBJECT


public:
    explicit RotControlFrameBase(QWidget *parent);
    ~RotControlFrameBase();


    void setFrameName(QString frameName);
    void setContest(BaseContestLog *c);
    int getAngle(QString brgSt);
    int getCurrentBearing();
    virtual void setRotatorBearing(const QString &s);
    void setSupportStopCommandFlag(bool state);
    void setCwCcwCmdEnable(bool s);

    void setRotatorMinAzimuth(const int minAz);
    void setRotatorMaxAzimuth(const int maxAz);
    void skyScanStartedSetFrameDisabled(bool disabled);
    void setRotatorState(const QString &s);

    void setRotatorAntennaName(const QString &s);
    void checkConnection();
    void setRotatorList();
    void on_ContestPageChanged();
    virtual void presetTurn(QString);

    void getRotDetails(memoryData::memData &m);


    virtual void activate();
    virtual void deactivate();

protected:

    virtual QWidget* getCompassObject() { return nullptr; }
    virtual QGroupBox* getRotatorControlGroupBoxObject() { return nullptr; }
    virtual QComboBox* getAntennaSelectObject() { return nullptr; }
    virtual QLabel* getRotConnectStateLabelObject() { return nullptr; }
    virtual QLabel* getRotatorStatMsgLabelObject() { return nullptr; }
    virtual BearingLineEdit* getBrgLineEditObject() { return nullptr; }
    virtual QToolButton* getRotateButtonObject() { return nullptr; }
    virtual QToolButton* getStopRotateObject() { return nullptr; }
    virtual QToolButton* getRotateLeftObject() { return nullptr; }
    virtual QToolButton* getRotateRightObject() { return nullptr; }
    virtual QToolButton* getnudgeLeftObject() { return nullptr; }
    virtual QToolButton* getnudgeRightObject() { return nullptr; }
    virtual QLabel* getRotBrgDisplayObject() { return nullptr; }
    virtual QShortcut* getNudgeRightShortCutObject() { return nullptr; };
    virtual QShortcut* getNudgeLeftShortCutObject() { return nullptr; }
    virtual QShortcut* getRotateLeftShortCutObject() { return nullptr; }
    virtual QShortcut* getRotateRightShortCutObject() { return nullptr; }
    virtual QShortcut* getTurnToBearingShortCutObject() { return nullptr; }
    virtual QShortcut* getRotateStopShortCutObject() { return nullptr; }

    virtual void keyPressEvent(QKeyEvent *event) override;


    QString convertBearingForDisplay(QString bearing);
    void setTurnDisplayText(QString brg);
    void turnTo(int angle);


    void rot_left_button_on();
    void rot_left_button_off();
    void rot_right_button_on();
    void rot_right_button_off();
    void showTurnButOn();
    void showTurnButOff();
    void showRotLeftButOn();
    void showRotLeftButOff();
    void showRotRightButOn();
    void showRotRightButOff();
    bool isRotatorLoaded();

    void traceMsg(QString msg);
    void setCwCcW_Items_Visible(bool visible);




    void initConnections();
    void clearBearingLineEdit();
    void clearRotatorIndicators();

    RotFrameData rotFrameData;

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:

    LoggerContestLog *ct = nullptr;






    void clearRotatorFlags();



    void activateSignalConnections();
    void deactivateSignalConnections();
signals:
    void selectRotator(QString);
    void sendRotator(rpcConstants::RotateDirection direction, int angle );

    void turnBearingReturn();
    void bearingEditReturn();

    void rotatorConnected(bool);




private slots:

    void onStopRotate_clicked();
    void onRotate_clicked();
    void onRotateLeft_clicked();
    void onRotateRight_clicked();


    void onNudgeLeft_clicked();
    void onNudgeRight_clicked();
    void onAntennaNameSel_activated(int arg1);


    void setBrgFromQSOLog(QString);
    void setBrgFromSpot(QString brg);
    void setBrgFromFrmMemory(QString Brg);
    void setBrgFromMatchFrame(QString brg);


};





#endif // ROTCONTROLFRAMEBASE_H
