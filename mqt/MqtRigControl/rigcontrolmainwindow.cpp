/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
// Hamlib Library
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"
#include "mqtUtils_pch.h"
#include "RPCCommandConstants.h"
#include "rigcontrolcommonconstants.h"
#include "rigcontrolmainwindow.h"
#include "ui_rigcontrolmainwindow.h"
#include "freqpresetdialog.h"
#include "rigcontrol.h"
#include "rigsetupdialog.h"
#include "rigcontrolrpc.h"
#include "rigutils.h"
#include "rigctldclient.h"
#include <QTimer>
#include <QMessageBox>
#include <QProcessEnvironment>

#include <QBitArray>
#include <QDebug>




RigControlMainWindow::RigControlMainWindow(QWidget *parent) :
   QMainWindow(parent),
   ui(new Ui::RigControlMainWindow),
   radioIndex(0),
   rigErrorFlag(false),
   cmdLockFlag(false),
   rigCtldConnectDelay(0),
   logRitOn(false),
   supVolume(false),
   supSignalStrength(false),
   curVfoFrq(0.0),
   curTransVertFrq(0.0),
   mgmModeFlag(false),
   rRitFreq(0),
   curVol(0),
   curSignalStrength(0),
   radioSupGetRit(false),
   radioSupSetRit(false),
   radioSupGetRitState(false),
   radioSupRitOnOff(false),
   radioRitOn(false),
   ritEnable(false),
   radioCommsOK(false)
{
    ui->setupUi(this);


    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();


    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    createCloseEvent();

    connect(&LogTimer, SIGNAL(timeout()), this, SLOT(LogTimerTimer()));
    LogTimer.start(100);

    msg = new RigControlRpc(this);

    rigCtldProcess = new QProcess(this);

    connect(rigCtldProcess, SIGNAL(readyReadStandardOutput()),this, SLOT(rigCtldMessage()) );
    connect(rigCtldProcess, SIGNAL(readyReadStandardError()), this, SLOT(rigCtldErrorMessage()) );
    connect(rigCtldProcess, SIGNAL(started()), this, SLOT(rigCtldStarted()));

    setRigCltdIndicatorVisible(false);

    getRigCtldConnectDelay();

    RigCtldStatusTimer = new QTimer(this);
    connect(RigCtldStatusTimer, SIGNAL(timeout()), this, SLOT(rigCtldStatusTimeout()));


    QSettings settings;
    geoStr = "geometry";
    if (appName.length() > 0)
    {
        geoStr = geoStr + appName;
    }



    ui->testRitButton->setVisible(false);
    ui->setRitSpinner->setVisible(false);

#ifdef RIGCONTROL_TEST
    // rit test
    ui->testRitButton->setVisible(true);
    ui->setRitSpinner->setVisible(true);
    ui->setRitSpinner->setRange(-9000, 9000);
    ui->setRitSpinner->setSingleStep(100);
    connect(ui->setRitSpinner, SIGNAL(editingFinished()), this, SLOT(incRit()));
    connect(ui->testRitButton, SIGNAL(clicked()), this, SLOT(ritbuttontoggle()));
    //****************************************************************************************

#endif

    QByteArray geometry = settings.value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    radio = new RigControl();
    radio->getRigList();

    loadVhfAndUpBands(bands);
    FreqPresetDialog::readSettings(presetFreq);

    setupRadio = new RigSetupDialog(radio, bands);
    setupRadio->setAppName(appName);

    QString fileName;

    fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RADIO_CONFIG_FILE;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("MGM_Modes");


    mgmModes = config.value("MgmModes", "").toStringList();

    config.endGroup();


    if (appName.length() > 0)
    {
        // init cache with radio data
        trace(QString("rigcontrol: Started by logger appname = %1").arg(appName));
        sendRadioListLogger();
        initCacheData();

        msg->rigCache.publish();
    }




    serialTVSw = new SerialTVSwitch();     // create local serial sw

    setSelectRadioBoxVisible(false);
    setRadioNameLabelVisible(false);
    testBoxesVisible(false);


    if (appName.length() > 0)
    {
        // connected to logger don't show radio selectbox
        setSelectRadioBoxVisible(false);
        setRadioNameLabelVisible(true);
    }
    else
    {
        setSelectRadioBoxVisible(true);
        testBoxesVisible(true);
        setRadioNameLabelVisible(false);
    }


    pollTimer = new QTimer(this);

    status = new QLabel;
    ui->statusBar->addWidget(status);
    ui->radioNameDisp->setText("");

    radio->set_serialConnected(false);
    initActionsConnections();

    initSelectRadioBox();

    setTransVertDisplayVisible(false);
    sendTransVertSwitchToLogger(TRANSSW_NUM_DEFAULT);
    //sendTransVertSwitchToComPort(TRANSSW_NUM_DEFAULT);


    logRitOn = false;
    setRitFreqDisplayVisible(false);
    setRitGetSetFreqIndicatorVisible(false);

    initialiseSupportedRadioDisplay();




    if (appName.length() > 0)
    {
        logMessage((QString("Radio Selection for Current Radio, for AppName %1, will be from logger").arg(appName)));

    }
    else
    {
        logMessage((QString("Read Current Radio for Local selection")));

        setupRadio->readCurrentRadio();

        if (setupRadio->getCurrentRadioName() == "")
        {
            logMessage(QString("No radio selected for this appName, %1").arg(appName));
            QString errmsg = "<font color='Red'>Please select a radio!</font>";
            showStatusMessage(errmsg);
            sendStatusLogger(errmsg);
        }
        else
        {
            ui->selectRadioBox->setCurrentText(setupRadio->getCurrentRadioName());
        }
    }

    setPolltime(250);

    readTraceLogFlag();

    ui->selectRadioBox->clearFocus();


    if (appName.length() == 0)
    {
        upDateRadio();
    }
    trace("*** Rig App Started ***");
}

RigControlMainWindow::~RigControlMainWindow()
{

    delete ui;
    delete msg;
}

void RigControlMainWindow::logMessage( QString s )
{

        trace( s );
}


void RigControlMainWindow::LogTimerTimer()
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

void RigControlMainWindow::closeEvent(QCloseEvent *event)
{

    LogTimer.stop();

    closeRadio();

    // and tidy up all loose ends

    QSettings settings;
    settings.setValue(geoStr, saveGeometry());
    trace("MinosRigControl Closing");
    QWidget::closeEvent(event);
}






