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
#include "rotatorRpc.h"
#include "rotatorlog.h"
#include "rotatormainwindow.h"
#include "rotatorcommon.h"
#include "ui_rotatormainwindow.h"
#include "minoscompass.h"
#include "rotcontrol.h"
#include "rotsetupdialog.h"
#include "logdialog.h"
#include <QString>
#include <QLabel>
#include <QMessageBox>
#include <QTimer>
#include <QTime>
#include <QSettings>
#include <QProcessEnvironment>
#include <QtDebug>

// QPushButton:clicked{\n	background-color: red;\n	border-style: outset;\n	border-width: 1px;\n	border-radius: 5px;\n	border-color: black;\n	min-width: 5em;\n	padding: 3px;\n}



RotatorMainWindow::RotatorMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RotatorMainWindow)
{
    ui->setupUi(this);

    serialData::translateSerialData();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    for (int i = 0; i < presetShortCutKeys.count(); i++)
    {
        shortCutKeyList.append(new QShortcut(QKeySequence(presetShortCutKeys[i]), this));
    }

    for (int i = 0; i < presetMenuShortCutKeys.count(); i++)
    {
        shiftShortCutKeyList.append(new QShortcut(QKeySequence(presetMenuShortCutKeys[i]), this));
    }


    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    // get the antenna name from host process
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    QDir dir(".");
    trace(QString("Directory %1").arg(dir.absolutePath()));

    createCloseEvent();
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

    initPresetButtons();


    rotator = new RotControl();
    rotator->getRotatorList();
    setupAntenna = new RotSetupDialog(rotator);
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

    rotator->set_serialConnected(false);

    refreshPresetLabels();
    initActionsConnections();

    selectAntenna = ui->selectAntennaBox;

    setupAntenna->setAppName(appName);



    if (appName.length() > 0)
    {
        setSelectAntennaBoxVisible(false);
        setAntennaNameLabelVisible(true);
    }
    else
    {
        setSelectAntennaBoxVisible(true);
        setAntennaNameLabelVisible(false);
    }

    brakedelay = 1 * 1000;


    rotatorBearing = COMPASS_ERROR; // force first update





    //setPolltime(POLLTIME);   // to allow variable controller polltime - not implemented! **************** remove ????
    rotTimeCount = 0;
    RotateTimer.start(200);  // to set timeout for antenna rotating

    readTraceLogFlag();

    rotlog->getBearingLogConfig();


    //logAntError = false;
    initSelectAntennaBox();

    if (appName.length() > 0)
    {
        logMessage((QString("Read Current Antenna for AppName %1 from logger").arg(appName)));

    }
    else
    {
        logMessage((QString("Read Current Antenna for Local selection")));

        setupAntenna->readCurrentAntenna();
        ui->selectAntennaBox->setCurrentText(setupAntenna->currentAntennaName);
        if (setupAntenna->currentAntennaName == "")
        {
            logMessage(QString("No antenna selected or no antenna found for this appName, %1").arg(appName));
            QString errmsg = HtmlFontColour(Qt::red) + tr("Please select an antenna or no antenna found!");
            showStatusMessage(errmsg);
            statusMsg = errmsg;
            sendStatusLogger();
        }

        upDateAntenna();
    }

    trace("*** Rotator Started ***");
}

RotatorMainWindow::~RotatorMainWindow()
{

    delete msg;
    delete ui;
}


void RotatorMainWindow::logMessage( QString s )
{
           trace( s );
}

void RotatorMainWindow::onStdInRead(QString cmd)
{
    bool doClose = false;
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        closeApp = true;
        doClose = true;
    }
    executeStdIn(cmd);
    if (doClose)
        close();
}

