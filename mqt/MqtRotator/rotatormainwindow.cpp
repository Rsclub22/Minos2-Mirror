/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
// Hamlib Library
//
/////////////////////////////////////////////////////////////////////////////

#include <QString>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QTime>
#include <QSettings>
#include <QProcessEnvironment>
#include <QDir>

#include "regsettings.h"
#include "RPCCommandConstants.h"
#include "rotatorRpc.h"
#include "rotatorlog.h"
#include "rotatorcommon.h"
#include "rotpresetdialog.h"
#include "serialCommonData.h"
#include "minoscompass.h"
#include "rotsetupdialog.h"
#include "logdialog.h"
#include "serialdata.h"
#include "pstconfigdialog.h"
#include "MTrace.h"
#include "LogEvents.h"
#include "rotatormainwindow.h"
#include "checkHamlibVersionIsValid.h"
#include "ui_rotatormainwindow.h"

RotatorMainWindow::RotatorMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RotatorMainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    serialData::translateSerialData();

    for (int i = 0; i < presetShortCutKeys.count(); i++)
    {
        shortCutKeyList.append(new QShortcut(QKeySequence(presetShortCutKeys[i]), this));
    }

    for (int i = 0; i < presetMenuShortCutKeys.count(); i++)
    {
        shiftShortCutKeyList.append(new QShortcut(QKeySequence(presetMenuShortCutKeys[i]), this));
    }

    connect(stdinReader, &StdInReader::stdinLine, this, &RotatorMainWindow::onStdInRead);

    // get the antenna name from host process
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    writeWindowTitle(appName);


    QDir dir(".");
    trace(QString("Directory %1").arg(dir.absolutePath()));

    createCloseEvent();
    connect(&LogTimer, &QTimer::timeout, this, &RotatorMainWindow::LogTimerTimer);
    LogTimer.start(100);
    msg = new RotatorRpc(this);

    RegSettings settings;
    geoStr = "geometry";
    geoStr = geoStr + appName;

    QByteArray geometry = settings.getSettings().value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    ui->rot_left_button->setShortcut(QKeySequence(ROTATE_CCW_KEY));
    ui->rot_right_button->setShortcut(QKeySequence(ROTATE_CW_KEY));
    ui->turnButton->setShortcut(QKeySequence(ROTATE_TURN_KEY));
    ui->stopButton->setShortcut(QKeySequence(ROTATE_STOP_KEY));

    //redText = new QPalette();
    //blackText = new QPalette();
    //redText->setColor(QPalette::ButtonText, Qt::red);
    //blackText->setColor(QPalette::ButtonText, Qt::black);

    // disable some menus for now
//    ui->actionHelp->setVisible(false);
//    ui->actionSkyScan->setVisible(false);
//    ui->actionAlways_On_Top->setVisible(false);

    initPresetButtons();


    rotator = nullptr;
    readTraceLogFlag();
    trace(QString("Starting rotator factory and adding rotators to the list"));
    rotFactory = new RotatorFactory(traceCommsFlag, this);

    //rotator->getRotatorList();
    //setupAntenna = new RotSetupDialog(rotator);
    setupAntenna = new RotSetupDialog(rotFactory);
    setupLog = new LogDialog;
    pollTimer = new QTimer(this);

    selectAntenna = new QComboBox;
    rotlog = new RotatorLog;

    status = new QLabel;
    offSetlbl = new QLabel;
    offSetDisplay = new QLabel;
    actualRotatorlbl = new QLabel;
    actualRotatorDisplay = new QLabel;
    rawRotatorlbl = new QLabel;
    rawRotatorDisplay = new QLabel;


    ui->statusbar->addWidget(status);

    ui->statusbar->addPermanentWidget(offSetlbl);
    offSetlbl->setText(tr("Offset: "));
    ui->statusbar->addPermanentWidget(offSetDisplay);

    ui->statusbar->addPermanentWidget(actualRotatorlbl);
    actualRotatorlbl->setText(tr("Actual: "));
    ui->statusbar->addPermanentWidget(actualRotatorDisplay);

    ui->statusbar->addPermanentWidget(rawRotatorlbl);
    rawRotatorlbl->setText(tr("RawRot: "));
    ui->statusbar->addPermanentWidget(rawRotatorDisplay);

//    ui->overlaplineEdit->setFixedSize(60,20);
    ui->antNameDisp->setText("");

    rot_left_button_off();
    rot_right_button_off();



    refreshPresetLabels();
    initActionsConnections();

    checkTestBearingBox();

    selectAntenna = ui->selectAntennaBox;

    setupAntenna->setAppName(appName);

    brakedelay = 1 * 1000;

    rotatorBearing = COMPASS_ERROR; // force first update

    rotTimeCount = 0;
    RotateTimer.start(200);  // to set timeout for antenna rotating

    rotlog->getBearingLogConfig();

    hamlibOk = false;

    logMessage(QString("Checking installed hamlib version"));
    int hamlibCheckErrorNum = checkHamlibVersionIsValid(hamlibOk, hamlib_version, MINIMUM_HAMLIB_VERSION);

    if (hamlibCheckErrorNum < 0)
    {

        if (hamlibCheckErrorNum == -2 )
        {
            QMessageBox::critical(nullptr, tr("Rotator Hamlib Library Version Error!"), tr("Installed hamlib version %1 is imcompatible.\nIt should be version %2 or greater.\n\nPlease check your installation.\nYou will not be able to select a radio until this is rectified!").arg(hamlib_version).arg(MINIMUM_HAMLIB_VERSION), QMessageBox::Ok);
            logMessage(QString("Error version number conversion to int failed - installed version = %1, minimum version = %2").arg(hamlib_version).arg(MINIMUM_HAMLIB_VERSION)); // error)
        }
        else if (hamlibCheckErrorNum == -1 )
        {
            // we should not get here....
            QMessageBox::critical(nullptr, tr("Hambib Version test Conversion Error!"), tr("Hamlib Version test conversion error. Please report error"));
            logMessage(QString("Error version number conversion to int failed - installed version = %1, minimum version = %2").arg(hamlib_version).arg(MINIMUM_HAMLIB_VERSION));
        }
        else if (hamlibCheckErrorNum == -3)
        {
            //we should not get here...
            logMessage(QString("Error: checkhamlibVersion faile error code = %1").arg(hamlibCheckErrorNum));
        }


    }

    if (hamlibOk)
    {
        initSelectAntennaBox();
    }
    else
    {
        showStatusMessage(tr("Error: Installed hamlib version %1 is incorrect, should be version %2 or greater").arg(hamlib_version).arg(MINIMUM_HAMLIB_VERSION));
    }



    setTestMode(appName.isEmpty());

    trace("*** Rotator Started ***");
}

RotatorMainWindow::~RotatorMainWindow()
{

    delete msg;
    delete ui;
}
void RotatorMainWindow::setTestMode(bool test)
{
    trace(QString("testMode is %1 test parameter is %2").arg(testMode).arg(test));
    if (test)
    {
        if (!testMode)
        {
            testMode = true;
            liveAntenna = setupAntenna->currentAntennaName;
            trace("save liveAntenna " + liveAntenna);
        }
        logMessage((QString("Read Current Antenna for Local selection")));
        ui->testButton->setText(tr("Set Antenna from Logger"));
    }

    else
    {
        if (testMode)
        {
            trace("restore liveAntenna " + liveAntenna);;
            testMode = false;

            ui->selectAntennaBox->setCurrentText(liveAntenna);
            setupAntenna->currentAntennaName = liveAntenna;
            setupAntenna->saveCurrentAntenna();
            msg->rotatorCache.invalidate();
        }
        ui->testButton->setText(tr("Test Antenna"));
        logMessage((QString("Antenna Selection for Current Antenna, for AppName %1, will be from logger").arg(appName)));

    }
    upDateAntenna();
    setSelectAntennaBoxVisible(testMode);
    setAntennaNameLabelVisible(!testMode);
}
void RotatorMainWindow::on_testButton_clicked()
{
    setTestMode(!testMode);
}


void RotatorMainWindow::logMessage( QString s )
{
           trace( s );
}

void RotatorMainWindow::onStdInRead(QString cmd)
{
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        trace("onStdInRead - Start shutdown");
        close();

        closeApp = true;
    }
}

void RotatorMainWindow::closeEvent(QCloseEvent *event)
{

    LogTimer.stop();
    closeRotator();

    // and tidy up all loose ends

    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());
    trace("MinosRotator Closing");
    QWidget::closeEvent(event);

}



void RotatorMainWindow::resizeEvent(QResizeEvent * event)
{

    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());

    QWidget::resizeEvent(event);
}



void RotatorMainWindow::LogTimerTimer(  )
{
    static bool closed = false;
    if ( !closed )
    {
        if ( checkCloseEvent() )
        {
            closed = true;
            close();
        }
    }
}


void RotatorMainWindow::onSelectAntennaBox(int)
{
    setupAntenna->currentAntennaName = ui->selectAntennaBox->currentText();
    setupAntenna->saveCurrentAntenna();
    upDateAntenna();
}


void RotatorMainWindow::onLoggerSelectAntenna(PubSubName s)
{
    if (closeApp)
        return;

    QString oldAntenna = setupAntenna->currentAntennaName;
    ui->selectAntennaBox->setCurrentText(s.key());
    setupAntenna->currentAntennaName = s.key();
    setupAntenna->saveCurrentAntenna();

    logMessage(QString("Logger Selected Antenna - %1").arg(setupAntenna->currentAntennaName));
    dumpRotatorToTraceLog();

    if (!s.isEmpty() && s.key() == oldAntenna)
    {
        refreshAntenna();
    }
    else
    {
        upDateAntenna();
    }
    msg->rotatorCache.invalidate();
}

void RotatorMainWindow::setSelectAntennaBoxVisible(bool visible)
{


    ui->antennaSelectlbl->setVisible(visible);
    ui->selectAntennaBox->setVisible(visible);



}

void RotatorMainWindow::setAntennaNameLabelVisible(bool visible)
{
    ui->antNameDispLbl->setVisible(visible);
    ui->antNameDisp->setVisible(visible);

}

void RotatorMainWindow::setCompassDialVisible(bool visible)
{
    ui->compassDial->setVisible(visible);
}