void RigControlMainWindow::onStdInRead(QString cmd)
{
    trace("Command read from stdin: " + cmd);
    if (cmd.indexOf("ShowServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(true);
    if (cmd.indexOf("HideServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(false);
}

void RigControlMainWindow::initActionsConnections()
{
    connect(ui->selectRadioBox, SIGNAL(activated(int)), this, SLOT(selectRadio()));
    connect(ui->actionSetup_Radios, SIGNAL(triggered()), this, SLOT(onLaunchSetup()));
    connect(ui->actionSetup_Band_Freq, SIGNAL(triggered(bool)), this, SLOT(setupBandFreq()));
    connect(ui->actionTraceComms, SIGNAL(toggled(bool)), this, SLOT(saveTraceLogFlag(bool)));    // set/clear comms tracing
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAbout_Radio_Config, SIGNAL(triggered()), this, SLOT(aboutRigConfig()));
    connect(pollTimer, SIGNAL(timeout()), this, SLOT(getRadioInfo()));
    connect(ui->ritEnableChk, SIGNAL(stateChanged(int)), this, SLOT(ritEnableChecked(int)));


    // configure radio dialog
    connect(setupRadio, SIGNAL(currentRadioSettingChanged(QString)), this, SLOT(currentRadioSettingChanged(QString)));
    connect(setupRadio, SIGNAL(radioNameChange()), this, SLOT(updateSelectRadioBox()));
    connect(setupRadio, SIGNAL(radioTabChanged()), this, SLOT(updateSelectRadioBox()));
    connect(setupRadio, SIGNAL(upDateRadioDetailsCache()), this, SLOT(updateRigDetailsCache()));



    // Message from Logger
    connect(msg, SIGNAL(setFreq(QString)), this, SLOT(loggerSetFreq(QString)));
    connect(msg, SIGNAL(setRitFreq(int)), this, SLOT(setRitFreq(int)));
    connect(msg, SIGNAL(setRitStatus(bool)), this, SLOT(setRitLogStatus(bool)));
    connect(msg, SIGNAL(setMode(QString)), this, SLOT(loggerSetMode(QString)));
    connect(msg, SIGNAL(selectLoggerRadio(PubSubName, QString)), this, SLOT(onSelectRadio(PubSubName, QString)));
    connect(msg, SIGNAL(setTpm(int, QString)), this, SLOT(setTpm(int, QString)));
    connect(msg, SIGNAL(setVolume(int)), this, SLOT(loggerSetVolume(int)));



    // Message from rigcontrol
    connect(radio, SIGNAL(debug_protocol(QString)), this, SLOT(logMessage(QString)));


    // standalone test
    connect(ui->selFreq, SIGNAL(clicked(bool)), this, SLOT(selFreqClicked()));
    connect(ui->freqInputBox, SIGNAL(editingFinished()), this, SLOT(selFreqClicked()));



}

void RigControlMainWindow::setupBandFreq()
{

    FreqPresetDialog  fPresetDialog(presetFreq, bands, &freqPresetChanged);

    fPresetDialog.exec();

    if (freqPresetChanged)
    {
        fPresetDialog.readSettings(presetFreq);
        logMessage(QString("RigControl: Band Freq Change, send new bandlist to logger"));
        //sendBandListLogger();
        freqPresetChanged = false;
    }
}

void RigControlMainWindow::currentRadioSettingChanged(QString radioName)
{

    switch( QMessageBox::question(
                this,
                tr("Minos RigControl"),
                tr("The settings for the current radio have been changed. \nDo you want to reload the settings for the radio now?"),
                QMessageBox::Yes |
                QMessageBox::No |
                QMessageBox::Cancel,
                QMessageBox::Cancel ) )
    {
    case QMessageBox::Yes:
        if (ui->selectRadioBox->currentText() != radioName)
        {
            bool ok;
            ui->selectRadioBox->setCurrentIndex(setupRadio->currentRadio.radioNumber.toInt(&ok, 10));
        }
        upDateRadio();
        break;
    case QMessageBox::No:

        break;
    case QMessageBox::Cancel:

        break;
    default:

        break;
    }
}


void RigControlMainWindow::updateSelectRadioBox()
{
    int curidx = ui->selectRadioBox->currentIndex();
    ui->selectRadioBox->clear();
    initSelectRadioBox();
    ui->selectRadioBox->setCurrentIndex(curidx);
}


void RigControlMainWindow::initSelectRadioBox()
{

    ui->selectRadioBox->addItem("");
    for (int i= 0; i < setupRadio->numAvailRadios; i++)
    {
        ui->selectRadioBox->addItem(setupRadio->availRadioData[i]->radioName);
    }

    //if (appName.length() > 0)
    //{
    //    sendRadioListLogger();

    //}
}

void RigControlMainWindow::selectRadio()
{
    setupRadio->setCurrentRadioName(ui->selectRadioBox->currentText());
    upDateRadio();
}


void RigControlMainWindow::setSelectRadioBoxVisible(bool visible)
{

    ui->SelectRadioTitle->setVisible(visible);
    ui->selectRadioBox->setVisible(visible);
}

void RigControlMainWindow::setRadioNameLabelVisible(bool visible)
{
    ui->radioNameDispLbl->setVisible(visible);
    ui->radioNameDisp->setVisible(visible);

}

void RigControlMainWindow::upDateRadio()
{
    int radioOpenStat = OPEN_FAILED;

    logMessage(QString("UpdateRadio: Radio requested = %1").arg(setupRadio->getCurrentRadioName()));

    if (appName.length() == 0)
    {
        logMessage(QString("UpdateRadio: Index Selected = %1").arg(QString::number(ui->selectRadioBox->currentIndex())));
    }


    pollTimer->stop();      // stop updates

    clrRigctldNames();
    clearSupportRitFlags();

    int ridx = 0;
    if (setupRadio->getCurrentRadioName() != "")
    {
        radioIndex = setupRadio->findCurrentRadio(setupRadio->getCurrentRadioName());
        ridx = radioIndex;
        if (ridx > -1 && ridx < setupRadio->numAvailRadios)  // find radio and update current radio pointer
        {
            // found radio, update currentRadio  to selected radiodata
            scatParams::copyRig(setupRadio->availRadioData[ridx], setupRadio->currentRadio);
            setupRadio->currentRadio.radioNumber = QString::number(ridx);           // save radio number

            if (radioCommsOK)
            {
                closeRadio();
            }

            if (setupRadio->currentRadio.radioModelNumber == 0)
            {
                //closeRadio();
                QMessageBox::critical(this, tr("Radio Error"), "Please configure a radio name and model");
                return;
            }

            if (setupRadio->currentRadio.rigCtldEnable)
            {
                radioOpenStat = openRigCtldRadio();
                setRigCltdIndicatorVisible(true);
                rigCtldIndicatorToggle(false);

                RigCtldStatusTimer->start(RIGCTLD_STATUS_TIMER_DUR);
            }
            else
            {
                radioOpenStat = openRadio();
            }


            if (radioOpenStat == OPEN_OK)
            {

 /*               if (setupRadio->currentRadio.radioModelNumber == hamlibData::RIGCTL)     // is it rigctl?
                {
                    getRigctldNames(setupRadio->currentRadio.networkAdd, setupRadio->currentRadio.networkPort.toUShort());
                    bool ok = false;
                    int rigNum = rigctld_radioNumber.toInt(&ok, 10);
                    if (ok)
                    {
                        irigctld_radioNumber = rigNum;
                    }
                    else
                    {
                        irigctld_radioNumber = 0;
                    }
                }
*/
                // setup local serial transvert switch
                if (radioCommsOK && setupRadio->currentRadio.transVertEnable
                        && setupRadio->currentRadio.enableTransSwitch
                        && setupRadio->currentRadio.enableLocTVSwMsg)
                {
                    selTransVertBandIndicator = "";     // force active tranvert indicator update
                    if (serialTVSw->getOpenFlag())
                    {
                        serialTVSw->closeComport();

                    }
                    if (serialTVSw->openComport(setupRadio->currentRadio.locTVSwComport))
                    {
                        //curTVComPort = setupRadio->currentRadio.locTVSwComport;
                        logMessage(QString("Local Transvert Switch Comport opened Ok = %1").arg(setupRadio->currentRadio.locTVSwComport));
                    }
                    else
                    {

                        QString errMsg = serialTVSw->error();
                        logMessage(QString("Local Transvert Switch Comport failed to open = %1 Error = %2").arg(setupRadio->currentRadio.locTVSwComport).arg(errMsg));
                    }

                }
                else
                {
                    if (serialTVSw->getOpenFlag())
                    {
                        serialTVSw->closeComport();

                    }

                }

                // only show transvert freq box is enabled
                setTransVertDisplayVisible(setupRadio->currentRadio.transVertEnable);
                sendTransVertEnabled(setupRadio->currentRadio.transVertEnable);   // send to logger
                sendTransVertSwitchToLogger(TRANSSW_NUM_DEFAULT);                                 // turn off transVerter Sw
                sendTransVertSwitchToComPort(TRANSSW_NUM_DEFAULT);
                transVertSwNum = TRANSSW_NUM_DEFAULT;
                selTransVertBandIndicator = "";     // force active tranvert indicator update

                setupRadio->saveCurrentRadio();

                ui->radioNameDisp->setText(setupRadio->currentRadio.radioName);

                if (appName.count() > 0)
                {
                    logMessage(QString("Update Radio: Logger Set Mode to %1").arg(selRadioMode));
                    loggerSetMode(selRadioMode);
                }
                else
                {

                    logMessage(QString("Update Radio: Set Mode USB Standalone"));
                    // initialise rig state

                    slogMode = "USB";
                    // set mode
                    //logMode = radio->convertQStrMode("USB");
                    setMode("USB", RIG_VFO_CURR);
                }

                // build supported band list for this radio
                // if it is a rigctld model, then use the radio model number connected to rigctld

                int modelNumber = setupRadio->currentRadio.radioModelNumber;
                if (modelNumber == hamlibData::RIGCTL)
                {
                    modelNumber = irigctld_radioNumber;
                }

                buildSupBandList(ridx, modelNumber, setupRadio->currentRadio.radioTransSupBands);

                // does the radio support control of volume control

                supVolume = radio->supportVolControl(modelNumber);
                logMessage(QString("Update Radio: Radio Supports Volume Control %1").arg(supVolume ? "True" : "False"));
                sendVolStatusToLog(ridx, supVolume);

                // does the radio support signal strength meter

                supSignalStrength = radio->supportSignalStrength(modelNumber);


                updateSupportedRadioIndicators();

                getRitSupportStatus(modelNumber);

                if (radioSupSetRit)
                {

                    if (ritEnable)
                    {
                        setRitFreqDisplayVisible(true);
                    }

                    setRitGetSetFreqIndicatorVisible(true);
                    ritSetFreqIndicatorToggle(radioSupSetRit);
                    ritGetFreqIndicatorToggle(radioSupGetRit);


                    //setRitFreqStr("0");             // turn off RIT
                }
                else
                {
                    setRitFreqDisplayVisible(false);
                    setRitGetSetFreqIndicatorVisible(false);
                    clearSupportRitFlags();

                }

                sendRitEnableStatusLogger();
                writeWindowTitle(appName);
                sendStatusToLogConnected();

                dumpRadioToTraceLog();


            }
            else
            {
                int msgOffSet = radioOpenStat;
                if (msgOffSet <= 0)
                {
                    if (msgOffSet < 0)
                    {
                        msgOffSet = msgOffSet * -1;
                        if (msgOffSet > radioOpenMessages.count())
                        {
                            trace(QString("UpdateRadio: Radio Failed to Connect - Error Message Number out of range = %1").arg(msgOffSet));
                        }
                    }
                }


                trace(QString("#### Radio Failed to connect Error Code = %1, %2  ####").arg(radioOpenStat).arg(radioOpenMessages[radioOpenStat * -1]));
                sendStatusToLogDisConnected();
            }


        }
    }
    else
    {   // no radio selected
        trace("No radio selected");
        ui->radioNameDisp->setText("");
        closeRadio();
        writeWindowTitle(appName);
    }


    if (radio->get_serialConnected())
    {
        if (setupRadio->currentRadio.pollInterval == RIG_DEFAULT_POLLINTERVAL)
        {
            pollTime = 500;
        }
        else
        {
            pollTime = 1000 * setupRadio->currentRadio.pollInterval.toInt();
        }

        pollTimer->start(pollTime);             // start timer to send poll radio
    }

    if (appName.length() > 0)
    {
        msg->rigCache.publish();
    }
}

void RigControlMainWindow::refreshRadio()
{
    logMessage(QString("refreshRadio: Index Selected = %1").arg(QString::number(ui->selectRadioBox->currentIndex())));

    int ridx = 0;
    radioIndex = ui->selectRadioBox->currentIndex();
    ridx = radioIndex;
    if (ridx >= 0)
    {
        if (radioCommsOK)
        {
            logMessage(QString("Refresh Radio: Logger Set Mode to %1").arg(selRadioMode));
            loggerSetMode(selRadioMode);
            writeWindowTitle(appName);
            sendStatusToLogConnected();
            dumpRadioToTraceLog();
        }
        else
        {
            upDateRadio();
        }
    }
}



int RigControlMainWindow::openRigCtldRadio()
{
    int retCode = 0;
    radioCommsOK = false;

    // check rigctld file exists
    setupRadio->getRigCtldExePathFromFile();
#if defined Q_OS_WIN32
    QString filename = setupRadio->getRigCtldExePath() + RIGCTL_WIN32_EXE_FILENAME;
#elif defined Q_OS_LINUX
    QString filename = setupRadio->getRigCtldExePath() + RIGCTL_LINUX_EXE_FILENAME;
#elif defined Q_OS_MAC
    QString filename = setupRadio->getRigCtldExePath() + RIGCTL_MAC_EXE_FILENAME;
#endif

    if (!FileExists(filename))
    {
        trace(QString("openRigCtld: rigctld is missing from %1").arg(filename));
        return RIGCTLD_EXE_MISSING;
    }

    trace(QString("openRigCtld: found rigctld = %1").arg(filename));

    if (rigCtldProcess->state() == QProcess::Running)
    {
        trace(QString("openRigCtldRadio: rigctld running - killing"));
        if (!rigCtldKill())
        {
            trace(QString("openRigCtldRadio: rigctld did not stop"));
            return RIGCTLD_FAILED_TO_STOP;
        }
    }

    rigCtldTrace::rigCtldTraceCodes traceCode = rigCtldTrace::rigCtldTraceCodes::NONE;
    if (radio->getTraceState())
    {
        traceCode = rigCtldTrace::rigCtldTraceCodes::VERBOSE;
    }

    // start rigctld
    trace(QString("openRigCtldRadio: starting rigctld"));
    runRigCtlDaemon(setupRadio->currentRadio.radioMfg_Name, QString::number(setupRadio->currentRadio.radioModelNumber), setupRadio->currentRadio.comport,
                                               QString::number(setupRadio->currentRadio.baudrate), QString::number(setupRadio->currentRadio.databits), setupRadio->currentRadio.civAddress, setupRadio->currentRadio.rigCtldNetworkAdd, setupRadio->currentRadio.rigCtldNetworkPort,
                                               QString::number(setupRadio->currentRadio.stopbits), setupRadio->currentRadio.parity, "ON", "ON", traceCode);


    // wait for rigctld to start
    int waitStartDur = 500;
    while (rigCtldProcess->state() != QProcess::Running && waitStartDur > 0)
    {
        sleepFor(100);
        waitStartDur--;
    }

    if (waitStartDur > 0)
    {
        trace(QString("openRigCtldRadio: rigctld running for radio %1").arg(setupRadio->currentRadio.radioModel));
    }
    else
    {
        trace(QString("openRigCtldRadio: rigctld failed for radio %1").arg(setupRadio->currentRadio.radioModel));
        return RIGCTLD_FAILED;
    }


    if (rigCtldConnectDelay != 0)
    {
        trace(QString("openRigCtldRadio: Delay = %1 secs before connecting to rigCtld").arg(rigCtldConnectDelay));
        delay(rigCtldConnectDelay);
    }

    // now open radio using rigctld model
    trace(QString("openRigCtldRadio: Open radio = %1, via Rigctld").arg(setupRadio->currentRadio.radioModel));
    retCode = radio->init(setupRadio->currentRadio, RIGCTLD_ON);
    if (retCode < 0)
    {
        radio->closeRig();
        logMessage(QString("openRigCtldRadio: Error Opening Radio %1, Error Code = %2").arg(setupRadio->currentRadio.radioModel).arg(QString::number(retCode)));
        hamlibError(retCode, "RigCtld Open Radio");
        return OPEN_FAILED;
    }

    // let's see if we can get freq from radio and confirm comms
    if (radio->get_serialConnected())
    {

        int retCode = RIG_OK;
        showStatusMessage(QString("Attempting to communicate with radio via Rigctld - %1").arg(setupRadio->currentRadio.radioName));
        retCode = radio->getFrequency(RIG_VFO_CURR, &rfrequency);


        if (retCode < RIG_OK)
        {
            logMessage(QString("openRigctldRadio: Test Communication - Get Freq error, code = %1").arg(QString::number(retCode)));
            hamlibError(retCode, "Test Radio Connection via Rigctld\n\nMinos tried to read the radio frequency,\nbut nothing was received from the radio.\n\nPlease check connections and/or settings.\nSome radios/interfaces may require CTS/RTS to be selected in Handshake.");
            //sendStatusToLogDisConnected();
            return OPEN_FAILED;
        }
        else
        {
            radioCommsOK = true;
        }

    }

    if (radioCommsOK)
    {
        logMessage(QString("openRigctldRadio: Radio Opened %1").arg(setupRadio->currentRadio.radioName));
        showStatusMessage(QString("Radio Opened Rigctld: %1").arg(setupRadio->currentRadio.radioName));

        if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_SERIAL)
        {
            showStatusMessage(QString("Connected via RigCtld: %1 - %2, %3, %4, %5, %6, %7, %8")
                              .arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.radioModel).trimmed().arg(setupRadio->currentRadio.comport).arg(setupRadio->currentRadio.baudrate).arg(setupRadio->currentRadio.databits)
                              .arg(setupRadio->currentRadio.stopbits).arg(radio->getParityCodeNames()[setupRadio->currentRadio.parity]).arg(radio->getHandShakeNames()[setupRadio->currentRadio.handshake]));
        }

        /*
        else if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_NETWORK || rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_UDP_NETWORK)
        {
            if (setupRadio->currentRadio.radioModelNumber == hamlibData::RIGCTL)
            {
                showStatusMessage(QString("Connected: %1 - %2, %3:%4 - %5 %6").arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.radioModel.trimmed()).arg(setupRadio->currentRadio.networkAdd).arg(setupRadio->currentRadio.networkPort).arg(rigctld_radioMfg).arg(rigctld_radioName));
            }
            else
            {
                showStatusMessage(QString("Connected: %1 - %2, %3:%4").arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.radioModel.trimmed()).arg(setupRadio->currentRadio.networkAdd).arg(setupRadio->currentRadio.networkPort));
            }

        }
        */
        else if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_NONE)
        {
            showStatusMessage(QString("Connected via Rigctld: %1 - %2").arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.radioModel.trimmed()));
        }

    }
    else
    {

        logMessage(QString("Radio Open Error"));
        showStatusMessage(tr("Radio Open error"));
        return OPEN_FAILED;
    }

    PubSubName psname(setupRadio->currentRadio.radioName);

    msg->rigCache.setTpm(psname, 1);
    msg->rigCache.publish();
    return OPEN_OK;



}








