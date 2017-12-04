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

#include "base_pch.h"
#include "RPCCommandConstants.h"
#include "rotatorCommonConstants.h"
#include "rotatorRpc.h"
#include "rotatorlog.h"
#include "rotatormainwindow.h"
#include "ui_rotatormainwindow.h"
#include "minoscompass.h"
#include "rotcontrol.h"
#include "setupdialog.h"
#include "editpresetsdialog.h"
#include "logdialog.h"
#include <QString>
#include <QLabel>
#include <QMessageBox>
#include <QSignalMapper>
#include <QTimer>
#include <QTime>
#include <QSettings>
#include <QProcessEnvironment>
#include <QtDebug>

// QPushButton:clicked{\n	background-color: red;\n	border-style: outset;\n	border-width: 1px;\n	border-radius: 5px;\n	border-color: black;\n	min-width: 5em;\n	padding: 3px;\n}

RotatorMainWindow *MinosRotatorForm;

static QStringList presetShortCut = {QString("Ctrl+1"),QString("Ctrl+2"),
                            QString("Ctrl+3"), QString("Ctrl+4"),
                            QString("Ctrl+5"), QString("Ctrl+6"),
                            QString("Ctrl+7"), QString("Ctrl+8"),
                            QString("Ctrl+9"), QString("Ctrl+0")};


RotatorMainWindow::RotatorMainWindow(QWidget *parent) :
    QMainWindow(parent),
    msg(0),
    ui(new Ui::RotatorMainWindow),
    rot_left_button_status(OFF),
    rot_right_button_status(OFF),
    turn_button_status(OFF),
    brakeflag(false),
    moving(false),
    movingCW(false),
    movingCCW(false),
    cwCcwCmdflag(false),
    stopCmdflag(false),
    rotCmdflag(false),
    reqBearCmdflag(false),
    overLapActiveflag(false),
    overLapStatus(NO_OVERLAP),
    southStopActiveflag(false),
    endStopType(ROT_0_360),
    rotErrorFlag(false),
    supportCwCcwCmd(false),
    rotLogFlg(true)


{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);


    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    // get the antenna name from host process
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    QDir dir(".");
    trace(QString("Directory %1").arg(dir.absolutePath()));

    createCloseEvent();
    MinosRotatorForm = this;
    connect(&LogTimer, SIGNAL(timeout()), this, SLOT(LogTimerTimer()));
    LogTimer.start(100);
    msg = new RotatorRpc(this);

    QSettings settings;
    geoStr = "geometry";
    if (appName.length() > 0)
    {
        geoStr = geoStr + appName;
    }
    QByteArray geometry = settings.value(geoStr).toByteArray();
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
    ui->actionHelp->setVisible(false);
    ui->actionSkyScan->setVisible(false);
    ui->actionAlways_On_Top->setVisible(false);

    presetButtons[0] = ui->presetButton1;
    presetButtons[1] = ui->presetButton2;
    presetButtons[2] = ui->presetButton3;
    presetButtons[3] = ui->presetButton4;
    presetButtons[4] = ui->presetButton5;
    presetButtons[5] = ui->presetButton6;
    presetButtons[6] = ui->presetButton7;
    presetButtons[7] = ui->presetButton8;
    presetButtons[8] = ui->presetButton9;
    presetButtons[9] = ui->presetButton10;


    QSignalMapper *preset_mapper = new QSignalMapper(this);

    for (int i = 0; i < NUM_PRESETS; i++ )
    {
        preset_mapper->setMapping(presetButtons[i], i);
        connect(presetButtons[i], SIGNAL(clicked()), preset_mapper, SLOT(map()));

    }
    connect(preset_mapper, SIGNAL(mapped(int)), this, SLOT(clickedPreset(int)));


    for (int i = 0; i < NUM_PRESETS; i++)
    {
        presetButtons[i]->setShortcut(presetShortCut[i]);
    }


    rotator = new RotControl();
    rotator->getRotatorList();
    selectRotator = new SetupDialog(rotator);
    editPresets = new EditPresetsDialog;
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
    offSetlbl->setText("Offset: ");
    ui->statusbar->addPermanentWidget(offSetDisplay);

    ui->statusbar->addPermanentWidget(actualRotatorlbl);
    actualRotatorlbl->setText("Actual: ");
    ui->statusbar->addPermanentWidget(actualRotatorDisplay);

    ui->statusbar->addPermanentWidget(rawRotatorlbl);
    rawRotatorlbl->setText("RawRot: ");
    ui->statusbar->addPermanentWidget(rawRotatorDisplay);

    ui->overlaplineEdit->setFixedSize(60,20);

    rot_left_button_off();
    rot_right_button_off();

    rotator->set_serialConnected(false);

    refreshPresetLabels();
    initActionsConnections();

    selectAntenna = ui->selectAntennaBox;

    selectRotator->setAppName(appName);
/*
    if (loggerAntenna.length() > 0)
    {
        ui->selectAntennaBox->hide();
        ui->antennaSelectlbl->hide();
        ui->antennaSelectln->hide();
    }
*/
    brakedelay = 1 * 1000;


    rotatorBearing = COMPASS_ERROR; // force first update



    //openRotator();

    setPolltime(POLLTIME);   // to allow variable controller polltime - not implemented!
    rotTimeCount = 0;
    RotateTimer.start(200);  // to set timeout for antenna rotating



    rotlog->getBearingLogConfig();
    readTraceLogFlag();

    //logAntError = false;
    initSelectAntennaBox();
/*
    if (appName.length() > 0)
    {
        logMessage(QString("AppName %1 from logger").arg(appName));
        int a = selectAntenna->findText(loggerAntenna);
        if (a == -1)
        {
            logMessage("Select an antenna in rotator control");
            showStatusMessage("<font color='Red'>Please select an antenna!</font>");
            logAntError = true;
        }
        else
        {
            selectAntenna->setCurrentIndex(a);
            selectRotator->readSettings();      // get antenna settings
            selectRotator->copyAntennaToCurrent(a);
        }

    }
    else
    {
        selectRotator->readCurrentAntenna();
        selectAntenna->setCurrentIndex(selectAntenna->findText(selectRotator->currentAntenna.antennaName));
    }

*/



    if (appName.length() > 0)
    {
        logMessage((QString("Read Current Antenna for AppName %1 from logger").arg(appName)));

    }
    else
    {
        logMessage((QString("Read Current Antenna for Local selection")));

    }

    selectRotator->readCurrentAntenna();

    if (selectRotator->currentAntenna.antennaName == "")
    {
        logMessage(QString("No antenna selected for this appName, %1").arg(appName));
        QString errmsg = "<font color='Red'>Please select an antenna!</font>";
        showStatusMessage(errmsg);
        statusMsg = errmsg;
        sendStatusLogger();
    }
    else
    {
        selectAntenna->setCurrentIndex(selectAntenna->findText(selectRotator->currentAntenna.antennaName));
    }

    upDateAntenna();


    trace("*** Rotator Started ***");





}