void RotatorMainWindow::onLoggerSetRotation(int direction, int angle)
{
    if (closeApp || rotator == nullptr)
        return;

    logMessage("Command From Logger command number = " + QString::number(direction) + ", angle = " + QString::number(angle));
    int dirCommand = direction;
    if (rotator->getRotConnected() && !rotErrorFlag)
    {
        if (dirCommand == rpcConstants::eRotateDirect)
        {
            ui->bearingEdit->setText(QString::number(angle));
            rotateTo(angle);
            sendStatusToLogTurn();
        }
        else if (dirCommand == rpcConstants::eRotateLeft)
        {
            rotateCCW(true);


        }
        else if (dirCommand == rpcConstants::eRotateRight)
        {
            rotateCW(true);


        }
        else if (dirCommand == rpcConstants::eRotateStop)
        {

            stopRotation(true);

        }
     }
    else
    {
        logMessage("Logger command discarded - no rotator connected or error");
    }


}

void RotatorMainWindow::onLoggerSetPreset(QString presetMsg)
{
    if (closeApp)
        return;


    QStringList msg = presetMsg.split(':');
    if (msg.count() != 3)
    {
        logMessage(QString("Preset from Logger - Message incomplete - %1").arg(presetMsg));
        return;
    }
    else
    {
        logMessage(QString("Preset from Logger - Save - Button Number %1, Name %2, Bearing %3").arg(msg[0], msg[1], msg[2]));
        int butNum = msg[0].toInt();
        RotPresetData d = RotPresetData(butNum, msg[1], msg[2]);
        setRotPresetButData(butNum, d);
        rotPresetButtonUpdate(butNum, d);
    }


}



int RotatorMainWindow::openRotator()
{

    int retCode = 0;

    if (setupAntenna->currentAntenna.antennaName == "")
    {
        logMessage(QString("Open Rotator: No rotator name!"));
        showStatusMessage(tr("Please select an Antenna"));
        return  OPEN_FAILED;
    }
    if (setupAntenna->currentAntenna.portType == RotCapConstants::PortType::serial)
    {
        if(setupAntenna->comportAvial(setupAntenna->currentAntenna.comport) == -1)
        {
            logMessage(QString("Open Rotator: Check comport - defined port %1 not available on computer").arg(setupAntenna->currentAntenna.comport));
            showStatusMessage(tr("Comport %1 no longer configured on computer?").arg(setupAntenna->currentAntenna.comport));
            return OPEN_FAILED;
        }

        if (setupAntenna->currentAntenna.comport == "")
        {
            logMessage(QString("Open Rotator: No comport"));
            showStatusMessage(tr("Please select a Comport"));
            return OPEN_FAILED;
        }

    }
    if (setupAntenna->currentAntenna.portType == RotCapConstants::PortType::network )
    {
        if (setupAntenna->currentAntenna.networkAdd == "" || (setupAntenna->currentAntenna.networkPort == ""))
        {
            logMessage(QString("Open Rotator: No network or Port Number"));
            showStatusMessage(tr("Please enter a network Address and Port Number"));
            return OPEN_FAILED;
        }

    }
    if (setupAntenna->currentAntenna.rotatorModel == "")
    {
        logMessage(QString("Open Rotator: No rotator model"));
        showStatusMessage(tr("Please select a rotator model"));
        return OPEN_FAILED;
    }


    rotator = rotFactory->createRotator(rotFactory->supported_rotators()->value(setupAntenna->currentAntenna.rotatorModelName).RotCapabilities::modelNumber);

    if (rotator == nullptr)
    {
        logMessage(QString("Failed to create rotator from factory"));
        return OPEN_FAILED;
    }

    dumpRotatorToTraceLog();

    connect(rotator, &RotatorBase::bearing_updated, this, &RotatorMainWindow::displayBearing);
    connect(rotator, &RotatorBase::traceCommsMsg, this, &RotatorMainWindow::logMessage);
    connect(rotator, &RotatorBase::bearing_updated, this, &RotatorMainWindow::checkMoving);
    connect(rotator, &RotatorBase::bearing_updated, rotlog, &RotatorLog::saveBearingLog);

    if (rotator->isPstRotator())
    {
        connect(rotator, &RotatorBase::sentCommandError, this, &RotatorMainWindow::onSentCommandError, Qt::QueuedConnection);
    }

    rotator->setTraceComms(traceCommsFlag);

    retCode = rotator->rotInit(setupAntenna->currentAntenna);
    if (retCode < 0)
    {
            return retCode;
    }


    if (rotator->getRotConnected())
    {
        // get poll interval timer
       if (setupAntenna->currentAntenna.pollInterval == "0.5")
       {
           pollTime = 500;
       }
       else
       {
           pollTime = 1000 * setupAntenna->currentAntenna.pollInterval.toInt();
       }

        pollTimer->start(pollTime);             // start timer to send message to controller
        if (setupAntenna->currentAntenna.portType == RotCapConstants::PortType::serial)
        {
            showStatusMessage(tr("Connected to: %1 - %2, %3, %4, %5, %6, %7,  Handshake %8, ForceDTR %9, ForceRTS %10")
                                  .arg(setupAntenna->currentAntenna.antennaName).arg(setupAntenna->currentAntenna.rotatorModel).arg(setupAntenna->currentAntenna.comport).arg(setupAntenna->currentAntenna.baudrate).arg(setupAntenna->currentAntenna.databits)
                                  .arg(setupAntenna->currentAntenna.stopbits).arg(serialCommonData::parityStr[setupAntenna->currentAntenna.parity]).arg(serialCommonData::handshakeStr[setupAntenna->currentAntenna.handshake]).arg(serialCommonData::forceLinesStr[setupAntenna->currentAntenna.forceDtr]).arg(serialCommonData::forceLinesStr[setupAntenna->currentAntenna.forceRts]));
        }
        else if (setupAntenna->currentAntenna.portType == RotCapConstants::PortType::network )
        {
            showStatusMessage(tr("Connected to: %1 - %2, %3").arg(setupAntenna->currentAntenna.antennaName, setupAntenna->currentAntenna.rotatorModel, setupAntenna->currentAntenna.networkAdd + ":" + setupAntenna->currentAntenna.networkPort));
        }
        else if (setupAntenna->currentAntenna.portType == RotCapConstants::PortType::none)
        {
                showStatusMessage(tr("Connected to: %1 - %2").arg(setupAntenna->currentAntenna.antennaName, setupAntenna->currentAntenna.rotatorModel));
        }

        //sendStatusToLogConnected();
    }
    else
    {
        //QMessageBox::critical(this, tr("Error"), serial->errorString());
        pollTimer->stop();

        stopRotation(false);           // clear flags
        showStatusMessage(tr("Rotator Open error"));
        sendStatusToLogDisConnected();
    }

    return OPEN_OK;

}

void RotatorMainWindow::closeRotator()
{
    if (moving || movingCW || movingCCW)
    {
        stop_rotation();
    }

    pollTimer->stop();

    if (rotator)
    {
        disconnect(rotator, &RotatorBase::bearing_updated, this, &RotatorMainWindow::displayBearing);
        disconnect(rotator, &RotatorBase::traceCommsMsg, this, &RotatorMainWindow::logMessage);
        disconnect(rotator, &RotatorBase::bearing_updated, this, &RotatorMainWindow::checkMoving);
        disconnect(rotator, &RotatorBase::bearing_updated, rotlog, &RotatorLog::saveBearingLog);

        if (rotator->isPstRotator())
        {
            disconnect(rotator, &RotatorBase::sentCommandError, this, &RotatorMainWindow::onSentCommandError);

        }

        if (rotator->getRotConnected())
        {
            rotator->closeRotator();

        }


        delete rotator;
        rotator = nullptr;
    }

    showStatusMessage(tr("Disconnected"));
    sendStatusToLogDisConnected();
    logMessage("Rotator Closed");
}






void RotatorMainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}


void RotatorMainWindow::sendStatusLogger( )
{
   QString message = connectStat + ':' + statusMsg;
   logMessage(QString("Send %1 message to logger, appName = %2").arg(message, appName));

   PubSubName psname(setupAntenna->currentAntennaName);
   msg->rotatorCache.setStatus(psname, message);
   msg->rotatorCache.publish();
}

void RotatorMainWindow::sendAntennaListLogger()
{
    QStringList ants;
    for (int i= 0; i < setupAntenna->numAvailAntennas; i++)
    {
        if (!setupAntenna->availAntData[i]->antennaName.isEmpty())
        {
            PubSubName r(setupAntenna->availAntData[i]->antennaName);
            ants.append(r.toString());
        }
    }
    msg->publishAntennaList(ants.join(":"));
}