int RigControlMainWindow::openRadio()
{

    int retCode = 0;
    radioCommsOK = false;

    if (setupRadio->currentRadio.radioName == "")
    {
        logMessage(QString("Open Radio: No radio name!"));
        showStatusMessage("Please select a Radio");
        return OPEN_FAILED;
    }

    logMessage(QString("Open Radio: Opening Radio %1 PortType %2").arg(setupRadio->currentRadio.radioName).arg(hamlibData::portTypeList[setupRadio->currentRadio.portType]));
    showStatusMessage(QString("Opening Radio: %1").arg(setupRadio->currentRadio.radioName));

    if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_SERIAL)
    {
        if(setupRadio->comportAvial(setupRadio->currentRadio.radioNumber.toInt(), setupRadio->currentRadio.comport) == -1)
        {
            logMessage(QString("Open Radio: Check comport - defined port %1 not available on computer").arg(setupRadio->currentRadio.comport));
            showStatusMessage(QString("Comport %1 no longer configured on computer?").arg(setupRadio->currentRadio.comport));
            return OPEN_FAILED;
        }

        if (setupRadio->currentRadio.comport == "")
        {
            logMessage(QString("Open Radio: No comport"));
            showStatusMessage("Please select a Comport");
            return OPEN_FAILED;
        }

    }

    if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_NETWORK || rig_port_e(setupRadio->currentRadio.portType == RIG_PORT_UDP_NETWORK))
    {
        if (setupRadio->currentRadio.networkAdd == "" || (setupRadio->currentRadio.networkPort == ""))
        {
            logMessage(QString("Open Radio: No network or Port Number"));
            showStatusMessage("Please enter a network Address and Port Number");
            return OPEN_FAILED;
        }

    }
    if (setupRadio->currentRadio.radioModel == "")
    {
        logMessage(QString("Open Radio: No radio model"));
        showStatusMessage("Please select a radio model");
        return OPEN_FAILED;
    }


    if (!radio->get_serialConnected())
    {
        // if radio is already open, don't reinit it
        retCode = radio->init(setupRadio->currentRadio, RIGCTLD_OFF);
        if (retCode < 0)
        {
            radio->closeRig();
            logMessage(QString("Error Opening Radio Error Code = %1").arg(QString::number(retCode)));
            hamlibError(retCode, "Open Radio");
            return OPEN_FAILED;
        }

    }


    // let's see if we can get freq from radio and confirm comms
    if (radio->get_serialConnected())
    {

        int retCode = RIG_OK;
        showStatusMessage(QString("Attempting to communicate with radio - %1").arg(setupRadio->currentRadio.radioName));
        //delay(1);
        retCode = radio->getFrequency(RIG_VFO_CURR, &rfrequency);


        if (retCode < RIG_OK)
        {
            logMessage(QString("Open Radio: Test Communication - Get Freq error, code = %1").arg(QString::number(retCode)));
            hamlibError(retCode, "Test Radio Connection\n\nMinos tried to read the radio frequency,\nbut nothing was received from the radio.\n\nPlease check connections and/or settings.\nSome radios/interfaces may require CTS/RTS to be selected in Handshake.");
            //sendStatusToLogDisConnected();
            return OPEN_FAILED;
        }
        else
        {
            radioCommsOK = true;
        }

    }




    if (radioCommsOK)
    {
        logMessage(QString("Open Radio: Radio Opened %1").arg(setupRadio->currentRadio.radioName));
        showStatusMessage(QString("Radio Opened: %1").arg(setupRadio->currentRadio.radioName));

        if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_SERIAL)
        {
            showStatusMessage(QString("Connected: %1 - %2, %3, %4, %5, %6, %7, %8")
                              .arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.radioModel).trimmed().arg(setupRadio->currentRadio.comport).arg(setupRadio->currentRadio.baudrate).arg(setupRadio->currentRadio.databits)
                              .arg(setupRadio->currentRadio.stopbits).arg(radio->getParityCodeNames()[setupRadio->currentRadio.parity]).arg(radio->getHandShakeNames()[setupRadio->currentRadio.handshake]));
        }
        else if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_NETWORK || rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_UDP_NETWORK)
        {
            if (setupRadio->currentRadio.radioModelNumber == hamlibData::RIGCTL)
            {
                showStatusMessage(QString("Connected: %1 - %2, %3:%4 - %5 %6").arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.radioModel.trimmed()).arg(setupRadio->currentRadio.networkAdd).arg(setupRadio->currentRadio.networkPort).arg(rigctld_radioMfg).arg(rigctld_radioName));
            }
            else
            {
                showStatusMessage(QString("Connected: %1 - %2, %3:%4").arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.radioModel.trimmed()).arg(setupRadio->currentRadio.networkAdd).arg(setupRadio->currentRadio.networkPort));
            }

        }
        else if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_NONE)
        {
            showStatusMessage(QString("Connected: %1 - %2").arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.radioModel.trimmed()));
        }

    }
    else
    {

        logMessage(QString("Radio Open Error"));
        showStatusMessage(tr("Radio Open error"));
        return OPEN_FAILED;
    }

    PubSubName psname(setupRadio->currentRadio.radioName);

    msg->rigCache.setTpm(psname, 1);
    msg->rigCache.publish();
    return OPEN_OK;
}

void RigControlMainWindow::closeRadio()
{
    pollTimer->stop();
    int retCode;

    if (radio->get_serialConnected())
    {
        logMessage(QString("closeRadio: closing radio"));
        retCode = radio->closeRig();
        if (retCode < 0)
        {
            logMessage(QString("closeRadio: error closing radio %1").arg(retCode));
        }
        else
        {
            logMessage(QString("closeRadio: radio closed successfully"));
        }


    }

    if (setupRadio->currentRadio.rigCtldEnable)
    {
        logMessage(QString("closeRadio: closing rigCtld"));

        if (rigCtldProcess)
        {
            if (rigCtldProcess->state() == QProcess::Running)
            {
                if (!rigCtldKill())
                {
                    logMessage(QString("closeRadio: rigCtld daemon failed to stop"));
                }
                else
                {
                    logMessage(QString("closeRadio: rigCtld daemon has stopped"));
                }
                setRigCltdIndicatorVisible(false);
                RigCtldStatusTimer->stop();
            }


        }

    }



    radioCommsOK = false;

    if (serialTVSw->getOpenFlag())
    {
        serialTVSw->closeComport();
    }

    showStatusMessage("Disconnected");
    sendStatusToLogDisConnected();
    displayFreqVfo(0.0);
    displayTransVertVfo(0.0);
    ui->transVertBandDisp->setText("");
    ui->transVertSwNum->setText("");
    turnOffAllsupRadioIndicators();
    displaySignalStrength(-54);
    setRigCltdIndicatorVisible(false);
    logMessage(QString("Radio Closed"));

    msg->rigCache.publish();
}






void RigControlMainWindow::writeWindowTitle(QString appName)
{
    if (appName.length() > 0)
    {
        this->setWindowTitle("Minos Rig Control - " + appName + " - Logger");
    }
    else
    {
        this->setWindowTitle("Minos Rig Control - Local");
    }

}


void RigControlMainWindow::setPolltime(int interval)
{
    pollTime = interval;
}

int RigControlMainWindow::getPolltime()
{

    return pollTime;
}

void RigControlMainWindow::cmdLockOn()
{
    cmdLockFlag = true;
    logMessage(QString("Lockon: Command Lock On"));
}

void RigControlMainWindow::cmdLockOff()
{
    cmdLockFlag = false;
    logMessage(QString("Lockoff: Command Lock Off"));
}

void RigControlMainWindow::getRadioInfo()
{
    logMessage(QString("Request radio info"));
    if (cmdLockFlag)
    {
        trace(QString("GetRadioInfo: Command Lock on"));
        return;
    }
    chkRadioMgmModeChanged();

    int retCode;
    if (radioCommsOK)
    {
        logMessage(QString("Get radio frequency"));
        retCode = getAndSendFrequency(RIG_VFO_CURR);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Freq error %1").arg(QString::number(retCode)));
            hamlibError(retCode, "Request Freq");

        }

    }

    if (radioCommsOK)
    {

        logMessage("Get radio mode");
        retCode = getAndSendMode(RIG_VFO_CURR);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Mode error %1").arg(QString::number(retCode)));
            hamlibError(retCode, "Request Mode");

        }
        else
        {
            logMessage(QString("Got Mode = %1").arg(radio->convertModeQstr(rmode)));
        }
    }




    if (radioCommsOK && radioSupSetRit && ritEnable)
    {
        logMessage((QString("Poll RIT Info - Get Rit Freq = %1 - Get Rit State = %2").arg(radioSupGetRit ? "True" : "False").arg(radioSupGetRitState ?  "True" : "False")));


        if (radioSupGetRit)
        {
            retCode = getRitFreq(RIG_VFO_CURR);
            if (retCode < 0)
            {
                // error
                logMessage(QString("Get radioInfo: Get RIT Freq error").arg(QString::number(retCode)));
                hamlibError(retCode, "Request RIT Freq");
            }
            else
            {
                logMessage(QString("Get radioInfo: Get RIT Freq = %1").arg(convertRitFreqToStr(rRitFreq)));
            }

        }



        if (radioSupGetRitState)
        {
            bool ritStatus = false;
            retCode = getRitRadioStatus(RIG_VFO_CURR, &ritStatus);
            if (retCode < 0)
            {
                //error
                logMessage(QString("Get radioInfo: Get RIT state error").arg(QString::number(retCode)));
                hamlibError(retCode, "Request RIT State");
            }
            else
            {
                if (ritStatus != radioRitOn)
                {
                    radioRitOn = ritStatus;
                    logMessage(QString("Get radioInfo: Radio Rit Status = %1").arg(radioRitOn ? "On" : "Off"));
                    ritIndicatorToggle(radioRitOn);
                    sendRadioRitStatusLogger(radioRitOn);
                }

            }

        }


    }

    if (radioCommsOK && supVolume)
    {
        retCode = getVolume(RIG_VFO_CURR);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Volume error").arg(QString::number(retCode)));
            hamlibError(retCode, "Request Volume");
        }

    }


    if (radioCommsOK && supSignalStrength)
    {
        retCode = getSignalStrength(RIG_VFO_CURR);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Volume error").arg(QString::number(retCode)));
            hamlibError(retCode, "Request Volume");
        }

    }
/*
    if (radioCommsOK)
    {

        retCode = getTXStatus(RIG_VFO_CURR);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get TXStatus error").arg(QString::number(retCode)));
            hamlibError(retCode, "Request TX Status");
        }

    }

*/
    msg->rigCache.publish();
}