void RotatorMainWindow::closeEvent(QCloseEvent *event)
{

    LogTimer.stop();
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
    bool show = getShowServers();
    if ( !isVisible() && show )
    {
        setVisible(true);
    }
    if ( isVisible() && !show )
    {
        setVisible(false);
    }
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


void RotatorMainWindow::onSelectAntennaBox()
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


void RotatorMainWindow::onLoggerSetRotation(int direction, int angle)
{
    if (closeApp)
        return;

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
        logMessage(QString("Preset from Logger - Save - Button Number %1, Name %2, Bearing %3").arg(msg[0]).arg(msg[1]).arg(msg[2]));
        int butNum = msg[0].toInt();
        RotPresetData d = RotPresetData(butNum, msg[1], msg[2]);
        setRotPresetButData(butNum, d);
        rotPresetButtonUpdate(butNum, d);
    }


}



void RotatorMainWindow::openRotator()
{

    int retCode = 0;

    if (setupAntenna->currentAntenna.antennaName == "")
    {
        logMessage(QString("Open Rotator: No rotator name!"));
        showStatusMessage(tr("Please select an Antenna"));
        return;
    }
    if (rig_port_e(setupAntenna->currentAntenna.portType) == RIG_PORT_SERIAL)
    {
        if(setupAntenna->comportAvial(setupAntenna->currentAntenna.comport) == -1)
        {
            logMessage(QString("Open Rotator: Check comport - defined port %1 not available on computer").arg(setupAntenna->currentAntenna.comport));
            showStatusMessage(tr("Comport %1 no longer configured on computer?").arg(setupAntenna->currentAntenna.comport));
            return;
        }

        if (setupAntenna->currentAntenna.comport == "")
        {
            logMessage(QString("Open Rotator: No comport"));
            showStatusMessage(tr("Please select a Comport"));
            return;
        }

    }
    if (rig_port_e(setupAntenna->currentAntenna.portType) == RIG_PORT_NETWORK || rig_port_e(setupAntenna->currentAntenna.portType == RIG_PORT_UDP_NETWORK))
    {
        if (setupAntenna->currentAntenna.networkAdd == "" || (setupAntenna->currentAntenna.networkPort == ""))
        {
            logMessage(QString("Open Rotator: No network or Port Number"));
            showStatusMessage(tr("Please enter a network Address and Port Number"));
            return;
        }

    }
    if (setupAntenna->currentAntenna.rotatorModel == "")
    {
        logMessage(QString("Open Rotator: No rotator model"));
        showStatusMessage(tr("Please select a rotator model"));
        return;
    }


    //srotParams p = setupAntenna->getCurrentAntenna();

    retCode = rotator->init(setupAntenna->currentAntenna);
        if (retCode < 0)
        {
            hamlibError(retCode, tr("Rotator Init"));
        }
    if (rotator->get_serialConnected())
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
        if (rig_port_e(setupAntenna->currentAntenna.portType) == RIG_PORT_SERIAL)
        {
            showStatusMessage(tr("Connected to: %1 - %2, %3, %4, %5, %6, %7, %8")
                                  .arg(setupAntenna->currentAntenna.antennaName).arg(setupAntenna->currentAntenna.rotatorModel).arg(setupAntenna->currentAntenna.comport).arg(setupAntenna->currentAntenna.baudrate).arg(setupAntenna->currentAntenna.databits)
                                  .arg(setupAntenna->currentAntenna.stopbits).arg(rotator->getParityCodeNames()[setupAntenna->currentAntenna.parity]).arg(rotator->getHandShakeNames()[setupAntenna->currentAntenna.handshake]));
        }
        else if (rig_port_e(setupAntenna->currentAntenna.portType == RIG_PORT_NETWORK || rig_port_e(setupAntenna->currentAntenna.portType == RIG_PORT_UDP_NETWORK)))
        {
            showStatusMessage(tr("Connected to: %1 - %2, %3").arg(setupAntenna->currentAntenna.antennaName).arg(setupAntenna->currentAntenna.rotatorModel).arg(setupAntenna->currentAntenna.networkAdd + ":" + setupAntenna->currentAntenna.networkPort));
        }
        else if (rig_port_e(setupAntenna->currentAntenna.portType) == RIG_PORT_NONE)
        {
                showStatusMessage(tr("Connected to: %1 - %2").arg(setupAntenna->currentAntenna.antennaName).arg(setupAntenna->currentAntenna.rotatorModel));
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



}

void RotatorMainWindow::closeRotator()
{
    if (moving || movingCW || movingCCW)
    {
        stop_rotation();
    }

    pollTimer->stop();

    if (rotator->get_serialConnected())
    {
        rotator->closeRotator();
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
   logMessage(QString("Send %1 message to logger, appName = %2").arg(message).arg(appName));

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

    connect(ui->selectAntennaBox, SIGNAL(activated(int)), this, SLOT(onSelectAntennaBox()));
    connect(setupAntenna, SIGNAL(antennaNameChange()), this, SLOT(updateSelectAntennaBox()));
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

    // click on compass rose

    connect(ui->compassDial, SIGNAL(sendClickBearing(int)), this, SLOT(compassClicked(int)));

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
    connect(ui->actionSetup_Antennas, SIGNAL(triggered()), this, SLOT(onLaunchSetup()));
    connect(setupAntenna, SIGNAL(currentAntennaSettingChanged(QString)), this, SLOT(currentAntennaSettingChanged(QString)));
    connect(setupAntenna, SIGNAL(antennaNameChange()), this, SLOT(updateSelectAntennaBox()));
    connect(setupAntenna, SIGNAL(antennaTabChanged()), this, SLOT(updateSelectAntennaBox()));

    // setup presets
    //connect(editPresets, SIGNAL(showEditPresetDialog()), editPresets, SLOT(show()));
    //connect(editPresets, SIGNAL(updatePresetButtonLabels()), this, SLOT(updatePresetLabels()));
    //connect(ui->actionEdit_Presets, SIGNAL(triggered()), editPresets, SLOT(loadPresetEditFieldsShow()));
    // Bearing Log
    connect(ui->actionLog_Heading, SIGNAL(triggered()), setupLog, SLOT(loadLogConfig()));
    connect(setupLog, SIGNAL(showLogDialog()), setupLog, SLOT(show()));
    connect(setupLog, SIGNAL(bearingLogConfigChanged()), rotlog, SLOT(getBearingLogConfig()));
    connect(rotator, SIGNAL(bearing_updated(int)), rotlog, SLOT(saveBearingLog(int)));

    connect(rotator, SIGNAL(debug_protocol(QString)), this, SLOT(logMessage(QString)));

    // Message from Logger
    connect(msg, SIGNAL(setRotation(int,int)), this, SLOT(onLoggerSetRotation(int,int)));
    connect(msg, SIGNAL(selectAntennaFromLog(PubSubName)), this, SLOT(onLoggerSelectAntenna(PubSubName)));
    connect(msg, SIGNAL(setRotPreset(QString)), this, SLOT(onLoggerSetPreset(QString)));


    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAbout_Rotator_Config, SIGNAL(triggered()), this, SLOT(aboutRotatorConfig()));
    connect(ui->actionTraceComms, SIGNAL(toggled(bool)), this, SLOT(saveTraceLogFlag(bool)));    // set/clear comms tracing
    //connect(ui->actionExit , SIGNAL(triggered()), this, SLOT(close()));



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


    // support a south stop with compass modue 0 - 360
    //if (setupAntenna->currentAntenna.southStopType == S_STOPCOMP && bearing > COMPASS_HALF && bearing <= COMPASS_MAX360)
    //{
    //    rotatorBearing = bearing - COMPASS_MAX360;
    //}
    //else if (setupAntenna->currentAntenna.southStopType == S_STOPINV)
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
        QString s = QString("%1:%2:%3").arg(QString::number(displayBearing)).arg(QString::number(rotatorBearing)).arg(ol);

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

    if (moving  || movingCCW || movingCW)
    {

        stopRotation(true);
    }

    if (setupAntenna->currentAntennaName != "")
    {
        currentAntIdx = setupAntenna->findCurrentAntenna(setupAntenna->currentAntennaName);
        trace(QString("idx is %1 avail is %2").arg(currentAntIdx).arg(setupAntenna->numAvailAntennas));
        if (currentAntIdx > -1 && currentAntIdx < setupAntenna->numAvailAntennas)  // find antenna and update current antenna pointer
        {
            // found antenna, update currentAntenna with selected antenna data
            srotParams::copyRot(setupAntenna->availAntData[currentAntIdx], setupAntenna->currentAntenna);


            if (setupAntenna->currentAntenna.rotatorModelNumber == 0)
            {
                closeRotator();
                QMessageBox::critical(this, tr("Antenna Error"), tr("Please configure an antenna name and rotator model"));
                return;
            }



            ui->antNameDisp->setText(setupAntenna->currentAntenna.antennaName);

            closeRotator();

            writeWindowTitle(appName);
            openRotator();
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

            dumpRotatorToTraceLog();

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
               sendStatusToLogStop();
               PubSubName psname(setupAntenna->currentAntennaName);
               trace(QString("Update Antenna - send to logger - maxAzimuth = %1, minAzimuth = %2, simulate CwCcwCmd = %3").arg(QString::number(setupAntenna->currentAntenna.max_azimuth)).arg(QString::number(setupAntenna->currentAntenna.min_azimuth)).arg(setupAntenna->currentAntenna.supportCwCcwCmd  ? "True" : "False"));
               msg->rotatorCache.setMaxAzimuth(psname, setupAntenna->currentAntenna.max_azimuth);
               msg->rotatorCache.setMinAzimuth(psname, setupAntenna->currentAntenna.min_azimuth);
               msg->rotatorCache.publish();
               if (setupAntenna->currentAntenna.supportCwCcwCmd)           // want to use simCwCccwCmd?
               {
                   msg->rotatorCache.setCwCcwCmdEnable(psname, true);
                   msg->rotatorCache.publish();
               }
               else
               {
                   msg->rotatorCache.setCwCcwCmdEnable(psname, setupAntenna->currentAntenna.simCwCcwCmd);
                   msg->rotatorCache.publish();
               }

           }
        }
    }
    else
    {   // no antenna selected
        trace("No antenna selected");
        ui->antNameDisp->setText("");
        closeRotator();
        if (appName.length() > 0)
        {
            writeWindowTitle(appName);
            sendStatusToLogDisConnected();
            sendStatusToLogStop();
        }
        else
        {
            writeWindowTitle(appName);
        }
    }

    if (appName.length() > 0)
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
        antennaIndex -= 1;
        if (setupAntenna->currentAntenna.rotatorModelNumber == 0)
        {
            closeRotator();
            QMessageBox::critical(this, tr("Antenna Error"), tr("Please configure an antenna name and rotator model"));
            return;
        }

        if (!rotator->get_serialConnected())
        {
            openRotator();
        }

        if (rotator->get_serialConnected())
        {
            sendStatusToLogConnected();
        }
        else
        {
            sendStatusToLogDisConnected();
        }

    }
    dumpRotatorToTraceLog();
    msg->rotatorCache.publish();
}

