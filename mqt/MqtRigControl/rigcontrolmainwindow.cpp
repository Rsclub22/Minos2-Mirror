/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
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
//#include "rigcontrolcommonconstants.h"
#include "rigcommon.h"
#include "rigcontrolmainwindow.h"
#include "ui_rigcontrolmainwindow.h"
#include "freqpresetdialog.h"
#include "rigsetupdialog.h"
#include "rigcontrolrpc.h"
#include "rigutils.h"
#include "rigctldclient.h"
#include <QTimer>
#include <QMessageBox>
#include <QProcessEnvironment>

#include <QBitArray>
#include <QDebug>

#include "cutils.h"


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
   radioSupSetRitState(false),
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

    radio = nullptr;
    rigFactory = new RigFactory(false, this);


    loadVhfAndUpBands(bands);
    FreqPresetDialog::readSettings(presetFreq);

    setupRadio = new RigSetupDialog(rigFactory, bands);
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
            QString errmsg = HtmlFontColour(Qt::red) + tr("Please select a radio!");
            showStatusMessage(errmsg);
            sendStatusLogger(errmsg);
        }
        else
        {
            ui->selectRadioBox->setCurrentText(setupRadio->getCurrentRadioName());
        }
    }

    setPolltime(1000);

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


MODE RigControlMainWindow::convertQStringToMode(QString modeStr)
{
    if (modeStr == "AM") return AM;
    if (modeStr == "CW") return CW;
    if (modeStr == "CW_R") return CW_R;
    if (modeStr == "USB") return USB;
    if (modeStr == "LSB") return LSB;
    if (modeStr == "RTTY") return FSK;
    if (modeStr == "RTTYR") return FSK_R;
    if (modeStr == "PKTLSB") return DIG_L;
    if (modeStr == "PKTUSB") return DIG_U;
    if (modeStr == "FM") return FM;
    if (modeStr == "PKT_FM") return DIG_FM;

    return USB; // default


}


QString RigControlMainWindow::convertModeToQString(MODE mode)
{
    switch (mode)
      {
      case AM: return "AM";
      case CW: return "CW";
      case CW_R: return "CW_R";
      case USB: return "USB";
      case LSB: return "LSB";
      case FSK: return "RTTY";
      case FSK_R: return "RTTYR";
      case DIG_L: return "PKTLSB";
      case DIG_U: return "PKTUSB";
      case FM: return "FM";
      case DIG_FM: return "PKTFM";
      default: break;
      }
    return "USB";

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
    connect(msg, SIGNAL(setVolume(int)), this, SLOT(loggerSetVolume(int)));



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
        radioIndex = setupRadio->findCurrentRadio(setupRadio->getCurrentRadioName()); // make sure radio exits in available radios
        ridx = radioIndex;
        if (ridx > -1 && ridx < setupRadio->numAvailRadios)
        {
            // found radio, update currentRadio from selected radiodata
            updateCurrentRadioFromAvailRadios(ridx);

            //scatParams::copyRig(setupRadio->availRadioData[ridx], setupRadio->currentRadio);
            setupRadio->currentRadio.radioNumber = QString::number(ridx);           // save radio number

            if (radioCommsOK)
            {
                closeRadio();
            }

            //if (setupRadio->currentRadio.radioModelNumber == 0)
            //{
                //closeRadio();
            //    QMessageBox::critical(this, tr("Radio Error"), tr("Please configure a radio name and model"));
            //    return;
           // }

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

                    slogMode = USB_STR;
                    // set mode
                    //logMode = radio->convertQStrMode("USB");
                    setMode(USB_STR, VFO::CURRENT_VFO);
                }

                // build supported band list for this radio
                // if it is a rigctld model, then use the radio model number connected to rigctld

                int modelNumber = setupRadio->currentRadio.rigModelNumber;
                if (modelNumber == hamlibData::RIGCTL)
                {
                    modelNumber = irigctld_radioNumber;
                }

                buildSupBandList(ridx, modelNumber, setupRadio->currentRadio.radioTransSupBands);

                // does the radio support control of volume control

                supVolume = rigFactory->supported_rigs()->value(setupRadio->currentRadio.rigModel).supportVolume;
                logMessage(QString("Update Radio: Radio Supports Volume Control %1").arg(supVolume ? "True" : "False"));
                sendVolStatusToLog(ridx, supVolume);

                // does the radio support signal strength meter

                supSignalStrength = rigFactory->supported_rigs()->value(setupRadio->currentRadio.rigModel).supportSMeter;

                setSmeterVisible(supSignalStrength);

                updateSupportedRadioIndicators();

                getRitSupportStatus();

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

                if (rigFactory->supported_rigs()->value(setupRadio->currentRadio.rigModel).pollData)
                {
                    if (radio != nullptr)
                    {
                        if (radio->getRigConnected())
                        {
                            if (setupRadio->currentRadio.pollInterval == "0.5")
                            {
                                pollTime = 500;
                            }
                            else
                            {
                                pollTime = 1000 * setupRadio->currentRadio.pollInterval.toInt();
                            }

                            pollTimer->start(pollTime);             // start timer to send poll radio

                        }
                    }
                }
                else
                {
                    // not polling get initial values
                    getAndSendFrequency(CURRENT_VFO);
                    getAndSendMode(CURRENT_VFO);
                    // connect signals for future value updates and errors
                    connect(radio, SIGNAL(newFreq()), this, SLOT(onNewFreq()), Qt::QueuedConnection); // QueuedConnection, ensure return to rigcontroller caller when not polling - eg Omnirig
                    connect(radio, SIGNAL(newMode()), this, SLOT(onNewMode()), Qt::QueuedConnection);
                    connect(radio, SIGNAL(rigStatus(int, QString)), this, SLOT(onRigStatus(int, QString)), Qt::QueuedConnection);
                }




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
        else
        {
            trace(QString("Saved Current Radio %1 does not match saved available radios.").arg(setupRadio->getCurrentRadioName()));
        }
    }
    else
    {   // no radio selected
        trace("No radio selected");
        setupRadio->saveCurrentRadio();
        ui->radioNameDisp->setText("");
        closeRadio();
        writeWindowTitle(appName);
    }



    if (appName.length() > 0)
    {
        msg->rigCache.publish();
    }
}