void RigControlMainWindow::onSelectRadio(PubSubName s, QString mode)
{

    logMessage(QString("Recieved SelectRadio from Logger = %1, mode = %2").arg(s.toString()).arg(mode));


    if (!mode.isEmpty())
    {
        selRadioMode = mode;
    }

    QString oldRadio = setupRadio->getCurrentRadioName();

    setupRadio->setCurrentRadioName(s.key());

    if (!s.isEmpty() && s.key() == oldRadio)
    {
        refreshRadio();
    }
    else
    {
        upDateRadio();
    }
    msg->rigCache.invalidate();
}
void RigControlMainWindow::setTpm(int t, QString f)
{
    // tpm received from logger, with freq
    logMessage(QString("Recieved tpm %1 from Logger, freq = %2").arg(t).arg(f));
    tpm = t;
    sendTpm(t);
    logger_freq = f;
    curVfoFrq = convertStrToFreq(f);
    setFreq(f, RIG_VFO_CURR);

}
void RigControlMainWindow::loggerSetFreq(QString freq)
{
    logMessage(QString("Recieved Freq from Logger = %1").arg(freq));
    if (radioCommsOK && !rigErrorFlag)
    {
        logMessage(QString("new freq %1, old freq %2, old tpm %3").arg(freq).arg(logger_freq).arg(tpm));
        if (freq == NO_BAND_SUPPORT)
        {
            logMessage(QString("loggerSetFreq: No transverter found for this band"));
            clearTransVertSupport();
            return;
        }

        double lf = convertStrToFreq(logger_freq);
        double f = convertStrToFreq(freq);
        if (!logger_freq.isEmpty() && std::abs(lf - f) > 1000.1)
        {
            logMessage("Setting tpm to null");
            tpm = 0;
            sendTpm(tpm);
        }
        logger_freq = freq;
        setFreq(freq, RIG_VFO_CURR);
    }
    // but the rig hasn't updated...
    msg->rigCache.publish();
}


void RigControlMainWindow::setFreq(QString freq, vfo_t vfo)
{
    cmdLockOn();    // lock get radio info
    bool ok = false;
    int retCode = 0;
    QString sfreq = freq;
    QString cb;
    int tvNum = 0;


    double f = sfreq.toDouble(&ok);
    logMessage(QString("SetFreq: Change to Freq = %1").arg(QString::number(f)));

    if (ok)
    {
        BandList &blist = BandList::getBandList();
        BandInfo bi;
        bool bandOK = blist.findBand(f, bi);
        if (bandOK)
        {
            cb = bi.uk;
            logMessage(QString("SetFreq: Band found = %1").arg(cb));
        }

        if (/*cb != selTvBand &&*/ setupRadio->currentRadio.transVertEnable && setupRadio->currentRadio.numTransverters != 0)
        {

            // does a transverter support this band?

            bool b = false;
            while (tvNum < setupRadio->currentRadio.numTransverters)
            {
                if (setupRadio->currentRadio.transVertSettings[tvNum]->band == cb)
                {
                    b = true;
                    logMessage(QString("SetFreq: Found Transverter %1 for this freq.").arg(setupRadio->currentRadio.transVertSettings[tvNum]->band));
                    break;
                }
                tvNum++;
            }

            if (b)  // found a tranverter supporting this band
            {
                selTvBand = cb;
                ui->transVertBandDisp->setText(cb);
                showActiveTransVertIndicator(cb);
                sendTransVertStatusToLog(true);

                if (setupRadio->currentRadio.enableTransSwitch)
                {
                    if (setupRadio->currentRadio.transVertSettings[tvNum]->transSwitchNum != transVertSwNum)
                    {
                        transVertSwNum = setupRadio->currentRadio.transVertSettings[tvNum]->transSwitchNum;
                        ui->transVertSwNum->setText(transVertSwNum);
                        transVertSwNum = setupRadio->currentRadio.transVertSettings[tvNum]->transSwitchNum;
                        sendTransVertSwitchToLogger(transVertSwNum);
                        sendTransVertSwitchToComPort(transVertSwNum);
                        logMessage(QString("SetFreq: Send TransVert Switch number - %1").arg(transVertSwNum));

                    }
                }
                else
                {
                    if (transVertSwNum != TRANSSW_NUM_DEFAULT)
                    {
                        transVertSwNum = TRANSSW_NUM_DEFAULT;
                        ui->transVertSwNum->setText(TRANSSW_NUM_DEFAULT);
                        sendTransVertSwitchToLogger(TRANSSW_NUM_DEFAULT);
                        sendTransVertSwitchToComPort(TRANSSW_NUM_DEFAULT);
                        updateSupportedRadioIndicators();
                    }
                    logMessage(QString("SetFreq: Transvert Switch not enabled - %1").arg(TRANSSW_NUM_DEFAULT));
                }

                // now calculate the freq
                f = f - setupRadio->currentRadio.transVertSettings[tvNum]->transVertOffset;
                logMessage(QString("SetFreq: Transvert Enabled Freq = %1").arg(QString::number(f)));

                sendTransVertOffsetToLogger(tvNum);


            }
            else
            {
                // no transverter found for this band
                logMessage(QString("SetFreq: No transverter found for this band"));
                clearTransVertSupport();
                sendTransVertStatusToLog(false);


            }

        }

        if (radioCommsOK)
        {

            retCode = radio->setFrequency(f, vfo);
            if (retCode != RIG_OK)
            {
                if (retCode == -9)
                {
                    logMessage(QString("SetFreq: Invalid Tx Freq for Radio, Freq = %1").arg(QString::number(f)));
                    cmdLockOff();
                    return;
                }

                logMessage(QString("SetFreq: Error Setting Freq Code = %1").arg(retCode));
                hamlibError(retCode, "SetFreq");
            }
            else
            {
                logMessage(QString("SetFreq: Rig set to Freq = %1").arg(QString::number(f)));
            }

        }
        else
        {
            logMessage(QString("SetFreq: Radio is not connected"));
        }
    }
    else
    {
        logMessage(QString("SetFreq:: Freq conversion from string %1 failed").arg(sfreq));
    }

    cmdLockOff();
}


void RigControlMainWindow::clearTransVertSupport()
{
    selTvBand = "";
    displayTransVertVfo(0.0);
    ui->transVertBandDisp->setText("");
    transVertSwNum = TRANSSW_NUM_DEFAULT;
    ui->transVertSwNum->setText(TRANSSW_NUM_DEFAULT);
    selTransVertBandIndicator = "";
    sendTransVertSwitchToLogger(TRANSSW_NUM_DEFAULT);
    sendTransVertSwitchToComPort(TRANSSW_NUM_DEFAULT);
    updateSupportedRadioIndicators();
}

int RigControlMainWindow::getAndSendFrequency(vfo_t vfo)
{
    double transVertF = 0;
    int retCode = 0;
    int tvNum = 0;
    bool b = false;

    retCode = radio->getFrequency(vfo, &rfrequency);
    if (retCode == RIG_OK)
    {
        curVfoFrq = rfrequency;

        if (setupRadio->currentRadio.transVertEnable && setupRadio->currentRadio.numTransverters > 0)
        {
            if (selTvBand != "")
            {
                logMessage(QString("Get Freq: Transvert enabled"));
                // look for supporting transverter

                while (tvNum < setupRadio->currentRadio.numTransverters)
                {
                    if (setupRadio->currentRadio.transVertSettings[tvNum]->band == selTvBand)
                    {
                        b = true;
                        break;
                    }
                    tvNum++;
                }

                if (b)
                {
                    logMessage(QString("Transverter %1 name %2 offset %3 rfreq %4").arg(tvNum)
                               .arg(setupRadio->currentRadio.transVertSettings[tvNum]->transVertName)
                               .arg(setupRadio->currentRadio.transVertSettings[tvNum]->transVertOffset)
                               .arg(rfrequency)
                               );

                    transVertF = rfrequency + setupRadio->currentRadio.transVertSettings[tvNum]->transVertOffset;
                    logMessage(QString("Get Freq: TransvertF = %1").arg(QString::number(transVertF)));
                }

                logMessage(QString("Get Freq: Transvert Freq. = %1").arg(QString::number(transVertF)));
                curTransVertFrq = transVertF;
                displayTransVertVfo(transVertF);

            }
            else
            {
                //setTransVertDisplayVisible(false);
                logMessage(QString("GetFreq: No transvert band set for this freq = %1").arg(QString::number(curVfoFrq)));
            }
        }

        displayFreqVfo(rfrequency);

        if (setupRadio->currentRadio.transVertEnable && b)
        {
            sendFreqToLog(transVertF);
        }
        else
        {
            sendFreqToLog(rfrequency);
        }
    }
    else
    {
        logMessage(QString("Get radioInfo: Get Freq error, code = %1").arg(QString::number(retCode)));
        hamlibError(retCode, "Request Frequency");
    }
    return retCode;
}


QString RigControlMainWindow::getBand(freq_t freq)
{
    for (int i = 0; i < setupRadio->bands.count(); i++)
    {
        if (freq >= setupRadio->bands[i]->fLow && freq <= setupRadio->bands[i]->fHigh)
        {
            return setupRadio->bands[i]->name;
        }
    }
    return "";
}

void RigControlMainWindow::setTransVertDisplayVisible(bool visible)
{
    ui->transVertFreqA->setVisible(visible);
    ui->TVertTitleA->setVisible(visible);
    ui->transVertSwNum->setVisible(visible);
    ui->transVertSwLbl->setVisible(visible);
    ui->transVertBandDisp->setVisible(visible);
    ui->transVertBandDispLbl->setVisible(visible);
}

void RigControlMainWindow::chkRadioMgmModeChanged()
{
    static QString mgmMode;
    if (setupRadio->currentRadio.mgmMode != mgmMode)
    {
        ui->mgmMode->setText(setupRadio->currentRadio.mgmMode);
    }
}


/******************** Rigctld *************************************/


void RigControlMainWindow::getRigctldNames(QString address, quint16 port)
{



                RigCtldClient *client;
                client = new RigCtldClient();

                if (!client->connectToHost(address, port))
                {
                    logMessage(QString("getRigctldNames - Connect Failed - %1:%2").arg(address).arg(QString::number(port)));
                }
                else
                {
                    QByteArray msg = QString("+\\dump_caps\n").toLatin1();
                    if (!client->writeData(msg))
                    {
                        logMessage(QString("getRigctldNames - Send Data Failed"));
                    }
                    else
                    {
                        logMessage(QString("getRigctldNames - Data sent ok"));
                        client->startRecvTimer(3000);
                        QEventLoop loop;
                        QObject::connect( client, SIGNAL( finished() ), &loop, SLOT( quit() ) );
                        loop.exec();
                    }

                    if (client->getRetCode() >=0 && client->checkMsgRecieved())
                    {
                        rigctld_radioNumber = client->getRadioModel();
                        rigctld_radioName = client->getRadioModelName();
                        rigctld_radioMfg = client->getRadioManufacturerName();
                        logMessage(QString("getrigctld - Got names ok - %1 %2 %3").arg(rigctld_radioNumber).arg(rigctld_radioMfg).arg(rigctld_radioName));
                    }
                    else
                    {
                        logMessage(QString("Error Getting radioModel from rigctld - error code = %1").arg(client->getRetCode()));
                    }

                    logMessage(QString("getRigctldNames - Disconnect from host"));
                    client->disconnectFromHost();
                }




}


void RigControlMainWindow::clrRigctldNames()
{
    rigctld_radioNumber = "";
    rigctld_radioName = "";
    rigctld_radioMfg = "";

}


