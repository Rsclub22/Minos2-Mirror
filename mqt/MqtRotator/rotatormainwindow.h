/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
// Hamlib Library
//
/////////////////////////////////////////////////////////////////////////////



#ifndef ROTATORMAINWINDOW_H
#define ROTATORMAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QKeyEvent>
#include <QPushButton>
#include <QShortcut>
#include <QTimer>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include "CommandReader.h"
#include "qlineedit.h"
#include "qspinbox.h"
#include "rotatorRpc.h"
#include "rotatorbase.h"
#include "rotatorfactory.h"
#include "rotatorcommon.h"
#include "presetbutton.h"
#include "skyscancontrol.h"
#include "toolbuttonupdown.h"

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


class CustomSpinBox : public QSpinBox {
public:
    using QSpinBox::QSpinBox; // Inherit constructors

    void setReadOnlyLineEdit(bool readOnly) {
        lineEdit()->setReadOnly(readOnly);
    }
};


class RotatorMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RotatorMainWindow(QWidget *parent = nullptr);
    ~RotatorMainWindow();

    // setup connections
    void initActionsConnections();

    const QString version = "2.20";

    int northCalcTarget(int targetBearing, int currentRotatorBearing);
    int calcRotZero360(int targetBearing);
    int calcRotNeg180_180(int targetBearing);
    int calclRot_0_450_Neg180_540(int targetBearing, int currentRotatorBearing);


    void setSkyScanCCWIndicatorOnOff(bool state);
    void setSkyScanCWIndicatorOnOff(bool state);
    void sendSkyScanEnabledToLogger(bool state);
    void sendSkyScanStartBearingAndEndBearingToLogger(int startBearing, int endBearing);
    void sendSkyScanNextStepToLogger(QString nextStepBearing);
    void sendSkyScanCountDownToLogger(QString countDown);
    void sendSkyScanButtonStateToLogger(int state);
    void sendSkyScanReverseScanToLogger(bool state);

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
    void displayActualBearing(QString);
    void presetRotateTo();

    void sendRotationStatusToSkyScan(int bearing, skyScanBearingStates brgState);



private:
    Ui::RotatorMainWindow *ui;

    QSharedPointer<CommandReader> commandReader = QSharedPointer<CommandReader>(new CommandReader(this));

    bool closeApp = false;
    RotatorRpc *msg;

    RotatorFactory* rotFactory;
    RotatorBase* rotator;

    bool testMode = false;
    QString liveAntenna;

    QTimer LogTimer;
    QTimer RotateTimer;

    QList<PresetButton *> presetButton;
    QVector<RotPresetData*> rotPresets;
    QList<QShortcut *> shortCutKeyList;
    QList<QShortcut *> shiftShortCutKeyList;

    QString appName = "";
    QLabel *status;
    QLabel *offSetlbl;
    QLabel *offSetDisplay;
    QLabel *actualRotatorlbl;
    QLabel *actualRotatorDisplay;
    QLabel *rawRotatorlbl;
    QLabel *rawRotatorDisplay;
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
    int targetBearing = 0;
    bool movingCW = false;
    bool movingCCW = false;
    bool cwCcwCmdflag = false;     // command sentflag
    bool stopCmdflag = false;
    bool rotCmdflag = false;
    bool reqBearCmdflag = false;
    bool supportStopCommandFlag = false;

    bool traceCommsFlag =  false;

    overlapStat overLapStatus = NO_OVERLAP;
    bool overLapActiveflag = true;
    bool rotErrorFlag = false;
    int rotatorBearing;
    int curBearingWithOffset;
    QString backBearingmsg;

    QString geoStr;         // geometry registry location

    QString connectStat;
    QString statusMsg;
    QString activeMsg;

    bool hamlibOk = false;

    QSharedPointer<SkyScanControl> skyScanControl;
    bool skyScanEnabled = false;
    bool skyScanActive = false;
    bool saveSkyScanOnClose = false;
    int startSkyScanBrg = 0;
    int endSkyScanBrg = 0;
    int skyScanStepDegrees = 0;

    int skyScanPauseMins = 0;

    int skyScanMinAzimuth = COMPASS_MIN0;
    int skyScanMaxAzimuth = COMPASS_MAX360;


    // skyScan Step Degrees Spinbox

    int minSkyScanStepDegrees = MIN_SKYSCAN_STEP_DEGREES;
    int maxSkyScanStepDegrees = MAX_SKYSCAN_STEP_DEGREES;
    int stepDegreesIncrement = SKYSCAN_STEP_DEGREES;

    // skyScan Start Bearing Spinbox

    int minSkyScanStartBearing = DEFAULT_VALUE_READ_INI;
    int maxSkyScanStartBearing = DEFAULT_VALUE_READ_INI;

    // skyScan Pause Mins Spinbox

    int minSkyScanPauseMins = MIN_SKYSCAN_PAUSE_MINS;
    int maxSkyScanPauseMins = MAX_SKYSCAN_PAUSE_MINS;
    int skyScanPauseStepIncrement = DEFAULT_VALUE_READ_INI;

    int openRotator();
    void closeRotator();

    void refreshPresetLabels();
    void showStatusMessage(const QString &, const QString &tt = QString());
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
    void saveSkyScanEnableSetting(QString currentAntennaName);