void RotatorMainWindow::sendPresetListLogger()
{
    QStringList presets;
    for (int i=0; i < rotPresets.count(); i++)
    {
        if (!rotPresets.isEmpty())
        {
            QStringList preset;
            preset.append(QString::number(rotPresets[i]->number));
            preset.append(rotPresets[i]->name);
            preset.append(rotPresets[i]->bearing);
            presets.append(preset.join(','));

        }
    }

    PubSubName psname(setupAntenna->currentAntennaName);
    msg->rotatorCache.setRotatorPresets(psname, presets.join(':'));
    msg->rotatorCache.publish();
}
void RotatorMainWindow::initActionsConnections()
{

    connect(ui->selectAntennaBox,QOverload<int>::of(&QComboBox::activated), this, &RotatorMainWindow::onSelectAntennaBox);
    connect(setupAntenna, &RotSetupDialog::antennaNameChange, this, &RotatorMainWindow::updateSelectAntennaBox);
    connect(ui->turnButton, &QPushButton::clicked, this, &RotatorMainWindow::rotateToController);
    connect(ui->bearingEdit, &BearingLineEdit::returnPressed, this, &RotatorMainWindow::rotateToController);
    connect(this, &RotatorMainWindow::presetRotateTo, this, &RotatorMainWindow::rotateToController);
    connect(this, &RotatorMainWindow::presetRotateTo, ui->bearingEdit, &BearingLineEdit::selectAll);
    connect(this, &RotatorMainWindow::presetRotateTo, ui->bearingEdit, QOverload<>::of(&BearingLineEdit::setFocus));
    connect(ui->turnButton, &QPushButton::clicked, ui->bearingEdit, &BearingLineEdit::selectAll);
    connect(ui->turnButton, &QPushButton::clicked, ui->bearingEdit, QOverload<>::of(&BearingLineEdit::setFocus));
    connect(ui->stopButton, &QPushButton::clicked, ui->bearingEdit, &BearingLineEdit::selectAll);
    connect(ui->stopButton, &QPushButton::clicked, ui->bearingEdit, QOverload<>::of(&BearingLineEdit::setFocus));
    connect(ui->bearingEdit, &BearingLineEdit::returnPressed, ui->bearingEdit, QOverload<>::of(&BearingLineEdit::setFocus));
    connect(ui->bearingEdit, &BearingLineEdit::returnPressed, ui->bearingEdit, &BearingLineEdit::selectAll);
    connect(ui->stopButton, &QPushButton::clicked, this, &RotatorMainWindow::stopButton);
    connect(ui->rot_right_button, &QPushButton::clicked, this, &RotatorMainWindow::rotateCW);
    connect(ui->rot_left_button, &QPushButton::clicked, this, &RotatorMainWindow::rotateCCW);
    connect(this, &RotatorMainWindow::escapePressed, this, &RotatorMainWindow::stop_rotation);

    // click on compass rose

    connect(ui->compassDial, &MinosCompass::sendClickBearing, this, &RotatorMainWindow::compassClicked);

    // display bearing
    connect(pollTimer, &QTimer::timeout, this, &RotatorMainWindow::request_bearing);
    connect(this, &RotatorMainWindow::sendCompassDial, ui->compassDial, &MinosCompass::compassDialUpdate);
    connect(this, &RotatorMainWindow::sendBearing, ui->bearingDisplay, &QLabel::setText);
    connect(this, &RotatorMainWindow::sendBackBearing, ui->backBearingDisplay, &QLabel::setText);
    connect(this, &RotatorMainWindow::displayActualBearing, actualRotatorDisplay, &QLabel::setText);
    connect(this, &RotatorMainWindow::displayOverlap, this ,&RotatorMainWindow::overLapDisplayBox);
    // check endstop and turn to rotation stop
    connect(&RotateTimer, &QTimer::timeout, this, &RotatorMainWindow::rotatingTimer);
    connect(this, &RotatorMainWindow::checkingEndStop, this, &RotatorMainWindow::checkEndStop);

    // Test Bearing Box
    connect(ui->testBearing, &QLineEdit::returnPressed, this, &RotatorMainWindow::onTestBearingEnter);

    // setup antennas
    //connect(ui->actionSetup_Antennas, &QAction::triggered, this, &RotatorMainWindow::onLaunchSetup);
    connect(setupAntenna, &RotSetupDialog::currentAntennaSettingChanged, this, &RotatorMainWindow::currentAntennaSettingChanged);
    connect(setupAntenna, &RotSetupDialog::antennaNameChange, this, &RotatorMainWindow::updateSelectAntennaBox);
    connect(setupAntenna, &RotSetupDialog::antennaTabChanged, this, &RotatorMainWindow::updateSelectAntennaBox);

    //connect(ui->actionPST_Rotator_Config, &QAction::triggered, this, &RotatorMainWindow::onPSTRotatorConfig);

    // Bearing Log
    //connect(ui->actionLog_Heading, &QAction::triggered, setupLog, &LogDialog::loadLogConfig);
    connect(setupLog, &LogDialog::showLogDialog, setupLog, &RotatorMainWindow::show);
    connect(setupLog, &LogDialog::bearingLogConfigChanged, rotlog, &RotatorLog::getBearingLogConfig);

    // Message from Logger
    connect(msg, &RotatorRpc::setRotation, this, &RotatorMainWindow::onLoggerSetRotation);
    connect(msg, &RotatorRpc::selectAntennaFromLog, this, &RotatorMainWindow::onLoggerSelectAntenna);
    connect(msg, &RotatorRpc::setRotPreset, this, &RotatorMainWindow::onLoggerSetPreset);


    //connect(ui->actionAbout, &QAction::triggered, this, &RotatorMainWindow::about);
    //connect(ui->actionAbout_Rotator_Config, &QAction::triggered, this, &RotatorMainWindow::aboutRotatorConfig);
    //connect(ui->actionTraceComms, &QAction::toggled, this, &RotatorMainWindow::saveTraceLogFlag);    // set/clear comms tracing

}




void RotatorMainWindow::keyPressEvent(QKeyEvent *event)
{

    int Key = event->key();

/*
    Qt::KeyboardModifiers mods = event->modifiers();
    bool shift = mods & Qt::ShiftModifier;
    bool ctrl = mods & Qt::ControlModifier;
    bool alt = mods & Qt::AltModifier;
*/

    if (Key == Qt::Key_Escape)
    {
        emit escapePressed();
    }
    return QMainWindow::keyPressEvent(event);
}

// receives updates from rotator
// update bearing displays
// backbearing display
// and also signal check of endstops

void RotatorMainWindow::displayBearing(int bearing)
{

    logMessage(QString("Bearing from Rotator %1").arg(QString::number(bearing)));

    if (bearing == rotatorBearing)
    {
        return;
    }

     if (setupAntenna->currentAntenna.southStopType == S_STOPINV)
    {
        rotatorBearing = bearing;
        if (bearing >= COMPASS_MIN0 && bearing <= COMPASS_HALF)
        {
            bearing += COMPASS_HALF;            // correct inversion for display
        }
        else if (bearing > COMPASS_HALF && bearing <= COMPASS_MAX360)
        {
            bearing -= COMPASS_HALF;
        }
    }
    else
    {
        rotatorBearing = bearing;
    }


    curBearingWithOffset = bearing + setupAntenna->currentAntenna.antennaOffset;

    logMessage(QString("Current Bearing + offset = %1").arg(QString::number( curBearingWithOffset)));

    int displayBearing = curBearingWithOffset;

    //if (curBearingWithOffset >= COMPASS_MAX360)
    if (curBearingWithOffset > COMPASS_MAX360)
    {
        displayBearing = curBearingWithOffset - COMPASS_MAX360;
    }
    else if (curBearingWithOffset < COMPASS_MIN0)
    {
        displayBearing = COMPASS_MAX360 + curBearingWithOffset;
    }
    logMessage(QString("Display Bearing = %1").arg( QString::number(displayBearing)));

    // send Bearing to displays

    // send to minos logger

    if (!testMode)
    {
        // send bearings to logger
        QString ol = "";
        if (overLapActiveflag)
        {
            ol = "1";
        }
        else
        {
            ol = "0";
        }
        QString s = QString("%1:%2:%3").arg(QString::number(displayBearing), QString::number(rotatorBearing), ol);

        PubSubName psname(setupAntenna->currentAntennaName);
        msg->rotatorCache.setBearing(psname, s);
        msg->rotatorCache.publish();
    }
    QString rotatorBearingmsg = QString::number(displayBearing);
    if (displayBearing < 10 && rotatorBearing > 0)
    {
        rotatorBearingmsg = "00" +rotatorBearingmsg;
    }
    else if (displayBearing < 100 && rotatorBearing > 0)
    {
        rotatorBearingmsg = "0" +rotatorBearingmsg;
    }
    // send rotatorBearing to actual rotatorBearingDisplay
    emit displayActualBearing(rotatorBearingmsg);
    // display raw rotator bearing on status line
    dispRawRotBearing(rotatorBearing);



    if (overLapActiveflag)
    {
        if (rotatorBearing > COMPASS_MAX360)
        {
            bearing -= COMPASS_MAX360;

            logMessage(QString("OverLapOn Positive Overlap - Rotator Bearing = %1").arg(QString::number(bearing)));
            overLapStatus = POS_OVERLAP;
        }
        else if (rotatorBearing < COMPASS_MIN0)
        {
            bearing += COMPASS_MAX360;
            logMessage(QString("OverLapOn Negative Overlap - Rotator Bearing = %1").arg(QString::number(bearing)));
            overLapStatus = NEG_OVERLAP;
        }
        else
        {
            overLapStatus = NO_OVERLAP;
            logMessage(QString("OverLapOff - Rotator Bearing = %1").arg(QString::number(bearing)));

        }

        emit displayOverlap(overLapStatus);


    }

    QString bearingmsg = QString::number(displayBearing, 10);
    if (displayBearing < 10)    // prevent display resizing
    {
        bearingmsg = "00" + bearingmsg;
    }
    else if (displayBearing < 100)
    {
        bearingmsg = "0" + bearingmsg;
    }

    emit sendBearing(bearingmsg);
    emit sendCompassDial(displayBearing);

    // check antenna is not at endstops when manually rotating
    emit checkingEndStop();

    // calc and send backbearing
    int backBearing = displayBearing;

    backBearing += COMPASS_HALF;
    //if (backBearing >= COMPASS_MAX360)
    if (backBearing > COMPASS_MAX360)
    {
        backBearing -= COMPASS_MAX360;
    }
    // send backBearing to display
    QString backBearingmsg = QString::number(backBearing, 10);
    if (backBearing < 10)
    {
        backBearingmsg = "00" + backBearingmsg;
    }
    else if (backBearing < 100)
    {
        backBearingmsg = "0" + backBearingmsg;
    }
    emit sendBackBearing(backBearingmsg);

}

void RotatorMainWindow::compassClicked(int brg)
{
    ui->bearingEdit->setText(QString::number(brg));
    emit presetRotateTo();
}


void RotatorMainWindow::dispRawRotBearing(int rotatorBearing)
{
    rawRotatorDisplay->setText(QString::number(rotatorBearing));
}



void RotatorMainWindow::clickedPreset(int buttonNumber)
{

    if (rotPresets[buttonNumber]->name != "")
    {
        if (rotPresets[buttonNumber]->bearing != "")
        {
           ui->bearingEdit->setText(rotPresets[buttonNumber]->bearing);
           emit presetRotateTo();
        }
    }
}



/*
void RotatorMainWindow::refreshPresetLabels()
{

    readPresets();

    for (int i = 0; i < NUM_PRESETS; i++)
    {
        if (rotPresets[i]->name != "" || rotPresets[i]->name != presetButtons[i]->text())
        {
            presetButtons[i]->setText(rotPresets[i]->name);
            presetButtons[i]->setShortcut(presetShortCut[i]);     // restore the shortcut
        }
    }
}




void RotatorMainWindow::updatePresetLabels()
{
    refreshPresetLabels();
    update();
}

*/







void RotatorMainWindow::initSelectAntennaBox()
{
    selectAntenna->addItem("");
    for (int i= 0; i < setupAntenna->numAvailAntennas; i++)
    {
        selectAntenna->addItem(setupAntenna->availAntData[i]->antennaName);
    }
    sendAntennaListLogger();
}