void RigControlMainWindow::runRigCtlDaemon(const QString& manufacturer, const QString& model, const QString& comport,
                                           const QString& baudRate, const QString& dataBits, const QString& civ, const QString& netAdd, const QString& portNum,
                                           const QString& stopBits, const int& parity, const QString& rtsState, const QString& dtrState,
                                           rigCtldTrace::rigCtldTraceCodes diagnostics)
{

    setupRadio->getRigCtldExePathFromFile();;

#if defined Q_OS_WIN32
    QString program = setupRadio->getRigCtldExePath() + RIGCTL_WIN32_EXE_FILENAME;
#elif defined Q_OS_LINUX
    QString program = setupRadio->getRigCtldExePath() + RIGCTL_LINUX_EXE_FILENAME;
#elif defined Q_OS_MAC
    QString program = setupRadio->getRigCtldExePath() + RIGCTL_MAC_EXE_FILENAME;
#endif

    QStringList arguments;

#if defined Q_OS_WIN32
    QString serPort = comport.trimmed();
#elif defined Q_OS_LINUX
    QString serPort = "/dev/" + comport.trimmed();
#elif defined Q_OS_MAC
    QString serPort = "/dev/" + comport.trimmed();
#endif

    QStringList parityNames;
    QString parityName;
    QString networkAdd = netAdd.trimmed();
    QString networkPort = portNum.trimmed();

    if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_SERIAL)
    {
        parityNames = radio->getParityCodeNames();
        parityName = parityNames[parity];
        arguments << "-m" + model.trimmed() << "-r" + serPort  << "-s" + baudRate.trimmed() << "--set-conf=data_bits=" + dataBits.trimmed() << "--set-conf=stop_bits=" + stopBits.trimmed()
                  << "--set-conf=serial_parity=" + parityName.trimmed() << "--set-conf=rts_state=" + rtsState.trimmed() << "--set-conf=dtr_state=" + dtrState.trimmed();

        if (manufacturer == "Icom")
        {
            if (!civ.isEmpty())
            {
               arguments << "--set-conf=civaddr=" + civ.trimmed();

               trace(QString("runRigCtlDaemon:: using icom civ address = %1").arg(civ.trimmed()));

            }
        }
    }
    else if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_NONE)
    {
        // for dummy radio
        arguments << "-m" + model.trimmed();
    }

    // this is the rigctld network address usually local host 127.0.0.1 and port usually 4532

    if (!networkAdd.isEmpty())
    {
        arguments << QString("--listen-addr=%1").arg(networkAdd);
    }
    else
    {
        networkAdd = RIGCTLD_LOCAL_HOST_ADDRESS;
        arguments << QString("--listen-addr=%1").arg(QString(networkAdd));
        trace(QString("runRigCtlDaemon:: network address is empty - using default %1").arg(networkAdd));
    }



    if (!networkPort.isEmpty())
    {
        arguments << QString("--port=%1").arg(networkPort);
    }
    else
    {
        networkPort = RIGCTLD_DEFAULT_PORT_ADDRESS;
        arguments << QString("--port=%1").arg(networkPort);
        trace(QString("runRigCtlDaemon:: port address is empty - using default %1").arg(networkPort));
    }

    if (diagnostics != rigCtldTrace::rigCtldTraceCodes::NONE)
    {
        arguments << rigCtldTrace::rigCtldTraceStr[diagnostics];
    }


    trace(QString("runRigCtlDaemon:: start rigCtlD - manufacturer = %1, model = %2, comport = %3, baudrate = %4, databits = %5, stopbits = %6, parity = %7, rtsState = %8, dtrState = %9, civ = %10, netaddress = %11, netPort = %12")
          .arg(manufacturer).arg(model).arg(serPort).arg(baudRate).arg(dataBits).arg(stopBits).arg(parityName).arg(rtsState).arg(dtrState).arg(civ).arg(networkAdd).arg(networkPort));

    rigCtldProcess->start(program, arguments);


}


void RigControlMainWindow::rigCtldMessage()
{
    if (rigCtldProcess->state() == QProcess::Running)
    {
        QString line = rigCtldProcess->readLine();
            trace(QString("rigCtld-StandardOut:: %1").arg(line));
    }


}


void RigControlMainWindow::rigCtldErrorMessage()
{
    if (rigCtldProcess->state() == QProcess::Running)
    {
        QString r = rigCtldProcess->readAllStandardError();
        trace(QString("rigCtld-ErrorOut:: %1").arg(r));
    }
}

void RigControlMainWindow::rigCtldStarted()
{
    trace(QString("rigCtld:: daemon started!"));
}


bool RigControlMainWindow::rigCtldKill()
{
    int killTimeout = RIGCTLD_PROCESS_TIMEOUT;
    trace(QString("rigCtldKill: starting to kill  rigCtld"));
    rigCtldProcess->kill();

    while (killTimeout > 0)
    {
        if (rigCtldProcess->state() == QProcess::NotRunning)
        {
            trace(QString("rigCtldKill: rigCtld kill complete"));
            return true;
        }

        sleepFor(100);
        killTimeout--;
    }

    trace(QString("rigCtldKill: rigCtld kill failed - timeout!"));
    return false;
}

void RigControlMainWindow::setRigCltdIndicatorVisible(bool visible)
{
    ui->rigCtldIndicator->setVisible(visible);
    ui->rigCtldIndicatorLabel->setVisible(visible);

}


void RigControlMainWindow::rigCtldIndicatorToggle(bool state)
{
    if (state)
    {
        ui->rigCtldIndicator->setStyleSheet(RIGCTLD_INDICATOR_ON);
    }
    else
    {
        ui->rigCtldIndicator->setStyleSheet(RIGCTLD_INDICATOR_OFF);
    }
}



void RigControlMainWindow::rigCtldStatusTimeout()
{
    if (setupRadio->currentRadio.rigCtldEnable)
    {

        if (rigCtldProcess->state() == QProcess::Running)
        {
           rigCtldIndicatorToggle(true);
        }
        else
        {
           rigCtldIndicatorToggle(false);
        }


    }
}


void RigControlMainWindow::getRigCtldConnectDelay()
{
    QString fileName;
    if (appName == "")
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOCAL + MINOS_RADIO_CONFIG_FILE;
    }
    else
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RADIO_CONFIG_FILE;
    }


    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("RigCtld");
    rigCtldConnectDelay = config.value("RigCtldConnectDelay", DEFAULT_RIGCTLD_CONNECT_DELAY).toInt();
    config.endGroup();

    if (rigCtldConnectDelay > MAX_RIGCTLD_CONNECT_DELAY)
    {
        logMessage(QString("ERROR rigctld connect delay value = %1, too high, setting max delay = %2").arg(rigCtldConnectDelay).arg(MAX_RIGCTLD_CONNECT_DELAY));
        rigCtldConnectDelay = 5;
        }


}



/*****************  cache data ***************/


void RigControlMainWindow::initCacheData()
{

    if (setupRadio->availRadioData.count() > 0)
    {

        for (int i = 0; i < setupRadio->availRadioData.count(); i++)
        {
            QStringList supBandList;
            int radioModelNumber = setupRadio->availRadioData[i]->radioModelNumber;
            buildSupBandList(i, radioModelNumber, supBandList);
            sendBandListLogger(i, supBandList);
            bool f = radio->supportVolControl(radioModelNumber);
            sendVolStatusToLog(i, f);
        }
    }


}

// this is a bit brutal...updates all rigdetails even if data hasn't changed...

void RigControlMainWindow::updateRigDetailsCache()
{
    // update riglist first
    sendRadioListLogger();
    // now rigdetails available before radio is opened
    initCacheData();
    msg->rigCache.publish();

}


/******************** Supported Bands  ********/



void RigControlMainWindow::buildSupBandList(int radioIdx, int radioModelNumber, QStringList &bandList)
{
    bandList.clear();
    //int radioModelNumber = setupRadio->availRadioData[radioIdx]->radioModelNumber;

    // find the bands the radio supports
    QStringList supBandsList;
    buildSupportedRadioBands(radioModelNumber, supBandsList);

    // merge radio bands and transverter bands
    if(setupRadio->availRadioData[radioIdx]->transVertEnable)
    {
        if (bands.count() > 0)
        {
            for (int i = 0; i < bands.count(); i++)
            {
                if (findSupRadioBand(bands[i]->name, supBandsList) ||  findSupTransBand(bands[i]->name, radioIdx))
                {
                    bandList.append(bands[i]->name);
                }
            }
        }
    }
    else
    {
        // no transverters enabled
        bandList = supBandsList;
    }



}



// probe radio for supported bands
void RigControlMainWindow::buildSupportedRadioBands(int radioModelNumber, QStringList& supBandList)
{


    RIG *my_rig = rig_init(radioModelNumber);
    if (my_rig)
    {

        for (int i = 0; i < bands.count(); i++)
        {
            if (radio->chkFreqRange(my_rig, bands[i]->fLow, "USB"))
            {
                supBandList.append(bands[i]->name);
            }
        }
    }

}




// is this band in the supported band list for this model
bool RigControlMainWindow::findSupRadioBand(const QString band, const QStringList& supBandsList)
{
    if (supBandsList.count() > 0)
    {
        for (int i = 0; i < supBandsList.count();i++)
        {
            if (band == supBandsList[i])
            {
                return true;
            }
        }

        return false;
    }

    return false;
}

// is this band in the transverter list for this radio
bool RigControlMainWindow::findSupTransBand(const QString band, const int radioIdx)
{
    if (setupRadio->availRadioData[radioIdx]->transVertNames.count() > 0)
    {
        for (int i = 0; i < setupRadio->availRadioData[radioIdx]->transVertNames.count();i++)
        {

            if (band == setupRadio->availRadioData[radioIdx]->transVertNames[i])
            {
                return true;
            }
        }

        return false;
    }

    return false;
}


/************************** Mode  *********************************/


int RigControlMainWindow::getAndSendMode(vfo_t vfo)
{

    int retCode = 0;

    retCode = radio->getMode(vfo, &rmode, &rwidth);
    // get passband state
    //hamlibData::pBandState pBState = modePbState[getMinosModeIndex(slogMode)];
    //QString spBState = QString::number(pBState);

    if (retCode == RIG_OK)
    {
        logMessage(QString("Get Mode: From Rx mode = %1, passband = %2").arg(radio->convertModeQstr(rmode)).arg(QString::number(rwidth)));
        curMode = rmode;
        sCurMode = radio->convertModeQstr(rmode);

        if (mgmModeFlag && sCurMode != setupRadio->currentRadio.mgmMode) // has mode been changed on the radio?
        {
            // yes clear MGM mode
            mgmModeFlag = false;

        }


        if (!mgmModeFlag)
        {
            // check to see if radio has been put in MGM mode, excluding USB
            if (mgmModes.contains(sCurMode) && sCurMode != hamlibData::USB)
            {

                    mgmModeFlag = true;
                    setupRadio->currentRadio.mgmMode = sCurMode;
                    displayModeVfo(hamlibData::MGM);
                    displayPassband(rwidth);
                    sendModeToLog(QString("%1:%2").arg(hamlibData::MGM).arg(setupRadio->currentRadio.mgmMode));

            }
            else
            {
                displayModeVfo(radio->convertModeQstr(rmode));
                displayPassband(rwidth);
                sendModeToLog(QString("%1:%2").arg(radio->convertModeQstr(rmode)).arg(" "));
            }


        }
        else
        {

            displayModeVfo(hamlibData::MGM);
            displayPassband(rwidth);
            sendModeToLog(QString("%1:%2").arg(hamlibData::MGM).arg(setupRadio->currentRadio.mgmMode));
        }

    }

    return retCode;
}

void RigControlMainWindow::loggerSetMode(QString mode)
{
    logMessage(QString("Log SetMode:: Mode Recieved from Logger = %1").arg(mode));
    //int retCode = RIG_OK;

    if (radioCommsOK && !rigErrorFlag)
    {
        slogMode = mode;
        //logMode = radio->convertQStrMode(mode);


        if (slogMode == hamlibData::MGM)
        {
            logMessage(QString("Log SetMode:MGM mode Selected"));
            if (mgmModeFlag)
            {
                logMessage(QString("Log SetMode: Mgm flag is set"));
                if (curMode !=  radio->convertQStrMode(setupRadio->currentRadio.mgmMode))
                {
                    setMode(setupRadio->currentRadio.mgmMode, RIG_VFO_CURR);
                    logMessage((QString("Log SetMode: MgmMode Flag alread set, Send to setmode MGM Mode = %1").arg(setupRadio->currentRadio.mgmMode)));

                }
            }
            else
            {
                mgmModeFlag = true;
                setMode(setupRadio->currentRadio.mgmMode, RIG_VFO_CURR);

                logMessage((QString("Log SetMode: Set MgmMode Flag, Send to setmode MGM Mode = %1").arg(setupRadio->currentRadio.mgmMode)));
            }
        }
        else
        {
            mgmModeFlag = false;
            logMessage(QString("Log SetMode: Clear mgmModeFlag, Set mode = %1").arg(mode));
            setMode(mode, RIG_VFO_CURR);

        }
    }
    // mode won't have changed yet
    //msg->rigCache.publish();
}

void RigControlMainWindow::setMode(QString mode, vfo_t vfo)
{
    int retCode = 0;

    cmdLockOn();      // lock get radio info
    logMessage(QString("SetMode: Mode Requested = %1").arg(mode));
    mode = mode.left(mode.indexOf(":"));
    rmode_t mCode = radio->convertQStrMode(mode);

    if (radioCommsOK)
    {
        retCode = radio->setMode(vfo, mCode, PASSBAND_NOCHANGE);
        if (retCode == RIG_OK)
        {
            logMessage(QString("SetMode: changed! Mode = %1").arg(radio->convertModeQstr(mCode)));

        }
        else
        {
            logMessage(QString("SetMode: Change Error Code = %1, Mode = %2").arg(QString::number(retCode)).arg(radio->convertModeQstr(mCode)));
            hamlibError(retCode, "Set Mode");
        }

    }
    else
    {
        logMessage(QString("Set Mode: radio not connected"));
    }
    cmdLockOff();
    // mode won't have changed yet
    //msg->rigCache.publish();
}


