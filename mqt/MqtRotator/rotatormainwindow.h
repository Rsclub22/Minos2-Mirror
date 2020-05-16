/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
// Hamlib Library
//
/////////////////////////////////////////////////////////////////////////////



#ifndef ROTATORMAINWINDOW_H
#define ROTATORMAINWINDOW_H

#include "base_pch.h"
#include "rotatorRpc.h"
#include "rotatorbase.h"
#include "rotatorfactory.h"
#include "rotatorcommon.h"
#include "presetbutton.h"
#include "rotpresetdialog.h"
#include <QMainWindow>
#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QKeyEvent>
#include <QPushButton>
#include <QShortcut>




#define NUM_PRESETS 10
#define OFF false
#define ON true

const int POLLTIME = 500;

class QLabel;
class QComboBox;
class RotSetupDialog;
class MinosCompass;
class HamlibRotControl;
class EditPresetsDialog;
class RotatorRpc;
class RotatorLog;
class LogDialog;


namespace Ui {
class RotatorMainWindow;
}

void delay(int sec);
void sleepFor(qint64 milliseconds);


class RotatorMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RotatorMainWindow(QWidget *parent = nullptr);
    ~RotatorMainWindow();


    void setPolltime(int interval);
    int getPolltime();

    // setup connections
    void initActionsConnections();

    const QString version = "2.20";

signals:

    void escapePressed();
    void rotate_cwKeyPressed();
    void rotate_ccwKeyPressed();
    void rotateStopKeyPressed();
    void rotateTurnKeyPressed();
    void rotateFunctionKeyPressed(int);

    void sendBearing(QString);
    void sendCompassDial(int);
    void displayOverlap(overlapStat);
    void checkingEndStop();
    void sendBackBearing(QString);
//   void displayOverlapBearing(QString);
    void displayActualBearing(QString);
    void presetRotateTo();



private:
    Ui::RotatorMainWindow *ui;

    StdInReader stdinReader;

    bool closeApp = false;
    RotatorRpc *msg;

    RotatorFactory* rotFactory;
    RotatorBase* rotator;

    QTimer LogTimer;
    QTimer RotateTimer;

    QList<PresetButton *> presetButton;
    QVector<RotPresetData*> rotPresets;
    QList<QShortcut *> shortCutKeyList;
    QList<QShortcut *> shiftShortCutKeyList;

    QComboBox *selectAntenna;
    //QPushButton* presetButtons[NUM_PRESETS];
    QString appName = "";
    QLabel *status;
    QLabel *offSetlbl;
    QLabel *offSetDisplay;
    QLabel *actualRotatorlbl;
    QLabel *actualRotatorDisplay;
    QLabel *rawRotatorlbl;
    QLabel *rawRotatorDisplay;
    //QPalette *redText;
    //QPalette *blackText;
    RotSetupDialog *setupAntenna;
    EditPresetsDialog *editPresets;
    LogDialog *setupLog;
    RotatorLog *rotlog;
    bool rotLogFlg = true;
    QTimer *pollTimer;
    int pollTime;
    int rotTimeCount;
    int brakedelay;
    bool rot_left_button_status = OFF;
    bool rot_right_button_status = OFF;
    bool turn_button_status = OFF;
    bool brakeflag = false;
    bool moving = false;
    bool movingCW = false;
    bool movingCCW = false;
    bool cwCcwCmdflag = false;     // command sentflag
    bool stopCmdflag = false;
    bool rotCmdflag = false;
    bool reqBearCmdflag = false;

    bool traceCommsFlag =  false;

    //endStop endStopType;
    overlapStat overLapStatus = NO_OVERLAP;
    bool overLapActiveflag = true;
    //bool southStopActiveflag;
    bool rotErrorFlag = false;
    //bool supportCwCcwCmd;
    int rotatorBearing;
    int curBearingWithOffset;
    //int rotatorMinAzimuth;
    //int rotatorMaxAzimuth;
    //int currentMinAzimuth;
    //int currentMaxAzimuth;
    //int rotatorCWEndStop;
    //int rotatorCCWEndStop;
    QString backBearingmsg;

    //QString presetName[NUM_PRESETS];
    //QString presetBearing[NUM_PRESETS];

    QString geoStr;         // geometry registry location

    QString connectStat;
    QString statusMsg;
    QString activeMsg;



    int openRotator();
    void closeRotator();

    void refreshPresetLabels();
    void showStatusMessage(const QString &);
    void sendAntennaListLogger();
    void sendStatusLogger();
    void readPresets();
    void savePresets();

    void initSelectAntennaBox();

    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *);

    void initPresetButtons();
    void saveRotPresetButton(RotPresetData &editData);
    void setRotPresetButData(int buttonNumber, RotPresetData &editData);
    void rotPresetButtonUpdate(int buttonNumber, RotPresetData &editData);



    void rotatorError(int errorCode, QString cmd);

    void rotatorActive();


public slots:
    void rotateToController();
    void displayBearing(int);
    void compassClicked(int);
    void updatePresetLabels();
    void clickedPreset(int buttonNumber);
    void logMessage( QString s );


private slots:
    void LogTimerTimer( );

    void onStdInRead(QString);

    void onLoggerSelectAntenna(PubSubName);
    void onLoggerSetRotation(int direction, int angle);
    void onLoggerSetPreset(QString);
    void checkEndStop();
    void checkMoving(int bearing);

    void rotatingTimer();
    void about();
    void saveTraceLogFlag(bool);
    void overLapDisplayBox(overlapStat status);
    void currentAntennaSettingChanged(QString);
    void updateSelectAntennaBox();
    void setSelectAntennaBoxVisible(bool visible);
    void setAntennaNameLabelVisible(bool visible);

    void upDateAntenna();
    void refreshAntenna();
    void request_bearing();
    void rotateCW(bool);
    void rotateCCW(bool);
    void stopButton();
    void stop_rotation();
    void stopRotation(bool);
    void sendStatusToLogConnected();
    void sendStatusToLogRotCCW();
    void sendStatusToLogRotCW();
    void sendStatusToLogStop();
    void sendStatusToLogTurn();
    void sendStatusToLogDisConnected();
    void sendStatusToLogError();


    void aboutRotatorConfig();

    void onSelectAntennaBox();
    void onLaunchSetup();
    void presetRead(int num);
    void presetWrite(int num);
    void presetEdit(int num);
    void presetClear(int num);



    void showPresetMenu(int buttonNumber);
    void onSentCommandError(int errorCode, QString cmd);
    void onTestBearingEnter();
private:
    void rotateTo(int bearing);
    int northCalcTarget(int targetBearing);




    void toggleOverLapDisplay(bool toggle);

//    bool getCwCcwCmdFlag(int rotatorNumber);
    void rot_right_button_off();
    void rot_right_button_on();
    void rot_left_button_off();
    void rot_left_button_on();
    void turn_button_on();
    void turn_button_off();
    void stop_button_on();
    void stop_button_off();
    void dispRawRotBearing(int);
    int calcRotZero360(int targetBearing);
    int calcRotNeg180_180(int targetBearing);
    int calclRot_0_450_Neg180_540(int targetBearing);
    void dumpRotatorToTraceLog();
    void writeWindowTitle(QString appName);
    void readTraceLogFlag();

    void sendPresetListLogger();
    void cwCCWControlVisible(bool visible);
    void savePreset(RotPresetData &editData);

    void checkTestBearingBox();
    void setCompassDialVisible(bool visible);
};

#endif // ROTATORMAINWINDOW_H