void RigControlMainWindow::updateCurrentRadioFromAvailRadios(int ridx)
{
    scatParams::copyRig(setupRadio->availRadioData[ridx], setupRadio->currentRadio);
    //gCapabilities rigCap = rigFactory->supported_rigs()->value(setupRadio->availRadioData[ridx]->rigModel);
    // this data isn't saved to availradios anymore
   //setupRadio->currentRadio.rigModel = setupRadio->availRadioData[ridx]->rigModel;
    //setupRadio->currentRadio.rigMfg_Name = rigCap.rigManufacturer;
    //setupRadio->currentRadio.rigModelName = rigCap.rigModelName;
    //setupRadio->currentRadio.rigModelNumber = rigCap.modelNumber;
    //setupRadio->currentRadio.portType = rigCap.portType;

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
    if (radio->getTraceComms())
    {
        traceCode = rigCtldTrace::rigCtldTraceCodes::VERBOSE;
    }

    QString handshake;
    QString rtsState;
//  need to sort this for rigctld **********************************************
//    handshake = serialData::rigctldHandshakeStr[setupRadio->currentRadio.handshake];

//    if (handshake == serialData::rigctldHandshakeStr[RIG_HANDSHAKE_HARDWARE])
//    {
//        rtsState = serialData::rigctldForeLinesStr[serialData::FORCE_LINE_NONE];
//    }
//    else
//    {
//        rtsState = serialData::rigctldForeLinesStr[setupRadio->currentRadio.forceRts];
//    }

//    QString dtrState = serialData::rigctldForeLinesStr[setupRadio->currentRadio.forceDtr];


    // start rigctld
    trace(QString("openRigCtldRadio: starting rigctld"));
//    runRigCtlDaemon(setupRadio->currentRadio.radioMfg_Name, QString::number(setupRadio->currentRadio.radioModelNumber), setupRadio->currentRadio.comport,
//                                               QString::number(setupRadio->currentRadio.baudrate), QString::number(setupRadio->currentRadio.databits), setupRadio->currentRadio.civAddress, setupRadio->currentRadio.rigCtldNetworkAdd, setupRadio->currentRadio.rigCtldNetworkPort,
//                                               QString::number(setupRadio->currentRadio.stopbits), setupRadio->currentRadio.parity, handshake, rtsState, dtrState, traceCode);


    // wait for rigctld to start
    int waitStartDur = 500;
    while (rigCtldProcess->state() != QProcess::Running && waitStartDur > 0)
    {
        sleepFor(100);
        waitStartDur--;
    }

    if (waitStartDur > 0)
    {
        trace(QString("openRigCtldRadio: rigctld running for radio %1").arg(setupRadio->currentRadio.rigModel));
    }
    else
    {
        trace(QString("openRigCtldRadio: rigctld failed for radio %1").arg(setupRadio->currentRadio.rigModel));
        return RIGCTLD_FAILED;
    }


    if (rigCtldConnectDelay != 0)
    {
        trace(QString("openRigCtldRadio: Delay = %1 secs before connecting to rigCtld").arg(rigCtldConnectDelay));
        delay(rigCtldConnectDelay);
    }

    // now open radio using rigctld model
    trace(QString("openRigCtldRadio: Open radio = %1, via Rigctld").arg(setupRadio->currentRadio.rigModel));
    retCode = radio->rigInit(setupRadio->currentRadio, RIGCTLD_ON);
    if (retCode < 0)
    {
        radio->closeRig();
        logMessage(QString("openRigCtldRadio: Error Opening Radio %1, Error Code = %2").arg(setupRadio->currentRadio.rigModel).arg(QString::number(retCode)));
        radioError(retCode, tr("RigCtld Open Radio"));
        return OPEN_FAILED;
    }

    // let's see if we can get freq from radio and confirm comms
    if (radio->getRigConnected())
    {

        int retCode = Rig_OK;
        showStatusMessage(tr("Attempting to communicate with radio via Rigctld - %1").arg(setupRadio->currentRadio.rigModel));
        retCode = radio->getFrequency(VFO::CURRENT_VFO, rfrequency);


        if (retCode < Rig_OK)
        {
            logMessage(QString("openRigctldRadio: Test Communication - Get Freq error, code = %1").arg(QString::number(retCode)));
            radioError(retCode, tr("Test Radio Connection via Rigctld\n\nMinos tried to read the radio frequency,\nbut nothing was received from the radio.\n\nPlease check connections and/or settings.\nSome radios/interfaces may require Force DTR or Force RTS to be set High, to power the interface."));
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
        logMessage(QString("openRigctldRadio: Radio Opened %1").arg(setupRadio->currentRadio.rigModel));
        showStatusMessage(tr("Radio Opened Rigctld: %1").arg(setupRadio->currentRadio.rigModel));

        if (setupRadio->currentRadio.portType == RigCapConstants::PortType::serial)
        {
            showStatusMessage(QString("Connected via RigCtld: %1 - %2, %3, %4, %5, %6, %7, Handshake %8, ForceDTR %9, ForceRTS %10")
                              .arg(setupRadio->currentRadio.rigMfg_Name).arg(setupRadio->currentRadio.rigModelName).trimmed().arg(setupRadio->currentRadio.comport).arg(setupRadio->currentRadio.baudrate).arg(setupRadio->currentRadio.databits)
                              .arg(setupRadio->currentRadio.stopbits).arg(serialCommonData::parityStr[setupRadio->currentRadio.parity]).arg(serialCommonData::handshakeStr[setupRadio->currentRadio.handshake]).arg(serialCommonData::forceLinesStr[setupRadio->currentRadio.forceDtr]).arg(serialCommonData::forceLinesStr[setupRadio->currentRadio.forceRts]));
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
        else if (setupRadio->currentRadio.portType == RigCapConstants::PortType::none)
        {
            showStatusMessage(tr("Connected via Rigctld: %1 - %2").arg(setupRadio->currentRadio.rigMfg_Name).arg(setupRadio->currentRadio.rigModelName));
        }

    }
    else
    {

        logMessage(QString("Radio Open Error"));
        showStatusMessage(tr("Radio Open error"));
        return OPEN_FAILED;
    }

    PubSubName psname(setupRadio->currentRadio.radioName);

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
        showStatusMessage(tr("Please select a Radio"));
        return OPEN_FAILED;
    }

    logMessage(QString("Open Radio: Opening Radio %1 PortType %2").arg(setupRadio->currentRadio.radioName).arg(hamlibData::portTypeList[setupRadio->currentRadio.portType]));
    showStatusMessage(tr("Opening Radio: %1").arg(setupRadio->currentRadio.radioName));

    if (setupRadio->currentRadio.portType == RigCapConstants::PortType::serial)
    {
        if(setupRadio->comportAvial(setupRadio->currentRadio.radioNumber.toInt(), setupRadio->currentRadio.comport) == -1)
        {
            logMessage(QString("Open Radio: Check comport - defined port %1 not available on computer").arg(setupRadio->currentRadio.comport));
            showStatusMessage(tr("Comport %1 no longer configured on computer?").arg(setupRadio->currentRadio.comport));
            return OPEN_FAILED;
        }

        if (setupRadio->currentRadio.comport == "")
        {
            logMessage(QString("Open Radio: No comport"));
            showStatusMessage(tr("Please select a Comport"));
            return OPEN_FAILED;
        }

    }

    else if (setupRadio->currentRadio.portType == RigCapConstants::PortType::network)
    {
        if (setupRadio->currentRadio.networkAdd == "" || (setupRadio->currentRadio.networkPort == ""))
        {
            logMessage(QString("Open Radio: No network or Port Number"));
            showStatusMessage(tr("Please enter a network Address and Port Number"));
            return OPEN_FAILED;
        }

    }
    if (setupRadio->currentRadio.rigModel == "")
    {
        logMessage(QString("Open Radio: No radio model"));
        showStatusMessage(tr("Please select a radio model"));
        return OPEN_FAILED;
    }

    radio = rigFactory->createRigs(rigFactory->supported_rigs()->value(setupRadio->currentRadio.rigModel).RigCapabilities::rigModelNumber);
    radio->setTraceComms(traceCommsFlag);
    rigCap = rigFactory->supported_rigs()->value(setupRadio->currentRadio.rigModel);

    // set state of trace hamlib comms

    // Message from rigcontrol
    //connect(radio, SIGNAL(debug_protocol(QString)), this, SLOT(logMessage(QString)));

    if (!radio->getRigConnected())
    {
        // if radio is already open, don't reinit it
        retCode = radio->rigInit(setupRadio->currentRadio, RIGCTLD_OFF);
        if (retCode < 0)
        {
            radio->closeRig();
            logMessage(QString("Error Opening Radio Error Code = %1").arg(QString::number(retCode)));
            radioError(retCode, tr("Open Radio"));
            return OPEN_FAILED;
        }

    }




    // let's see if we can get freq from radio and confirm comms
    if (radio->getRigConnected())
    {

        int retCode = Rig_OK;
        showStatusMessage(tr("Attempting to communicate with radio - %1").arg(setupRadio->currentRadio.radioName));
        //delay(1);
        retCode = radio->getFrequency(VFO::CURRENT_VFO, rfrequency);


        if (retCode < Rig_OK)
        {
            logMessage(QString("Open Radio: Test Communication - Get Freq error, code = %1").arg(QString::number(retCode)));
            radioError(retCode, tr("Test Radio Connection\n\nMinos tried to read the radio frequency,\nbut nothing was received from the radio.\n\nPlease check connections and/or settings.\nSome radios/interfaces may require Force DTR or Force RTS to be set High, to power the interface."));
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
        showStatusMessage(tr("Radio Opened: %1").arg(setupRadio->currentRadio.radioName));

        if (setupRadio->currentRadio.portType == RigCapConstants::PortType::serial)
        {
            showStatusMessage(tr("Connected: %1 - %2, %3, %4, %5, %6, %7, Handshake %8, ForceDTR %9, ForceRTS %10")
                              .arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.rigModelName).trimmed().arg(setupRadio->currentRadio.comport).arg(setupRadio->currentRadio.baudrate).arg(setupRadio->currentRadio.databits)
                              .arg(setupRadio->currentRadio.stopbits).arg(serialCommonData::parityStr[setupRadio->currentRadio.parity]).arg(serialCommonData::handshakeStr[setupRadio->currentRadio.handshake]).arg(serialCommonData::forceLinesStr[setupRadio->currentRadio.forceDtr]).arg(serialCommonData::forceLinesStr[setupRadio->currentRadio.forceRts]));
        }
        else if (setupRadio->currentRadio.portType == RigCapConstants::PortType::network)
        {
            if (setupRadio->currentRadio.rigModelNumber == hamlibData::RIGCTL)
            {
                showStatusMessage(tr("Connected: %1 - %2, %3:%4 - %5 %6").arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.rigModelName).arg(setupRadio->currentRadio.networkAdd).arg(setupRadio->currentRadio.networkPort).arg(rigctld_radioMfg).arg(rigctld_radioName));
            }
            else
            {
                showStatusMessage(tr("Connected: %1 - %2, %3:%4").arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.rigModelName).arg(setupRadio->currentRadio.networkAdd).arg(setupRadio->currentRadio.networkPort));
            }

        }
        else if (setupRadio->currentRadio.portType == RigCapConstants::PortType::none)
        {
            showStatusMessage(tr("Connected: %1 - %2").arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.rigModelName));
        }

    }
    else
    {

        logMessage(QString("Radio Open Error"));
        showStatusMessage(tr("Radio Open error"));
        return OPEN_FAILED;
    }

    PubSubName psname(setupRadio->currentRadio.radioName);

    msg->rigCache.publish();
    return OPEN_OK;
}