int RigControlMainWindow::getMinosModeIndex(QString mode)
{
    int index = 0;
    for (int i = 0; i < hamlibData::supModeList.count(); i++)
    {
        if (mode == hamlibData::supModeList[i])
        {
            index = i;
            return index;
        }
    }
    return index;
}

void RigControlMainWindow::loggerSetVolume(int level)
{

    logMessage(QString("Set Volume: From Logger, level = %1").arg(level));
    setVolume(RIG_VFO_CURR, level);

}

/************************** RIT *********************************/


void RigControlMainWindow::clearSupportRitFlags()
{
    radioSupGetRit = false;
    radioSupSetRit = false;
    radioSupGetRitState = false;
    radioSupRitOnOff = false;
    radioRitOn = false;
    ritEnable = false;
}

void RigControlMainWindow::getRitSupportStatus(int modelNumber)
{


    // Does radio support getting Rit Freq?
    radioSupGetRit = radio->supportGetRit(modelNumber);
    logMessage(QString("Get Rit Support Status - getRit support is  = %1").arg(radioSupGetRit ? "True" : "False"));


    // Does radio support setting Rit Freq?
    radioSupSetRit = radio->supportSetRit(modelNumber);
    logMessage(QString("Get Rit Support Status - setRit support is  = %1").arg(radioSupSetRit ? "True" : "False"));
    if (radioSupSetRit)
    {
        ritEnable = readRitEnableChk();
        if (ritEnable)
        {
            ui->ritEnableChk->setCheckState(Qt::Checked);
        }
        else
        {
            ui->ritEnableChk->setCheckState(Qt::Unchecked);
        }

    }

    // Does radio support turning Rit on/off

    radioSupRitOnOff = radio->supportRitOnOff(modelNumber);
    logMessage(QString("Get Rit Support Status - set Rit on/off support is  = %1").arg(radioSupRitOnOff ? "True" : "False"));

    // Does radio support getting Rit on/off state?
    radioSupGetRitState = radio->supportGetRitState(modelNumber);
    logMessage(QString("Get Rit Support Status - Rit On/Off state support is  = %1").arg(radioSupGetRitState ? "True" : "False"));
}



void RigControlMainWindow::setRitLogStatus(bool status)
{
    logRitOn = status;
    logMessage(QString("Logger RIT Status received = %1").arg(status ? "True" : "False"));
    int retCode = 0;
    ritIndicatorToggle(logRitOn);

    if (radioSupRitOnOff)
    {
        logMessage(QString("Radio Support RIT On/off switching"));
        if (radioCommsOK)
        {
            // radio supports turning RIT on and off
            retCode = radio->toggleRitState(RIG_VFO_CURR, logRitOn);
            if (retCode < 0)
            {
                logMessage(QString("Error attempting to turn on/off RIT on Radio - Error = %1").arg(retCode));
                hamlibError(retCode, "Turn Rit Off/On");
            }
            else
            {
                logMessage(QString("Rit On/Off Success"));
            }
        }

    }
    else
    {

        logMessage(QString("Radio Doesn't support Rit Off/On - Send 0 freq to radio"));
        if (radioCommsOK && radioSupSetRit && ritEnable)
        {
            setRitFreq(0);
            sendRitFreqLogger(0);
        }


    }

    if (!radioSupGetRitState)
    {
        ritIndicatorToggle(logRitOn);
    }
 }




void RigControlMainWindow::setRitFreqDisplayVisible(bool state)
{
    ui->ritLbl->setVisible(state);
    ui->ritFreq->setVisible(state);
    ui->ritStatusInd->setVisible(state);
}

/*
void RigControlMainWindow::setRitStatusIndicatorsVisible(bool state)
{
    ui->ritStatusInd->setVisible(state);

}
*/


void RigControlMainWindow::ritIndicatorToggle(bool state)
{
    if (state)
    {
        ui->ritStatusInd->setStyleSheet(RIT_STATUS_ON_STYLE);
        //ui->ritStatusInd->setText("On");
    }
    else
    {
        ui->ritStatusInd->setStyleSheet(RIT_STATUS_OFF_STYLE);
        //ui->ritStatusInd->setText("Off");
    }
}


void RigControlMainWindow::setRitGetSetFreqIndicatorVisible(bool state)
{
    ui->GetRitFreqLbl->setVisible(state);
    ui->SetRitFreqLbl->setVisible(state);
    ui->SetRitFreqInd->setVisible(state);
    ui->GetRitFreqInd->setVisible(state);
    ui->ritEnableChk->setVisible(state);
}


void RigControlMainWindow::ritSetFreqIndicatorToggle(bool state)
{
    if (state)
    {
        ui->SetRitFreqInd->setStyleSheet(RIT_RADIO_GETSETFREQ_INDICATOR_TRUE);
    }
    else
    {
        ui->SetRitFreqInd->setStyleSheet(RIT_RADIO_GETSETFREQ_INDICATOR_FALSE);
    }
}





void RigControlMainWindow::ritGetFreqIndicatorToggle(bool state)
{
    if (state)
    {
        ui->GetRitFreqInd->setStyleSheet(RIT_RADIO_GETSETFREQ_INDICATOR_TRUE);
    }
    else
    {
        ui->GetRitFreqInd->setStyleSheet(RIT_RADIO_GETSETFREQ_INDICATOR_FALSE);
    }
}


void RigControlMainWindow::saveRitEnableChk(bool state)
{
    QString fileNameRadio = RADIO_PATH_LOGGER + FILENAME_AVAIL_RADIOS;
    QSettings config(fileNameRadio, QSettings::IniFormat);
    config.beginGroup(setupRadio->currentRadio.radioName);
    config.setValue("ritEnable", state);
    config.endGroup();
}


bool RigControlMainWindow::readRitEnableChk()
{
    QString fileNameRadio = RADIO_PATH_LOGGER + FILENAME_AVAIL_RADIOS;
    QSettings config(fileNameRadio, QSettings::IniFormat);
    config.beginGroup(setupRadio->currentRadio.radioName);
    bool ritEnableState = config.value("ritEnable", false).toBool();
    config.endGroup();
    return ritEnableState;
}

void RigControlMainWindow::ritEnableChecked(int chkState)
{
    ritEnable = false;
    if (chkState == Qt::Unchecked)
    {
        saveRitEnableChk(ritEnable);
        setRitFreqDisplayVisible(ritEnable);

    }
    else if (chkState == Qt::Checked)
    {
        ritEnable = true;
        saveRitEnableChk(ritEnable);
        setRitFreqDisplayVisible(ritEnable);
    }

    sendRitEnableStatusLogger();
}


int RigControlMainWindow::getRitFreq(vfo_t vfo)
{
    int retCode = 0;
    long ritFreq;
    static int oldritFreq = 50000;

    retCode = radio->getRit(vfo, &ritFreq);
    if (retCode == RIG_OK)
    {
        int iRitFreq = static_cast<int>(ritFreq);
        if (oldritFreq != iRitFreq)
        {
           rRitFreq = iRitFreq;
           ui->ritFreq->setText(convertRitFreqToStr(rRitFreq));
           sendRitFreqLogger(rRitFreq);
        }
    }
    return retCode;
}


void RigControlMainWindow::setRitFreq(int ritFreq)
{
    if (ritEnable)
    {
        int retCode = 0;
        shortfreq_t rFreq = static_cast<long>(ritFreq);
        retCode = setRitFreq(RIG_VFO_CURR, rFreq);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Set RIT freq error").arg(QString::number(retCode)));
            hamlibError(retCode, "Set RIT Freq.");
        }
        else
        {
            if (!radioSupGetRit)
            {
                // get rit is not available, update local rit display
                // and send to logger to update logger
                ui->ritFreq->setText(convertRitFreqToStr(ritFreq));
                sendRitFreqLogger(ritFreq);
            }
        }
    }
}



int RigControlMainWindow::setRitFreq(vfo_t vfo, shortfreq_t ritFreq)
{
    int retCode = 0;
    cmdLockOn();
    retCode = radio->setRit(vfo, ritFreq);
    cmdLockOff();
    return retCode;
}


int  RigControlMainWindow::getRitRadioStatus(vfo_t vfo, bool *status)
{
    cmdLockOn();
    int retCode = radio->getRitState(vfo, status);
    cmdLockOff();
    return retCode;
}


void RigControlMainWindow::sendRadioRitStatusLogger(bool status)
{
    if (appName.length() > 0)
    {

        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setRadioRitStatus(psname, status);
        logMessage(QString("Send Radio Rit status to logger = %1 psn=%2").arg(status ? "On" : "Off").arg(psname.toString()));

    }
}

void RigControlMainWindow::sendRitFreqLogger(int ritFreq)
{
    if (appName.length() > 0)
    {
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setRadioRitFreq(psname, ritFreq);
        logMessage(QString("Send Rit freq to logger = %1 psn=%2").arg(convertRitFreqToStr(ritFreq)).arg(psname.toString()));

    }
}





/************************** Volume *********************************/


int RigControlMainWindow::getVolume(vfo_t vfo)
{
    int retCode = 0;
    value_t value;
    retCode = radio->getVolume(vfo, &value);
    if (retCode >= 0)
    {
        int vol = 0;
        value.f = value.f * VOLMULT;
        vol = qRound(value.f);
        if (vol > 200)
        {
            vol = 200;
        }
        if (vol < 0)
        {
            vol = 0;
        }

        if (vol != curVol)
        {
            curVol = vol;
            sendVolToLog(curVol);
        }
    }


    return retCode;

}


int RigControlMainWindow::setVolume(vfo_t vfo, int level)
{
    logMessage(QString("Set volume to level = %1").arg(level));
    int retCode = 0;
    float volLevel = level;
    volLevel = volLevel/VOLMULT;
    retCode = radio->setVolume(vfo, volLevel);
    return retCode;
}


/******************* Signal Strength **************************/


int RigControlMainWindow::getSignalStrength(vfo_t vfo)
{
    int retCode = 0;
    value_t value;
    retCode = radio->getSignalStrength(vfo, &value);
    if (retCode >= 0)
    {
        if (curSignalStrength != value.i)
        {
            curSignalStrength = value.i;
            displaySignalStrength(curSignalStrength);
        }
    }

    return retCode;

}


void RigControlMainWindow::displaySignalStrength(int level)
{

    int signal = level + 60;
    if (signal > MAX_SMETER_LEVEL || signal < MIN_SMETER_LEVEL)
    {
        logMessage(QString("Display Signal Level: SMeter Level out of range %1").arg(level));
    }
    qreal rSignal = static_cast<qreal>(signal)/static_cast<qreal>(110);
    // display signal bar
    ui->sMeter->setLevel(rSignal);

    // display text

    int i = 0;
    while(i < 17)
    {
         if ((level >= SMETERVALUES[i][0]) & (level <= SMETERVALUES[i][1]))
         {
             if ((i >= 15) & (i <=16))
             {
                 ui->sMeterLabel->setText(QString::number(level) + "  ");
             }
             else
             {
                 ui->sMeterLabel->setText(SMETERTEXT[i]);
             }
             break;
         }
         i++;
     }



}

void RigControlMainWindow::displayPassband(pbwidth_t width)
{

    ui->passBandlbl->setText(QString::number(width));
}

void RigControlMainWindow::displayFreqVfo(double frequency)
{

    ui->radioFreqA->setText(convertFreqStrDisp(convertFreqToStr(frequency)));
}


void RigControlMainWindow::displayTransVertVfo(double frequency)
{
    ui->transVertFreqA->setText(convertFreqStrDisp(convertFreqToStr(frequency)));

}

void RigControlMainWindow::displayModeVfo(QString mode)
{
    ui->modeA->setText(mode);
}

void RigControlMainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}

void RigControlMainWindow::hamlibError(int errorCode, QString cmd)
{

    pollTimer->stop();


    if ( errorCode >= 0)
    {
        return;
    }
    if(appName.count() > 0)
    {
        sendStatusToLogError();
    }

    rigErrorFlag = true;

    errorCode *= -1;
    QString errorMsg = radio->gethamlibErrorMsg(errorCode);
    logMessage(QString("Hamlib Error - Code = %1 - %2").arg(QString::number(errorCode)).arg(errorMsg));

    QMessageBox::critical(this, "RigControl hamlib Error - " + setupRadio->currentRadio.radioName, QString::number(errorCode) + " - " + errorMsg + "\n" + "Command - " + cmd);

    closeRadio();
    rigErrorFlag = false;
    if (appName.length() >0)
    {
        sendStatusToLogDisConnected();
    }
}