void RotatorMainWindow::writeWindowTitle(QString appName)
{
    if (appName.length() > 0)
    {
        this->setWindowTitle(tr("Minos Rotator Control - %1 - Logger").arg(appName));
    }
    else
    {
        this->setWindowTitle(tr("Minos Rotator Control - Local"));
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
            hamlibError(retCode, tr("Request Bearing"));
        }
    }
    reqBearCmdflag = false;
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


    rotCmdflag = true;


    if (ui->bearingEdit->isValid())
    {
        rotateTo(ui->bearingEdit->getBearing());
    }
    else
    {
         QMessageBox::critical(this, tr("Bearing Error"),
                               tr("Invalid Bearing\nPlease enter %1 - %2").arg(QString::number(COMPASS_MIN0)).arg(QString::number(COMPASS_MAX360)));
    }

    rotCmdflag = false;
}


void RotatorMainWindow::rotateTo(int bearing)
{

    int retCode = 0;
    int rotateTo = bearing;
    logMessage(QString("RotateTo Bearing = %1").arg(QString::number(bearing)));



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


    logMessage(QString("Rotate to Bearing = %1, adjusted with offset = %2").arg(QString::number(rotateTo)).arg(QString::number(setupAntenna->currentAntenna.antennaOffset)));



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
            hamlibError(retCode, tr("Rotate to Bearing"));
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
        logMessage(QString("NCalc - 1 - Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing)).arg(QString::number(rotatorBearing)));
    }
    else
    {
        logMessage(QString("NCalc - EndStop Type - ROT_0_360 - S_STOPOFF"));
        logMessage(QString("NCalc - 1 - Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing)).arg(QString::number(rotatorBearing)));
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


    logMessage(QString("Target Bearing = %1, rotator Bearing = %2").arg(QString::number(targetBearing)).arg(QString::number(rotatorBearing)));
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
                   .arg(QString::number(rotatorBearing)).arg(QString::number(targetBearing)).arg(QString::number(target)));
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
        if (setupAntenna->currentAntenna.rotatorModelNumber == 1701)
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
        if (rotator->get_serialConnected())
        {
            if (setupAntenna->currentAntenna.supportCwCcwCmd)
            {
                logMessage(QString("Send CW rotator command, rotator speed = %1").arg(QString::number(rotator->get_rotatorSpeed())));
                retCode = rotator->rotateClockwise(rotator->get_rotatorSpeed());
            }
            else
            {

                logMessage(QString("Send rotate to maxAzimuth, instead of CW rotator command, maxAzimuth = %1").arg(QString::number(setupAntenna->currentAntenna.max_azimuth)));
                retCode = rotator->rotate_to_bearing(setupAntenna->currentAntenna.max_azimuth);

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
        if (rotator->get_serialConnected())
        {
            if (setupAntenna->currentAntenna.supportCwCcwCmd)
            {
                logMessage(QString("Send CCW rotator command, rotator speed = " + QString::number(rotator->get_rotatorSpeed())));
                retCode = rotator->rotateCClockwise(rotator->get_rotatorSpeed());
            }
            else
            {

                logMessage(QString("Send rotate to minAzimuth, instead of CCW rotator command, minAzimuth = %1").arg(QString::number(setupAntenna->currentAntenna.min_azimuth)));
                retCode = rotator->rotate_to_bearing(setupAntenna->currentAntenna.min_azimuth);
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
    logMessage(QString("Hamlib Error - Code = %1 - %2").arg(errorCode).arg(errorMsg));


     pollTimer->stop();

     QMessageBox::critical(this, tr("Rotator hamlib Error - %1").arg( setupAntenna->currentAntennaName), tr("%1 - %2\nCommand - %3").arg(errorCode).arg(errorMsg).arg(cmd));

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
        style = "background-color: green; color black; font-weight: bold;\n";
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

    ui->actionTraceComms->setChecked(state);
    rotator->enableTraceComms(state);             // set state of trace hamlib comms
}

void RotatorMainWindow::saveTraceLogFlag(bool state)
{

    // set state of hamlib commms tracing

    rotator->enableTraceComms(state);

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

        connect(presetButton[i], &PresetButton::presetShortCutRecall, [this, i]() {presetRead(i);});
        connect(presetButton[i], &PresetButton::presetShiftShortCutRecall, [this, i]() {showPresetMenu(i);});
        connect(presetButton[i], &PresetButton::presetReadAction, [this, i]() {presetRead(i);});
        connect(presetButton[i], &PresetButton::presetEditAction, [this, i]() {presetEdit(i);});
        connect(presetButton[i], &PresetButton::presetWriteAction, [this, i]() {presetWrite(i);});
        connect(presetButton[i], &PresetButton::presetClearAction, [this, i]() {presetClear(i);});


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
        rotateTo(rotPresets[buttonNumber]->bearing.toInt());
        ui->bearingEdit->setText(rotPresets[buttonNumber]->bearing);
        ui->bearingEdit->setFocus();
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
    presetButton[buttonNumber]->presetButton->setText(QString("%1: %2\r\n%3").arg(QString::number(buttonNumber + 1)).arg(editData.name).arg(editData.bearing) );
    QString tTipStr = tr("Bearing = %1").arg(editData.bearing);
    presetButton[buttonNumber]->presetButton->setToolTip(tTipStr);
}

void RotatorMainWindow::saveRotPresetButton(RotPresetData& editData)
{
    QString msg;
    msg = QString("%1:%2:%3").arg(QString::number(editData.number)).arg(editData.name).arg(editData.bearing);
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




void RotatorMainWindow::aboutRotatorConfig()
{


    QString msg = tr("*** Rotator Configuration ***\n\n");

    if (setupAntenna->currentAntenna.antennaName != "")
    {

        msg.append(tr("App Instance Name  = %1\n").arg(appName));
        msg.append(tr("Hamlib Version = %1\n").arg(rotator->gethamlibVersion()));
        msg.append(tr("Antenna Name = %1\n").arg(setupAntenna->currentAntenna.antennaName));
        msg.append(tr("Antenna Number = %1\n").arg(setupAntenna->currentAntenna.antennaNumber));
        msg.append(tr("Rotator Model = %1\n").arg(setupAntenna->currentAntenna.rotatorModel));
        msg.append(tr("Rotator Number = %1\n").arg(QString::number(setupAntenna->currentAntenna.rotatorModelNumber)));
        msg.append(tr("Rotator Manufacturer = %1\n").arg(setupAntenna->currentAntenna.rotatorManufacturer));
        msg.append(tr("Rotator Type = %1\n").arg(endStopNames[setupAntenna->currentAntenna.rotType]));
        msg.append(tr("Rotator CW EndStop = %1\n").arg(QString::number(setupAntenna->currentAntenna.rotatorCWEndStop)));
        msg.append(tr("Rotator CCW EndStop = %1\n").arg(QString::number(setupAntenna->currentAntenna.rotatorCCWEndStop)));
        msg.append(tr("Rotator PortType = %1\n").arg(hamlibData::portTypeList[setupAntenna->currentAntenna.portType]));
        msg.append(tr("Network Address = %1\n").arg(setupAntenna->currentAntenna.networkAdd));
        msg.append(tr("Network Port = %1\n").arg(setupAntenna->currentAntenna.networkPort));
        msg.append(tr("Rotator Comport = %1\n").arg(setupAntenna->currentAntenna.comport));
        msg.append(tr("Baudrate = %1\n").arg(QString::number(setupAntenna->currentAntenna.baudrate)));
        msg.append(tr("Databits = %1\n").arg(QString::number(setupAntenna->currentAntenna.databits)));
        msg.append(tr("Stop bits = %1\n").arg(QString::number(setupAntenna->currentAntenna.stopbits)));
        msg.append(tr("Parity = %1\n").arg(rotator->getParityCodeNames()[setupAntenna->currentAntenna.parity]));
        msg.append(tr("Handshake = %1\n").arg(rotator->getHandShakeNames()[setupAntenna->currentAntenna.handshake]));
        msg.append(tr("Antenna Offset = %1\n").arg(QString::number(setupAntenna->currentAntenna.antennaOffset)));
        msg.append(tr("Current Rotator Type = %1\n").arg(endStopNames[setupAntenna->currentAntenna.endStopType]));
        msg.append(tr("Current Max Azimuth = %1\n").arg(QString::number(setupAntenna->currentAntenna.max_azimuth)));
        msg.append(tr("Current Min Azimuth = %1\n").arg(QString::number(setupAntenna->currentAntenna.min_azimuth)));
        msg.append(tr("South Stop Type = %1\n").arg(southStopNames[setupAntenna->currentAntenna.southStopType]));
        msg.append(tr("Overrun flag = %1\n").arg(overLapActiveflag ? "True" : "False"));
        msg.append(tr("Support CW and CCW Commands = %1\n").arg(setupAntenna->currentAntenna.supportCwCcwCmd ? tr("True") : tr("False")));
        msg.append(tr("Simulate CW and CCW Commands selected = %1\n").arg(setupAntenna->currentAntenna.simCwCcwCmd ? tr("True") : tr("False")));
        msg.append(tr("Rotator Max Baudrate = %1\n").arg(QString::number(setupAntenna->currentAntenna.maxBaudRate)));
        msg.append(tr("Rotator Min Baud rate = %1\n").arg(QString::number(setupAntenna->currentAntenna.minBaudRate)));
        msg.append(tr("Rotator Polltime = %1\n").arg(setupAntenna->currentAntenna.pollInterval));
        msg.append(tr("Tracelog = %1\n").arg(ui->actionTraceComms->isChecked() ? tr("True") : tr("False")));


    }
    else
    {
        msg.append(tr("No Antenna Selected\n"));
    }



    QMessageBox::about(this, tr("Minos Rotator"), msg);

}



void RotatorMainWindow::dumpRotatorToTraceLog()
{

    if (setupAntenna->currentAntenna.antennaName != "")
    {
        trace("*** Antenna Updated ***");
        trace(QString("App Instance Name  = %1").arg(appName));
        if (rotator != nullptr)
        {
            trace(QString("Library Version = %1").arg(rotator->gethamlibVersion()));

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
        trace(QString("Network Address = %1").arg(setupAntenna->currentAntenna.networkAdd));
        trace(QString("Network Port = %1").arg(setupAntenna->currentAntenna.networkPort));
        trace(QString("Rotator Comport = %1").arg(setupAntenna->currentAntenna.comport));
        trace(QString("Baudrate = %1").arg(QString::number(setupAntenna->currentAntenna.baudrate)));
        trace(QString("Databits = %1").arg(QString::number(setupAntenna->currentAntenna.databits)));
        trace(QString("Stop bits = %1").arg(QString::number(setupAntenna->currentAntenna.stopbits)));
        trace(QString("Parity = %1").arg(rotator->getParityCodeNames()[setupAntenna->currentAntenna.parity]));
        trace(QString("Handshake = %1").arg(rotator->getHandShakeNames()[setupAntenna->currentAntenna.handshake]));
        trace(QString("Antenna Offset = %1").arg(QString::number(setupAntenna->currentAntenna.antennaOffset)));
        trace(QString("Current Rotator Type = %1").arg(endStopNames[setupAntenna->currentAntenna.endStopType]));
        trace(QString("Current Max Azimuth = %1").arg(QString::number(setupAntenna->currentAntenna.max_azimuth)));
        trace(QString("Current Min Azimuth = %1").arg(QString::number(setupAntenna->currentAntenna.min_azimuth)));
        trace(QString("South Stop Type = %1\n").arg(southStopNames[setupAntenna->currentAntenna.southStopType]));
        trace(QString("Overrun flag = %1").arg(overLapActiveflag ? "True" : "False"));
        trace(QString("Support CW and CCW Commands = %1").arg(setupAntenna->currentAntenna.supportCwCcwCmd ? "True" : "False"));
        trace(QString("Simulate CW and CCW Commands selected = %1").arg(setupAntenna->currentAntenna.simCwCcwCmd ? "True" : "False"));
        trace(QString("Rotator Max Baudrate = %1").arg(QString::number(setupAntenna->currentAntenna.maxBaudRate)));
        trace(QString("Rotator Min Baud rate = %1").arg(QString::number(setupAntenna->currentAntenna.minBaudRate)));
        trace(QString("Rotator Polltime = %1").arg(setupAntenna->currentAntenna.pollInterval));
        trace(QString("Tracelog = %1").arg(ui->actionTraceComms->isChecked() ? "True" : "False"));


    }
    else
    {
        trace(QString("No Antenna Selected"));
    }

}