void RigControlMainWindow::closeRadio()
{

    if (rigFactory->supported_rigs()->value(setupRadio->currentRadio.rigModel).pollData)
    {
        pollTimer->stop();
    }
    else
    {
        if (radio != nullptr)
        {
            disconnect(radio, SIGNAL(newFreq()), this, SLOT(onNewFreq));
            disconnect(radio, SIGNAL(onNewMode()), this, SLOT(onNewMode()));
            disconnect(radio, SIGNAL(rigStatus(int, QString)), this, SLOT(onRigStatus(int, QString)));
        }

    }



    int retCode;

    if (radio != nullptr)
    {
        if (radio->getRigConnected())
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

    if (radio != nullptr)
    {
        delete radio;
    }


    showStatusMessage(tr("Disconnected"));
    sendStatusToLogDisConnected();
    displayFreqVfo(0.0);
    displayTransVertVfo(0.0);
    ui->transVertBandDisp->setText("");
    ui->transVertSwNum->setText("");
    turnOffAllsupRadioIndicators();
    displaySignalStrength(-54);
    setRigCltdIndicatorVisible(false);
    rigCap = RigCapabilities();
    logMessage(QString("Radio Closed"));

    msg->rigCache.publish();
}






void RigControlMainWindow::writeWindowTitle(QString appName)
{
    if (appName.length() > 0)
    {
        this->setWindowTitle(tr("Minos Rig Control - %1 - Logger").arg(appName));
    }
    else
    {
        this->setWindowTitle(tr("Minos Rig Control - Local"));
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
        retCode = getAndSendFrequency(VFO::CURRENT_VFO);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Freq error %1").arg(QString::number(retCode)));
            radioError(retCode, tr("Request Freq"));

        }

    }

    if (radioCommsOK)
    {

        logMessage("Get radio mode");
        retCode = getAndSendMode(VFO::CURRENT_VFO);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Mode error %1").arg(QString::number(retCode)));
            radioError(retCode, tr("Request Mode"));

        }
        else
        {
            logMessage(QString("Got Mode = %1").arg(convertModeToQString(rmode)));
        }
    }




    if (radioCommsOK && radioSupSetRit && ritEnable)
    {
        logMessage((QString("Poll RIT Info - Get Rit Freq = %1").arg(radioSupGetRit ? "True" : "False")));


        if (radioSupGetRit)
        {
            retCode = getRitFreq(VFO::CURRENT_VFO);
            if (retCode < 0)
            {
                // error
                logMessage(QString("Get radioInfo: Get RIT Freq error").arg(QString::number(retCode)));
                radioError(retCode, tr("Request RIT Freq"));
            }
            else
            {
                logMessage(QString("Get radioInfo: Get RIT Freq = %1").arg(convertRitFreqToStr(rRitFreq)));
            }

        }



        if (radioSupGetRit)
        {
            bool ritStatus = false;
            retCode = getRitRadioStatus(VFO::CURRENT_VFO, &ritStatus);
            if (retCode < 0)
            {
                //error
                logMessage(QString("Get radioInfo: Get RIT state error").arg(QString::number(retCode)));
                radioError(retCode, tr("Request RIT State"));
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
        retCode = getVolume(VFO::CURRENT_VFO);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Volume error").arg(QString::number(retCode)));
            radioError(retCode, tr("Request Volume"));
        }

    }


    if (radioCommsOK && supSignalStrength)
    {
        retCode = getSignalStrength(VFO::CURRENT_VFO);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Volume error").arg(QString::number(retCode)));
            radioError(retCode, tr("Request Volume"));
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


void RigControlMainWindow::loggerSetFreq(QString freq)
{
    logMessage(QString("Recieved Freq from Logger = %1").arg(freq));
    if (radioCommsOK && !rigErrorFlag)
    {
        logMessage(QString("new freq %1, old freq %2").arg(freq).arg(logger_freq));
        if (freq == NO_BAND_SUPPORT)
        {
            logMessage(QString("loggerSetFreq: No transverter found for this band"));
            clearTransVertSupport();
            return;
        }

        logger_freq = freq;
        setFreq(freq, VFO::CURRENT_VFO);
    }
    // but the rig hasn't updated...
    msg->rigCache.publish();
}


void RigControlMainWindow::setFreq(QString freq, VFO vfo)
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

            retCode = radio->setFrequency(static_cast<Frequency>(f), vfo);
            if (retCode != Rig_OK)
            {
                if (retCode == -9)
                {
                    logMessage(QString("SetFreq: Invalid Tx Freq for Radio, Freq = %1").arg(QString::number(f)));
                    cmdLockOff();
                    return;
                }

                logMessage(QString("SetFreq: Error Setting Freq Code = %1").arg(retCode));
                radioError(retCode, tr("SetFreq"));
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

int RigControlMainWindow::getAndSendFrequency(VFO vfo)
{
    double transVertF = 0;
    int retCode = 0;
    int tvNum = 0;
    bool b = false;

    retCode = radio->getFrequency(vfo, rfrequency);
    if (retCode == Rig_OK)
    {
        curVfoFrq = rfrequency;
        logMessage(QString("Get Freq: Read Freq from Radio = %1").arg(QString::number(curVfoFrq, 'f', 0)));
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
        radioError(retCode, tr("Request Frequency"));
    }
    return retCode;
}

void RigControlMainWindow::onNewFreq()
{
    getAndSendFrequency(CURRENT_VFO);
}

QString RigControlMainWindow::getBand(Frequency freq)
{
    for (int i = 0; i < setupRadio->bands.count(); i++)
    {
        if (freq >= setupRadio->bands[i].fLow && freq <= setupRadio->bands[i].fHigh)
        {
            return setupRadio->bands[i].name;
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


void RigControlMainWindow::runRigCtlDaemon(const QString manufacturer, const QString model, const QString comport,
                                           const QString baudRate, const QString dataBits, const QString civ, const QString netAdd, const QString portNum,
                                           const QString stopBits, const int& parity, const QString handshake, const QString rtsState, const QString dtrState,
                                           rigCtldTrace::rigCtldTraceCodes diagnostics)
{

    setupRadio->getRigCtldExePathFromFile();

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
/* ******************************************************************
    if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_SERIAL)
    {
        parityNames = radio->getParityCodeNames();
        parityName = parityNames[parity];
        arguments << "-m" + model.trimmed() << "-r" + serPort  << "-s" + baudRate.trimmed() << "--set-conf=data_bits=" + dataBits.trimmed() << "--set-conf=stop_bits=" + stopBits.trimmed()
                  << "--set-conf=serial_parity=" + parityName.trimmed() << "--set-conf=serial_handshake=" + handshake.trimmed() << "--set-conf=rts_state=" + rtsState.trimmed() << "--set-conf=dtr_state=" + dtrState.trimmed();

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


    trace(QString("runRigCtlDaemon:: start rigCtlD - manufacturer = %1, model = %2, comport = %3, baudrate = %4, databits = %5, stopbits = %6, parity = %7, handshake = %8, rtsState = %9, dtrState = %10, civ = %11, netaddress = %12, netPort = %13")
          .arg(manufacturer).arg(model).arg(serPort).arg(baudRate).arg(dataBits).arg(stopBits).arg(parityName).arg(handshake).arg(rtsState).arg(dtrState).arg(civ).arg(networkAdd).arg(networkPort));

    rigCtldProcess->start(program, arguments);

*/
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
            int radioModelNumber = setupRadio->availRadioData[i]->rigModelNumber;
            buildSupBandList(i, radioModelNumber, supBandList);
            qDebug() << "support list initCache" << supBandList;
            sendBandListLogger(i, supBandList);
            bool f = rigFactory->supported_rigs()->value(setupRadio->availRadioData[i]->rigModel).supportVolume;
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
    buildSupportedRadioBands(radioIdx, radioModelNumber, supBandsList);
    qDebug() << "buildSupBandList" << radioModelNumber << supBandsList;

    // merge radio bands and transverter bands
    if(setupRadio->availRadioData[radioIdx]->transVertEnable)
    {
        if (bands.count() > 0)
        {
            for (int i = 0; i < bands.count(); i++)
            {
                if (findSupRadioBand(bands[i].name, supBandsList) ||  findSupTransBand(bands[i].name, radioIdx))
                {
                    bandList.append(bands[i].name);
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
void RigControlMainWindow::buildSupportedRadioBands(int radioIdx, int radioModelNumber, QStringList& supBandList)
{

    if (radioModelNumber <= RigId::NonHamlibBaseId)
    {
        for (int i = 0; i < bands.count(); i++)
        {

            if (rigFactory->checkForBands(radioModelNumber, bands[i].fLow))
            {
                supBandList.append(bands[i].name);

            }
        }
    }
    else
    {
        // non hamlib radios
        if (setupRadio->availRadioData[radioIdx]->support50MHz)
        {
            supBandList.append(bands[0].name);
        }
        if (setupRadio->availRadioData[radioIdx]->support70MHz)
        {
            supBandList.append(bands[1].name);
        }
        if (setupRadio->availRadioData[radioIdx]->support144MHz)
        {
            supBandList.append(bands[2].name);
        }
        if (setupRadio->availRadioData[radioIdx]->support432MHz)
        {
            supBandList.append(bands[3].name);
        }
        if (setupRadio->availRadioData[radioIdx]->support1296MHz)
        {
            supBandList.append(bands[4].name);
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


int RigControlMainWindow::getAndSendMode(VFO vfo)
{

    int retCode = 0;

    retCode = radio->getMode(vfo, rmode);
    // get passband state
    //hamlibData::pBandState pBState = modePbState[getMinosModeIndex(slogMode)];
    //QString spBState = QString::number(pBState);

    if (retCode == Rig_OK)
    {
        logMessage(QString("Get Mode: From Rx mode = %1").arg(convertModeToQString(rmode)));
        curMode = rmode;

        sCurMode = convertModeToQString(rmode);

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
                    //displayPassband(rwidth);
                    sendModeToLog(QString("%1:%2").arg(hamlibData::MGM).arg(setupRadio->currentRadio.mgmMode));

            }
            else
            {
                displayModeVfo(convertModeToQString(rmode));
                //displayPassband(rwidth);
                sendModeToLog(QString("%1:%2").arg(convertModeToQString(rmode)).arg(" "));
            }


        }
        else
        {

            displayModeVfo(hamlibData::MGM);
            //displayPassband(rwidth);
            sendModeToLog(QString("%1:%2").arg(hamlibData::MGM).arg(setupRadio->currentRadio.mgmMode));
        }

    }

    return retCode;
}


void RigControlMainWindow::onNewMode()
{
    getAndSendMode(CURRENT_VFO);
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
                if (curMode !=  convertQStringToMode(setupRadio->currentRadio.mgmMode))
                {
                    setMode(setupRadio->currentRadio.mgmMode, VFO::CURRENT_VFO);
                    logMessage((QString("Log SetMode: MgmMode Flag alread set, Send to setmode MGM Mode = %1").arg(setupRadio->currentRadio.mgmMode)));

                }
            }
            else
            {
                mgmModeFlag = true;
                setMode(setupRadio->currentRadio.mgmMode, VFO::CURRENT_VFO);

                logMessage((QString("Log SetMode: Set MgmMode Flag, Send to setmode MGM Mode = %1").arg(setupRadio->currentRadio.mgmMode)));
            }
        }
        else
        {
            mgmModeFlag = false;
            logMessage(QString("Log SetMode: Clear mgmModeFlag, Set mode = %1").arg(mode));
            setMode(mode, VFO::CURRENT_VFO);

        }
    }
    // mode won't have changed yet
    //msg->rigCache.publish();
}

void RigControlMainWindow::setMode(QString mode, VFO vfo)
{
    int retCode = 0;

    cmdLockOn();      // lock get radio info
    logMessage(QString("SetMode: Mode Requested = %1").arg(mode));
    mode = mode.left(mode.indexOf(":"));
    MODE mCode = convertQStringToMode(mode);

    if (radioCommsOK)
    {
        retCode = radio->setMode(vfo, mCode);
        if (retCode == Rig_OK)
        {
            logMessage(QString("SetMode: changed! Mode = %1").arg(convertModeToQString(mCode)));

        }
        else
        {
            logMessage(QString("SetMode: Change Error Code = %1, Mode = %2").arg(QString::number(retCode)).arg(convertModeToQString(mCode)));
            radioError(retCode, tr("Set Mode"));
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


MODE RigControlMainWindow::mapQStrMode(QString mode)
{
    if (mode == "AM") return MODE::AM;
    if (mode == "CW") return MODE::CW;
    if (mode == "CW_R") return MODE::CW_R;
    if (mode == "USB") return MODE::USB;
    if (mode == "LSB") return MODE::LSB;
    if (mode == "FSK") return MODE::FSK;
    if (mode == "FSK_R") return MODE::FSK_R;
    if (mode == "DIG_L") return MODE::DIG_L;
    if (mode == "DIG_U") return MODE::DIG_U;
    if (mode == "FM") return MODE::DIG_FM;
    if (mode == "DIG_FM") return MODE::DIG_FM;
    else return MODE::USB;


}


int RigControlMainWindow::getMinosModeIndex(QString mode)
{
    int index = 0;
    for (int i = 0; i < supModeList.count(); i++)
    {
        if (mode == supModeList[i])
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
    setVolume(VFO::CURRENT_VFO, level);

}

/************************** RIT *********************************/


void RigControlMainWindow::clearSupportRitFlags()
{
    radioSupGetRit = false;
    radioSupSetRit = false;
    radioSupGetRitState = false;
    radioSupSetRitState = false;
    radioRitOn = false;
    ritEnable = false;
}

void RigControlMainWindow::getRitSupportStatus()
{


    // Does radio support getting Rit Freq?
    radioSupGetRit = rigCap.supportGetRit;
    logMessage(QString("Get Rit Support Status - getRit support is  = %1").arg(radioSupGetRit ? "True" : "False"));


    // Does radio support setting Rit Freq?
    radioSupSetRit = rigCap.supportSetRit;
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

    radioSupSetRitState = rigCap.supportSetRitState;
    logMessage(QString("Get Rit Support Status - set Rit on/off support is  = %1").arg(radioSupSetRitState ? "True" : "False"));

    // Does radio support getting Rit on/off state?
    radioSupGetRitState = rigCap.supportGetRitState;
    logMessage(QString("Get Rit Support Status - Rit On/Off state support is  = %1").arg(radioSupGetRitState ? "True" : "False"));

}



void RigControlMainWindow::setRitLogStatus(bool status)
{


    logRitOn = status;
    logMessage(QString("Logger RIT Status received = %1").arg(status ? "True" : "False"));
    int retCode = 0;
    ritIndicatorToggle(logRitOn);

    if (radioSupGetRit || radioSupSetRit)
    {
        logMessage(QString("Radio Support RIT On/off switching"));
        if (radioCommsOK)
        {
            // radio supports turning RIT on and off
            retCode = radio->setRitState(VFO::CURRENT_VFO, logRitOn);
            if (retCode < 0)
            {
                logMessage(QString("Error attempting to turn on/off RIT on Radio - Error = %1").arg(retCode));
                radioError(retCode, tr("Turn Rit Off/On"));
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


int RigControlMainWindow::getRitFreq(VFO vfo)
{

    int retCode = 0;
    ShortFreq ritFreq;
    static ShortFreq oldritFreq = 50000;

    retCode = radio->getRit(vfo, ritFreq);
    if (retCode == Rig_OK)
    {
        //ShortFreq iRitFreq = ritFreq;
        if (oldritFreq != ritFreq)
        {
           rRitFreq = ritFreq;
           oldritFreq = ritFreq;
           ui->ritFreq->setText(convertRitFreqToStr(rRitFreq));
           sendRitFreqLogger(static_cast<int>(rRitFreq));
        }
    }


    return retCode;
}


void RigControlMainWindow::setRitFreq(VFO vfo, ShortFreq ritFreq)
{
    if (ritEnable)
    {
        int retCode = 0;

        retCode = radio->setRit(vfo, ritFreq);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Set RIT freq error").arg(QString::number(retCode)));
            radioError(retCode, tr("Set RIT Freq."));
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

void RigControlMainWindow::setRitFreq(int freq)
{
    setRitFreq(VFO::CURRENT_VFO, static_cast<ShortFreq>(freq));
}

/*
int RigControlMainWindow::setRitFreq(vfo_t vfo, shortfreq_t ritFreq)
{
    int retCode = 0;
    cmdLockOn();
    retCode = radio->setRit(vfo, ritFreq);
    cmdLockOff();
    return retCode;
}
*/
int  RigControlMainWindow::getRitRadioStatus(VFO vfo, bool *status)
{
    cmdLockOn();
    bool s;
    int retCode = radio->getRitState(vfo, s);
    cmdLockOff();

    if (retCode == Rig_OK)
    {
        *status = s;
    }
    return retCode;
}


void RigControlMainWindow::sendRadioRitStatusLogger(bool status)
{
    if (appName.length() > 0)
    {

        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setRadioRitStatus(psname, status);
        msg->rigCache.publish();
        logMessage(QString("Send Radio Rit status to logger = %1 psn=%2").arg(status ? "On" : "Off").arg(psname.toString()));

    }
}

void RigControlMainWindow::sendRitFreqLogger(int ritFreq)
{
    if (appName.length() > 0)
    {
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setRadioRitFreq(psname, ritFreq);
        msg->rigCache.publish();
        logMessage(QString("Send Rit freq to logger = %1 psn=%2").arg(convertRitFreqToStr(ritFreq)).arg(psname.toString()));

    }
}





/************************** Volume *********************************/


int RigControlMainWindow::getVolume(VFO vfo)
{
    int retCode = 0;
    float value;
    retCode = radio->getVolume(vfo, &value);
    if (retCode >= 0)
    {
        int vol = 0;
        //value.f = value.f * VOLMULT;
        //vol = qRound(value.f);
        vol = qRound(value);
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


int RigControlMainWindow::setVolume(VFO vfo, int level)
{
    logMessage(QString("Set volume to level = %1").arg(level));
    int retCode = 0;
    float volLevel = level;
    volLevel = volLevel/VOLMULT;
    retCode = radio->setVolume(vfo, volLevel);
    return retCode;

}


/******************* Signal Strength **************************/


void RigControlMainWindow::setSmeterVisible(bool visible)
{
    ui->sMeter->setVisible(visible);
    ui->sMeterLabel->setVisible(visible);
}





int RigControlMainWindow::getSignalStrength(VFO vfo)
{

    int retCode = 0;
    int value;
    retCode = radio->getSignalStrength(vfo, &value);
    if (retCode >= 0)
    {
        if (curSignalStrength != value)
        {
            curSignalStrength = value;
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
/*
void RigControlMainWindow::displayPassband(pbwidth_t width)
{

    ui->passBandlbl->setText(QString::number(width));
}
*/
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


void RigControlMainWindow::onRigStatus(int status, QString cmd)
{
    if (status < Rig_OK)
    {
        radioError(status, cmd);
    }
}

void RigControlMainWindow::radioError(int errorCode, QString cmd)
{

    pollTimer->stop();


    if ( errorCode >= 0)
    {
        return;
    }


    rigErrorFlag = true;

    errorCode *= -1;
    QString errorMsg = radio->getErrorMsgText(errorCode);

    if(appName.count() > 0)
    {
        sendStatusToLogError(errorMsg);
    }

    logMessage(QString("%1 library Error - Code = %2 - %3").arg(radio->getLibraryName()).arg(QString::number(errorCode)).arg(errorMsg));

    QMessageBox::critical(this, tr("RigControl %1 library Error").arg(radio->getLibraryName()), tr("%1\n%2 - %3\nCommand: %4").arg(setupRadio->currentRadio.radioName).arg(errorCode).arg(errorMsg).arg(cmd));

    closeRadio();
    rigErrorFlag = false;
    if (appName.length() >0)
    {
        sendStatusToLogDisConnected();
    }
}


/********************* PTT ****************************************/

// not implemented yet..
/*
int RigControlMainWindow::getTXStatus(vfo_t vfo)
{

    ptt_t pttStatus;
    int retCode = radio->getPttStatus(vfo, &pttStatus);
    if (retCode == rigErrorCodes::RIG_OK)
    {
       if (pttStatus == RIG_PTT_ON)
       {
           // turn on indicator

       }
    }

   return retCode;

}
*/

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
    traceCommsFlag = config.value("TraceLog", false).toBool();
    config.endGroup();

    ui->actionTraceComms->setChecked(traceCommsFlag);
}

void RigControlMainWindow::saveTraceLogFlag(bool state)
{

    // set state of hamlib commms tracing

    radio->setTraceComms(state);

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
    QMessageBox::about(this, tr("Minos RigControl"), tr("Minos QT RigControl\nCopyright D Balharrie G8FKH/M0DGB 2019"));
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
        logMessage(QString("Send radio %1, name %2").arg(QString::number(i)).arg(radioList[i]));
    }
    logMessage(QString("radiolist complete"));
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
        msg->rigCache.publish();
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
        msg->rigCache.publish();
    }
}


void RigControlMainWindow::sendStatusToLogConnected()
{
    logMessage(QString("Send status to logger connected"));
    sendStatusLogger(tr(RIG_STATUS_CONNECTED));
}

void RigControlMainWindow::sendStatusToLogDisConnected()
{
    logMessage(QString("Send status to logger disconnected"));
    sendStatusLogger(tr(RIG_STATUS_DISCONNECTED));
}


void RigControlMainWindow::sendStatusToLogError(QString errMsg)
{
    logMessage(QString("Send error status to logger - %1").arg(errMsg));
    sendStatusLogger(QString("%1:%2").arg(tr(RIG_STATUS_ERROR)).arg(errMsg));
}




void RigControlMainWindow::sendFreqToLog(Frequency freq)
{

    if (appName.length() > 0)
    {
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setRadioFreq(psname, static_cast<double>(freq));
        msg->rigCache.publish();

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
        msg->rigCache.publish();
    }
}

void RigControlMainWindow::sendVolToLog(int level)
{
    if (appName.length() > 0)
    {
        logMessage(QString("Send volume to logger = %1").arg(QString::number(level)));
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setRadioVolume(psname, level);
        msg->rigCache.publish();
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
        msg->rigCache.publish();

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
        msg->rigCache.publish();

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
        msg->rigCache.publish();

    }
}

void RigControlMainWindow::sendTransVertOffsetToLogger(int tvNum)
{
    QString f = convertFreqToStr(setupRadio->currentRadio.transVertSettings[tvNum]->transVertOffset);
    logMessage(QString("Send Transvert Offset to logger = %1%2").arg(setupRadio->currentRadio.transVertEnable ? f = "-" : f = "+").arg(f));
    PubSubName psname(setupRadio->currentRadio.radioName);
    msg->rigCache.setTransverterOffset(psname, setupRadio->currentRadio.transVertSettings[tvNum]->transVertOffset);
    msg->rigCache.publish();
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
    msg->rigCache.publish();
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
        msg->rigCache.publish();

    }
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

        msg.append(tr("App Instance Name  = %1\n").arg(appName));
        if (radio != nullptr)
        {
            msg.append(tr("Hamlib Version = %1\n").arg(radio->getRigLibVersion()));
            msg.append(tr("Radio Name = %1\n").arg(rigCap.rigName));
            msg.append(tr("Radio Number = %1\n").arg(setupRadio->currentRadio.radioNumber));
            msg.append(tr("Rig Model = %1\n").arg(rigCap.rigModelName));
            msg.append(tr("Rig Number = %1\n").arg(rigCap.rigModelNumber));
            msg.append(tr("Rig Manufacturer = %1\n").arg(rigCap.rigManufacturer));
            if (rigCap.rigManufacturer == "Icom")
            {
                if (setupRadio->currentRadio.civAddress == "")
                {
                    msg.append(tr("Icom CIV address = Using Default CIV Address\n"));
                }
                else
                {
                    msg.append(tr("Icom CIV address = %1\n").arg(setupRadio->currentRadio.civAddress));
                }

            }
            msg.append(QString("\n"));
            msg.append(tr("Rig PortType = %1\n").arg(hamlibData::portTypeList[setupRadio->currentRadio.portType]));
            msg.append(tr("Network Address = %1\n").arg(setupRadio->currentRadio.networkAdd));
            msg.append(tr("Network Port = %1\n").arg(setupRadio->currentRadio.networkPort));
            msg.append(tr("Comport = %1\n").arg(setupRadio->currentRadio.comport));
            msg.append(tr("Baudrate = %1\n").arg(setupRadio->currentRadio.baudrate));
            msg.append(tr("Stop bits = %1\n").arg(QString::number(setupRadio->currentRadio.stopbits)));
            msg.append(tr("Parity = %1\n").arg(serialCommonData::parityStr[setupRadio->currentRadio.parity]));
            msg.append(tr("Handshake = %1\n").arg(serialCommonData::handshakeStr[setupRadio->currentRadio.handshake]));
            msg.append(tr("ForceDTR = %1\n").arg(serialCommonData::forceLinesStr[setupRadio->currentRadio.forceDtr]));
            msg.append(tr("ForceRTS = %1\n").arg(serialCommonData::forceLinesStr[setupRadio->currentRadio.forceRts]));

            if (setupRadio->currentRadio.rigCtldEnable)
            {
                msg.append(QString("\n"));
                msg.append(tr("Using rigctld daemon = %1\n").arg(setupRadio->currentRadio.rigCtldEnable ? tr("True") : tr("False")));
                msg.append(tr("Rigctld path = %1\n").arg(setupRadio->getRigCtldExePath()));
                msg.append(tr("Rigctld network address = %1\n").arg(setupRadio->currentRadio.rigCtldNetworkAdd));
                msg.append(tr("Rigctld port address = %1\n").arg(setupRadio->currentRadio.rigCtldNetworkPort));
                msg.append(tr("Rigctld Connect delay = %1\n").arg(rigCtldConnectDelay));
            }

            msg.append(QString("\n"));
            msg.append(tr("TransVert Enable = %1\n").arg(setupRadio->currentRadio.transVertEnable ? tr("True") : tr("False")));
            msg.append(tr("Number of TransVerters = %1\n").arg(setupRadio->currentRadio.numTransverters));

            for (int i = 0; i < setupRadio->currentRadio.numTransverters; i++)
            {
                msg.append(QString("\n"));
                msg.append(tr("Transverter %1\n").arg(i));
                msg.append(tr("Transverter Name = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->transVertName));
                msg.append(tr("Transverter Band = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->band));
                msg.append(tr("Transverter Offset = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->transVertOffsetStr));
                msg.append(tr("Transverter Switch num = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->transSwitchNum));
                msg.append(tr("Transverter Switch enable = %1\n").arg(setupRadio->currentRadio.enableTransSwitch  ? tr("True") : tr("False")));
            }

            msg.append(QString("\n"));
            msg.append(tr("Radio Supports RIT = %1\n").arg(radioSupSetRit ? tr("True") : tr("False")));
            if (radioSupSetRit)
            {
                msg.append(tr("Rit Enable On = %1\n").arg(ritEnable  ? tr("True") : tr("False")));
                msg.append(tr("Radio Supports Get RIT Freq = %1\n").arg(radioSupGetRit ? tr("True") : tr("False")));
                msg.append(tr("Radio Supports Set RIT Freq = %1\n").arg(radioSupSetRit ? tr("True") : tr("False")));
                //msg.append(tr("Radio Supports Get RIT State On/Off = %1\n").arg(radioSupGetRitState ? tr("True") : tr("False")));
                //msg.append(tr("Radio Supports Set RIT State On/Off = %1\n").arg(radioSupRitOnOff ? tr("True") : tr("False")));
            }
            msg.append(tr("Radio Polltime = %1\n").arg(setupRadio->currentRadio.pollInterval));
            msg.append(tr("Tracelog = %1\n").arg(ui->actionTraceComms->isChecked() ? tr("True") : tr("False")));
        }
        else
        {
            msg.append(tr("No Radio selected\n"));
        }
        QMessageBox::about(this, tr("Minos RigControl"), msg);



        }









}

void RigControlMainWindow::dumpRadioToTraceLog()
{

    if (setupRadio->currentRadio.radioName != "" && radio != nullptr)
    {
        trace("*** Radio Update ***");
        trace(QString("App Instance Name  = %1").arg(appName));
        trace(QString("Hamlib Version = %1").arg(radio->getRigLibVersion()));
        trace(QString("Radio Name = %1").arg(rigCap.rigName));
        trace(QString("Radio Number = %1").arg(setupRadio->currentRadio.radioNumber));
        trace(QString("Rig Model = %1").arg(rigCap.rigModelName));
        trace(QString("Rig Number = %1").arg(rigCap.rigModelNumber));
        trace(QString("Rig Manufacturer = %1").arg(rigCap.rigManufacturer));
        if (rigCap.rigManufacturer == "Icom")
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
        trace(QString("Parity = %1").arg(serialCommonData::parityStr[setupRadio->currentRadio.parity]));
        trace(QString("Handshake = %1").arg(serialCommonData::handshakeStr[setupRadio->currentRadio.handshake]));
        trace(QString("ForceDTR = %1").arg(serialCommonData::forceLinesStr[setupRadio->currentRadio.forceDtr]));
        trace(QString("ForceRTS = %1").arg(serialCommonData::forceLinesStr[setupRadio->currentRadio.forceRts]));

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
        trace(QString("Radio Supports RIT = %1").arg(radioSupSetRit ? "True" : "False"));
        if (radioSupSetRit)
        {
            trace(QString("Rit Enable On = %1").arg(ritEnable  ? "True" : "False"));
            trace(QString("Radio Supports Get RIT Freq = %1").arg(radioSupGetRit ? "True" : "False"));
            trace(QString("Radio Supports Set RIT Freq = %1").arg(radioSupSetRit ? "True" : "False"));
            //trace(QString("Radio Supports Get RIT State On/Off = %1").arg(radioSupGetRitState ? "True" : "False"));
            //trace(QString("Radio Supports Set RIT State On/Off = %1").arg(radioSupRitOnOff ? "True" : "False"));
        }

        trace(QString("Radio Polltime = %1").arg(setupRadio->currentRadio.pollInterval));
        trace(QString("Tracelog = %1").arg(ui->actionTraceComms->isChecked() ? "True" : "False"));

    }
    else
    {
        trace(QString("No Radio selected"));
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

    if (valInputFreq(f, tr("Invalid freq!")))
    {
        // convert radio freq
        f = convertSinglePeriodFreqToFullDigit(f).remove('.');
        setFreq(f, VFO::CURRENT_VFO);
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