/********************* PTT ****************************************/

// not implemented yet..
int RigControlMainWindow::getTXStatus(vfo_t vfo)
{

    ptt_t pttStatus;
    int retCode = radio->getPttStatus(vfo, &pttStatus);
    if (retCode == RIG_OK)
    {
       if (pttStatus == RIG_PTT_ON)
       {
           // turn on indicator

       }
    }

   return retCode;

}


bool RigControlMainWindow::readTestStandAloneFlag()
{
    QString fileName;
    if (appName == "")
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOCAL + MINOS_RADIO_CONFIG_FILE;
    }
    else
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RADIO_CONFIG_FILE;
    }


    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("TestStandAlone");
    bool state = config.value("TestStandAlone", false).toBool();
    config.endGroup();

    return state;
}






void RigControlMainWindow::readTraceLogFlag()
{
    QString fileName;
    if (appName == "")
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOCAL + MINOS_RADIO_CONFIG_FILE;
    }
    else
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RADIO_CONFIG_FILE;
    }


    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("TraceLog");
    bool state = config.value("TraceLog", false).toBool();
    config.endGroup();

    ui->actionTraceComms->setChecked(state);
    radio->enableTraceComms(state);             // set state of trace hamlib comms
}

void RigControlMainWindow::saveTraceLogFlag(bool state)
{

    // set state of hamlib commms tracing

    radio->enableTraceComms(state);

    // save to ini for restart

    QString fileName;
    if (appName == "")
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOCAL + MINOS_RADIO_CONFIG_FILE;
    }
    else
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RADIO_CONFIG_FILE;
    }

    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("TraceLog");

    config.setValue("TraceLog", state);

    config.endGroup();
    trace("Tracelog Changed in " + fileName + " = " + QString::number(state));
}

void RigControlMainWindow::about()
{
    QMessageBox::about(this, "Minos RigControl", "Minos QT RigControl\nCopyright D Balharrie G8FKH/M0DGB 2019");
}




void RigControlMainWindow::sendRadioListLogger()
{
    QStringList radioList;
    for (int i= 0; i < setupRadio->numAvailRadios; i++)
    {
        if (!setupRadio->availRadios[i].isEmpty())
        {
            PubSubName r(setupRadio->availRadios[i]);
            radioList.append(r.toString());
        }
    }
    logMessage(QString("Sending radiolist to logger"));
    for (int i = 0; i < radioList.count(); i++)
    {
        logMessage(QString("Send radiolist - radio %1, name %2").arg(QString::number(i)).arg(radioList[i]));
    }
    msg->publishRadioNames(radioList);
}

void RigControlMainWindow::sendBandListLogger(const int radioIdx, const QStringList& supBandList)
{
    QString fileName;
    fileName = RADIO_PATH_LOGGER + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("FreqPresets");

    QStringList bandList;
    if (!supBandList.isEmpty())
    {
        for (int i = 0; i < supBandList.count(); i++)
        {
            // get index to band
            int k = 0;
            bool match = false;
            do
            {
                if (supBandList[i] == freqPresetData::presetBands[k])
                {
                    match = true;
                    break;
                }
                k++;
            }while (k < freqPresetData::presetBands.count() && !match);

            QString lookupBand = supBandList[i];
            lookupBand = lookupBand.remove('\x20').replace('.', '_');   // remove space or convert period to underscore to correctly lookup band
            bandList.append(QString("%1-%2").arg(supBandList[i])
                                        .arg(config.value(lookupBand, freqPresetData::bandFreq[k]).toString()));
        }


        PubSubName psname(setupRadio->availRadioData[radioIdx]->radioName);
        QString bands = bandList.join(":");
        logMessage(QString("Send bandlist to logger: for radio %1 - %2").arg(setupRadio->availRadioData[radioIdx]->radioName).arg(bands));
        msg->rigCache.setBandList(psname, bands);
    }
    else
    {
        logMessage(QString("Send bandlist to logger: error radio bandlist empty"));
    }
    config.endGroup();
}

void RigControlMainWindow::sendStatusLogger(const QString &message )
{
    if (appName.length() > 0)
    {
        logMessage(QString("Send status to logger = %1").arg(message));
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setStatus(psname, message);
    }
}


void RigControlMainWindow::sendStatusToLogConnected()
{
    logMessage(QString("Send status to logger connected"));
    sendStatusLogger(RIG_STATUS_CONNECTED);
}

void RigControlMainWindow::sendStatusToLogDisConnected()
{
    logMessage(QString("Send status to logger disconnected"));
    sendStatusLogger(RIG_STATUS_DISCONNECTED);
}


void RigControlMainWindow::sendStatusToLogError()
{
    logMessage(QString("Send error status to logger"));
    sendStatusLogger(RIG_STATUS_ERROR);
}


void RigControlMainWindow::sendErrorMessageToLogger(QString errMsg)
{

    logMessage(QString("Send error message to logger: %1").arg(errMsg));
    PubSubName psname(setupRadio->currentRadio.radioName);
    msg->rigCache.setStatus(psname, errMsg);

}

void RigControlMainWindow::sendFreqToLog(freq_t freq)
{

    if (appName.length() > 0)
    {
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setRadioFreq(psname, freq);
        logMessage(QString("Send freq to logger = %1 psn=%2").arg(convertFreqToStr(freq)).arg(psname.toString()));
    }
}




void RigControlMainWindow::sendModeToLog(QString mode)
{
    if (appName.length() > 0)
    {
        logMessage(QString("Send mode to logger = %1").arg(mode));
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setRadioMode(psname, mode);
    }
}

void RigControlMainWindow::sendVolToLog(int level)
{
    if (appName.length() > 0)
    {
        logMessage(QString("Send volume to logger = %1").arg(QString::number(level)));
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setRadioVolume(psname, level);
    }
}


void RigControlMainWindow::sendVolStatusToLog(const int radIdx, bool status)
{
    if (appName.length() > 0)
    {
        QString f = "";
        status  ? f = "True" : f = "False";
        logMessage(QString("Send Volume Status to logger = %1").arg(f));
        PubSubName psname(setupRadio->availRadioData[radIdx]->radioName);
        msg->rigCache.setVolumeStatus(psname, status);

    }
}

void RigControlMainWindow::sendTransVertEnabled(bool status)
{
    //QString flag;
    if (appName.length() > 0)
    {
        QString f = "";
        status  ? f = "True" : f = "False";
        logMessage(QString("Send Transvert Enabled to logger = %1").arg(f));
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setTransverterEnabled(psname, status);

    }
}



void RigControlMainWindow::sendTransVertStatusToLog(bool status)
{
    //QString flag;
    if (appName.length() > 0)
    {
        QString f = "";
        status  ? f = "True" : f = "False";
        logMessage(QString("Send Transvert Status to logger = %1").arg(f));
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setTransverterStatus(psname, status);

    }
}

void RigControlMainWindow::sendTransVertOffsetToLogger(int tvNum)
{
    QString f = convertFreqToStr(setupRadio->currentRadio.transVertSettings[tvNum]->transVertOffset);
    logMessage(QString("Send Transvert Offset to logger = %1%2").arg(setupRadio->currentRadio.transVertEnable ? f = "-" : f = "+").arg(f));
    PubSubName psname(setupRadio->currentRadio.radioName);
    msg->rigCache.setTransverterOffset(psname, setupRadio->currentRadio.transVertSettings[tvNum]->transVertOffset);
}

void RigControlMainWindow::sendTransVertSwitchToLogger(const QString &swNum)
{
    if (swNum == "")
    {
        logMessage(QString("Send Transvert Switch Number to Logger - Switch Number Empty - Nothing Sent"));
        return;
    }

    logMessage(QString("Send Transvert Switch Number to logger = %1").arg(swNum));
    PubSubName psname(setupRadio->currentRadio.radioName);
    msg->rigCache.setTransverterSwitch(psname, swNum.toInt());
}


void RigControlMainWindow::sendTransVertSwitchToComPort(const QString &swNum)
{
    if (swNum == "")
    {
        logMessage(QString("Send Transvert Switch Number to Comport - Switch Number Empty - Nothing Sent"));
        return;
    }

    QByteArray msg = swNum.toUtf8();
    msg.prepend(TVSWMSG_START);
    msg.append(TVSWMSG_TERM);


    if (setupRadio->currentRadio.transVertEnable
            && setupRadio->currentRadio.enableTransSwitch
            && setupRadio->currentRadio.enableLocTVSwMsg
            && serialTVSw->getOpenFlag())      // com port should be open!
    {
        logMessage(QString("Send Transvert Switch Number to Comport = %1, message = %2").arg(setupRadio->currentRadio.locTVSwComport).arg(QString::fromLocal8Bit(msg)));
        serialTVSw->sendTVSwMessage(msg);
    }
}


void RigControlMainWindow::sendRitEnableStatusLogger()
{
    if (radioSupSetRit & ritEnable)
    {
        sendRitEnableStatus(true);
    }
    else
    {
        sendRitEnableStatus(false);
    }

}




void RigControlMainWindow::sendRitEnableStatus(bool status)
{

    if (appName.length() > 0)
    {
        logMessage(QString("Send Rit Enable Status to logger = %1").arg(status  ? "True" : "False"));
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setRitEnableStatus(psname, status);

    }
}

void RigControlMainWindow::sendTpm(int tpm)
{
    logMessage(QString("Send Tpm to logger = %1").arg(tpm));
    PubSubName psname(setupRadio->currentRadio.radioName);
    msg->rigCache.setTpm(psname, tpm);
}


void RigControlMainWindow::onLaunchSetup()
{
    setupRadio->setTabToCurrentRadio();
    setupRadio->loadAvailComports();
    setupRadio->exec();
}