void RotatorMainWindow::upDateAntenna()
{


    int currentAntIdx = -1;

    trace(QString("updateAntenna to %1").arg(setupAntenna->currentAntennaName));

    if (rotator)
    {
        if (moving  || movingCCW || movingCW)
        {

            stopRotation(true);
        }
    }


    if (setupAntenna->currentAntennaName != "")
    {
        currentAntIdx = setupAntenna->findCurrentAntenna(setupAntenna->currentAntennaName);
        trace(QString("idx is %1 avail is %2").arg(currentAntIdx).arg(setupAntenna->numAvailAntennas));
        if (currentAntIdx > -1 && currentAntIdx < setupAntenna->numAvailAntennas)  // find antenna and update current antenna pointer
        {
            // found antenna, update currentAntenna with selected antenna data
            srotParams::copyRot(setupAntenna->availAntData[currentAntIdx], setupAntenna->currentAntenna);

            if (rotator)
            {
              closeRotator();
            }


            if (openRotator() != OPEN_OK)
            {
                return;
            }


            ui->antNameDisp->setText(setupAntenna->currentAntenna.antennaName);
            ui->usingLibText->setText(rotator->getLibraryName());

            offSetDisplay->setText(QString::number(setupAntenna->currentAntenna.antennaOffset));

            // don't display overlap if rotator doesn't support or user turned off overlap
            toggleOverLapDisplay(setupAntenna->currentAntenna.overRunFlag);

           // for test ******************************************************************************************
            //setupAntenna->currentAntenna.supportCwCcwCmd = false;
           // setupAntenna->currentAntenna.simCwCcwCmd = false;

            if (setupAntenna->currentAntenna.supportCwCcwCmd)           // want to use simCwCccwCmd?
            {
                cwCCWControlVisible(true);
            }
            else
            {
                cwCCWControlVisible(setupAntenna->currentAntenna.simCwCcwCmd);
            }



            RotCapabilities rotCap = rotFactory->supported_rotators()->value(setupAntenna->currentAntenna.rotatorModel);
            if (rotCap.enableSelectDisplayDial && !setupAntenna->currentAntenna.showCompassDialFlag)
            {
                setCompassDialVisible(false);
            }
            else
            {
                setCompassDialVisible(true);
            }

            if (rotCap.supportStopCommand)
            {
                supportStopCommandFlag = true;
                ui->stopButton->setVisible(true);
            }
            else
            {
                supportStopCommandFlag = false;
                ui->stopButton->setVisible(false);
            }



            if (rotator->getRotConnected())
            {
                sendStatusToLogConnected();
            }
            else
            {
                sendStatusToLogDisConnected();
            }

            rotatorBearing = 9999;      // force display update
           // update logger
           if (!testMode)
           {
               sendStatusToLogStop();
               PubSubName psname(setupAntenna->currentAntennaName);
               trace(QString("Update Antenna - send to logger - maxAzimuth = %1, minAzimuth = %2, simulate CwCcwCmd = %3").arg(QString::number(setupAntenna->currentAntenna.max_azimuth)).arg(QString::number(setupAntenna->currentAntenna.min_azimuth), setupAntenna->currentAntenna.supportCwCcwCmd  ? "True" : "False"));
               msg->rotatorCache.setMaxAzimuth(psname, setupAntenna->currentAntenna.max_azimuth);
               msg->rotatorCache.setMinAzimuth(psname, setupAntenna->currentAntenna.min_azimuth);

               if (setupAntenna->currentAntenna.supportCwCcwCmd)           // want to use simCwCccwCmd?
               {
                   msg->rotatorCache.setCwCcwCmdEnable(psname, true);

               }
               else
               {
                   msg->rotatorCache.setCwCcwCmdEnable(psname, setupAntenna->currentAntenna.simCwCcwCmd);

               }

               msg->rotatorCache.setSupportStopCommand(psname, supportStopCommandFlag);
               msg->rotatorCache.publish();

           }



        }
    }
    else
    {   // no antenna selected
        trace("No antenna selected");
        ui->antNameDisp->setText("");
        ui->usingLibText->setText("");
        closeRotator();
        if (!testMode)
        {
            sendStatusToLogDisConnected();
            sendStatusToLogStop();
        }

    }

    if (!testMode)
    {
        msg->rotatorCache.publish();
    }

}
void RotatorMainWindow::refreshAntenna()
{
    trace("refreshAntenna");
    int antennaIndex = ui->selectAntennaBox->currentIndex();
    if (antennaIndex > 0)
    {
        if (setupAntenna->currentAntenna.rotatorModelNumber == 0)
        {
            closeRotator();
            QMessageBox::critical(this, tr("Antenna Error"), tr("Please configure an antenna name and rotator model"));
            return;
        }

        if (rotator == nullptr)
        {

            if (openRotator() != OPEN_OK)
            {
                return;
            }
        }

        if (rotator->getRotConnected())
        {
            sendStatusToLogConnected();
        }
        else
        {
            sendStatusToLogDisConnected();
        }

    }

    msg->rotatorCache.publish();
}

void RotatorMainWindow::writeWindowTitle(QString appName)
{
    if (appName.length() > 0)
    {
        setWindowTitle(tr("Minos Rotator Control - Logger - %1").arg(appName));
    }
    else
    {
        setWindowTitle(tr("Minos Rotator Control - Local"));
    }

}

void RotatorMainWindow::request_bearing()
{

    logMessage(QString("Request Bearing"));

    if (rotator)
    {
        if (ui->testBearing->isVisible() && ui->testBearingChkbox->isChecked())
        {

            logMessage("Using Test Bearing Box");
            return;
        }


        reqBearCmdflag = true;
        int retCode = 0;
        if (brakeflag || cwCcwCmdflag || rotCmdflag) return;
        if (rotator->getRotConnected())
        {
            retCode = rotator->request_bearing();
            logMessage(QString("Sent request bearing cmd - retcode = %1").arg(QString::number(retCode)));
            if (retCode < 0)
            {
                logMessage(QString("Request bearing: error"));
                rotatorError(retCode, tr("Request Bearing"));
            }
        }
        reqBearCmdflag = false;
    }


}


void RotatorMainWindow::checkEndStop()
{


    logMessage(QString("Check EndStop"));
    logMessage(QString("curBearingWithOffset = %1").arg(QString::number(curBearingWithOffset)));
    logMessage(QString("rotatorBearing = %1").arg(QString::number(rotatorBearing)));
    logMessage(QString("currentMaxAzimuth = %1").arg(QString::number(setupAntenna->currentAntenna.max_azimuth)));
    logMessage(QString("currentMinAzimuth = %1").arg(QString::number(setupAntenna->currentAntenna.min_azimuth)));
    if (movingCW)
    {

        if (setupAntenna->currentAntenna.southStopType == S_STOPCOMP)
        {
            if (rotatorBearing >= setupAntenna->currentAntenna.max_azimuth && rotatorBearing <= setupAntenna->currentAntenna.min_azimuth)
            {
                logMessage(QString("S_STOPCOMP - Max Endstop reached!"));
                stopButton();
            }
        }
        else if (rotatorBearing >= setupAntenna->currentAntenna.max_azimuth)
        {
                logMessage(QString("Max Endstop reached!"));
                stopButton();
            }
        }
    else if (movingCCW)
    {
        if (setupAntenna->currentAntenna.southStopType == S_STOPCOMP)
        {
            if (rotatorBearing <= setupAntenna->currentAntenna.min_azimuth && rotatorBearing >= setupAntenna->currentAntenna.max_azimuth)
            {
                logMessage(QString("S_STOPCOMP - Min Endstop reached!"));
                stopButton();
            }
        }
        else if (rotatorBearing <= setupAntenna->currentAntenna.min_azimuth)
        {
                logMessage(QString("Min Endstop reached!"));
                stopButton();
            }
        }
}


void RotatorMainWindow::rotatingTimer()
{

     rotTimeCount++;

}


void RotatorMainWindow::checkMoving(int bearing)
{

    static int oldBearing;
    logMessage(QString("Check Moving"));
    if (!moving)
    {
        return;
    }

    if ((abs(targetBearing - bearing) <= 2) && (oldBearing != bearing))
    {
        if (rotTimeCount > 1)
        {
            logMessage(QString("Rotator is near target for too long"));
            stopButton();
            sendStatusToLogStop();
        }
    }
    else if (oldBearing != bearing)
    {
            oldBearing = bearing;
            rotTimeCount = 0;
            logMessage(QString("Rotator is moving"));
            return;
    }
    else
    {
        if (rotTimeCount > ROTATE_MOVE_TIMEOUT)
        {
            logMessage(QString("Rotator has stopped moving"));
            stopButton();
            sendStatusToLogStop();
        }

    }

}

void RotatorMainWindow::rotateToController()
{

    if (reqBearCmdflag || brakeflag)
    {
        return;
    }


    rotCmdflag = true;


    if (ui->bearingEdit->isValid())
    {
        rotateTo(ui->bearingEdit->getBearing());
    }
    else
    {
         QMessageBox::critical(this, tr("Bearing Error"),
                               tr("Invalid Bearing\nPlease enter %1 - %2").arg(QString::number(COMPASS_MIN0), QString::number(COMPASS_MAX360)));
    }

    rotCmdflag = false;
}


void RotatorMainWindow::rotateTo(int bearing)
{

    int retCode = 0;
    int rotateTo = bearing;
    logMessage(QString("RotateTo Bearing = %1").arg(QString::number(bearing)));


    if (rotator)
    {

        // adjust bearing with offset
        if (setupAntenna->currentAntenna.antennaOffset < 0)
        {
            rotateTo = rotateTo - setupAntenna->currentAntenna.antennaOffset;
            if (rotateTo >= COMPASS_MAX360)
            {
                rotateTo = rotateTo - COMPASS_MAX360;
            }
        }
        else
        {
            rotateTo = rotateTo - setupAntenna->currentAntenna.antennaOffset;
            if (rotateTo < COMPASS_MIN0)
            {
                rotateTo = COMPASS_MAX360 + rotateTo;
            }
        }


        logMessage(QString("Rotate to Bearing = %1, adjusted with offset = %2").arg(QString::number(rotateTo), QString::number(setupAntenna->currentAntenna.antennaOffset)));



        if (rotateTo > COMPASS_MAX360 || rotateTo < COMPASS_MIN0)
        {
            logMessage(QString("Error - Rotate To Bearing = %1").arg(QString::number(rotateTo)));
            return; //error
        }



        // calculate target bearing based on current position
        rotateTo  = northCalcTarget(rotateTo);

        logMessage(QString("rotateTo calculated bearing %1").arg(QString::number(rotateTo)));

        // check if we are already at bearing
        if (rotateTo == rotatorBearing)
        {
            return;
        }

        if (movingCW || movingCCW)
        {

            stopRotation(true);
        }



        if (rotator->getRotConnected())
        {

            if ((setupAntenna->currentAntenna.max_azimuth == COMPASS_MAX360 || setupAntenna->currentAntenna.max_azimuth == COMPASS_MAX359) && rotateTo == COMPASS_MAX360)
            {
                rotateTo = rotateTo - 1;        // some hamlib and PST Rotator do not like 360
            }

            targetBearing = rotateTo;
            retCode = rotator->rotate_to_bearing(rotateTo);
            if (retCode < 0)
            {
                rotatorError(retCode, tr("Rotate to Bearing"));
            }
            else
            {
                moving = true;
                turn_button_on();
                sendStatusToLogTurn();
                rotTimeCount = 0;           // clear timer count
            }

        }
    }
    else
    {
        logMessage(QString("rotator = nullptr"));
    }




}