public slots:
    void rotateToController();
    void displayBearing(int);
    void compassClicked(int);
    void updatePresetLabels();
    void clickedPreset(int buttonNumber);
    void logMessage( QString s );


private slots:
    void LogTimerTimer( );

    void onCommandRead(QString);

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

    void onSelectAntennaBox(int);
    void onLaunchSetup();
    void presetRead(int num);
    void presetWrite(int num);
    void presetEdit(int num);
    void presetClear(int num);



    void showPresetMenu(int buttonNumber);
    void onSentCommandError(int errorCode, QString cmd);
    void onTestBearingEnter();
    void onPSTRotatorConfig();
    void on_reconnectPushButton_clicked();

    void on_testButton_clicked();

    void on_setupAntennas_clicked();

    void on_aboutRC_clicked();

    void on_logHeadings_clicked();

    void on_PSTConfig_clicked();

    void on_traceDataComms_stateChanged(int arg1);

    void on_rotSplitter_splitterMoved(int pos, int index);

    void on_antSplitter_splitterMoved(int pos, int index);

    void on_rotTabs_currentChanged(int index);

    void skyScanStopPbPressed();
    void skyScanStartPbPressed();


    void skyScanSettingsOnCloseChkBoxChanged();
    void skyScanEnableChkBoxChanged();

    void skyScanPausePbPressed();

    void skyScanRotateTo(int bearing);
    void displaySkyScanNextStepBearing(int bearing);
    void displaySkyScanPauseIntervalCount(int count);

    void skyScanStepDegreesSpinBoxValueChanged(int value);
    void skyScanPauseTimeSpinBoxValueChanged(int value);



    void skyScanStartBearingToolbuttonValueChanged(int value);
    void skyScanEndBearingToolbuttonValueChanged(int value);
private:
    void rotateTo(int bearing);
    //int northCalcTarget(int targetBearing);




    void toggleOverLapDisplay(bool toggle);

    void rot_right_button_off();
    void rot_right_button_on();
    void rot_left_button_off();
    void rot_left_button_on();
    void turn_button_on();
    void turn_button_off();
    void stop_button_on();
    void stop_button_off();
    void dispRawRotBearing(int);
    //int calcRotZero360(int targetBearing);
    //int calcRotNeg180_180(int targetBearing);
    //int calclRot_0_450_Neg180_540(int targetBearing);
    void dumpRotatorToTraceLog();
    void writeWindowTitle(QString appName);
    void readTraceLogFlag();

    void sendPresetListLogger();
    void cwCCWControlVisible(bool visible);
    void savePreset(RotPresetData &editData);

    void checkTestBearingBox();
    void setCompassDialVisible(bool visible);
    void setTestMode(bool test);

    void setSkyScanEnableChkBoxEnabled(bool enabled);
    void saveSkyScanSettings(QString currentAntennaName);
    void readSkyScanSettings(QString currentAntennaName);
    void setSkyScanComponentsEnabled(bool enabled);
    void closeSkyScan(QString currentAntennaName);
    void openSkyScan(QString currentAntennaName);
    void setSkyScanEndBrgLineEditValidator();
    void setSkyScanStartBrgLineEditValidator();
    bool readSkyScanEnableSetting(QString currentAntennaName);
    void readSkyScanCommonSettings();
    void dumpSkyScanSettingsToTraceLog();
    void setSkyScanGroupBoxEnabled(bool enabled);
    bool isSkyScanEnabled();
    void setTargetTabEnabled(bool enabled);
    void setPresetsGroupBoxEnabled(bool enabled);
    void setSkyScanSpinBoxesEnabled(bool enabled);
    void setSkyScanToolButtonUpDownEnabled(bool enabled);
    void stopSkyScan();
    void setSkyScanStartButtonColour(QString style);
    void setSkyScanStopButtonColour(QString style);



    void initialiseSpinBox(CustomSpinBox *spinBox, int min, int max, int interval, int initialValue, bool readOnly);
    void saveSkyScanCommonSettings();
    void initialiseSkyScannerSpinners();

    void initialiseToolButtonUpDown(ToolButtonUpDown *toolButtonUpDown, int min, int max, int interval, int initialValue);
    void setSkyScanStartBearingToolButtonUpDown();
    void setSkyScanEndBearingToolButtonUpDown();
    void skyScanDisplayRotatorMinAzMaxAz(int minAz, int maxAz);

};

#endif // ROTATORMAINWINDOW_H