void RigControlMainWindow::aboutRigConfig()
{
    QString msg = QString("*** Rig Configuration ***\n\n");

    if (setupRadio->currentRadio.radioName != "")
    {

        msg.append(QString("App Instance Name  = %1\n").arg(appName));
        msg.append(QString("Hamlib Version = %1\n").arg(radio->gethamlibVersion()));
        msg.append(QString("Radio Name = %1\n").arg(setupRadio->currentRadio.radioName));
        msg.append(QString("Radio Number = %1\n").arg(setupRadio->currentRadio.radioNumber));
        msg.append(QString("Rig Model = %1\n").arg(setupRadio->currentRadio.radioModel));
        msg.append(QString("Rig Number = %1\n").arg(QString::number(setupRadio->currentRadio.radioModelNumber)));
        msg.append(QString("Rig Manufacturer = %1\n").arg(setupRadio->currentRadio.radioMfg_Name));
        if (setupRadio->currentRadio.radioMfg_Name == "Icom")
        {
            if (setupRadio->currentRadio.civAddress == "")
            {
                msg.append(QString("Icom CIV address = Using Default CIV Address\n"));
            }
            else
            {
                msg.append(QString("Icom CIV address = %1\n").arg(setupRadio->currentRadio.civAddress));
            }

        }
        msg.append(QString("\n"));
        msg.append(QString("Rig PortType = %1\n").arg(hamlibData::portTypeList[setupRadio->currentRadio.portType]));
        msg.append(QString("Network Address = %1\n").arg(setupRadio->currentRadio.networkAdd));
        msg.append(QString("Network Port = %1\n").arg(setupRadio->currentRadio.networkPort));
        msg.append(QString("Comport = %1\n").arg(setupRadio->currentRadio.comport));
        msg.append(QString("Baudrate = %1\n").arg(setupRadio->currentRadio.baudrate));
        msg.append(QString("Stop bits = %1\n").arg(QString::number(setupRadio->currentRadio.stopbits)));
        msg.append(QString("Parity = %1\n").arg(radio->getParityCodeNames()[setupRadio->currentRadio.parity]));
        msg.append(QString("Handshake = %1\n").arg(radio->getHandShakeNames()[setupRadio->currentRadio.handshake]));
        if (setupRadio->currentRadio.rigCtldEnable)
        {
            msg.append(QString("\n"));
            msg.append(QString("Using rigctld daemon = %1\n").arg(setupRadio->currentRadio.rigCtldEnable ? "True" : "False"));
            msg.append(QString("Rigctld path = %1\n").arg(setupRadio->getRigCtldExePath()));
            msg.append(QString("Rigctld network address = %1\n").arg(setupRadio->currentRadio.rigCtldNetworkAdd));
            msg.append(QString("Rigctld port address = %1\n").arg(setupRadio->currentRadio.rigCtldNetworkPort));
            msg.append(QString("Rigctld Connect delay = %1\n").arg(rigCtldConnectDelay));
        }

        msg.append(QString("\n"));
        msg.append(QString("TransVert Enable = %1\n").arg(setupRadio->currentRadio.transVertEnable ? "True" : "False"));
        msg.append(QString("Number of TransVerters = %1\n").arg(setupRadio->currentRadio.numTransverters));

        for (int i = 0; i < setupRadio->currentRadio.numTransverters; i++)
        {
            msg.append(QString("\n"));
            msg.append(QString("Transverter %1\n").arg(i));
            msg.append(QString("Transverter Name = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->transVertName));
            msg.append(QString("Transverter Band = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->band));
            msg.append(QString("Transverter Offset = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->transVertOffsetStr));
            msg.append(QString("Transverter Switch num = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->transSwitchNum));
            msg.append(QString("Transverter Switch enable = %1\n").arg(setupRadio->currentRadio.enableTransSwitch  ? "True" : "False"));
        }

        msg.append(QString("\n"));
        msg.append(QString("Radio Supports RIT = %1\n").arg(radioSupSetRit ? "True" : "False"));
        if (radioSupSetRit)
        {
            msg.append(QString("Rit Enable On = %1\n").arg(ritEnable  ? "True" : "False"));
            msg.append(QString("Radio Supports Get RIT Freq = %1\n").arg(radioSupGetRit ? "True" : "False"));
            msg.append(QString("Radio Supports Set RIT Freq = %1\n").arg(radioSupSetRit ? "True" : "False"));
            msg.append(QString("Radio Supports Get RIT State On/Off = %1\n").arg(radioSupGetRitState ? "True" : "False"));
            msg.append(QString("Radio Supports Set RIT State On/Off = %1\n").arg(radioSupRitOnOff ? "True" : "False"));
        }
        msg.append(QString("Tracelog = %1\n").arg(ui->actionTraceComms->isChecked() ? "True" : "False"));
    }
    else
    {
        msg.append(QString("No radio selected"));
    }
    QMessageBox::about(this, "Minos RigControl", msg);
}

void RigControlMainWindow::dumpRadioToTraceLog()
{

    if (setupRadio->currentRadio.radioName != "")
    {
        trace("*** Radio Update ***");
        trace(QString("App Instance Name  = %1").arg(appName));
        trace(QString("Hamlib Version = %1").arg(radio->gethamlibVersion()));
        trace(QString("Radio Name = %1").arg(setupRadio->currentRadio.radioName));
        trace(QString("Radio Number = %1").arg(setupRadio->currentRadio.radioNumber));
        trace(QString("Rig Model = %1").arg(setupRadio->currentRadio.radioModel).trimmed());
        trace(QString("Rig Number = %1").arg(QString::number(setupRadio->currentRadio.radioModelNumber)));
        trace(QString("Rig Manufacturer = %1").arg(setupRadio->currentRadio.radioMfg_Name));
        if (setupRadio->currentRadio.radioMfg_Name == "Icom")
        {
            if (setupRadio->currentRadio.civAddress == "")
            {
                trace(QString("Icom CIV address = Using Default CIV Address"));
            }
            else
            {
                trace(QString("Icom CIV address = %1").arg(setupRadio->currentRadio.civAddress));
            }

        }
        trace(QString("Rig PortType = %1").arg(hamlibData::portTypeList[setupRadio->currentRadio.portType]));
        trace(QString("Network Address = %1").arg(setupRadio->currentRadio.networkAdd));
        trace(QString("Network Port = %1").arg(setupRadio->currentRadio.networkPort));
        trace(QString("Comport = %1").arg(setupRadio->currentRadio.comport));
        trace(QString("Baudrate = %1").arg(setupRadio->currentRadio.baudrate));
        trace(QString("Stop bits = %1").arg(QString::number(setupRadio->currentRadio.stopbits)));
        trace(QString("Parity = %1").arg(radio->getParityCodeNames()[setupRadio->currentRadio.parity]));
        trace(QString("Handshake = %1").arg(radio->getHandShakeNames()[setupRadio->currentRadio.handshake]));
        if (setupRadio->currentRadio.rigCtldEnable)
        {
            trace(QString("Using rigctld daemon = %1").arg(setupRadio->currentRadio.rigCtldEnable ? "True" : "False"));
            trace(QString("Rigctld network address = %1").arg(setupRadio->currentRadio.rigCtldNetworkAdd));
            trace(QString("Rigctld port address = %1").arg(setupRadio->currentRadio.rigCtldNetworkPort));
            trace(QString("Rigctld path = %1").arg(setupRadio->getRigCtldExePath()));
            trace(QString("Rigctld Connect delay = %1").arg(rigCtldConnectDelay));
        }
        trace(QString("MGM mode = %1").arg(setupRadio->currentRadio.mgmMode));
        trace(QString("TransVert Enable = %1").arg(setupRadio->currentRadio.transVertEnable ? "True" : "False"));
        trace(QString("Number of TransVerters = %1").arg(setupRadio->currentRadio.numTransverters));
        for (int i = 0; i < setupRadio->currentRadio.numTransverters; i++)
        {
            trace(QString("Transverter %1").arg(i));
            trace(QString("Transverter Name = %1").arg(setupRadio->currentRadio.transVertSettings[i]->transVertName));
            trace(QString("Transverter Band = %1").arg(setupRadio->currentRadio.transVertSettings[i]->band));
            trace(QString("Transverter Offset = %1").arg(setupRadio->currentRadio.transVertSettings[i]->transVertOffsetStr));
            trace(QString("Transverter Switch num = %1").arg(setupRadio->currentRadio.transVertSettings[i]->transSwitchNum));
            trace(QString("Transverter Switch enable = %1").arg(setupRadio->currentRadio.enableTransSwitch  ? "True" : "False"));
        }
        trace(QString("Radio Supports RIT = %1\n").arg(radioSupSetRit ? "True" : "False"));
        if (radioSupSetRit)
        {
            trace(QString("Rit Enable On = %1\n").arg(ritEnable  ? "True" : "False"));
            trace(QString("Radio Supports Get RIT Freq = %1\n").arg(radioSupGetRit ? "True" : "False"));
            trace(QString("Radio Supports Set RIT Freq = %1\n").arg(radioSupSetRit ? "True" : "False"));
            trace(QString("Radio Supports Get RIT State On/Off = %1\n").arg(radioSupGetRitState ? "True" : "False"));
            trace(QString("Radio Supports Set RIT State On/Off = %1\n").arg(radioSupRitOnOff ? "True" : "False"));
        }
        trace(QString("Tracelog = %1").arg(ui->actionTraceComms->isChecked() ? "True" : "False"));

    }
    else
    {
        trace(QString("No radio selected"));
    }
}

/************************** Band Information ***********************************/
/*
void RigControlMainWindow::loadBands()
{
    BandList &blist = BandList::getBandList();

    for (int i = 0; i < blist.bandList.size(); i++)   // just load VHF/UHF bands
    {
        // don't use bands > 10GHz (can't support Freq display)
        if ( blist.bandList[i].uk != "24 GHz" && blist.bandList[i].uk != "47 GHz"
             && blist.bandList[i].uk != "76 GHz" && blist.bandList[i].uk != "120 GHz"
             && blist.bandList[i].uk != "134 GHz" && blist.bandList[i].uk != "248 GHz")

        {
            if (blist.bandList[i].getType().compare("VHF", Qt::CaseInsensitive) == 0 || blist.bandList[i].getType().compare("MWave", Qt::CaseInsensitive) == 0)
                bands.append(new BandDetail(blist.bandList[i].uk, blist.bandList[i].flow, blist.bandList[i].fhigh));
        }
    }

}

*/






/********************************* Supported Radio Display *****************************/



void RigControlMainWindow::initialiseSupportedRadioDisplay()
{
    // table of indicators
    supRadioInd.append(ui->_50mhz_Indicator);
    supRadioInd.append(ui->_70mhz_Indicator);
    supRadioInd.append(ui->_144mhz_Indicator);
    supRadioInd.append(ui->_432mhz_Indicator);
    supRadioInd.append(ui->_1296mhz_Indicator);
    supRadioInd.append(ui->_2300mhz_Indicator);
    supRadioInd.append(ui->_3_4ghz_Indicator);
    supRadioInd.append(ui->_5_6ghz_Indicator);
    supRadioInd.append(ui->_10ghz_Indicator);

    turnOffAllsupRadioIndicators();

}


void RigControlMainWindow::showActiveTransVertIndicator(QString cb)
{

    if (cb != selTransVertBandIndicator)
    {
        // turn off previous active transverter indicator
        if (selTransVertBandIndicator != "")
        {
            for (int i = 0; i < freqPresetData::presetBands.count(); i++)
            {
               if (selTransVertBandIndicator == freqPresetData::presetBands[i])
                {
                   supRadioIndToggle(i, displayIndicator::TRANSVERT);
                   break;
               }
            }
        }

        // turn on new indicator
        for (int i = 0; i < freqPresetData::presetBands.count(); i++)
        {
           if (cb == freqPresetData::presetBands[i])
            {
               supRadioIndToggle(i, displayIndicator::TRANSVERT_ON);
               selTransVertBandIndicator = cb;
               break;
           }
        }
    }
}



void RigControlMainWindow::updateSupportedRadioIndicators()
{
    turnOffAllsupRadioIndicators();

    for (int i = 0; i < setupRadio->currentRadio.radioTransSupBands.count(); i++)
    {

        for (int b = 0; b < freqPresetData::presetBands.count(); b++)
        {
           if (setupRadio->currentRadio.radioTransSupBands[i] == freqPresetData::presetBands[b])
            {
               supRadioIndToggle(b, displayIndicator::RADIO);
               break;
           }
        }
    }

    if (setupRadio->currentRadio.transVertEnable)
    {
       for (int i = 0; i < setupRadio->currentRadio.transVertSettings.count(); i++)
       {
          for (int b = 0; b < freqPresetData::presetBands.count(); b++)
          {
                if (setupRadio->currentRadio.transVertSettings[i]->band == freqPresetData::presetBands[b])
                {
                    supRadioIndToggle(b, displayIndicator::TRANSVERT);
                    break;
                }
            }
        }
    }



}


void RigControlMainWindow::turnOffAllsupRadioIndicators()
{

    for (int i = 0; i < supRadioInd.count(); i++)
    {
        supRadioIndToggle(i, displayIndicator::OFF);
    }

}


void RigControlMainWindow::supRadioIndToggle(int offset, displayIndicator::indicatorType type)
{

    if (type == displayIndicator::OFF)
    {
        supRadioInd[offset]->setStyleSheet(SUP_RADIO_INDICATOR_OFF_STYLE);
    }
    else if (type == displayIndicator::RADIO)
    {
       supRadioInd[offset]->setStyleSheet(SUP_RADIO_INDICATOR_RADIO_STYLE);
    }
    else if (type == displayIndicator::TRANSVERT)
    {
       supRadioInd[offset]->setStyleSheet(SUP_RADIO_INDICATOR_TRANSVERT_STYLE);
    }
    else if (type == displayIndicator::TRANSVERT_ON)
    {
       supRadioInd[offset]->setStyleSheet(SUP_RADIO_INDICATOR_TRANSVERT_ON_STYLE);
    }


}




/*********************************** test *********************************************/

void RigControlMainWindow::selFreqClicked()
{
    // check freq valid format
    QString f = ui->freqInputBox->text().trimmed().remove( QRegExp("^[0]*"));

    if (valInputFreq(f, "Invalid freq!"))
    {
        // convert radio freq
        f = convertSinglePeriodFreqToFullDigit(f).remove('.');
        setFreq(f, RIG_VFO_CURR);
    }
}

void RigControlMainWindow::testBoxesVisible(bool visible)
{
    ui->selFreq->setVisible(visible);
    ui->freqInputBox->setVisible(visible);
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


#ifdef RIGCONTROL_TEST

void RigControlMainWindow::incRit()
{
    int retCode;
    bool ok;
    QString sFreq = ui->setRitSpinner->text();
    long dFreq = sFreq.toLong(&ok);
    retCode = setRitFreq(RIG_VFO_CURR, dFreq);
    if (retCode < 0)
    {
        qDebug()<< "test set rit errror";
    }

}


void RigControlMainWindow::ritbuttontoggle()
{
    bool status = !radioRitOn;
    int retCode = 0;
    retCode = radio->toggleRitState(RIG_VFO_CURR, status);
    if (retCode < 0)
    {
        qDebug() << "Rit toggle = " << retCode;
    }
}

#endif