int RotatorMainWindow::northCalcTarget(int targetBearing)
{

    int target = targetBearing;

    if (setupAntenna->currentAntenna.endStopType == ROT_0_360)
    {
        target = calcRotZero360(targetBearing);

    }
    else if (setupAntenna->currentAntenna.endStopType == ROT_NEG180_180)
    {
        target = calcRotNeg180_180(targetBearing);

    }
    else if (setupAntenna->currentAntenna.endStopType == ROT_0_450 || setupAntenna->currentAntenna.endStopType == ROT_NEG180_540)
    {
        target = calclRot_0_450_Neg180_540(targetBearing);
    }

    return target;

}



int RotatorMainWindow::calcRotZero360(int targetBearing)
{
    if (setupAntenna->currentAntenna.southStopType == S_STOPINV)
    {
        if (targetBearing >= COMPASS_MIN0 && targetBearing <= COMPASS_HALF)
        {
            targetBearing += COMPASS_HALF;
        }
        else if (targetBearing > COMPASS_HALF && targetBearing <= COMPASS_MAX360)
        {
            targetBearing -= COMPASS_HALF;
        }
        logMessage(QString("NCalc - EndStop Type - ROT_0_360 - S_STOPINV"));
        logMessage(QString("NCalc - 1 - Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing), QString::number(rotatorBearing)));
    }
    else
    {
        logMessage(QString("NCalc - EndStop Type - ROT_0_360 - S_STOPOFF"));
        logMessage(QString("NCalc - 1 - Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing), QString::number(rotatorBearing)));
    }

    return targetBearing;
}


int RotatorMainWindow::calcRotNeg180_180(int targetBearing)
{
    int target = targetBearing;

    logMessage(QString("NCalc - EndStop Type - ROT_NEG180_180"));
    if (targetBearing > COMPASS_HALF && targetBearing <= COMPASS_MAX360)
    {
        target = targetBearing - COMPASS_MAX360;
        logMessage(QString("NCalc - 2 - Target bearing > 180 and < 360, calculated target = %1").arg(QString::number(target)));
    }


    logMessage(QString("Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing), QString::number(rotatorBearing)));
    return target;

}

int RotatorMainWindow::calclRot_0_450_Neg180_540(int targetBearing)
{
    int target = targetBearing;
    logMessage(QString("NCalc - EndStop Type - ROT_0_450 or ROT_NEG180_540"));
    if (setupAntenna->currentAntenna.endStopType == ROT_NEG180_540)
    {
        if (rotatorBearing >= COMPASS_MIN0 && rotatorBearing <= COMPASS_HALF)
        {
            if (targetBearing >= COMPASS_MIN0 && targetBearing <= COMPASS_HALF)
            {
                target = targetBearing;
                logMessage(QString("NCalc - EndStop Type - ROT_NEG180_450"));
                logMessage(QString("NCalc - 4 - - Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing), QString::number(rotatorBearing)));
            }
            else if (targetBearing > COMPASS_HALF && targetBearing <= COMPASS_MAX360)
            {
                if (COMPASS_MAX360 - targetBearing + rotatorBearing < targetBearing - rotatorBearing)
                {
                    target = targetBearing - COMPASS_MAX360;
                }
                logMessage(QString("NCalc - EndStop Type - ROT_NEG180_450"));
                logMessage(QString("NCalc - 5 - - Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing), QString::number(rotatorBearing)));
                return target;
            }
        }
        else if (rotatorBearing < COMPASS_MIN0)
        {
            if (targetBearing > 180 && targetBearing <= 360)
            {
                target = targetBearing - COMPASS_MAX360;
            }
            logMessage(QString("NCalc - EndStop Type - ROT_NEG180_450"));
            logMessage(QString("NCalc - 6 - - Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing), QString::number(rotatorBearing)));
            return target;
        }

    }

    if (rotatorBearing >= COMPASS_MAX360 && ((targetBearing + COMPASS_MAX360) <= setupAntenna->currentAntenna.max_azimuth))
    {
        target = targetBearing + COMPASS_MAX360;
        logMessage(QString("NCalc - 7 - Rotator Bearing = %1 >= 360, target bearing = %2, target bearing + 360 = %3 is < maxAzimuth = %4, calculated target = %5")
                   .arg(QString::number(rotatorBearing)).arg(QString::number(targetBearing)).arg(QString::number(targetBearing + COMPASS_MAX360))
                       .arg(QString::number(setupAntenna->currentAntenna.max_azimuth)).arg(QString::number(target)));
        return target;
    }
    else if (rotatorBearing >= COMPASS_MAX360 && targetBearing < COMPASS_MAX360)
    {
        target = targetBearing;
        logMessage(QString("NCalc - 8 - Rotator Bearing = %1 >= 360, target bearing = %2 is < 360, target = %3")
                   .arg(QString::number(rotatorBearing), QString::number(targetBearing), QString::number(target)));
        return target;
    }
    else if (rotatorBearing > COMPASS_HALF && rotatorBearing <= COMPASS_MAX360)
    {
        if ((COMPASS_MAX360 + targetBearing) <= setupAntenna->currentAntenna.max_azimuth)
        {
            if (COMPASS_MAX360 + targetBearing - rotatorBearing < rotatorBearing - targetBearing)
            {
                target = COMPASS_MAX360 + targetBearing;
            }
        }

        logMessage(QString("NCalc - 9 - Rotator Bearing = %1 > 180 and <= 360, target bearing = %2, target = %3")
                   .arg(QString::number(rotatorBearing), QString::number(targetBearing), QString::number(target)));
        return target;
    }
    else if (rotatorBearing >= COMPASS_MIN0 && rotatorBearing <= COMPASS_HALF)
    {
        target = targetBearing;
        logMessage(QString("NCalc - 10 - Rotator Bearing = %1 >= 0 and <= 180, target bearing = %2, target = %3")
                    .arg(QString::number(rotatorBearing), QString::number(targetBearing), QString::number(target)));
        return target;
    }
    return target;
}





void RotatorMainWindow::stopButton()
{

    logMessage(QString("StopButton"));
    if (rotator)
    {
        stopRotation(rotator->getRotConnected());
    }
    else
    {
        logMessage(QString("rotator = nullptr"));
    }

}

void RotatorMainWindow::stop_rotation()
{

    logMessage(QString("stop_rotation"));
    if (rotator)
    {
        stopRotation(rotator->getRotConnected());
    }
    else
    {
        logMessage(QString("rotator = nullptr"));
    }

}

void RotatorMainWindow::stopRotation(bool sendStop)
{

    logMessage(QString("Stop Rotation"));
    if (rotator)
    {
        int retCode = 0;
        stop_button_on();
        brakeflag = true;
        stopCmdflag = true;

        if (sendStop)
        {
            retCode = rotator->stop_rotation();
            logMessage(QString("Stop cmd sent to rotator - retcode = %1").arg(QString::number(retCode)));

            if (retCode < 0)
            {
                rotatorError(retCode, "Stop Rotation");
                sendStatusToLogError();

            }
        }


        if (rot_left_button_status)
        {
            rot_left_button_off();
        }
        if (rot_right_button_status)
        {
            rot_right_button_off();
        }
        if (turn_button_status)
        {
            turn_button_off();
        }

        sendStatusToLogStop();
        sleepFor(brakedelay);
        brakeflag = false;
        moving = false;
        movingCW = false;
        movingCCW = false;
        stopCmdflag = false;
        stop_button_off();
        logMessage(QString("Stop Cmd Successful"));
    }
    else
    {
        logMessage(QString("rotator = nullptr"));
    }


}


void RotatorMainWindow::rotateCW(bool /*clicked*/)
{

    logMessage(QString("Start rotateCW"));

    if (brakeflag)
    {
        logMessage(QString("Brakeflag = %1").arg(brakeflag ? "True" : "False"));
        return;
    }

    if (rotator)
    {
        cwCcwCmdflag = true;

        if (!rotator->getRotConnected())
        {
            logMessage(QString("rotateCW - Rotator not connected!"));

        }
        else if (rot_right_button_status)
        {
            // button on, turn off
            stopButton();
        }
        else
        {

            // check if at endstop
            if (setupAntenna->currentAntenna.southStopType == S_STOPCOMP)
            {
                if (rotatorBearing == setupAntenna->currentAntenna.max_azimuth)
                {
                    logMessage(QString("CCW - S_STOPCMP - Max Endstop"));
                    cwCcwCmdflag = false;
                    return;
                }

            }
            else if (rotatorBearing >= setupAntenna->currentAntenna.max_azimuth)
            {
                logMessage(QString("Rotator Bearing > currentMaxAzimuth"));
                cwCcwCmdflag = false;
                return;
            }


            if (moving || movingCW || movingCCW)
            {
                logMessage(QString("RotateCW - rotator already moving - stop"));
                stopButton();
            }


            int retCode = 0;
            if (rotator->getRotConnected())
            {
                if (setupAntenna->currentAntenna.supportCwCcwCmd)
                {
                    logMessage(QString("Send CW rotator command, rotator speed = %1").arg(QString::number(rotator->get_rotatorSpeed())));
                    retCode = rotator->rotateClockwise(rotator->get_rotatorSpeed());
                }
                else
                {

                    int bearing = setupAntenna->currentAntenna.max_azimuth;
                    if ( bearing == COMPASS_MAX360)
                    {
                        bearing -=  1;  // some rotators change 360 to 0, but we want max here
                    }

                    retCode = rotator->rotate_to_bearing(bearing);
                    logMessage(QString("Send rotate to maxAzimuth, instead of CW rotator command, maxAzimuth = %1").arg(QString::number(bearing)));

                }
                if (retCode < 0)
                {
                    rotatorError(retCode, "Rotate CW");
                    movingCW = false;
                    sendStatusToLogError();

                }
                else
                {

                    movingCW = true;
                    if (!setupAntenna->currentAntenna.supportCwCcwCmd)
                    {
                        moving = true;
                    }
                    sendStatusToLogRotCW();
                    rot_right_button_on();

                    logMessage(QString("RotateCW Successful"));
                }
            }
        }



        cwCcwCmdflag = false;
    }
    else
    {
        logMessage(QString("rotator = nullptr"));
    }

}




void RotatorMainWindow::rotateCCW(bool /*toggle*/)
{

    logMessage(QString("Start rotateCCW"));

    if (brakeflag)
    {
        logMessage(QString("Brakeflag = %1").arg(brakeflag ? "True" : "False"));
        return;
    }

    if (rotator)
    {
        cwCcwCmdflag = true;
        logMessage(QString("Start rotateCCW"));
        // check connected
        if (!rotator->getRotConnected())
        {
            logMessage(QString("rotateCCW - Rotator not connected!"));

        }
        else if (rot_left_button_status)
        {
            // button on, turn off
            stopButton();
        }
        else
        {
            // check if at endstop
            if (setupAntenna->currentAntenna.southStopType == S_STOPCOMP)
            {
                if (rotatorBearing == setupAntenna->currentAntenna.min_azimuth)
                {
                    logMessage(QString("CCW - S_STOPCMP - Min Endstop"));
                    cwCcwCmdflag = false;
                    return;
                }

            }
            else if (rotatorBearing < setupAntenna->currentAntenna.min_azimuth)
            {
                logMessage(QString("CCW - Rotator Bearing < currentMinAzimuth"));

            }


            if (moving || movingCW || movingCCW)
            {
                logMessage(QString("RotateCCW - rotator already moving - stop"));
                stopButton();
            }


            int retCode = 0;
            if (rotator->getRotConnected())
            {
                if (setupAntenna->currentAntenna.supportCwCcwCmd)
                {
                    logMessage(QString("Send CCW rotator command, rotator speed = " + QString::number(rotator->get_rotatorSpeed())));
                    retCode = rotator->rotateCClockwise(rotator->get_rotatorSpeed());
                }
                else
                {

                    logMessage(QString("Send rotate to minAzimuth, instead of CCW rotator command, minAzimuth = %1").arg(QString::number(setupAntenna->currentAntenna.min_azimuth)));
                    retCode = rotator->rotate_to_bearing(setupAntenna->currentAntenna.min_azimuth + 1); // +1 for Spid Rotator
                }

                if (retCode < 0)
                {
                    rotatorError(retCode, "Rotate CCW");
                    movingCCW = false;
                    sendStatusToLogError();

                }
                else
                {

                    movingCCW = true;
                    if (!setupAntenna->currentAntenna.supportCwCcwCmd)
                    {
                        moving = true;
                    }
                    sendStatusToLogRotCCW();
                    rot_left_button_on();
                    logMessage(QString("RotateCCW Successful"));
                }
            }
        }

        cwCcwCmdflag = false;
    }
    else
    {
        logMessage(QString("rotator = nullptr"));
    }


}


void RotatorMainWindow::turn_button_on()
{
    turn_button_status = ON;
    //ui->turnButton->setPalette(*redText);
    ui->turnButton->setStyleSheet(BUTTON_ON_STYLE);
    ui->turnButton->setText(tr("Turn"));
}

void RotatorMainWindow::turn_button_off()
{
    turn_button_status = OFF;
    //ui->turnButton->setPalette(*blackText);
    ui->turnButton->setStyleSheet(BUTTON_OFF_STYLE);
    ui->turnButton->setText(tr("Turn"));
}


void RotatorMainWindow::rot_left_button_on()
{
    rot_left_button_status = ON;
    //ui->rot_left_button->setPalette(*redText);
    ui->rot_left_button->setStyleSheet(BUTTON_ON_STYLE);
    ui->rot_left_button->setText(tr("(CCW) Left"));
}

void RotatorMainWindow::rot_left_button_off()
{
    rot_left_button_status = OFF;
    //ui->rot_left_button->setPalette(*blackText);
    ui->rot_left_button->setStyleSheet(BUTTON_OFF_STYLE);
    ui->rot_left_button->setText(tr("(CCW) Left"));
}

void RotatorMainWindow::rot_right_button_on()
{
    rot_right_button_status = ON;
    //ui->rot_right_button->setPalette(*redText);

    ui->rot_right_button->setStyleSheet(BUTTON_ON_STYLE);
    ui->rot_right_button->setText(tr("(CW) Right"));
}

void RotatorMainWindow::rot_right_button_off()
{
    rot_right_button_status = OFF;
    //ui->rot_right_button->setPalette(*blackText);
    ui->rot_right_button->setStyleSheet(BUTTON_OFF_STYLE);
    ui->rot_right_button->setText(tr("(CW) Right"));
}



void RotatorMainWindow::stop_button_on()
{

    //ui->stopButton->setPalette(*redText);
    ui->stopButton->setStyleSheet(BUTTON_ON_STYLE);
    ui->stopButton->setText(tr("Stop"));
}

void RotatorMainWindow::stop_button_off()
{

    //ui->stopButton->setPalette(*blackText);
    ui->stopButton->setStyleSheet(BUTTON_OFF_STYLE);
    ui->stopButton->setText(tr("Stop"));
}

void RotatorMainWindow::onSentCommandError(int errorCode, QString cmd)
{
    rotatorError(errorCode, cmd);
}
void RotatorMainWindow::rotatorError(int errorCode, QString cmd )
{

    if ( errorCode >= 0)
    {
        return;
    }

    errorCode *= -1;
    rotErrorFlag = true;
    if (!testMode)
    {
        sendStatusToLogError();
    }
    // log all errors
    QString errorMsg;
    if (rotator)
    {
        errorMsg = rotator->getErrorMsgText(errorCode);
        logMessage(QString("%1 library Error - Code = %2 - %3").arg(rotator->getLibraryName(), QString::number(errorCode), errorMsg));

    }


     pollTimer->stop();

     if (rotator)
     {
         QMessageBox::critical(this, tr("Rotator %1 library Error").arg(rotator->getLibraryName()), tr("%1\n%2 - %3\nCommand: %4").arg(setupAntenna->currentAntenna.antennaName).arg(errorCode).arg(errorMsg, cmd));

     }

     closeRotator();
     rotErrorFlag = false;
     if (!testMode)
     {
         sendStatusToLogDisConnected();
     }
}


void RotatorMainWindow::sendStatusToLogConnected()
{
    connectStat = ROT_STATUS_CONNECTED;
    statusMsg = "";
    sendStatusLogger();
}

void RotatorMainWindow::sendStatusToLogDisConnected()
{
    connectStat = ROT_STATUS_DISCONNECTED;
    sendStatusLogger();
}

void RotatorMainWindow::sendStatusToLogRotCCW()
{
    statusMsg = ROT_STATUS_ROTATE_CCW;
    sendStatusLogger();
}

void RotatorMainWindow::sendStatusToLogRotCW()
{
    statusMsg = ROT_STATUS_ROTATE_CW;
    sendStatusLogger();
}

void RotatorMainWindow::sendStatusToLogStop()
{
    statusMsg = ROT_STATUS_STOP;
    sendStatusLogger();
}

void RotatorMainWindow::sendStatusToLogTurn()
{
    statusMsg = ROT_STATUS_TURN_TO;
    sendStatusLogger();
}



void RotatorMainWindow::sendStatusToLogError()
{
    statusMsg = ROT_STATUS_ERROR;
    sendStatusLogger();
}


void delay(int sec)
{
    QTime dieTime= QTime::currentTime().addSecs(sec);
    while (QTime::currentTime() < dieTime)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

void sleepFor(qint64 milliseconds)
{
    qint64 timeToExitFunction = QDateTime::currentMSecsSinceEpoch() + milliseconds;
    while(timeToExitFunction > QDateTime::currentMSecsSinceEpoch())
    {
        QApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}


void RotatorMainWindow::about()
{
    QMessageBox::about(this, tr("Minos Rotator"), tr("Minos Rotator\nCopyright D Balharrie G8FKH/M0DGB 2016 - 2018"));
}


void RotatorMainWindow::overLapDisplayBox(overlapStat status)
{
    QString style;
    QString btext;
    if (status == POS_OVERLAP)
    {
        style = "background-color: red; color black; font-weight: bold;\n";
        btext = tr("Positive Overlap");
    }
    else if (status == NEG_OVERLAP)
    {
        style = "background-color: blue; color black; font-weight: bold;\n";
        btext = tr("Negative Overlap");
    }
    else
    {
        style = "background-color: lightGrey; color lightGrey\n";
        btext = tr("No Overlap");
    }

    ui->overlapIndicator->setStyleSheet(style);
    ui->overlapIndicator->setText(btext);

    update();
}


void RotatorMainWindow::toggleOverLapDisplay(bool toggle)
{
    // don't display overlap if rotator doesn't support or user turned off overlap
//    ui->overlap->setVisible(toggle);
    ui->overlapIndicator->setVisible(toggle);

}


void RotatorMainWindow::currentAntennaSettingChanged(QString antennaName)
{

    switch( QMessageBox::question(
                        this,
                        tr("Minos Rotator"),
                        tr("The settings for the current antenna have been changed. \nDo you want to reload the settings for the antenna now?"),
                        QMessageBox::Yes |
                        QMessageBox::No |
                        QMessageBox::Cancel,
                         QMessageBox::Cancel ) )
    {
        case QMessageBox::Yes:
            if (selectAntenna->currentText() != antennaName)
            {
                bool ok;
                selectAntenna->setCurrentIndex(setupAntenna->currentAntenna.antennaNumber.toInt(&ok, 10));
            }
            upDateAntenna();
            break;
        case QMessageBox::No:

            break;
        case QMessageBox::Cancel:

            break;
        default:

            break;
    }
}


void RotatorMainWindow::updateSelectAntennaBox()
{
    int curidx = selectAntenna->currentIndex();
    selectAntenna->clear();
    initSelectAntennaBox();
    selectAntenna->setCurrentIndex(curidx);
}


void RotatorMainWindow::onLaunchSetup()
{
    setupAntenna->setTabToCurrentAntenna();
    setupAntenna->loadAvailComports();
    setupAntenna->exec();
}

void RotatorMainWindow::onPSTRotatorConfig()
{

    PstConfigDialog pstConfigDialog;
    pstConfigDialog.setWindowTitle(tr("PST Rotator Config"));
    int res = pstConfigDialog.exec();
    if ( res == QDialog::Accepted )
    {
        QString fileName = PST_CONFIG_FILE;
        QSettings  config(fileName, QSettings::IniFormat);

        config.beginGroup("PSTRotatorPath");

        if (pstConfigDialog.getPstRotatorFilePathx86Text() != config.value("pstRotatorPathx86", "C:/Program Files (x86)/PstRotator/").toString())
        {
            trace(QString("pstRotatorPathx86 has been changed to %1").arg(pstConfigDialog.getPstRotatorFilePathx86Text()));
            config.setValue("pstRotatorPathx86", pstConfigDialog.getPstRotatorFilePathx86Text());
        }

        if (pstConfigDialog.getpstRotatorFilePathText() != config.value("pstRotatorPath", "C:/Program Files/PstRotator/").toString())
        {
            trace(QString("pstRotatorPath has been changed to %1").arg(pstConfigDialog.getpstRotatorFilePathText()));
            config.setValue("pstRotatorPath", pstConfigDialog.getpstRotatorFilePathText());
        }

        if (pstConfigDialog.getPstRotatorAzFilePathx86Text() != config.value("pstRotatorAzPathx86", "C:/Program Files (x86)/PstRotatorAz/").toString())
        {
            trace(QString("pstRotatorAzPathx86 has been changed to %1").arg(pstConfigDialog.getPstRotatorAzFilePathx86Text()));
            config.setValue("pstRotatorAzPathx86", pstConfigDialog.getPstRotatorAzFilePathx86Text());
        }

        if (pstConfigDialog.getPstRotatorAzFilePathText() != config.value("pstRotatorAzPath", "C:/Program Files/PstRotatorAz/").toString())
        {
            trace(QString("pstRotatorAzPath has been changed to %1").arg(pstConfigDialog.getPstRotatorAzFilePathText()));
            config.setValue("pstRotatorAzPath", pstConfigDialog.getPstRotatorAzFilePathText());
        }

        config.endGroup();

    }

}


void RotatorMainWindow::cwCCWControlVisible(bool visible)
{
    ui->rot_left_button->setVisible(visible);
    ui->rot_right_button->setVisible(visible);
}


void RotatorMainWindow::readTraceLogFlag()
{
    QString fileName;
    if (appName == "")
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOCAL + MINOS_ROTATOR_CONFIG_FILE;
    }
    else
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_ROTATOR_CONFIG_FILE;
    }


    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("TraceLog");
    bool state = config.value("TraceLog", false).toBool();
    config.endGroup();

    ui->traceDataComms->setChecked(state);
//    ui->actionTraceComms->setChecked(state);
    traceCommsFlag = state;             // set state of trace hamlib comms
}

void RotatorMainWindow::saveTraceLogFlag(bool state)
{

    // set state of hamlib commms tracing
    if (rotator != nullptr)
    {
       rotator->setTraceComms(state);
    }


    // save to ini for restart

    QString fileName;
    if (appName == "")
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOCAL + MINOS_ROTATOR_CONFIG_FILE;
    }
    else
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_ROTATOR_CONFIG_FILE;
    }

    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("TraceLog");

    config.setValue("TraceLog", state);

    config.endGroup();
    trace("Tracelog Changed in " + fileName + " = " + QString::number(state));
}




/**************************** Quick Preset Buttons **************************/


void RotatorMainWindow::initPresetButtons()
{

    QList<QToolButton*> ui_presetbuttons;
    ui_presetbuttons << ui->presetButton0 << ui->presetButton1 << ui->presetButton2 << ui->presetButton3 << ui->presetButton4
                     << ui->presetButton5 << ui->presetButton6 << ui->presetButton7 << ui->presetButton8 << ui->presetButton9;

    QStringList buttonLabels;
    for (unsigned int i = 0; i < sizeof(RotPresetData::presetButtonLabels)/sizeof(const char *); i++)
    {
        buttonLabels.append(RotPresetData::tr(RotPresetData::presetButtonLabels[i]));
    }

    for (int i = 0; i < ui_presetbuttons.count(); i++)
    {

        presetButton.append(new PresetButton(ui_presetbuttons[i], i, shortCutKeyList[i], shiftShortCutKeyList[i], buttonLabels));

        connect(presetButton[i], &PresetButton::presetShortCutRecall, this, [this, i]() {presetRead(i);});
        connect(presetButton[i], &PresetButton::presetShiftShortCutRecall, this, [this, i]() {showPresetMenu(i);});
        connect(presetButton[i], &PresetButton::presetReadAction, this, [this, i]() {presetRead(i);});
        connect(presetButton[i], &PresetButton::presetEditAction, this, [this, i]() {presetEdit(i);});
        connect(presetButton[i], &PresetButton::presetWriteAction, this, [this, i]() {presetWrite(i);});
        connect(presetButton[i], &PresetButton::presetClearAction, this, [this, i]() {presetClear(i);});


    }

}

void RotatorMainWindow::showPresetMenu(int buttonNumber)
{
    presetButton[buttonNumber]->showButtonMenu();
}


void RotatorMainWindow::presetRead(int buttonNumber)
{
    if (!rotPresets.isEmpty()  && buttonNumber < rotPresets.count())
    {
        if (!rotPresets[buttonNumber]->bearing.isEmpty())
        {
            rotateTo(rotPresets[buttonNumber]->bearing.toInt());
            ui->bearingEdit->setText(rotPresets[buttonNumber]->bearing);
            ui->bearingEdit->setFocus();
        }
    }

}

void RotatorMainWindow::presetEdit(int buttonNumber)
{


    if (!rotPresets.isEmpty()  && buttonNumber < rotPresets.count())
    {
        RotPresetData editData(buttonNumber, rotPresets[buttonNumber]->name, rotPresets[buttonNumber]->bearing);
        RotPresetData curData(buttonNumber, rotPresets[buttonNumber]->name, rotPresets[buttonNumber]->bearing);

        logMessage(QString("Preset Edit Selected = %1").arg(QString::number(buttonNumber + 1)));
        RotPresetDialog presetDialog(this, buttonNumber, &editData, &curData, "Edit");


        if (presetDialog.exec() == QDialog::Accepted)
        {
            if (editData.name != curData.name || editData.bearing != curData.bearing)
            {
                setRotPresetButData(buttonNumber, editData);
                rotPresetButtonUpdate(buttonNumber, editData);
            }

        }
    }


}

void RotatorMainWindow::presetClear(int buttonNumber)
{
    logMessage(QString("Preset Clear Selected = %1").arg(QString::number(buttonNumber +1)));
    if (!rotPresets.isEmpty() && buttonNumber < rotPresets.count())
    {
        // clear this preset
        RotPresetData pData(0, "", "0");
        rotPresetButtonUpdate(buttonNumber, pData);
        rotPresetButtonUpdate(buttonNumber, pData);
    }
}

/*

void RotatorMainWindow::presetButtonUpdate(int buttonNumber)
{

}
*/

void RotatorMainWindow::presetWrite(int buttonNumber)
{
    logMessage(QString("Preset Write Selected = %1").arg(QString::number(buttonNumber +1)));
    if (!rotPresets.isEmpty()  && buttonNumber < rotPresets.count())
    {
        RotPresetData editData(buttonNumber, "", "0");
        RotPresetData curData(buttonNumber, "", "0");

        logMessage(QString("Preset Edit Selected = %1").arg(QString::number(buttonNumber + 1)));
        RotPresetDialog presetDialog(this, buttonNumber, &editData, &curData, "New");


        if (presetDialog.exec() == QDialog::Accepted)
        {
            if (editData.name != curData.name || editData.bearing != curData.bearing)
            {
                setRotPresetButData(buttonNumber, editData);
                rotPresetButtonUpdate(buttonNumber, editData);
            }

        }
    }
}


void RotatorMainWindow::setRotPresetButData(int buttonNumber, RotPresetData& editData)
{
    rotPresets[buttonNumber]->name = editData.name;
    rotPresets[buttonNumber]->bearing = editData.bearing;
    saveRotPresetButton(editData);
}


void RotatorMainWindow::rotPresetButtonUpdate(int buttonNumber, RotPresetData& editData)
{
    presetButton[buttonNumber]->presetButton->setText(QString("%1: %2\r\n%3").arg(QString::number(buttonNumber + 1), editData.name, editData.bearing) );
    QString tTipStr = tr("Bearing = %1").arg(editData.bearing);
    presetButton[buttonNumber]->presetButton->setToolTip(tTipStr);
}

void RotatorMainWindow::saveRotPresetButton(RotPresetData& editData)
{
    QString msg;
    msg = QString("%1:%2:%3").arg(QString::number(editData.number), editData.name, editData.bearing);
    savePreset(editData);
    sendPresetListLogger();
    //emit sendRotatorPreset(msg);
}


void RotatorMainWindow::readPresets()
{
    QSettings config("./Configuration/MinosRotatorConfig.ini", QSettings::IniFormat);
    config.beginGroup("Presets");
    if (presetButton.count() > 0)
    {
        for (int i = 0; i < presetButton.count(); i++)
        {
        rotPresets.append(new RotPresetData(i, config.value("preset" +  QString::number(i+1)).toString(),
                                        config.value("bearing" +  QString::number(i+1)).toString()));
    }
    }


    config.endGroup();
}


void RotatorMainWindow:: savePreset(RotPresetData& editData)
{


    QSettings config("./Configuration/MinosRotatorConfig.ini", QSettings::IniFormat);
    config.beginGroup("Presets");
        config.setValue("preset" + QString::number(editData.number + 1), editData.name);
        config.setValue("bearing" + QString::number(editData.number + 1), editData.bearing);
    config.endGroup();
        //emit updatePresetButtonLabels();
}



void RotatorMainWindow::refreshPresetLabels()
{
    readPresets();
    if (rotPresets.count() > 0)
    {
        for (int i = 0; i < rotPresets.count(); i++)
        {
            if (rotPresets[i]->name != "" || rotPresets[i]->name != presetButton[i]->getText())
            {
                RotPresetData d = RotPresetData(i, rotPresets[i]->name, rotPresets[i]->bearing);
                rotPresetButtonUpdate(i, d);
                //presetButton[i]->setShortcut(presetShortCut[i]);     // restore the shortcut
            }
        }
    }
    sendPresetListLogger();

}



void RotatorMainWindow::updatePresetLabels()
{
    refreshPresetLabels();
//    update();
}

// this is used to allow entry of bearings for test

void RotatorMainWindow::checkTestBearingBox()
{
    QSettings config(CONFIGURATION_FILEPATH_LOGGER + MINOS_ROTATOR_CONFIG_FILE, QSettings::IniFormat);
    config.beginGroup("TestBearings");

    if (config.value("testbearings", false).toBool())
    {
        ui->testBearing->setVisible(true);
        ui->testBearingChkbox->setVisible(true);
        logMessage(QString("enabling test bearing box"));
    }
    else
    {
        ui->testBearing->setVisible(false);
        ui->testBearingChkbox->setVisible(false);
    }
    config.endGroup();
}


void RotatorMainWindow::onTestBearingEnter()
{


    bool ok = false;
    int brg = ui->testBearing->text().toInt(&ok);

    if (ok)
    {
        logMessage(QString("Test Bearing = %1").arg(brg));
        displayBearing(brg);
    }
}




void RotatorMainWindow::aboutRotatorConfig()
{


    QString msg = tr("*** Rotator Configuration ***\n\n");

    if (setupAntenna->currentAntenna.antennaName != "")
    {
        RotCapabilities rotCap = rotFactory->supported_rotators()->value(setupAntenna->currentAntenna.rotatorModel);

        msg.append(tr("App Instance Name  = %1\n\n").arg(appName));
        if (rotator != nullptr)
        {
            msg.append(tr("Hamlib Version = %1\n").arg(rotator->getRotLibVersion()));

        }
        else
        {
            msg.append(tr("Unable to determine library version at this time!\n"));

        }
        msg.append(tr("Antenna Name = %1\n").arg(setupAntenna->currentAntenna.antennaName));
        msg.append(tr("Antenna Number = %1\n").arg(setupAntenna->currentAntenna.antennaNumber));
        msg.append(tr("Rotator Model = %1\n").arg(setupAntenna->currentAntenna.rotatorModel));
        msg.append(tr("Rotator Number = %1\n").arg(QString::number(setupAntenna->currentAntenna.rotatorModelNumber)));
        msg.append(tr("Rotator Manufacturer = %1\n").arg(setupAntenna->currentAntenna.rotatorManufacturer));
        msg.append(tr("Rotator Type = %1\n").arg(endStopNames[setupAntenna->currentAntenna.rotType]));
        msg.append(tr("Rotator CW EndStop = %1\n").arg(QString::number(setupAntenna->currentAntenna.rotatorCWEndStop)));
        msg.append(tr("Rotator CCW EndStop = %1\n").arg(QString::number(setupAntenna->currentAntenna.rotatorCCWEndStop)));
        msg.append(tr("Rotator PortType = %1\n").arg(hamlibData::portTypeList[setupAntenna->currentAntenna.portType]));
        if (rotCap.portType == RotCapConstants::PortType::network)
        {
            msg.append(tr("Network Address = %1\n").arg(setupAntenna->currentAntenna.networkAdd));
            msg.append(tr("Network Port = %1\n").arg(setupAntenna->currentAntenna.networkPort));
        }
        if (rotCap.portType == RotCapConstants::PortType::serial)
        {
            msg.append(tr("Rotator Comport = %1\n").arg(setupAntenna->currentAntenna.comport));
            msg.append(tr("Baudrate = %1\n").arg(QString::number(setupAntenna->currentAntenna.baudrate)));
            msg.append(tr("Databits = %1\n").arg(QString::number(setupAntenna->currentAntenna.databits)));
            msg.append(tr("Stop bits = %1\n").arg(QString::number(setupAntenna->currentAntenna.stopbits)));
            msg.append(tr("Parity = %1\n").arg(serialCommonData::parityStr[setupAntenna->currentAntenna.parity]));
            msg.append(tr("Handshake = %1\n").arg(serialCommonData::handshakeStr[setupAntenna->currentAntenna.handshake]));
            msg.append(tr("ForceDTR = %1\n").arg(serialCommonData::forceLinesStr[setupAntenna->currentAntenna.forceDtr]));
            msg.append(tr("ForceRTS = %1\n").arg(serialCommonData::forceLinesStr[setupAntenna->currentAntenna.forceRts]));
        }
        msg.append(tr("Antenna Offset = %1\n").arg(QString::number(setupAntenna->currentAntenna.antennaOffset)));
        msg.append(tr("Current Rotator Type = %1\n").arg(endStopNames[setupAntenna->currentAntenna.endStopType]));
        msg.append(tr("Current Max Azimuth = %1\n").arg(QString::number(setupAntenna->currentAntenna.max_azimuth)));
        msg.append(tr("Current Min Azimuth = %1\n").arg(QString::number(setupAntenna->currentAntenna.min_azimuth)));
        msg.append(tr("South Stop Type = %1\n").arg(southStopNames[setupAntenna->currentAntenna.southStopType]));
        msg.append(tr("Overrun flag = %1\n").arg(overLapActiveflag ? "True" : "False"));
        msg.append(tr("Support CW and CCW Commands = %1\n").arg(setupAntenna->currentAntenna.supportCwCcwCmd ? "True" : "False"));
        msg.append(tr("Simulate CW and CCW Commands selected = %1\n").arg(setupAntenna->currentAntenna.simCwCcwCmd ? "True" : "False"));
        msg.append(tr("Rotator Polltime = %1\n").arg(setupAntenna->currentAntenna.pollInterval));
        msg.append(tr("Tracelog = %1\n").arg(ui->traceDataComms->isChecked() ? "True" : "False"));


    }
    else
    {
        msg.append(tr("No Antenna Selected\n"));
    }



    QMessageBox::about(this, tr("Minos Rotator"), msg);

}



void RotatorMainWindow::dumpRotatorToTraceLog()
{

    RotCapabilities rotCap = rotFactory->supported_rotators()->value(setupAntenna->currentAntenna.rotatorModel);

    if (setupAntenna->currentAntenna.antennaName != "")
    {

        trace("*** Antenna Selected ***");
        trace(QString("App Instance Name  = %1").arg(appName));
        if (rotator != nullptr)
        {
            trace(QString("Library Version = %1").arg(rotator->getRotLibVersion()));

        }
        else
        {
            trace(QString("Unable to determine library version at this time!"));

        }

        trace(QString("Antenna Name = %1").arg(setupAntenna->currentAntenna.antennaName));
        trace(QString("Antenna Number = %1").arg(setupAntenna->currentAntenna.antennaNumber));
        trace(QString("Rotator Model = %1").arg(setupAntenna->currentAntenna.rotatorModel));
        trace(QString("Rotator Number = %1").arg(QString::number(setupAntenna->currentAntenna.rotatorModelNumber)));
        trace(QString("Rotator Manufacturer = %1").arg(setupAntenna->currentAntenna.rotatorManufacturer));
        trace(QString("Rotator Type = %1").arg(endStopNames[setupAntenna->currentAntenna.rotType]));
        trace(QString("Rotator CW EndStop = %1").arg(QString::number(setupAntenna->currentAntenna.rotatorCWEndStop)));
        trace(QString("Rotator CCW EndStop = %1").arg(QString::number(setupAntenna->currentAntenna.rotatorCCWEndStop)));
        trace(QString("Rotator PortType = %1").arg(hamlibData::portTypeList[setupAntenna->currentAntenna.portType]));

        if (rotCap.portType == RotCapConstants::PortType::network)
        {
            trace(QString("Network Address = %1").arg(setupAntenna->currentAntenna.networkAdd));
            trace(QString("Network Port = %1").arg(setupAntenna->currentAntenna.networkPort));
        }
        if (rotCap.portType == RotCapConstants::PortType::serial)
        {
            trace(QString("Rotator Comport = %1").arg(setupAntenna->currentAntenna.comport));
            trace(QString("Baudrate = %1").arg(QString::number(setupAntenna->currentAntenna.baudrate)));
            trace(QString("Databits = %1").arg(QString::number(setupAntenna->currentAntenna.databits)));
            trace(QString("Stop bits = %1").arg(QString::number(setupAntenna->currentAntenna.stopbits)));
            trace(QString("Parity = %1").arg(serialCommonData::parityStr[setupAntenna->currentAntenna.parity]));
            trace(QString("Handshake = %1").arg(serialCommonData::handshakeStr[setupAntenna->currentAntenna.handshake]));
            trace(QString("ForceDTR = %1").arg(serialCommonData::forceLinesStr[setupAntenna->currentAntenna.forceDtr]));
            trace(QString("ForceRTS = %1").arg(serialCommonData::forceLinesStr[setupAntenna->currentAntenna.forceRts]));


        }
        trace(QString("Antenna Offset = %1").arg(QString::number(setupAntenna->currentAntenna.antennaOffset)));
        trace(QString("Current Rotator Type = %1").arg(endStopNames[setupAntenna->currentAntenna.endStopType]));
        trace(QString("Current Max Azimuth = %1").arg(QString::number(setupAntenna->currentAntenna.max_azimuth)));
        trace(QString("Current Min Azimuth = %1").arg(QString::number(setupAntenna->currentAntenna.min_azimuth)));
        trace(QString("South Stop Type = %1\n").arg(southStopNames[setupAntenna->currentAntenna.southStopType]));
        trace(QString("Overrun flag = %1").arg(overLapActiveflag ? "True" : "False"));
        trace(QString("Support CW and CCW Commands = %1").arg(setupAntenna->currentAntenna.supportCwCcwCmd ? "True" : "False"));
        trace(QString("Simulate CW and CCW Commands selected = %1").arg(setupAntenna->currentAntenna.simCwCcwCmd ? "True" : "False"));
        trace(QString("Rotator Polltime = %1").arg(setupAntenna->currentAntenna.pollInterval));
        trace(QString("Tracelog = %1").arg(ui->traceDataComms->isChecked() ? "True" : "False"));


    }
    else
    {
        trace(QString("No Antenna Selected"));
    }

}

void RotatorMainWindow::on_reconnectPushButton_clicked()
{
    refreshAntenna();
}


void RotatorMainWindow::on_setupAntennas_clicked()
{
    onLaunchSetup();
}


void RotatorMainWindow::on_aboutRC_clicked()
{
    aboutRotatorConfig();
}


void RotatorMainWindow::on_logHeadings_clicked()
{
    setupLog->loadLogConfig();
}


void RotatorMainWindow::on_PSTConfig_clicked()
{
    onPSTRotatorConfig();
}


void RotatorMainWindow::on_traceDataComms_stateChanged(int /*arg1*/)
{
    saveTraceLogFlag(ui->traceDataComms->isChecked());
}

