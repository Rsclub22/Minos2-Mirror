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

    LoggerContestLog *ct = nullptr;
public:
    explicit RotControlFrameBase(QWidget *parent);
    ~RotControlFrameBase();





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


protected:

    virtual QWidget* getCompassObject() = 0;
    virtual QGroupBox* getRotatorControlGroupBoxObject() = 0;
    virtual QComboBox* getAntennaSelectObject() = 0;
    virtual QLabel* getRotConnectStateLabelObject() = 0;
    virtual QLabel* getRotatorStatMsgLabelObject() = 0;
    virtual BearingLineEdit* getBrgLineEditObject() = 0;
    virtual QToolButton* getRotateButtonObject() = 0;
    virtual QToolButton* getStopRotateObject() = 0;
    virtual QToolButton* getRotateLeftObject() = 0;
    virtual QToolButton* getRotateRightObject() = 0;
    virtual QToolButton* getnudgeLeftObject() = 0;
    virtual QToolButton* getnudgeRightObject() = 0;
    virtual QLabel* getRotBrgDisplayObject() = 0;
    virtual QShortcut* getNudgeRightShortCutObject() = 0;
    virtual QShortcut* getNudgeLeftShortCutObject() = 0;
    virtual QShortcut* getRotateLeftShortCutObject() = 0;
    virtual QShortcut* getRotateRightShortCutObject() = 0;
    virtual QShortcut* getTurnToBearingShortCutObject() = 0;
    virtual QShortcut* getRotateStopShortCutObject() = 0;

    virtual void keyPressEvent(QKeyEvent *event) override;



    void setContest(BaseContestLog *c);
    int getAngle(QString brgSt);
    int getCurrentBearing();
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
    void setCwCcwCmdEnable(bool s);

    void setSupportStopCommandFlag(bool state);
    void setRotatorBearing(const QString &s);
    void initConnections();
    void clearBearingLineEdit();
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
    QString frameName;



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





#endif // ROTCONTROLFRAMEBASE_H