RotatorMainWindow::~RotatorMainWindow()
{

    delete msg;
    delete ui;
}


void RotatorMainWindow::logMessage( QString s )
{
   if (ui->actionTraceLog->isChecked())
        trace( s );
}

void RotatorMainWindow::onStdInRead(QString cmd)
{
    trace("Command read from stdin: " + cmd);
    if (cmd.indexOf("ShowServers", Qt::CaseInsensitive) >= 0)
        setShowServers(true);
    if (cmd.indexOf("HideServers", Qt::CaseInsensitive) >= 0)
        setShowServers(false);
}

void RotatorMainWindow::closeEvent(QCloseEvent *event)
{


    closeRotator();

    // and tidy up all loose ends

    QSettings settings;
    settings.setValue(geoStr, saveGeometry());
    trace("MinosRotator Closing");
    QWidget::closeEvent(event);

}



void RotatorMainWindow::resizeEvent(QResizeEvent * event)
{

    QSettings settings;
    settings.setValue(geoStr, saveGeometry());

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



void RotatorMainWindow::onLoggerSetRotation(int direction, int angle)
{

    logMessage("Command From Logger command number = " + QString::number(direction) + ", angle = " + QString::number(angle));
    int dirCommand = direction;
    if (rotator->get_serialConnected() && !rotErrorFlag)
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





void RotatorMainWindow::openRotator()
{

    int retCode = 0;

    if (selectRotator->currentAntenna.antennaName == "")
    {
        logMessage(QString("Open Rotator: No rotator name!"));
        showStatusMessage("Please select an Antenna");
        return;
    }
    if (rig_port_e(selectRotator->currentAntenna.portType) == RIG_PORT_SERIAL)
    {
        if(selectRotator->comportAvial(selectRotator->currentAntenna.antennaNumber.toInt(), selectRotator->currentAntenna.comport) == -1)
        {
            logMessage(QString("Open Rotator: Check comport - defined port %1 not available on computer").arg(selectRotator->currentAntenna.comport));
            showStatusMessage(QString("Comport %1 no longer configured on computer?").arg(selectRotator->currentAntenna.comport));
            return;
        }

        if (selectRotator->currentAntenna.comport == "")
        {
            logMessage(QString("Open Rotator: No comport"));
            showStatusMessage("Please select a Comport");
            return;
        }

    }
    if (rig_port_e(selectRotator->currentAntenna.portType) == RIG_PORT_NETWORK || rig_port_e(selectRotator->currentAntenna.portType == RIG_PORT_UDP_NETWORK))
    {
        if (selectRotator->currentAntenna.networkAdd == "" || (selectRotator->currentAntenna.networkPort == ""))
        {
            logMessage(QString("Open Rotator: No network or Port Number"));
            showStatusMessage("Please enter a network Address and Port Number");
            return;
        }

    }
    if (selectRotator->currentAntenna.rotatorModel == "")
    {
        logMessage(QString("Open Rotator: No rotator model"));
        showStatusMessage("Please select a rotator model");
        return;
    }


    srotParams p = selectRotator->getCurrentAntenna();

    retCode = rotator->init(selectRotator->currentAntenna);
    if (retCode < 0)
    {
        hamlibError(retCode, "Rotator Init");
    }
    if (rotator->get_serialConnected())
    {

        pollTimer->start(pollTime);             // start timer to send message to controller
        if (rig_port_e(selectRotator->currentAntenna.portType) == RIG_PORT_SERIAL)
        {
            showStatusMessage(tr("Connected to: %1 - %2, %3, %4, %5, %6, %7, %8")
                                  .arg(p.antennaName).arg(p.rotatorModel).arg(p.comport).arg(p.baudrate).arg(p.databits)
                                  .arg(p.stopbits).arg(rotator->getParityCodeNames()[p.parity]).arg(rotator->getHandShakeNames()[p.handshake]));
        }
        else if (rig_port_e(selectRotator->currentAntenna.portType == RIG_PORT_NETWORK || rig_port_e(selectRotator->currentAntenna.portType == RIG_PORT_UDP_NETWORK)))
        {
            showStatusMessage(QString("Connected to: %1 - %2, %3").arg(selectRotator->currentAntenna.antennaName).arg(selectRotator->currentAntenna.rotatorModel).arg(selectRotator->currentAntenna.networkAdd + ":" + selectRotator->currentAntenna.networkPort));
        }
        else if (rig_port_e(selectRotator->currentAntenna.portType) == RIG_PORT_NONE)
        {
                showStatusMessage(QString("Connected to: %1 - %2").arg(selectRotator->currentAntenna.antennaName).arg(selectRotator->currentAntenna.rotatorModel));
        }

        //sendStatusToLogConnected();
    }
    else
    {
//        QMessageBox::critical(this, tr("Error"), serial->errorString());
        pollTimer->stop();

        stopRotation(false);           // clear flags
        showStatusMessage(tr("Rotator Open error"));
        sendStatusToLogDisConnected();
    }


}

void RotatorMainWindow::closeRotator()
{
    if (moving || movingCW || movingCCW)
    {
        stop_rotation();
    }
    rotator->closeRotator();
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
   logMessage(QString("Send %1 message to logger, appName = %2").arg(message).arg(appName));
   if (appName.length() > 0)
   {
        msg->publishState(message);
   }
}


void RotatorMainWindow::sendAntennaNameLogger(const QString antennaName)
{
    if (appName.trimmed() == antennaName.trimmed() || appName.length() == 0)
    {
        msg->publishAntennaName(antennaName);
    }
    else
    {
        msg->publishAntennaName(QString("%1 : %2").arg(appName, antennaName));
    }

}



void RotatorMainWindow::initActionsConnections()
{

    connect(ui->selectAntennaBox, SIGNAL(activated(int)), this, SLOT(upDateAntenna()));
    connect(selectRotator, SIGNAL(antennaNameChange()), this, SLOT(updateSelectAntennaBox()));
    connect(ui->actionTraceLog, SIGNAL(changed()), this, SLOT(saveTraceLogFlag()));
    connect(ui->turnButton, SIGNAL(clicked(bool)), this, SLOT(rotateToController()));
    connect(ui->bearingEdit, SIGNAL(returnPressed()), this, SLOT(rotateToController()));
    connect(this, SIGNAL(presetRotateTo()), this, SLOT(rotateToController()));
    connect(this, SIGNAL(presetRotateTo()), ui->bearingEdit, SLOT(selectAll()));
    connect(this, SIGNAL(presetRotateTo()), ui->bearingEdit, SLOT(setFocus()));
    connect(ui->turnButton, SIGNAL(clicked(bool)), ui->bearingEdit, SLOT(selectAll()));
    connect(ui->turnButton, SIGNAL(clicked(bool)), ui->bearingEdit, SLOT(setFocus()));
    connect(ui->stopButton, SIGNAL(clicked(bool)), ui->bearingEdit, SLOT(selectAll()));
    connect(ui->stopButton, SIGNAL(clicked(bool)), ui->bearingEdit, SLOT(setFocus()));
    connect(ui->bearingEdit, SIGNAL(returnPressed()), ui->bearingEdit, SLOT(setFocus()));
    connect(ui->bearingEdit, SIGNAL(returnPressed()), ui->bearingEdit, SLOT(selectAll()));
    connect(ui->stopButton, SIGNAL(clicked(bool)), this, SLOT(stopButton()));
    connect(ui->rot_right_button, SIGNAL(clicked(bool)), this, SLOT(rotateCW(bool)));
    connect(ui->rot_left_button, SIGNAL(clicked(bool)), this, SLOT(rotateCCW(bool)));
    connect(this, SIGNAL(escapePressed()), this, SLOT(stop_rotation()));

    // display bearing
    connect(pollTimer, SIGNAL(timeout()), this, SLOT(request_bearing()));
    connect(rotator, SIGNAL(bearing_updated(int)), this, SLOT(displayBearing(int)));
    connect(this, SIGNAL(sendCompassDial(int)), ui->compassDial, SLOT(compassDialUpdate(int)));
    connect(this, SIGNAL(sendBearing(QString)), ui->bearingDisplay, SLOT(setText(const QString &)));
    connect(this, SIGNAL(sendBackBearing(QString)), ui->backBearingDisplay, SLOT(setText(const QString &)));
    connect(this, SIGNAL(displayActualBearing(QString)), actualRotatorDisplay, SLOT( setText(const QString &)));
    connect(this, SIGNAL(displayOverlap(overlapStat)), this ,SLOT(overLapDisplayBox(overlapStat)));
    // check endstop and turn to rotation stop
    connect(&RotateTimer, SIGNAL(timeout()), this, SLOT(rotatingTimer()));
    connect(this, SIGNAL(checkingEndStop()), this, SLOT(checkEndStop()));
    connect(rotator, SIGNAL(bearing_updated(int)), this, SLOT(checkMoving(int)));
    //connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(closeSerialPort()));


    // setup antennas
    connect(ui->actionSetup_Antennas, SIGNAL(triggered()), selectRotator, SLOT(show()));
    connect(selectRotator, SIGNAL(currentAntennaSettingChanged(QString)), this, SLOT(currentAntennaSettingChanged(QString)));

    // setup presets
    connect(editPresets, SIGNAL(showEditPresetDialog()), editPresets, SLOT(show()));
    connect(editPresets, SIGNAL(updatePresetButtonLabels()), this, SLOT(updatePresetLabels()));
    connect(ui->actionEdit_Presets, SIGNAL(triggered()), editPresets, SLOT(loadPresetEditFieldsShow()));
    // Bearing Log
    connect(ui->actionLog_Heading, SIGNAL(triggered()), setupLog, SLOT(loadLogConfig()));
    connect(setupLog, SIGNAL(showLogDialog()), setupLog, SLOT(show()));
    connect(setupLog, SIGNAL(bearingLogConfigChanged()), rotlog, SLOT(getBearingLogConfig()));
    connect(rotator, SIGNAL(bearing_updated(int)), rotlog, SLOT(saveBearingLog(int)));

    connect(rotator, SIGNAL(debug_protocol(QString)), this, SLOT(logMessage(QString)));

    // Message from Logger
    connect(msg, SIGNAL(setRotation(int,int)), this, SLOT(onLoggerSetRotation(int,int)));


    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAbout_Rotator_Config, SIGNAL(triggered()), this, SLOT(aboutRotatorConfig()));
    connect(ui->actionExit , SIGNAL(triggered()), this, SLOT(close()));



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

    rotatorBearing = bearing;

    curBearingWithOffset = bearing + selectRotator->currentAntenna.antennaOffset;

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
    //QString s = QString::number(displayBearing);
    if (appName.length() > 0)
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
        QString s = QString("%1:%2:%3").arg(QString::number(displayBearing)).arg(QString::number(rotatorBearing).arg(ol));
        msg->publishBearing(s);
    }

    //qDebug() << QString("Bearing = %1").arg(displayBearing);


//    QString rotatorBearingmsg = QString::number(rotatorBearing);
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


void RotatorMainWindow::dispRawRotBearing(int rotatorBearing)
{
    rawRotatorDisplay->setText(QString::number(rotatorBearing));
}



void RotatorMainWindow::clickedPreset(int buttonNumber)
{

    if (presetName[buttonNumber] != "")
    {
        if (presetBearing[buttonNumber] != "")
        {
           ui->bearingEdit->setText(presetBearing[buttonNumber]);
           emit presetRotateTo();
        }
    }
}






void RotatorMainWindow::readPresets()
{
    QString fileName;
    if (appName == "")
    {
        fileName = CONFIGURATION_FILEPATH_LOCAL + MINOS_ROTATOR_CONFIG_FILE;
    }
    else
    {
        fileName = CONFIGURATION_FILEPATH_LOGGER + MINOS_ROTATOR_CONFIG_FILE;
    }

    QSettings config(fileName, QSettings::IniFormat);


    //QSettings config("./Configuration/MinosRotatorConfig.ini", QSettings::IniFormat);
    config.beginGroup("Presets");
    for (int i = 0; i < NUM_PRESETS; i++)
    {
        presetName[i] = config.value("preset" +  QString::number(i+1)).toString();
        presetBearing[i] = config.value("bearing" +  QString::number(i+1)).toString();
    }
    config.endGroup();
}




void RotatorMainWindow::refreshPresetLabels()
{

    readPresets();

    for (int i = 0; i < NUM_PRESETS; i++)
    {
        if (presetName[i] != "" || presetName[i] != presetButtons[i]->text())
        {
            presetButtons[i]->setText(presetName[i]);
            presetButtons[i]->setShortcut(presetShortCut[i]);     // restore the shortcut
        }
    }
}




void RotatorMainWindow::updatePresetLabels()
{
    refreshPresetLabels();
    update();
}









void RotatorMainWindow::initSelectAntennaBox()
{
    selectAntenna->addItem("");
    for (int i= 0; i < NUM_ANTENNAS; i++)
    {
        selectAntenna->addItem(selectRotator->availAntennas[i].antennaName);
    }
}




void RotatorMainWindow::upDateAntenna()
{

    if (moving  || movingCCW || movingCW)
    {

        stopRotation(true);
    }

    int antennaIndex = ui->selectAntennaBox->currentIndex();
    if (antennaIndex > 0)
    {
        antennaIndex -= 1;
        if (selectRotator->availAntennas[antennaIndex].rotatorModelNumber == 0)
        {
            closeRotator();
            QMessageBox::critical(this, tr("Antenna Error"), "Please configure a antenna name and rotator model");
            return;
        }

        selectRotator->currentAntenna = selectRotator->availAntennas[antennaIndex];

        selectRotator->saveCurrentAntenna();


       if (rotator->get_serialConnected())
       {
           closeRotator();


       }

       writeWindowTitle(appName);

       openRotator();

       offSetDisplay->setText(QString::number(selectRotator->currentAntenna.antennaOffset));

       southStopActiveflag = selectRotator->currentAntenna.southStopFlag;

       // open rotator to get this info...
       rotatorMaxAzimuth = rotator->getMaxAzimuth();
       rotatorMinAzimuth = rotator->getMinAzimuth();

       if (rotatorMaxAzimuth == 180 && rotatorMinAzimuth == -180)
       {
           currentMaxAzimuth = rotatorMaxAzimuth;
           currentMinAzimuth = rotatorMinAzimuth;
           rotatorCWEndStop = COMPASS_HALF - 1;
           rotatorCCWEndStop = COMPASS_HALF + 1;
           endStopType = ROT_NEG180_180;
           overLapActiveflag = false;
       }
       //else if (rotator->getMaxAzimuth() > COMPASS_MAX360 && selectRotator->currentAntenna.overRunFlag)
       else if (rotatorMinAzimuth == COMPASS_MIN0 && rotatorMaxAzimuth == COMPASS_MAX360)
       {
           currentMaxAzimuth = rotatorMaxAzimuth; // + selectRotator->currentAntenna.antennaOffset;
           currentMinAzimuth = rotatorMinAzimuth;

           if (!southStopActiveflag)
           {
                // + selectRotator->currentAntenna.antennaOffset;
                rotatorCWEndStop = rotatorMaxAzimuth;
                rotatorCCWEndStop = rotatorMinAzimuth;
                endStopType = ROT_0_360;
           }
           else
           {
               rotatorCWEndStop = COMPASS_HALF - 1;
               rotatorCCWEndStop = COMPASS_HALF + 1;
               endStopType = ROT_180_180;
           }

            overLapActiveflag = false;
       }
       else if (rotatorMinAzimuth == COMPASS_MIN0 && rotatorMaxAzimuth > COMPASS_MAX360)
       {
           overLapActiveflag = selectRotator->currentAntenna.overRunFlag;
           if (!overLapActiveflag)
           {
               //override end stop and type Yaesu overlap to 360
               rotatorMaxAzimuth = COMPASS_MAX360;
               endStopType = ROT_0_360;

           }
           else
           {
               endStopType = ROT_0_450;
           }
           currentMaxAzimuth = rotatorMaxAzimuth;
           currentMinAzimuth = rotatorMinAzimuth;
           rotatorCWEndStop = rotatorMaxAzimuth;
           rotatorCCWEndStop = rotatorMinAzimuth;



       }
       else if (rotatorMinAzimuth < COMPASS_MIN0 && rotatorMaxAzimuth > COMPASS_MAX360)
       {
           currentMaxAzimuth = rotatorMaxAzimuth;
           currentMinAzimuth = rotatorMinAzimuth;
           rotatorCWEndStop = rotatorMaxAzimuth;
           rotatorCCWEndStop = rotatorMinAzimuth;
           endStopType = ROT_NEG180_540;
           overLapActiveflag = true;
       }



       // don't display overlap if rotator doesn't support or user turned off overlap
       toggleOverLapDisplay(overLapActiveflag);



       // flag if rotator supports CW and CCW commands
       supportCwCcwCmd = getCwCcwCmdFlag(selectRotator->currentAntenna.rotatorModelNumber);

       if (rotator->get_serialConnected())
       {
           sendStatusToLogConnected();
       }
       else
       {
           sendStatusToLogDisConnected();
       }

        rotatorBearing = 9999;      // force display update
       // update logger
       if (appName.length() > 0)
       {
           sendAntennaNameLogger(selectRotator->currentAntenna.antennaName);

           sendStatusToLogStop();
           msg->publishMaxAzimuth(QString::number(currentMaxAzimuth));
           msg->publishMinAzimuth(QString::number(currentMinAzimuth));
       }
    }
    else
    {   // no antenna selected
        trace("No antenna selected");
        closeRotator();
        if (appName.length() > 0)
        {
            writeWindowTitle(appName);
            sendAntennaNameLogger("No rotator");
            sendStatusToLogDisConnected();
            sendStatusToLogStop();
            msg->publishMaxAzimuth(QString::number(0));
            msg->publishMinAzimuth(QString::number(0));
        }
        else
        {
            writeWindowTitle(appName);
        }



    }

    dumpRotatorToTraceLog();




}


void RotatorMainWindow::writeWindowTitle(QString appName)
{
    if (appName.length() > 0)
    {
        this->setWindowTitle("Minos Rotator Control - " + appName + " - Logger");
    }
    else
    {
        this->setWindowTitle("Minos Rotator Control - Local");
    }

}

void RotatorMainWindow::request_bearing()
{
    logMessage(QString("Request Bearing"));
    reqBearCmdflag = true;
    int retCode = 0;
    if (brakeflag || cwCcwCmdflag || rotCmdflag) return;
    if (rotator->get_serialConnected())
    {
        retCode = rotator->request_bearing();
        logMessage(QString("Sent request bearing cmd - retcode = %1").arg(QString::number(retCode)));
        if (retCode < 0)
        {
            logMessage(QString("Request bearing: error"));
            hamlibError(retCode, "Request Bearing");
        }
    }
    reqBearCmdflag = false;
}


void RotatorMainWindow::checkEndStop()
{

    logMessage(QString("Check EndStop"));
    logMessage(QString("curBearingWithOffset = %1").arg(QString::number(curBearingWithOffset)));
    logMessage(QString("rotatorBearing = %1").arg(QString::number(rotatorBearing)));
    logMessage(QString("currentMaxAzimuth = %1").arg(QString::number(currentMaxAzimuth)));
    logMessage(QString("currentMinAzimuth = %1").arg(QString::number(currentMinAzimuth)));
    if (movingCW)
    {
        if (rotatorBearing >= currentMaxAzimuth)
        {
            logMessage(QString("Max Endstop reached!"));
            stopButton();
        }
    }
    else if (movingCCW)
    {
        if (rotatorBearing <= currentMinAzimuth)
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

    if (oldBearing != bearing)
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
            logMessage(QString("Rotator has stoped moving"));
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
    bool ok;
    int intBearing;

    rotCmdflag = true;
    QString bearing = ui->bearingEdit->text();

    intBearing = bearing.toInt(&ok, 10);

    //if (intBearing >= currentMinAzimuth && intBearing <= currentMaxAzimuth && ok)
    if (intBearing >= COMPASS_MIN0 && intBearing <= COMPASS_MAX360 && ok)
    {
        rotateTo(intBearing);
        logMessage(QString("Rotate to bearing %1").arg(bearing));
    }
    else
    {
        //QString s = "Invalid Bearing\nPlease enter " + QString::number(currentMinAzimuth) + " - " + QString::number(currentMaxAzimuth);
        QString s = QString("Invalid Bearing\nPlease enter %1 - %2").arg(QString::number(COMPASS_MIN0)).arg(QString::number(COMPASS_MAX360));
        QMessageBox::critical(this, tr("Bearing Error"), s);
    }
    rotCmdflag = false;
}


void RotatorMainWindow::rotateTo(int bearing)
{

    int retCode = 0;
    int rotateTo = bearing;
    logMessage(QString("RotateTo Bearing = %1").arg(QString::number(bearing)));

//    if (bearing > currentMaxAzimuth || bearing < currentMinAzimuth)
//    {
//        logMessage("Rotate To Bearing Error - Bearing = " + QString::number(bearing));
//        return; //error
//    }


    // adjust bearing with offset
    if (selectRotator->currentAntenna.antennaOffset < 0)
    {
        rotateTo = rotateTo - selectRotator->currentAntenna.antennaOffset;
        if (rotateTo >= COMPASS_MAX360)
        {
            rotateTo = rotateTo - COMPASS_MAX360;
        }
    }
    else
    {
        rotateTo = rotateTo - selectRotator->currentAntenna.antennaOffset;
        if (rotateTo < COMPASS_MIN0)
        {
            rotateTo = COMPASS_MAX360 + rotateTo;
        }
    }


    logMessage(QString("Rotate to Bearing = %1, adjusted with offset = %2").arg(QString::number(rotateTo)).arg(QString::number(selectRotator->currentAntenna.antennaOffset)));



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



    if (rotator->get_serialConnected())
    {

        retCode = rotator->rotate_to_bearing(rotateTo);
        if (retCode < 0)
        {
            hamlibError(retCode, "Rotate to Bearing");
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








int RotatorMainWindow::northCalcTarget(int targetBearing)
{

    int target = targetBearing;

    if (endStopType == ROT_0_360)
    {
        target = calcRotZero360(targetBearing);

    }
    else if (endStopType == ROT_NEG180_180)
    {
        target = calcRotNeg180_180(targetBearing);

    }
    else if (endStopType == ROT_0_450 || ROT_NEG180_540)
    {
        target = calclRot_0_450_Neg180_540(targetBearing);
    }

    return target;

}



int RotatorMainWindow::calcRotZero360(int targetBearing)
{
    logMessage(QString("NCalc - EndStop Type - ROT_0_360"));
    logMessage(QString("NCalc - 1 - Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing)).arg(QString::number(rotatorBearing)));
    return targetBearing;
}


int RotatorMainWindow::calcRotNeg180_180(int targetBearing)
{
    int target = targetBearing;
    logMessage(QString("NCalc - EndStop Type - ROT_NEG180_360"));
    if (targetBearing > COMPASS_HALF && targetBearing <= COMPASS_MAX360)
    {
        target = targetBearing - COMPASS_MAX360;
        logMessage(QString("NCalc - 2 - Target bearing > 180 and < 360, calculated target = %1").arg(QString::number(target)));
    }
    logMessage(QString("Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing)).arg(QString::number(rotatorBearing)));
    return target;

}

int RotatorMainWindow::calclRot_0_450_Neg180_540(int targetBearing)
{
    int target = targetBearing;
    logMessage(QString("NCalc - EndStop Type - ROT_0_450 or ROT_NEG180_540"));
    if (endStopType == ROT_NEG180_540)
    {
        if (rotatorBearing >= COMPASS_MIN0 && rotatorBearing <= COMPASS_HALF)
        {
            if (targetBearing >= COMPASS_MIN0 && targetBearing <= COMPASS_HALF)
            {
                target = targetBearing;
                logMessage(QString("NCalc - EndStop Type - ROT_NEG180_450"));
                logMessage(QString("NCalc - 4 - - Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing)).arg(QString::number(rotatorBearing)));
            }
            else if (targetBearing > COMPASS_HALF && targetBearing <= COMPASS_MAX360)
            {
                if (COMPASS_MAX360 - targetBearing + rotatorBearing < targetBearing - rotatorBearing)
                {
                    target = targetBearing - COMPASS_MAX360;
                }
                logMessage(QString("NCalc - EndStop Type - ROT_NEG180_450"));
                logMessage(QString("NCalc - 5 - - Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing)).arg(QString::number(rotatorBearing)));
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
            logMessage(QString("NCalc - 6 - - Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing)).arg(QString::number(rotatorBearing)));
            return target;
        }

    }

    if (rotatorBearing >= COMPASS_MAX360 && ((targetBearing + COMPASS_MAX360) <= currentMaxAzimuth))
    {
        target = targetBearing + COMPASS_MAX360;
        logMessage(QString("NCalc - 7 - Rotator Bearing = %1 >= 360, target bearing = %2, target bearing + 360 = %3 is < maxAzimuth = %4, calculated target = %5")
                   .arg(QString::number(rotatorBearing)).arg(QString::number(targetBearing)).arg(QString::number(targetBearing + COMPASS_MAX360))
                       .arg(QString::number(currentMaxAzimuth)).arg(QString::number(target)));
        return target;
    }
    else if (rotatorBearing >= COMPASS_MAX360 && targetBearing < COMPASS_MAX360)
    {
        target = targetBearing;
        logMessage(QString("NCalc - 8 - Rotator Bearing = %1 >= 360, target bearing = %2 is < 360, target = %3")
                   .arg(QString::number(rotatorBearing)).arg(QString::number(targetBearing)).arg(QString::number(target)));
        return target;
    }
    else if (rotatorBearing > COMPASS_HALF && rotatorBearing <= COMPASS_MAX360)
    {
        if ((COMPASS_MAX360 + targetBearing) <= currentMaxAzimuth)
        {
            if (COMPASS_MAX360 + targetBearing - rotatorBearing < rotatorBearing - targetBearing)
            {
                target = COMPASS_MAX360 + targetBearing;
            }
        }

        logMessage(QString("NCalc - 9 - Rotator Bearing = %1 > 180 and <= 360, target bearing = %2, target = %3")
                   .arg(QString::number(rotatorBearing)).arg(QString::number(targetBearing)).arg(QString::number(target)));
        return target;
    }
    else if (rotatorBearing >= COMPASS_MIN0 && rotatorBearing <= COMPASS_HALF)
    {
        target = targetBearing;
        logMessage(QString("NCalc - 10 - Rotator Bearing = %1 >= 0 and <= 180, target bearing = %2, target = %3")
                    .arg(QString::number(rotatorBearing)).arg(QString::number(targetBearing)).arg(QString::number(target)));
        return target;
    }
    return target;
}




/*
int RotatorMainWindow::northCalcTarget(int targetBearing)
{

    int target = rotatorBearing;
    logMessage("northCalcTarget - targetBearing = " + QString::number(targetBearing));
    logMessage("northCalcTarget - rotatorBearing = " + QString::number(rotatorBearing));

    if (rotatorBearing >= COMPASS_MAX360 && rotatorBearing <= currentMaxAzimuth)
    {
        if (targetBearing >= COMPASS_MAX360 && targetBearing <= currentMaxAzimuth)
        {

            target = targetBearing;
            logMessage("target1 = " + QString::number(target));
            return target;
        }
        else if (targetBearing >= COMPASS_MIN0 && targetBearing <= currentMaxAzimuth - COMPASS_MAX360)
        {
            target = targetBearing + COMPASS_MAX360;
            logMessage("target2 = " + QString::number(target));
            return target;
        }
        else if (targetBearing >= currentMaxAzimuth - COMPASS_MAX360 && targetBearing < COMPASS_MAX360)
        {
            target = targetBearing;
            logMessage("target3 = " + QString::number(target));
            return target;
        }

    }
    else if (rotatorBearing >= COMPASS_HALF && rotatorBearing < COMPASS_MAX360)
    {
        if (targetBearing >= COMPASS_HALF && targetBearing <= currentMaxAzimuth)
        {

            target = targetBearing;
            logMessage("target4 = " + QString::number(target));
            return target;
        }
        else if (targetBearing >= COMPASS_MIN0 && targetBearing < COMPASS_HALF)
        {
            if (COMPASS_MAX360 - rotatorBearing + targetBearing < rotatorBearing - targetBearing)
            {

                if (targetBearing + COMPASS_MAX360 <= currentMaxAzimuth)
                {
                    target =  targetBearing + COMPASS_MAX360;
                    logMessage("target5 = " + QString::number(target));
                    return target;
                }
                else
                {
                    target = targetBearing;
                    logMessage("target6 = " + QString::number(target));
                    return target;
                }
            }
            else
            {
                target = targetBearing;
                logMessage("target7 = " + QString::number(target));
                return target;
            }
        }
    }
    else if (rotatorBearing >= COMPASS_MIN0 && rotatorBearing <= COMPASS_HALF)
    {
        if (targetBearing > COMPASS_HALF && targetBearing <= COMPASS_MAX360)
        {
            if ((rotatorBearing + COMPASS_MAX360 - targetBearing < targetBearing - rotatorBearing) && currentMinAzimuth < COMPASS_MIN0)
            {
                target = COMPASS_MIN0 - (COMPASS_MAX360 - targetBearing);
                logMessage("target8 = " + QString::number(target));
                return target;
            }
            else
            {
                target = targetBearing;
                logMessage("target9 = " + QString::number(target));
                return target;
            }
        }
        else
        {
            target = targetBearing;
            logMessage("target10 = " + QString::number(target));
            return target;
        }
    }
    else if (rotatorBearing >= currentMinAzimuth && rotatorBearing< COMPASS_MIN0)
    {
        target = targetBearing;
        logMessage("target11 = " + QString::number(target));
        return target;
    }
    // this is to handle situation when rotator can handle overlap, but it is turned off in rotator
    // and rotator has overrun current max and min azimuth at current endstop
    else if (!overLapActiveflag)
    {
        if ((rotatorBearing < currentMinAzimuth || rotatorBearing > currentMaxAzimuth) && targetBearing <= currentMaxAzimuth && targetBearing >= currentMinAzimuth)
        {
            target = targetBearing;
            logMessage("target12 = " + QString::number(target));
            return target;
        }
    }

    logMessage("target12 (error?) = " + QString::number(target));
    return target;


}

*/


void RotatorMainWindow::stopButton()
{

    logMessage(QString("StopButton"));

    stopRotation(rotator->get_serialConnected());
}

void RotatorMainWindow::stop_rotation()
{

    stopRotation(rotator->get_serialConnected());
}

void RotatorMainWindow::stopRotation(bool sendStop)
{

    logMessage(QString("Stop Rotation"));
    int retCode = 0;
    stop_button_on();
    brakeflag = true;
    stopCmdflag = true;
    if (sendStop)
    {
        // if it is a Prosistel Rotator - to stop use rotate_to_bearing = 999
        if (selectRotator->currentAntenna.rotatorModelNumber == 1701)
        {
            logMessage(QString("Stop Rotation: Prosistel Rotator"));
            //retCode = rotator->rotate_to_bearing(999);

        }
        else
        {
            retCode = rotator->stop_rotation();
            logMessage(QString("Stop cmd sent to rotator - retcode = %1").arg(QString::number(retCode)));
        }

        if (retCode < 0)
        {
            hamlibError(retCode, "Stop Rotation");
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


void RotatorMainWindow::rotateCW(bool /*clicked*/)
{

    if (brakeflag)
    {
        return;
    }

    cwCcwCmdflag = true;
    logMessage(QString("Start rotateCW"));
    if (!rotator->get_serialConnected())
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
        // button off

        if (rotatorBearing >= currentMaxAzimuth)
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
        if (rotator->get_serialConnected())
        {
            if (supportCwCcwCmd)
            {
                logMessage(QString("Send CW rotator command, rotator speed = %1").arg(QString::number(rotator->get_rotatorSpeed())));
                retCode = rotator->rotateClockwise(rotator->get_rotatorSpeed());
            }
            else
            {
                if (endStopType == ROT_180_180)
                {
                    logMessage(QString("Send rotate to end stop, instead of CW rotator command, endstop = %1").arg(QString::number(rotatorCWEndStop)));
                    retCode = rotator->rotate_to_bearing(rotatorCWEndStop);
                }
                else
                {
                    logMessage(QString("Send rotate to minAzimuth, instead of CW rotator command, maxAzimuth = %1").arg(QString::number(currentMaxAzimuth)));
                    retCode = rotator->rotate_to_bearing(currentMaxAzimuth);
                }
            }
            if (retCode < 0)
            {
                hamlibError(retCode, "Rotate CW");
                movingCW = false;
                sendStatusToLogError();

            }
            else
            {

                movingCW = true;
                if (!supportCwCcwCmd)
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




void RotatorMainWindow::rotateCCW(bool /*toggle*/)
{
    if (brakeflag)
    {
        return;
    }

    cwCcwCmdflag = true;
    logMessage(QString("Start rotateCCW"));
    // check connected
    if (!rotator->get_serialConnected())
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
        // button off

        if (rotatorBearing < currentMinAzimuth)
        {
            logMessage(QString("CCW - Rotator Bearing < currentMinAzimuth"));
            cwCcwCmdflag = false;
            return;
        }


        if (moving || movingCW || movingCCW)
        {
            logMessage(QString("RotateCCW - rotator already moving - stop"));
            stopButton();
        }


        int retCode = 0;
        if (rotator->get_serialConnected())
        {
            if (supportCwCcwCmd)
            {
                logMessage(QString("Send CCW rotator command, rotator speed = " + QString::number(rotator->get_rotatorSpeed())));
                retCode = rotator->rotateCClockwise(rotator->get_rotatorSpeed());
            }
            else
            {
                if (endStopType == ROT_180_180)
                {
                    logMessage(QString("Send rotate to end stop, instead of CCW rotator command, endstop = %1").arg(QString::number(rotatorCCWEndStop)));
                    retCode = rotator->rotate_to_bearing(rotatorCCWEndStop);
                }
                else
                {
                    logMessage(QString("Send rotate to minAzimuth, instead of CCW rotator command, minAzimuth = %1").arg(QString::number(currentMinAzimuth)));
                    retCode = rotator->rotate_to_bearing(currentMinAzimuth);
                }

            }
            if (retCode < 0)
            {
                hamlibError(retCode, "Rotate CCW");
                movingCCW = false;
                sendStatusToLogError();

            }
            else
            {

                movingCCW = true;
                if (!supportCwCcwCmd)
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


void RotatorMainWindow::turn_button_on()
{
    turn_button_status = ON;
    //ui->turnButton->setPalette(*redText);
    ui->turnButton->setStyleSheet(BUTTON_ON_STYLE);
    ui->turnButton->setText("Turn");
}

void RotatorMainWindow::turn_button_off()
{
    turn_button_status = OFF;
    //ui->turnButton->setPalette(*blackText);
    ui->turnButton->setStyleSheet(BUTTON_OFF_STYLE);
    ui->turnButton->setText("Turn");
}


void RotatorMainWindow::rot_left_button_on()
{
    rot_left_button_status = ON;
    //ui->rot_left_button->setPalette(*redText);
    ui->rot_left_button->setStyleSheet(BUTTON_ON_STYLE);
    ui->rot_left_button->setText("(CCW) Left");
}

void RotatorMainWindow::rot_left_button_off()
{
    rot_left_button_status = OFF;
    //ui->rot_left_button->setPalette(*blackText);
    ui->rot_left_button->setStyleSheet(BUTTON_OFF_STYLE);
    ui->rot_left_button->setText("(CCW) Left");
}

void RotatorMainWindow::rot_right_button_on()
{
    rot_right_button_status = ON;
    //ui->rot_right_button->setPalette(*redText);

    ui->rot_right_button->setStyleSheet(BUTTON_ON_STYLE);
    ui->rot_right_button->setText("(CW) Right");
}

void RotatorMainWindow::rot_right_button_off()
{
    rot_right_button_status = OFF;
    //ui->rot_right_button->setPalette(*blackText);
    ui->rot_right_button->setStyleSheet(BUTTON_OFF_STYLE);
    ui->rot_right_button->setText("(CW) Right");
}

void RotatorMainWindow::stop_button_on()
{

    //ui->stopButton->setPalette(*redText);
    ui->stopButton->setStyleSheet(BUTTON_ON_STYLE);
    ui->stopButton->setText("Stop");
}

void RotatorMainWindow::stop_button_off()
{

    //ui->stopButton->setPalette(*blackText);
    ui->stopButton->setStyleSheet(BUTTON_OFF_STYLE);
    ui->stopButton->setText("Stop");
}



void RotatorMainWindow::setPolltime(int interval)
{
    pollTime = interval;
}

int RotatorMainWindow::getPolltime()
{

    return pollTime;
}


void RotatorMainWindow::hamlibError(int errorCode, QString cmd )
{

    if ( errorCode >= 0)
    {
        return;
    }

    errorCode *= -1;
    rotErrorFlag = true;
    if (appName.length() >0)
    {
        sendStatusToLogError();
    }
    // log all errors
    QString errorMsg = rotator->gethamlibErrorMsg(errorCode);
    logMessage(QString("Hamlib Error - Code = %1 - %2").arg(QString::number(errorCode).arg(errorMsg)));


     pollTimer->stop();

     QMessageBox::critical(this, "Rotator hamlib Error - " + selectRotator->currentAntenna.antennaName, QString::number(errorCode) + " - " + errorMsg + "\n" + "Command - " + cmd);

     closeRotator();
     rotErrorFlag = false;
     if (appName.length() >0)
     {
         sendStatusToLogDisConnected();
     }




}


void RotatorMainWindow::sendStatusToLogConnected()
{
    connectStat = ROT_STATUS_CONNECTED;
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

void RotatorMainWindow::sleepFor(qint64 milliseconds)
{
    qint64 timeToExitFunction = QDateTime::currentMSecsSinceEpoch()+milliseconds;
    while(timeToExitFunction>QDateTime::currentMSecsSinceEpoch()){
        QApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}


void RotatorMainWindow::readTraceLogFlag()
{

    QString fileName;
    if (appName == "")
    {
        fileName = CONFIGURATION_FILEPATH_LOCAL + MINOS_ROTATOR_CONFIG_FILE;
    }
    else
    {
        fileName = CONFIGURATION_FILEPATH_LOGGER + MINOS_ROTATOR_CONFIG_FILE;
    }

    QSettings config(fileName, QSettings::IniFormat);

    //QSettings config("./Configuration/MinosRotatorConfig.ini", QSettings::IniFormat);
    config.beginGroup("TraceLog");


    ui->actionTraceLog->setChecked(config.value("TraceLog", false).toBool());

    config.endGroup();
}

void RotatorMainWindow::saveTraceLogFlag()
{
    QString fileName;
    if (appName == "")
    {
        fileName = CONFIGURATION_FILEPATH_LOCAL + MINOS_ROTATOR_CONFIG_FILE;
    }
    else
    {
        fileName = CONFIGURATION_FILEPATH_LOGGER + MINOS_ROTATOR_CONFIG_FILE;
    }

    QSettings config(fileName, QSettings::IniFormat);


    //QSettings config("./Configuration/MinosRotatorConfig.ini", QSettings::IniFormat);
    config.beginGroup("TraceLog");

    config.setValue("TraceLog", ui->actionTraceLog->isChecked());

    config.endGroup();
    trace("Tracelog Changed = " + QString::number(ui->actionTraceLog->isChecked()));
}


bool RotatorMainWindow::getCwCcwCmdFlag(int rotatorNumber)
{
    int retCode = 0;
    bool value = false;
    retCode = rotator->getSupportCwCcwCmd(rotatorNumber, &value);
    if (retCode < 0)
    {
        logMessage(QString("Error getting CwCcwSupport flag for rotator number %1").arg(QString::number(rotatorNumber)));
    }

    return value;

}


void RotatorMainWindow::about()
{
    QMessageBox::about(this, "Minos Rotator", "Minos QT Rotator\nCopyright D Balharrie G8FKH/M0DGB 2017");
}


void RotatorMainWindow::overLapDisplayBox(overlapStat status)
{
    QPalette palette;

    if (status == POS_OVERLAP)
    {
        palette.setColor(QPalette::Base,Qt::red);
    }
    else if (status == NEG_OVERLAP)
    {
        palette.setColor(QPalette::Base,Qt::green);
    }
    else
    {
        palette.setColor(QPalette::Base,Qt::lightGray);
    }

    ui->overlaplineEdit->setPalette(palette);
}


void RotatorMainWindow::toggleOverLapDisplay(bool toggle)
{
    // don't display overlap if rotator doesn't support or user turned off overlap
    ui->overlap->setVisible(toggle);
    ui->overlaplineEdit->setVisible(toggle);

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
                selectAntenna->setCurrentIndex(selectRotator->currentAntenna.antennaNumber.toInt(&ok, 10));
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

void RotatorMainWindow::aboutRotatorConfig()
{
    QString msg = QString("*** Rotator Configuration ***\n\n");
    msg.append(QString("App Instance Name  = %1\n").arg(appName));
    msg.append(QString("Antenna Name = %1\n").arg(selectRotator->currentAntenna.antennaName));
    msg.append(QString("Antenna Number = %1\n").arg(selectRotator->currentAntenna.antennaNumber));
    msg.append(QString("Rotator Model = %1\n").arg(selectRotator->currentAntenna.rotatorModel));
    msg.append(QString("Rotator Number = %1\n").arg(QString::number(selectRotator->currentAntenna.rotatorModelNumber)));
    msg.append(QString("Rotator Manufacturer = %1\n").arg(selectRotator->currentAntenna.rotatorManufacturer));
    msg.append(QString("Minos Rotator Type = %1\n").arg(endStopNames[endStopType]));
    msg.append(QString("Rotator PortType = %1\n").arg(hamlibData::portTypeList[selectRotator->currentAntenna.portType]));
    msg.append(QString("Network Address = %1\n").arg(selectRotator->currentAntenna.networkAdd));
    msg.append(QString("Network Port = %1\n").arg(selectRotator->currentAntenna.networkPort));
    msg.append(QString("Rotator Comport = %1\n").arg(selectRotator->currentAntenna.comport));
    msg.append(QString("Baudrate = %1\n").arg(QString::number(selectRotator->currentAntenna.baudrate)));
    msg.append(QString("Databits = %1\n").arg(QString::number(selectRotator->currentAntenna.databits)));
    msg.append(QString("Stop bits = %1\n").arg(QString::number(selectRotator->currentAntenna.stopbits)));
    msg.append(QString("Parity = %1\n").arg(rotator->getParityCodeNames()[selectRotator->currentAntenna.parity]));
    msg.append(QString("Handshake = %1\n").arg(rotator->getHandShakeNames()[selectRotator->currentAntenna.handshake]));
    msg.append(QString("Antenna Offset = %1\n").arg(QString::number(selectRotator->currentAntenna.antennaOffset)));
    msg.append(QString("Current Max Azimuth = %1\n").arg(QString::number(currentMaxAzimuth)));
    msg.append(QString("Current Min Azimuth = %1\n").arg(QString::number(currentMinAzimuth)));
    msg.append(QString("Current CW EndStop = %1\n").arg(QString::number(rotatorCWEndStop)));
    msg.append(QString("Current CCW EndStop = %1\n").arg(QString::number(rotatorCCWEndStop)));
    QString f;
    selectRotator->currentAntenna.southStopFlag ? f = "True" : f = "False";
    msg.append(QString("South Stop Flag = %1\n").arg(f));
    overLapActiveflag ? f = "True" : f = "False";
    msg.append(QString("Overrun flag = %1\n").arg(f));
    supportCwCcwCmd ? f = "True" : f = "False";
    msg.append(QString("Support CW and CCW Commands = %1\n").arg(f));
    msg.append(QString("Rotator Max Baudrate = %1\n").arg(QString::number(rotator->getMaxBaudRate())));
    msg.append(QString("Rotator Min Baud rate = %1\n").arg(QString::number(rotator->getMinBaudRate())));
    ui->actionTraceLog->isChecked() ? f = "True" : f = "False";
    msg.append(QString("Tracelog = %1\n").arg(f));



    QMessageBox::about(this, "Minos Rotator", msg);
}



void RotatorMainWindow::dumpRotatorToTraceLog()
{
    trace("*** Antenna Updated ***");
    trace(QString("App Instance Name  = %1").arg(appName));
    trace(QString("Antenna Name = %1").arg(selectRotator->currentAntenna.antennaName));
    trace(QString("Antenna Number = %1").arg(selectRotator->currentAntenna.antennaNumber));
    trace(QString("Rotator Model = %1").arg(selectRotator->currentAntenna.rotatorModel));
    trace(QString("Rotator Number = %1").arg(QString::number(selectRotator->currentAntenna.rotatorModelNumber)));
    trace(QString("Rotator Manufacturer = %1").arg(selectRotator->currentAntenna.rotatorManufacturer));
    trace(QString("Minos Rotator Type = %1").arg(endStopNames[endStopType]));
    trace(QString("Rotator PortType = %1").arg(hamlibData::portTypeList[selectRotator->currentAntenna.portType]));
    trace(QString("Network Address = %1").arg(selectRotator->currentAntenna.networkAdd));
    trace(QString("Network Port = %1").arg(selectRotator->currentAntenna.networkPort));
    trace(QString("Rotator Comport = %1").arg(selectRotator->currentAntenna.comport));
    trace(QString("Baudrate = %1").arg(QString::number(selectRotator->currentAntenna.baudrate)));
    trace(QString("Databits = %1").arg(QString::number(selectRotator->currentAntenna.databits)));
    trace(QString("Stop bits = %1").arg(QString::number(selectRotator->currentAntenna.stopbits)));
    trace(QString("Parity = %1").arg(rotator->getParityCodeNames()[selectRotator->currentAntenna.parity]));
    trace(QString("Handshake = %1").arg(rotator->getHandShakeNames()[selectRotator->currentAntenna.handshake]));
    trace(QString("Antenna Offset = %1").arg(QString::number(selectRotator->currentAntenna.antennaOffset)));
    trace(QString("Current Max Azimuth = %1").arg(QString::number(currentMaxAzimuth)));
    trace(QString("Current Min Azimuth = %1").arg(QString::number(currentMinAzimuth)));
    trace(QString("Current CW EndStop = %1").arg(QString::number(rotatorCWEndStop)));
    trace(QString("Current CCW EndStop = %1").arg(QString::number(rotatorCCWEndStop)));
    QString f;
    selectRotator->currentAntenna.southStopFlag ? f = "True" : f = "False";
    trace(QString("South Stop Flag = %1").arg(f));
    overLapActiveflag ? f = "True" : f = "False";
    trace(QString("Overrun flag = %1").arg(f));
    supportCwCcwCmd ? f = "True" : f = "False";
    trace(QString("Support CW and CCW Commands = %1").arg(f));
    trace(QString("Rotator Max Baudrate = %1").arg(QString::number(rotator->getMaxBaudRate())));
    trace(QString("Rotator Min Baud rate = %1").arg(QString::number(rotator->getMinBaudRate())));
    ui->actionTraceLog->isChecked() ? f = "True" : f = "False";
    trace(QString("Tracelog = %1").arg(f));
}
