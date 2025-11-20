/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include <QTimer>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QFileDialog>
#include <QBitArray>
#include <QMetaType>
#include <QDir>

#include "cutils.h"
#include "regsettings.h"
#include "serialCommonData.h"
#include "MShowMessageDlg.h"
#include "fileutils.h"
#include "rigcommon.h"
#include "rigsetupdialog.h"
#include "rigcontrolrpc.h"
#include "rigutils.h"
#include "rigctldclient.h"
#include "serialdata.h"
#include "LogEvents.h"
#include "MTrace.h"
#include "checkHamlibVersionIsValid.h"

#include "rigcontrolmainwindow.h"
#include "ui_rigcontrolmainwindow.h"
#include "voicekeyerCommonConstants.h"

RigControlMainWindow *mainWindow = nullptr;



const bool PUBLISH_NOW = true;
const bool DONT_PUBLISH_NOW = false;

// hamlib conf token strings
const char * HAMLIB_RETRY = "retry";
const char * HAMLIB_TIMEOUT = "timeout";

RigControlMainWindow::RigControlMainWindow(QWidget *parent) :
   QMainWindow(parent),
   ui(new Ui::RigControlMainWindow),
   cmdLockFlag(false)

{
    ui->setupUi(this);
    mainWindow = this;

    //setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    serialData::translateSerialData();

    connect(commandReader.data(), &CommandReader::commandLine, this, &RigControlMainWindow::onCommandRead);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    writeWindowTitle(appName);
    //testMode = (appName.length() == 0);

    testMode = false;

    createCloseEvent();

    connect(&LogTimer, &QTimer::timeout, this, &RigControlMainWindow::LogTimerTimer);
    LogTimer.start(100);

    msg = new RigControlRpc(this);

    rigCtldDetails = new RigCtldDetails();

    rigCtldProcess = new QProcess(this);

    connect(rigCtldProcess, &QProcess::readyReadStandardOutput, this, &RigControlMainWindow::rigCtldMessage);
    connect(rigCtldProcess, &QProcess::readyReadStandardError, this, &RigControlMainWindow::rigCtldErrorMessage);
    connect(rigCtldProcess, &QProcess::started, this, &RigControlMainWindow::rigCtldStarted);

    setRigCltdIndicatorVisible(false);
    setMemoryGroupVisible(false);
    setPttGroupItemsVisible(false);
    setSmeterVisible(false);


    getRigCtldConnectDelay();

    RigCtldStatusTimer = new QTimer(this);
    connect(RigCtldStatusTimer, &QTimer::timeout, this, &RigControlMainWindow::rigCtldStatusTimeout);


    //selectedRigSupCap = new RigSupCapabilities();
    loggerRequests = new LoggerRequests();
    rigStateDetails = new RigStateDetails();


    RegSettings settings;
    geoStr = "geometry";
    geoStr = geoStr + appName;

    ui->testRitButton->setVisible(false);
    ui->setRitSpinner->setVisible(false);

    QByteArray geometry = settings.getSettings().value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    checkIniFileVersion();  // converts earlier availRadio ini formats



    trace("Create Rigfactory and add rigs to list");
    radio = nullptr;
    rigFactory = new RigFactory(false, this);


    bool filtret = BandList::getBandList().loadAllBands(bands, true);  // filter by available
    if (filtret)
    {
        ui->supportBandsLabel->setText(tr("Minos Selected Bands"));
    }

    QString fileName = RIG_CONFIGURATION_FILEPATH_LOGGER() + MINOS_RADIO_CONFIG_FILE;
    QSettings config(fileName, QSettings::IniFormat);

    rigStateDetails->mgmModes = config.value("MGM_Modes/MgmModes", "").toStringList();
    rigStateDetails->RTTYModes = config.value("RTTY_Modes/RTTYModes", "").toStringList();
    rigStateDetails->PSKModes = config.value("PSK_Modes/PSKModes", "").toStringList();

    //ritTestEnabled = config.value("Rit_Test_Control/Rit_test_on", false).toBool();

    hamlibOk = false;

    logMessage(QString("Checking installed Hamlib version"));
    int hamlibCheckErrorNum = checkHamlibVersionIsValid(hamlibOk, hamlib_version, MINIMUM_HAMLIB_VERSION);

    if (hamlibCheckErrorNum < 0)
    {

        if (hamlibCheckErrorNum == -2 )
        {
            QMessageBox::critical(nullptr, tr("RigControl Hamlib Library Version Error!"), tr("Installed Hamlib version %1 is incompatible.\nIt should be version %2 or greater.\n\nPlease check your installation.\nYou will not be able to select a radio until this is rectified!").arg(hamlib_version).arg(MINIMUM_HAMLIB_VERSION), QMessageBox::Ok);
            logMessage(QString("Error version number conversion to int failed - installed version = %1, minimum version = %2").arg(hamlib_version).arg(MINIMUM_HAMLIB_VERSION)); // error)
        }
        else if (hamlibCheckErrorNum == -1 )
        {
            // we should not get here....
            QMessageBox::critical(nullptr, tr("Hamlib Version Test conversion error!"), tr("Hamlib Version Test conversion error. Please report error"));
            logMessage(QString("Error version number conversion to int failed - installed version = %1, minimum version = %2").arg(hamlib_version).arg(MINIMUM_HAMLIB_VERSION));
        }
        else if (hamlibCheckErrorNum == -3)
        {
            //we should not get here...
            logMessage(QString("Error: checkhamlibVersion fails error code = %1").arg(hamlibCheckErrorNum));
        }


    }




    if (hamlibOk)
    {

        logMessage(QString("installed Hamlib version %1 is OK").arg(hamlib_version));

        // init cache with radio data
        trace(QString("rigcontrol: Started by logger appname = %1").arg(appName));
        QStringList availRadios;
        getAvailRadiosList(availRadios);
        sendRadioListLogger(availRadios);

        initCacheData(availRadios);
        msg->rigCache.publish();
    }


    serialTVSw = new SerialTVSwitch();     // create local serial sw



    if (appName.isEmpty())
    {
        // standalone mode
        ui->testRadioButton->setVisible(false);
        setTestControlsVisible(false);
        setTestMode(true);
        loadTestModeCombo();
    }
    else
    {
        // started by logger don't show radio selectbox
        setSelectRadioBoxVisible(testMode);
        ui->testActionsGroupBox->setVisible(false);
        setTestControlsVisible(testMode);
        setRadioNameLabelVisible(!testMode);
    }





    pollTimer = new QTimer(this);

    status = new QLabel;
    ui->statusBar->addWidget(status);
    ui->radioNameDisp->setText("");


    readTraceLogFlag();

    initActionsConnections();

    if (hamlibOk)
    {
        initSelectRadioBox();
    }



    setTransVertDisplayVisible(false);
    sendTransVertSwitchToLogger(TRANSSW_NUM_DEFAULT);
    //sendTransVertSwitchToComPort(TRANSSW_NUM_DEFAULT);

    setRitFreqDisplayVisible(false);
    setRitGetSetFreqIndicatorVisible(false);

    initialiseSupportedRadioDisplay();

    setPolltime(1000);

    ui->selectRadioBox->clearFocus();





    upDateRadio(currentRadioName);


    if (!hamlibOk)
    {
        showStatusMessage(tr("Error: Installed Hamlib version %1 is incorrect, should be version %2 or greater").arg(hamlib_version).arg(MINIMUM_HAMLIB_VERSION));
    }

    trace("*** Rig App Started ***");
}

RigControlMainWindow::~RigControlMainWindow()
{
    trace("RigControlMainWindow::~RigControlMainWindow()");

    delete ui;
    delete msg;
}



void RigControlMainWindow::onSetPttOnOff(bool pttOnState)
{
    trace(QString("PTT On/Off message from Logger - %1").arg(pttOnState ? "On" : "Off"));

    setPttOnOff(pttOnState);
}

void RigControlMainWindow::setPttOnOff(bool pttOnState)
{
    trace(QString("Set PTT On/Off State: %1").arg(pttOnState ? "On" : "Off") );

    if (radio)
    {
        if (radio->getRigConnected())
        {
            setTxState(rigStateDetails->curVfo, pttOnState);
        }
        else
        {
            trace(QString("Rig Not connected ignore change ptt state"));
        }
    }
}

void RigControlMainWindow::logMessage( QString s )
{

        trace( s );
}


void RigControlMainWindow::LogTimerTimer()
{
    static bool closed = false;
    if ( !closed )
    {
        if ( checkCloseEvent() )
        {
            trace("close event seen");
            closed = true;
            close();
        }
    }
}


void RigControlMainWindow::closeEvent(QCloseEvent *event)
{
    trace("MinosRigControl::closeEvent");

    LogTimer.stop();
    pollTimer->stop();
    cwMessageTestTimer->stop();
    RigCtldStatusTimer->stop();

    closeRadio();

    // and tidy up all loose ends

    RegSettings settings;
    settings.getSettings().setValue(geoStr, saveGeometry());
    trace("MinosRigControl Closing");
    QWidget::closeEvent(event);
}

void RigControlMainWindow::onCommandRead(QString cmd)
{
    bool doClose = false;
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        trace("onCommandRead - Start shutdown");
        closeApp = true;
        closeRadio();
        doClose = true;

        LogTimer.stop();
        pollTimer->stop();
        RigCtldStatusTimer->stop();
    }
    if (doClose)
        close();

    // we definitely know now how we are connected, so update the window title
    writeWindowTitle(appName);
}

void RigControlMainWindow::initActionsConnections()
{
    connect(ui->selectRadioBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &RigControlMainWindow::selectRadio);

    connect(ui->setupRadios, &QPushButton::clicked, this,  &RigControlMainWindow::onLaunchSetup);
    connect(ui->configRigctld, &QPushButton::clicked, this,  &RigControlMainWindow::onConfigureRigctld);
    connect(ui->aboutRC, &QPushButton::clicked,  this, &RigControlMainWindow::aboutRigConfig);
    connect(pollTimer, &QTimer::timeout,  this, &RigControlMainWindow::pollRadioInfo);

    // Message from Logger

    connect(msg, &RigControlRpc::setFreq,  this, &RigControlMainWindow::loggerSetFreq);
    connect(msg, &RigControlRpc::setBand,  this, &RigControlMainWindow::loggerSetBand);
    connect(msg, &RigControlRpc::setRitFreq,  this, &RigControlMainWindow::setRitFreq);
    connect(msg, &RigControlRpc::setRitStatus,  this, &RigControlMainWindow::setRitLogStatus);
    connect(msg, &RigControlRpc::setMode,  this, &RigControlMainWindow::loggerSetMode);
    connect(msg, &RigControlRpc::selectLoggerRadio,  this, &RigControlMainWindow::onSelectRadio);
    connect(msg, &RigControlRpc::setVolume,  this, &RigControlMainWindow::loggerSetVolume);
    connect(msg, &RigControlRpc::setVoiceMessageNum, this, &RigControlMainWindow::onSetVoiceMessageNum);
    connect(msg, &RigControlRpc::setStopVoiceMessage, this, &RigControlMainWindow::onSetStopVoiceMessage);
    connect(msg, &RigControlRpc::setPttOnOff, this, &RigControlMainWindow::onSetPttOnOff);
    connect(msg, &RigControlRpc::setCwTXMessage, this, &RigControlMainWindow::onSetCwTxMessage);
    connect(msg, &RigControlRpc::rereadConfig, this, &RigControlMainWindow::updateRigDetailsCache);


    // test

     connect(ui->txPttTestPb, &QPushButton::clicked, this, &RigControlMainWindow::onTxPttTestPbClicked);


    cwMessageTestTimer = new QTimer(this);
    connect(cwMessageTestTimer, &QTimer::timeout, this, &RigControlMainWindow::onCWMessageTimerTimeout);
    connect(ui->cwKeyerPb, &QPushButton::clicked, this, &RigControlMainWindow::onCwKeyerPbClicked);
    connect(ui->testModeComboBox, &QComboBox::currentTextChanged, this, &RigControlMainWindow::onTestModeComboBoxTextChanged);
    connect(ui->cwKeyerStopPb, &QPushButton::clicked, this, &RigControlMainWindow::onCwKeyerStopPbClicked);

    connect(ui->selFreq, &QPushButton::clicked,  this, &RigControlMainWindow::selFreqClicked);
    connect(ui->freqInputBox, &QLineEdit::editingFinished,  this, &RigControlMainWindow::selFreqClicked);
    connect(ui->selectRadioFromLoggerRb, &QRadioButton::clicked, this, &RigControlMainWindow::onSelectRadioFromLoggerClicked);
    connect(ui->selectRadioFromRigControlRb, &QRadioButton::clicked, this, &RigControlMainWindow::onSelectRadioFromRigControlClicked);
    connect(ui->voiceMessageSpinBox, &QSpinBox::textChanged, this, &RigControlMainWindow::onVoiceMessageSpinBoxTextChanged);
    connect(ui->voiceMessagePlayPB, &QPushButton::clicked, this, &RigControlMainWindow::onVoiceMessagePlayClicked);
    connect(ui->voiceMessageStopPB, &QPushButton::clicked, this, &RigControlMainWindow::onVoiceMessageStopClicked);

    ui->setRitSpinner->setSingleStep(100);
    connect(ui->setRitSpinner, QOverload<int>::of(&QSpinBox::valueChanged), this, &RigControlMainWindow::testIncRit);
    connect(ui->testRitButton, &QPushButton::clicked, this, &RigControlMainWindow::ritbuttontoggle);


    ui->reconnectButton->setVisible(false);
}


void RigControlMainWindow::pollRadioInfo()
{
    getRadioInfo(PUBLISH_NOW);
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
            ui->selectRadioBox->setCurrentIndex(currentRadio.radioNumber.toInt(&ok, 10));
        }
        upDateRadio(radioName);
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
    QString curSelect = ui->selectRadioBox->currentText();
    initSelectRadioBox();
    int curidx = ui->selectRadioBox->findText(curSelect);
    if (curidx < 0)
    {
        curidx = 0;     // if it has been deleted, set to space
    }
    ui->selectRadioBox->setCurrentIndex(curidx);
}


void RigControlMainWindow::initSelectRadioBox()
{
    ui->selectRadioBox->clear();

    ui->selectRadioBox->addItem("");
    QStringList availRadios;
    getAvailRadiosList(availRadios);
    for (const auto &r: QASCONST(availRadios))
    {
        ui->selectRadioBox->addItem(r);
    }
}

void RigControlMainWindow::selectRadio(int index)
{
    Q_UNUSED(index)
    currentRadioName = ui->selectRadioBox->currentText();
    upDateRadio(currentRadioName);
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

void RigControlMainWindow::upDateRadio(QString radioName)
{
    int radioOpenStat = OPEN_FAILED;


    logMessage(QString("UpdateRadio: Radio requested = %1").arg(radioName));

     pollTimer->stop();      // stop updates

     if (radioCommsOK)  // close existing radio before clearing data
     {

         closeRadio();
     }



    clrRigctldNames();
    clearSupportRitFlags();
    currentRadio.clear();

    rigStateDetails->curTransVertFreq.clear();
    rigStateDetails->selTvBand.clear();

    setRigCltdIndicatorVisible(false);
    setMemoryGroupVisible(false);
    setPttGroupItemsVisible(false);
    setSmeterVisible(false);


    if (radioName.isEmpty())
    {
        // no radio selected
           trace("No radio selected");
           saveCurrentRadio(radioName);
           ui->radioNameDisp->setText("");
           ui->usingLibText->setText("");
           return;
    }

    // Does radio still exist on disc, it might of been deleted by another
    // rigcontrol app on same machine
    if (!availRadiosContains(radioName))
    {
        logMessage("Update Radio - Radio No longer exists!");

        if (testMode)
        {
            // No longer available.
            QMessageBox msgBox;
            msgBox.setWindowTitle(tr("RigControl Select Radio"));
            msgBox.setText(tr("Radio No Longer Exists, please add"));
            msgBox.exec();
        }
        else
        {
            sendStatusToLogError(tr("Radio No Longer Exists, please add"));
        }



    }

    // found radio, update currentRadio from selected radiodata

    currentRadioName = radioName;

    // get current radio user settings
    updateCurrentRadioFromAvailRadios(currentRadioName);

    if (!rigFactory->supported_rigs()->contains(currentRadio.rigModel))
    {
        // No longer available.
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("RigControl Select Radio"));
        QString mess = tr("Radio type \"%1\" no longer exists in Hamlib, Please choose something different")
                           .arg(currentRadio.rigModel);
        sendStatusToLogError(mess);
        msg->rigCache.publish();        // publish all changes are complete

        msgBox.setText(mess);
        msgBox.exec();

        return;
    }

    // get the selected radio rig  capabilities

    selectedRadioSupportCap = rigFactory->supported_rigs()->value(currentRadio.rigModel);

    dumpRadioToTraceLog();


    if (currentRadio.rigCtldEnable)
    {
        radioOpenStat = openRigCtldRadio(currentRadio.startMinosRigCtld);
        setRigCltdIndicatorVisible(true);
        setRigCtldIndicator(RIGCTLD_IND_OFF);

        RigCtldStatusTimer->start(RIGCTLD_STATUS_TIMER_DUR);
    }
    else
    {
        radioOpenStat = openRadio();
    }


    if (radioOpenStat == OPEN_OK && radio)
    {
        //initCacheData();
        int retCode;

        dumpRadioToTraceLog();

        logMessage(QString("Update Radio - Supports reading Vfo = %1").arg(selectedRadioSupportCap.getSupportGetVfo() ? "true" : "false"));



        if (selectedRadioSupportCap.getSupportGetVfo())
        {
            // get current VFO
            retCode = radio->getVfo(&rigStateDetails->curVfo);
            if (retCode != RIG_OK)
            {
                trace(QString("Update Radio - GetVfo Error: %1").arg(retCode));
            }
            else
            {
                trace(QString("Update Radio - Get initial vfo = %1").arg(vfoToStr(rigStateDetails->curVfo)));

            }



            // this is a test for hamlib, check radio supports targetted VFO's in hamlib
            // FT857 and FT897 don't in hamlib
            logMessage(QString("Check radio can setMode with supported VFO, curVfo = %1").arg(vfoToStr(rigStateDetails->curVfo)));
            retCode = radio->setMode(rigStateDetails->curVfo, MODE::USB);
            if (retCode != RIG_OK)
            {
                // no doesn't support targetted Vfo, default to Current Vfo
                trace(QString("Update Radio - Does not support targetted Vfo, use Current_VFO"));

                rigStateDetails->curVfo = CURRENT_VFO;
                selectedRadioSupportCap.setSupportGetVfo(false);      // don't bother with Vfo selection
                selectedRadioSupportCap.setSupportSetVfo(false);
            }
        }
        else
        {
            rigStateDetails->curVfo = CURRENT_VFO;
        }

        trace(QString("Update Radio - Supports writing Vfo = %1").arg(selectedRadioSupportCap.getSupportSetVfo() ? "true" : "false"));
        trace(QString("Update Radio - VFO = %1").arg(vfoToStr(rigStateDetails->curVfo)));

        ui->usingLibText->setText(rigStateDetails->rigCap.getLibraryName());
        trace(QString("Update Radio - Using library = %1").arg(rigStateDetails->rigCap.getLibraryName()));
        if (currentRadio.rigModelNumber == hamlibData::RIGCTL)     // is it rigctl?
        {
            getRigctldNames(currentRadio.networkAdd, currentRadio.networkPort.toUShort());
            bool ok = false;
            int rigNum = rigCtldDetails->rigctld_radioNumber.toInt(&ok, 10);
            if (ok)
            {
                rigCtldDetails->irigctld_radioNumber = rigNum;
            }
            else
            {
                rigCtldDetails->irigctld_radioNumber = 0;
            }
        }

        logMessage(QString("Update Radio - Setup Transverters"));
        setupTransVerter();

        saveCurrentRadio(currentRadioName);

        ui->radioNameDisp->setText(currentRadio.radioName);

        logMessage(QString("Update Radio - Build Supported Bandlist"));
        buildSupBandList(&currentRadio, currentRadio.radioTransSupBands);

        logMessage(QString("Update Radio - check Supported Cat Features"));
        checkSupportCatFeatures();


        updateSupportedRadioIndicators();

        if (testMode)
        {

            logMessage(QString("Update Radio - Set Mode USB Standalone"));
            // initialise rig state

            loggerRequests->slogMode = hamlibData::USB;
            // set mode
            setMode(hamlibData::USB, rigStateDetails->curVfo);
        }
        else
        {
            logMessage(QString("Update Radio - Logger Set Freq = %1, Set Mode = %2").arg(loggerRequests->selRadioFreq.traceStr(), loggerRequests->selRadioMode));
            loggerSetFreq(loggerRequests->selRadioFreq);
            loggerSetMode(loggerRequests->selRadioMode);
        }

        logMessage(QString("Update Radio - Get Initial Radio Info"));
        getRadioInfo(DONT_PUBLISH_NOW);

        sendRitEnableStatusLogger();

        sendStatusToLogConnected();

        checkSupportPollRadio();


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
                    logMessage(QString("Update Radio - Radio Failed to Connect - Error Message Number out of range = %1").arg(msgOffSet));
                }
            }
        }


        logMessage(QString("Update Radio - #### Radio Failed to connect Error Code = %1, %2  ####").arg(radioOpenStat).arg(radioOpenMessages[radioOpenStat * -1]));
        sendStatusToLogDisConnected();

    }

    if (!testMode)
    {
        logMessage(QString("Update Radio - publish to logger"));
        msg->rigCache.publish();        // publishes all the changes
        logMessage(QString("Update Radio - send radio change complete to logger"));
        sendRadioSwitchCompleteToLogger();
        msg->rigCache.publish();        // publish all changes are complete

    }
}

bool RigControlMainWindow::availRadiosContains(const QString radioName)
{

    QString fileName = RADIO_PATH_LOGGER() + FILENAME_AVAIL_RADIOS;
    QSettings  settings(fileName, QSettings::IniFormat);

    return settings.childGroups().contains(radioName);
}


void RigControlMainWindow::checkSupportCatFeatures()
{

    checkSupportVolume();

    checkSupportSMeter();

    checkSupportRit();

    bool supVoiceMem = checkSupportVoiceMemory();
    bool supCwMem = checkSupportCwKeyerMemory();

    if (supVoiceMem || supCwMem)
    {
        setMemoryGroupVisible(true);
    }
    else
    {
        setMemoryGroupVisible(false);
    }



    checkSupportPtt();

}


void RigControlMainWindow::setupTransVerter()
{
    // setup local serial transvert switch
    if (radioCommsOK && currentRadio.transVertEnable
            && currentRadio.enableTransSwitch
            && currentRadio.enableLocTVSwMsg)
    {
        selTransVertBandIndicator = "";     // force active tranvert indicator update
        if (serialTVSw->getOpenFlag())
        {
            serialTVSw->closeComport();

        }
        if (serialTVSw->openComport(currentRadio.locTVSwComport))
        {
            //curTVComPort = setupRadio->currentRadio.locTVSwComport;
            logMessage(QString("Local Transvert Switch Comport opened Ok = %1").arg(currentRadio.locTVSwComport));
        }
        else
        {

            QString errMsg = serialTVSw->error();
            logMessage(QString("Local Transvert Switch Comport failed to open = %1 Error = %2").arg(currentRadio.locTVSwComport, errMsg));
        }

    }
    else
    {
        if (serialTVSw->getOpenFlag())
        {
            serialTVSw->closeComport();

        }

    }


    setTransVertDisplayVisible(currentRadio.transVertEnable);
    sendTransVertEnabled(currentRadio.transVertEnable);   // send to logger
    sendTransVertSwitchToLogger(TRANSSW_NUM_DEFAULT);                                 // turn off transVerter Sw
    sendTransVertSwitchToComPort(TRANSSW_NUM_DEFAULT);
    transVertSwNum = TRANSSW_NUM_DEFAULT;
    selTransVertBandIndicator = "";     // force active tranvert indicator update

}


void RigControlMainWindow::checkSupportVolume()
{
    // does the library support control of volume control

    if (selectedRadioSupportCap.getSupportVolume() && currentRadio.enableDisableCatFeature.volumeEnable)
    {
        logMessage(QString("Update Radio: Radio Supports Volume Control %1").arg(selectedRadioSupportCap.getSupportVolume() ? "True" : "False"));
        addVolStatusToRigCache(selectedRadioSupportCap.getSupportVolume());
    }
}


void RigControlMainWindow::checkSupportSMeter()
{
    // does the library support signal strength meter

    if (selectedRadioSupportCap.getSupportSMeter() && currentRadio.enableDisableCatFeature.sMeterEnable)
    {
       setSmeterVisible(selectedRadioSupportCap.getSupportSMeter());

    }

}

void RigControlMainWindow::checkSupportRit()
{
    getRitSupportStatus();

    rigStateDetails->ritEnable = currentRadio.enableDisableCatFeature.ritEnable;
    if (rigStateDetails->ritEnable && (selectedRadioSupportCap.getSupportSetRit() || selectedRadioSupportCap.getSupportGetRit()))
    {


        setRitFreqDisplayVisible(true);

        if (testMode)
        {

            setRitTestControlsVisible(true);
        }

        setRitGetSetFreqIndicatorVisible(true);
        ritSetFreqIndicatorToggle(selectedRadioSupportCap.getSupportSetRit());
        ritGetFreqIndicatorToggle(selectedRadioSupportCap.getSupportGetRit());


        //setRitFreqStr("0");             // turn off RIT
    }
    else
    {

        ui->ritGroupBox->setVisible(false);
        setRitFreqDisplayVisible(false);
        setRitGetSetFreqIndicatorVisible(false);
        clearSupportRitFlags();

        if (testMode)
        {

            setRitTestControlsVisible(false);
        }


    }


}

bool RigControlMainWindow::checkSupportVoiceMemory()
{


    if (selectedRadioSupportCap.getSupportVoiceMemory() && currentRadio.enableDisableCatFeature.voiceMemEnable)
    {

        if (radio)
        {


            if (selectedRadioSupportCap.getSupportVoiceMemory())
            {
                setVoiceMemIndVisible(true);
                setVoiceMemIndOnOff(true);
                addVoiceMemStatusToRigCache(true);
                int voiceMemNum = 0;
                // they all seem to start at 1
                if (selectedRadioSupportCap.getStartVoiceMemoryNumber() == 1)
                {
                    voiceMemNum = selectedRadioSupportCap.getEndVoiceMemoryNumber();
                    addVoiceNumberMessagesToRigCache(voiceMemNum);
                }

                bool supportStopCmd = true;  // hamlib Yaesu doesn't support stop function, use msg no. 0 to stop.
                //if (currentRadio.rigMfg_Name == "Yaesu")
                //{
                //    supportStopCmd = false;
                //}

                addVoiceKeyerSupportStopCmdToRigCache(supportStopCmd);

                if (testMode)
                {
                    setVoiceMemTestControlsVisible(true);
                    ui->voiceMessageSpinBox->setMaximum(voiceMemNum);
                }

                return true;

            }
            else
            {
                if (testMode)
                {
                    setVoiceMemTestControlsVisible(false);
                }
            }

        }
    }


    setVoiceMemIndVisible(false);
    setVoiceMemIndOnOff(false);
    addVoiceMemStatusToRigCache(false);
    return false;

}

bool RigControlMainWindow::checkSupportCwKeyerMemory()
{

    if (selectedRadioSupportCap.getSupportCwMemory() && currentRadio.enableDisableCatFeature.cWMemEnable)
    {

        if (radio)
        {


            setCwMemIndVisible(true);
            setCwMemIndOnOff(true);
            addRigModelToRigCache(currentRadio.rigModel);
            if (currentRadio.rigMfg_Name == "Yaesu")
            {
                trace("Send to logger Rig CW keyer type is Yaesu");
                addCwKeyerTypeToRigCache(hamlibData::CW_MEMORY_TYPES::YAESU);
            }
            else if (currentRadio.rigMfg_Name == "Kenwood")
            {
                trace("Send to logger Rig CW keyer type is Kenwood");
                addCwKeyerTypeToRigCache(hamlibData::CW_MEMORY_TYPES::KENWOOD);
            }
            else if (currentRadio.rigMfg_Name == "Icom")
            {
                trace("Send to logger Rig CW keyer type is Icom");
                addCwKeyerTypeToRigCache(hamlibData::CW_MEMORY_TYPES::ICOM);
            }
            else if (currentRadio.rigMfg_Name == "Elecraft")
            {
                trace("Send to logger Rig CW keyer type is Elecraft");
                addCwKeyerTypeToRigCache(hamlibData::CW_MEMORY_TYPES::ELECRAFT);
            }
            else if (currentRadio.rigMfg_Name == "Flex-radio")
            {
                trace("Send to logger Rig CW keyer type is Flex-Radio");
                addCwKeyerTypeToRigCache(hamlibData::CW_MEMORY_TYPES::FLEX_RADIO);
            }
            else if (currentRadio.rigMfg_Name == "OpenHPSDR")
            {
                trace("Send to logger Rig CW keyer type is OpenHPSDR");
                addCwKeyerTypeToRigCache(hamlibData::CW_MEMORY_TYPES::OPENHPSDR);
            }
            else if (currentRadio.rigMfg_Name == "Flex-radio/Apache")
            {
                trace("Send to logger Rig CW keyer type is Flex-radio/Apache");
                addCwKeyerTypeToRigCache(hamlibData::CW_MEMORY_TYPES::FLEX_RADIO_APACHE);
            }
            else if (currentRadio.rigMfg_Name == "QRPLabs")
            {
                trace("Send to logger Rig CW keyer type is QRPLabs");
                addCwKeyerTypeToRigCache(hamlibData::CW_MEMORY_TYPES::QRPLABS);
            }
            else if (currentRadio.rigMfg_Name == "Thetis")
            {
                trace("Send to logger Rig CW keyer type is Thetis");
                addCwKeyerTypeToRigCache(hamlibData::CW_MEMORY_TYPES::THETIS);
            }
            else
            {
                // not supported
                setCwMemIndVisible(false);
                setCwMemIndOnOff(false);
                addCwKeyerTypeToRigCache(hamlibData::CW_MEMORY_TYPES::NONE);
                addRigModelToRigCache("");
                if (testMode)
                {
                    setCwMemTestControlsVisible(false);
                }

                return false;
            }

            //bool supportStopCmd = true;
            //if (!selectedRadioSupportCap.getSupportCwMemoryStop())
            //{
            //    supportStopCmd = false;
            //}

            if (testMode)
            {
                setCwMemTestControlsVisible(true);
            }


            addCwKeyerSupportStopCmdToRigCache(selectedRadioSupportCap.getSupportCwMemoryStop());


            return true;



         }
    }


    setCwMemIndVisible(false);
    setCwMemIndOnOff(false);
    addCwKeyerTypeToRigCache(hamlibData::CW_MEMORY_TYPES::NONE);
    return false;

}




void RigControlMainWindow::checkSupportPtt()
{

    if (radio)
    {
        if (selectedRadioSupportCap.getSupportPttPortType() == RigCapConstants::RigPttPortType::RIG_PTT_NONE
                || selectedRadioSupportCap.getSupportPttPortType() == RigCapConstants::RigPttPortType::RIG_PTT_RIG
                || selectedRadioSupportCap.getSupportPttPortType() == RigCapConstants::RigPttPortType::RIG_PTT_RIG_MICDATA
            )
        {
            if (selectedRadioSupportCap.getRigManufacturer() == OMINRIG_MFR_NAME )
            {
                ui->supportedPttLbl->setText("Cat");
            }
            else
            {
                if (selectedRadioSupportCap.getSupportPttPortType() == RigCapConstants::RigPttPortType::RIG_PTT_NONE)
                {
                    ui->supportedPttLbl->setText("Serial");
                }
                else
                {
                    ui->supportedPttLbl->setText("CAT/Serial");
                }
            }



            setPttGroupItemsVisible(true);


            if (currentRadio.enablePTT)
            {
              setPttIndOnOff(true);

                serialCommonData::MINOS_PTT_TYPES pttType = currentRadio.pttType;

              if (pttType == serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_CAT)
              {
                  ui->pttLbl->setText("CAT");
              }
              else if (pttType == serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_RTS)
              {
                  ui->pttLbl->setText("RTS");
              }
              else if (pttType == serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_DTR)
              {
                 ui->pttLbl->setText("DTR");
              }
              else if (pttType == serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE)
              {
                 ui->pttLbl->setText("None");
              }

              addPTTEnabledStatusToRigCache(true);
              sendPttTypeLogger();
              ui->txPttTestPb->setVisible(true);

              if (testMode)
              {
                  setPttTestControlsVisible(true);
              }
            }
            else
            {
                setPttIndOnOff(false);
                ui->pttLbl->setText("");
                addPTTEnabledStatusToRigCache(false);
                sendPttTypeLogger();
                ui->txPttTestPb->setVisible(false);

                if (testMode)
                {
                    setPttTestControlsVisible(false);
                }

            }

            setTxRxIndOnOff(false);

        }
        else
        {
            ui->pttLbl->setText("None");
            setPttGroupItemsVisible(false);
            setPttIndOnOff(false);
            addPTTEnabledStatusToRigCache(false);
            sendPttTypeLogger();

         }
    }


}



void RigControlMainWindow::checkSupportPollRadio()
{
    if (selectedRadioSupportCap.getPollData())
    {
        if (radio != nullptr)
        {
            if (radio->getRigConnected())
            {
                if (currentRadio.pollInterval == "0.5")
                {
                    rigStateDetails->pollTime = 500;
                }
                else
                {
                    rigStateDetails->pollTime = 1000 * currentRadio.pollInterval.toInt();
                }

                pollTimer->start(rigStateDetails->pollTime);             // start timer to send poll radio

            }
        }
    }
    else
    {
        // not polling get initial values
        trace(QString("Not polling, set connections for future updates"));
        // connect signals for future value updates and errors
        connect(radio, &RigBase::newRxFreq, this, &RigControlMainWindow::onNewRxFreq, Qt::QueuedConnection); // QueuedConnection, ensure return to rigcontroller caller when not polling - eg Omnirig
        connect(radio, &RigBase::newVfo, this, &RigControlMainWindow::onNewVfo, Qt::QueuedConnection);
        connect(radio, &RigBase::newMode, this, &RigControlMainWindow::onNewMode, Qt::QueuedConnection);
        connect(radio, &RigBase::rigStatus, this, &RigControlMainWindow::onRigStatus, Qt::QueuedConnection);
        connect(radio, &RigBase::ritOn, this, &RigControlMainWindow::onRitOn, Qt::QueuedConnection);
        connect(radio, &RigBase::ritOff, this, &RigControlMainWindow::onRitOff, Qt::QueuedConnection);
        connect(radio, &RigBase::ritOffset, this, &RigControlMainWindow::onRitOffset, Qt::QueuedConnection);
        connect(radio, &RigBase::rit0, this, &RigControlMainWindow::onRit0, Qt::QueuedConnection);
        connect(radio, &RigBase::pttState, this, &RigControlMainWindow::onPttState, Qt::QueuedConnection);
    }
}


void RigControlMainWindow::updateCurrentRadioFromAvailRadios(QString radioName)
{
    getRadioConfigData(&currentRadio, radioName);
    // retrieve this data from the factory as we don't save it
    RigCapabilities rigCap = rigFactory->supported_rigs()->value(currentRadio.rigModel);
    currentRadio.rigMfg_Name = rigCap.getRigManufacturer();
    currentRadio.rigModelName = rigCap.getRigModelName();
    currentRadio.rigModelNumber = rigCap.getRigModelNumber();
}



void RigControlMainWindow::refreshRadio()
{
    if (radioCommsOK)
        {
            if (loggerRequests->selRadioFreq.isClear())
            {
                if (loggerRequests->selBand != rigStateDetails->selTvBand && currentRadio.transVertEnable && currentRadio.transVertSettings.count() != 0)
                {
                    rigStateDetails->selTransverterNum = NO_TRANSVERTER_NUM;
                    if (findTransverter(rigStateDetails->selTvBand, loggerRequests->selBand))
                    {
                        getAndSendTransVertSwNum(rigStateDetails->selTvBand);
                        logMessage(QString("Refresh radio: Transvert Enabled select Transverter for %1").arg(loggerRequests->selBand));

                    }
                }
            }
            else
            {
                loggerSetFreq(loggerRequests->selRadioFreq);
            }
            logMessage(QString("Refresh Radio: Logger Set Mode to %1").arg(loggerRequests->selRadioMode));
            loggerSetMode(loggerRequests->selRadioMode);

            //writeWindowTitle(appName);
            sendStatusToLogConnected();
            dumpRadioToTraceLog();
            msg->rigCache.publish();
            sendRadioSwitchCompleteToLogger();
            msg->rigCache.publish();
        }
        else
        {
            upDateRadio(currentRadioName);
        }

}

void RigControlMainWindow::getFreqFromRadio(int &retCode)
{
    retCode = radio->getFrequency(VFO::CURRENT_VFO, rigStateDetails->rfrequency);
    if (rigStateDetails->RTTYModeFlag)
    {
        rigStateDetails->rfrequency =
            rigStateDetails->rfrequency - Frequency(currentRadio.rttyOffset);
    }
    else if (rigStateDetails->PSKModeFlag)
    {
        rigStateDetails->rfrequency =
            rigStateDetails->rfrequency + Frequency(currentRadio.pskOffset);
    }
}
int RigControlMainWindow::openRigCtldRadio(bool localRigCtld) {
    int retCode = 0;
    radioCommsOK = false;

    if (localRigCtld) {

        trace(QString("Starting Local rigctld"));
        // check rigctld file exists
        QString filename = getRigCtldExePath() + getRigCtldExeName();

        if (!FileExecutable(filename)) {
            trace(QString("openRigCtld: rigctld %1 is not executable").arg(filename));
            return RIGCTLD_EXE_MISSING;
        }

        trace(QString("openRigCtld: found rigctld = %1").arg(filename));

        if (rigCtldProcess->state() == QProcess::Running) {
            trace(QString("openRigCtldRadio: rigctld running - killing"));
            if (!rigCtldKill()) {
                trace(QString("openRigCtldRadio: rigctld did not stop"));
                return RIGCTLD_FAILED_TO_STOP;
            }
        }

        rigCtldTrace::rigCtldTraceCodes traceCode =
            rigCtldTrace::rigCtldTraceCodes::rctNONE;
        if (ui->traceDataComms->isChecked()) {
            traceCode = rigCtldTrace::rigCtldTraceCodes::rctVERBOSE;
        }

        QString parity;
        QString handshake;
        QString rtsState;

        parity =
            serialData::rigctldParityStr[static_cast<int>(currentRadio.parity)];

        handshake = serialData::rigctldHandshakeStr[static_cast<int>(
            currentRadio.handshake)];

        if (handshake == serialData::rigctldHandshakeStr[RIG_HANDSHAKE_HARDWARE]) {
            rtsState = serialData::rigctldForceLinesStr[serialData::FORCE_LINE_NONE];
        } else {
            rtsState = serialData::rigctldForceLinesStr[static_cast<int>(
                currentRadio.forceRts)];
        }

        QString dtrState = serialData::rigctldForceLinesStr[static_cast<int>(
            currentRadio.forceDtr)];

        RigCtldParameters rigCtldPar;

        rigCtldPar.setManufacturer(currentRadio.rigMfg_Name);
        rigCtldPar.setModelNumber(QString::number(currentRadio.rigModelNumber));
        rigCtldPar.setComport(currentRadio.comport);
        rigCtldPar.setBaudRate(QString::number(currentRadio.baudrate));
        rigCtldPar.setDataBits(QString::number(currentRadio.databits));
        rigCtldPar.setCiv(currentRadio.civAddress.trimmed().trimmed());
        rigCtldPar.setNetworkAddress(currentRadio.rigCtldNetworkAdd.trimmed());
        rigCtldPar.setPortNum(currentRadio.rigCtldNetworkPort.trimmed());
        rigCtldPar.setStopBits(QString::number(currentRadio.stopbits));
        rigCtldPar.setParity(parity);
        rigCtldPar.setHandshake(handshake);
        rigCtldPar.setRtsState(rtsState);
        rigCtldPar.setDtrState(dtrState);
        rigCtldPar.setTraceCode(traceCode);
        rigCtldPar.setPttEnabled(currentRadio.enablePTT);
        rigCtldPar.setPttComport(currentRadio.pttSerialPort);
        rigCtldPar.setPttType(currentRadio.pttType);

        // start rigctld
        trace(QString("openRigCtldRadio: starting rigctld"));
        trace(QString("rigctld parameters - %1, %2, %3, %4, %5, %6, %7, %8, %9, "
                      "%10, %11, %12, %13, %14, %15, %16, %17")
                  .arg(rigCtldPar.getManufacturer())
                  .arg(rigCtldPar.getModelNumber())
                  .arg(rigCtldPar.getComport())
                  .arg(rigCtldPar.getBaudRate())
                  .arg(rigCtldPar.getBaudRate())
                  .arg(rigCtldPar.getCiv())
                  .arg(rigCtldPar.getNetworkAddress())
                  .arg(rigCtldPar.getPortNum())
                  .arg(rigCtldPar.getStopBits())
                  .arg(rigCtldPar.getParity())
                  .arg(rigCtldPar.getHandshake())
                  .arg(rigCtldPar.getRtsState())
                  .arg(rigCtldPar.getDtrState())
                  .arg(rigCtldPar.getTraceCode())
                  .arg(rigCtldPar.getPttEnabled() ? "PTT Enabled" : "PTT Disabled")
                  .arg(rigCtldPar.getPttComport())
                  .arg(serialCommonData::pttTypeStr[static_cast<int>(
                      rigCtldPar.getPttType())]));

        runRigCtlDaemon(rigCtldPar);

        // wait for rigctld to start
        int waitStartDur = 500;
        while (rigCtldProcess->state() != QProcess::Running && waitStartDur > 0) {
            sleepFor(100);
            waitStartDur--;
        }

        if (waitStartDur > 0) {
            trace(QString("openRigCtldRadio: rigctld running for radio %1")
                      .arg(currentRadio.rigModel));
        } else {
            trace(QString("openRigCtldRadio: rigctld failed for radio %1")
                      .arg(currentRadio.rigModel));
            return RIGCTLD_FAILED;
        }

        if (rigCtldDetails->rigCtldConnectDelay != 0) {
            trace(
                QString(
                    "openRigCtldRadio: Delay = %1 secs before connecting to rigCtld")
                    .arg(rigCtldDetails->rigCtldConnectDelay));
            delay(rigCtldDetails->rigCtldConnectDelay);
        }
    }

    if (!localRigCtld) {
        trace(QString("using external rigctld - now try to connect"));
    }

    // now open radio using rigctld model

    radio = rigFactory->createRigs(HamlibRigCtld);

    if (radio == nullptr) {
        logMessage(QString("Error Creating a rig in the factory - rigctld"));
        QMessageBox::critical(this, tr("RigControl Open Radio Error"),
                              tr("Failed to create a radio"));
        return OPEN_FAILED;
    }

    trace(QString("openRigCtldRadio: Open radio = %1, via Rigctld")
              .arg(currentRadio.rigModel));
    retCode = radio->rigInit(currentRadio, RIGCTLD_ON);
    if (retCode < 0) {
        radio->closeRig();
        logMessage(
            QString("openRigCtldRadio: Error Opening Radio %1, Error Code = %2")
                .arg(currentRadio.rigModel, QString::number(retCode)));
        radioError(retCode, tr("RigCtld Open Radio"));
        return OPEN_FAILED;
    }

    logMessage(QString("Open Radio is connected = %1")
                   .arg(radio->getRigConnected() ? "yes" : "no"));
    // let's see if we can get freq from radio and confirm comms
    if (radio->getRigConnected()) {

        int retCode = Rig_OK;
        showStatusMessage(
            tr("Attempting to communicate with radio via rigctld - %1")
                .arg(currentRadio.rigModel));
        getFreqFromRadio(retCode);
        if (retCode < Rig_OK) {
            logMessage(QString("openRigctldRadio: Test Communication - Get Freq "
                               "error, code = %1")
                           .arg(QString::number(retCode)));
            radioError(retCode,
                       tr("Test Radio Connection via rigctld\n\nMinos tried to read "
                          "the radio frequency,\nbut nothing was received from the "
                          "radio.\n\nPlease check connections and/or settings.\nSome "
                          "radios/interfaces may require Force DTR or Force RTS to "
                          "be set High, to power the interface."));
            // sendStatusToLogDisConnected();
            return OPEN_FAILED;
        } else {
            radioCommsOK = true;
        }
    }

    if (radioCommsOK) {
        logMessage(QString("openRigctldRadio: Radio Opened %1")
                       .arg(currentRadio.rigModel));
        showStatusMessage(
            tr("Radio Opened rigctld: %1").arg(currentRadio.rigModel));

        if (currentRadio.catPortType == RigCapConstants::PortType::serial) {

            showStatusMessage(
                QString("Connected via RigCtld: %1 - %2, %3, %4, %5, %6, %7, "
                        "Handshake %8, ForceDTR %9, ForceRTS %10")
                    .arg(currentRadio.rigMfg_Name)
                    .arg(currentRadio.rigModelName)
                    .trimmed()
                    .arg(currentRadio.comport)
                    .arg(currentRadio.baudrate)
                    .arg(currentRadio.databits)
                    .arg(currentRadio.stopbits)
                    .arg(serialCommonData::parityStr[static_cast<int>(
                        currentRadio.parity)])
                    .arg(serialCommonData::handshakeStr[static_cast<int>(
                        currentRadio.handshake)])
                    .arg(serialCommonData::forceLinesStr[static_cast<int>(
                        currentRadio.forceDtr)])
                    .arg(serialCommonData::forceLinesStr[static_cast<int>(
                        currentRadio.forceRts)]));
        }

        else if (currentRadio.catPortType == RigCapConstants::PortType::none) {
            showStatusMessage(
                tr("Connected via rigctld: %1 - %2")
                    .arg(currentRadio.rigMfg_Name, currentRadio.rigModelName));
        }

    } else {

        logMessage(QString("Radio Open Error"));
        showStatusMessage(tr("Radio Open error"));
        return OPEN_FAILED;
    }

    PubSubName psname(currentRadio.radioName);

    msg->rigCache.publish();
    return OPEN_OK;
}

int RigControlMainWindow::openRadio()
{

    int retCode = 0;
    radioCommsOK = false;

    if (currentRadio.radioName == "")
    {
        logMessage(QString("Open Radio: No radio name!"));
        showStatusMessage(tr("Please select a Radio"));
        return OPEN_FAILED;
    }

    logMessage(QString("Open Radio: Opening Radio %1 PortType %2").arg(currentRadio.radioName, hamlibData::portTypeList[currentRadio.catPortType]));
    showStatusMessage(tr("Opening Radio: %1").arg(currentRadio.radioName));

    if (currentRadio.catPortType == RigCapConstants::PortType::serial)
    {
        logMessage(QString("Using COM port %1").arg(currentRadio.comport));

        if (currentRadio.comport == "")
        {
            logMessage(QString("Open Radio: No comport"));
            showStatusMessage(tr("Please select a Comport"));
            return OPEN_FAILED;
        }
        else if(!isComportAvail(currentRadio.comport))
        {
            logMessage(QString("Open Radio: Check comport - defined port %1 not available on computer").arg(currentRadio.comport));
            showStatusMessage(tr("Comport %1 no longer configured on computer?").arg(currentRadio.comport));
            return OPEN_FAILED;
        }
    }

    else if (currentRadio.catPortType == RigCapConstants::PortType::network)
    {
        logMessage(QString("Using network address <%1> port <%2>").arg(currentRadio.networkAdd, currentRadio.networkPort));

        if (currentRadio.networkAdd == "" || (currentRadio.networkPort == ""))
        {
            logMessage(QString("Open Radio: No network or Port Number"));
            showStatusMessage(tr("Please enter a network Address and Port Number"));
            return OPEN_FAILED;
        }

    }
    if (currentRadio.rigModel == "")
    {
        logMessage(QString("Open Radio: No radio model"));
        showStatusMessage(tr("Please select a radio model"));
        return OPEN_FAILED;
    }

    radio = rigFactory->createRigs(rigFactory->supported_rigs()->value(currentRadio.rigModel).getRigModelNumber());

    if (radio == nullptr)
    {
        logMessage(QString("Error Creating a rig in the factory"));
        QMessageBox::critical(this, tr("RigControl Open Radio Error"), tr("Failed to create a radio"));
        return OPEN_FAILED;
    }
    else
    {
        logMessage(QString("Rig Created in the factory Ok"));

    }

    radio->setTraceComms(rigStateDetails->traceCommsFlag);
    rigStateDetails->rigCap = rigFactory->supported_rigs()->value(currentRadio.rigModel);

    if (rigStateDetails->rigCap.getRigManufacturer() != OMINRIG_MFR_NAME)
    {
        logMessage(QString("Hamlib Library Version = %1").arg(rigStateDetails->rigCap.getDetailedLibraryVersion()));     // show library version in trace log before connection.
    }




    // set state of trace hamlib comms

    // Message from rigcontrol
    //connect(radio, &RigBase:(debug_protocol, this, &RigControlMainWindow::logMessage);

    logMessage(QString("Radio Connected? %1").arg(radio->getRigConnected() ? "yes" : "no"));



    if (!radio->getRigConnected())
    {

        // if radio is already open, don't reinit it
        logMessage(QString("Running rigInit"));
        retCode = radio->rigInit(currentRadio, RIGCTLD_OFF);
        logMessage(QString("RigInit Error Code = %1").arg(retCode));
        if (retCode < 0)
        {
            radio->closeRig();
            logMessage(QString("Error Opening Radio Error Code = %1").arg(QString::number(retCode)));
            radioError(retCode, tr("Open Radio"));
            return OPEN_FAILED;
        }

        if (rigStateDetails->rigCap.getRigManufacturer() == OMINRIG_MFR_NAME)
        {
            logMessage(QString("Library Version = %1").arg(rigStateDetails->rigCap.getLibraryVersion()));    // Omnirig COM needs to exist to do this..
        }
    }


    logMessage(QString("Connect Status after init = %1").arg(radio->getRigConnected() ? "yes" : "no"));



    // reduce the radio error timeouts from default
    // timeouts in hamlib seem to be increasing, maybe need to reduce this for other radios?
    if (rigStateDetails->rigCap.getRigModelNumber() == RIG_MODEL_FT817 || rigStateDetails->rigCap.getRigModelNumber() == RIG_MODEL_FT818)
    {

        QString fileName = RADIO_PATH_LOGGER() + MINOS_RADIO_CONFIG_FILE;
        QSettings  settings(fileName, QSettings::IniFormat);
        settings.beginGroup(currentRadio.radioName);
        QString retryValue = settings.value("retry", DEFAULT_FT817_RADIO_RETRY).toString();

        // set the timeouts for these radios
        retCode = radio->setConfigurationParameter(HAMLIB_RETRY, retryValue);
        if (retCode == RIG_OK)
        {
            logMessage(QString("set hamlib config param - retry set ok, value = %1").arg(retryValue));
        }
        else
        {
            logMessage(QString("set hamlib config param - retry failed, error code = %1").arg(retCode));

        }

        QString timeOutValue = settings.value("retry", DEFAULT_FT817_RADIO_TIMEOUT).toString();
        settings.endGroup();
        retCode = radio->setConfigurationParameter(HAMLIB_TIMEOUT, timeOutValue);
        if(retCode == RIG_OK)
        {
            logMessage(QString("set hamlib config param - timeout set ok, value = %1").arg(timeOutValue));
        }
        else
        {
            logMessage(QString("set hamlib config param = timeout failed, error code = %1").arg(retCode));
        }
    }





    // let's see if we can get freq from radio and confirm comms
    if (radio->getRigConnected())
    {

        int retCode = Rig_OK;
        showStatusMessage(tr("Attempting to communicate with radio - %1").arg(currentRadio.radioName));
        //delay(1);
        getFreqFromRadio(retCode);

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
        logMessage(QString("Open Radio: Radio Opened %1").arg(currentRadio.radioName));
        showStatusMessage(tr("Radio Opened: %1").arg(currentRadio.radioName));

        if (currentRadio.catPortType == RigCapConstants::PortType::serial)
        {
            showStatusMessage(tr("Connected: %1 - %2, %3, %4, %5, %6, %7, Handshake %8, ForceDTR %9, ForceRTS %10")
                              .arg(currentRadio.radioName).arg(currentRadio.rigModelName).trimmed().arg(currentRadio.comport).arg(currentRadio.baudrate).arg(currentRadio.databits)
                                  .arg(currentRadio.stopbits).arg(serialCommonData::parityStr[static_cast<int>(currentRadio.parity)]).arg(serialCommonData::handshakeStr[static_cast<int>(currentRadio.handshake)]).arg(serialCommonData::forceLinesStr[static_cast<int>(currentRadio.forceDtr)]).arg(serialCommonData::forceLinesStr[static_cast<int>(currentRadio.forceRts)]));
        }
        else if (currentRadio.catPortType == RigCapConstants::PortType::network)
        {
            if (currentRadio.rigModelNumber == hamlibData::RIGCTL)
            {
                showStatusMessage(tr("Connected: %1 - %2, %3:%4 - %5 %6").arg(currentRadio.radioName).arg(currentRadio.rigModelName).arg(currentRadio.networkAdd).arg(currentRadio.networkPort).arg(rigCtldDetails->rigctld_radioMfg).arg(rigCtldDetails->rigctld_radioName));
            }
            else
            {
                showStatusMessage(tr("Connected: %1 - %2, %3:%4").arg(currentRadio.radioName).arg(currentRadio.rigModelName).arg(currentRadio.networkAdd).arg(currentRadio.networkPort));
            }

        }
        else if (currentRadio.catPortType == RigCapConstants::PortType::none)
        {
            showStatusMessage(tr("Connected: %1 - %2").arg(currentRadio.radioName, currentRadio.rigModelName));
        }



    }
    else
    {

        logMessage(QString("Radio Open Error"));
        showStatusMessage(tr("Radio Open error"));
        return OPEN_FAILED;
    }

    PubSubName psname(currentRadio.radioName);

    msg->rigCache.publish();
    return OPEN_OK;
}

void RigControlMainWindow::closeRadio()
{

    if (testMode)
    {
       setTestControlsVisible(false);
    }


    if (currentRadio.enablePTT && rigStateDetails->curPttStatus)
    {
        if (radio && radioCommsOK)
        {
           onSetPttOnOff(false);     // turn off PTT in case it is on
           setTestPttButtonIndOnOff(false);
        }

    }


    if (rigFactory->supported_rigs()->value(currentRadio.rigModel).getPollData())
    {
        pollTimer->stop();

    }
    else
    {
        if (radio != nullptr)
        {
            disconnect(radio, &RigBase::newRxFreq, this, &RigControlMainWindow::onNewRxFreq);
            disconnect(radio, &RigBase::newVfo, this, &RigControlMainWindow::onNewVfo);
            disconnect(radio, &RigBase::newMode, this, &RigControlMainWindow::onNewMode);
            disconnect(radio, &RigBase::rigStatus, this, &RigControlMainWindow::onRigStatus);

            disconnect(radio, &RigBase::ritOn, this, &RigControlMainWindow::onRitOn);
            disconnect(radio, &RigBase::ritOff, this, &RigControlMainWindow::onRitOff);
            disconnect(radio, &RigBase::ritOffset, this, &RigControlMainWindow::onRitOffset);
            disconnect(radio, &RigBase::rit0, this, &RigControlMainWindow::onRit0);
            disconnect(radio, &RigBase::pttState, this, &RigControlMainWindow::onPttState);


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



    if (currentRadio.rigCtldEnable && currentRadio.startMinosRigCtld)
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

        delete radio;
        radio = nullptr;
    }



    showStatusMessage(tr("Disconnected"));
    sendStatusToLogDisConnected();
    displayFreqVfo(Frequency());
    displayTransVertVfo(Frequency());
    ui->transVertBandDisp->setText("");
    ui->transVertSwNum->setText("");
    turnOffAllsupRadioIndicators();
    displaySignalStrength(-54);
    setRigCltdIndicatorVisible(false);
    rigStateDetails->rigCap = RigCapabilities();
    logMessage(QString("Radio Closed"));

    msg->rigCache.publish();
}






void RigControlMainWindow::writeWindowTitle(QString appName)
{
    if (appName.length() > 0)
    {
        setWindowTitle(tr("Minos Rig Control - Logger - %1").arg(appName));
    }
    else
    {
        setWindowTitle(tr("Minos Rig Control - Local"));
    }

}


void RigControlMainWindow::setPolltime(int interval)
{
    rigStateDetails->pollTime = interval;
}

int RigControlMainWindow::getPolltime()
{

    return rigStateDetails->pollTime;
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



void RigControlMainWindow::getRadioInfo(bool pubNow)
{
    if (closeApp)
        return;

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
        /*retCode = */getAndSendVfo();
    }

    if (radioCommsOK && currentRadio.enablePTT)
    {
        if (currentRadio.pttType == serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_CAT)
        {
            logMessage(QString("Get CAT PTT Status"));

            retCode = getTXStatus(rigStateDetails->curVfo);
            if (retCode < 0)
            {
                // error
                logMessage(QString("Get radioInfo: Get CAT TXStatus error").arg(QString::number(retCode)));
                radioError(retCode, "Request TX Status");
            }
        }
        else if (currentRadio.pttType == serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_DTR ||  currentRadio.pttType == serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_RTS)
        {
            if (!currentRadio.pttSerialPort.isEmpty())
            {
                logMessage(QString("Get Serial PTT Status for Comport %1 and Control Line Type %2").arg(currentRadio.pttSerialPort, serialCommonData::pttTypeStr[static_cast<int>(currentRadio.pttType)]));

                retCode = getTXStatus(rigStateDetails->curVfo);
                if (retCode < 0)
                {
                    // error
                    logMessage(QString("Get radioInfo: Get RTS/DTR TXStatus error").arg(QString::number(retCode)));
                    radioError(retCode, "Request TX Status");
                }
            }
            else
            {
                // no serial comport defined for PTT
                logMessage(QString("Get radioInfo: No comport defined for Serial PTT control"));

            }
        }



    }


    if (radioCommsOK)
    {
        logMessage(QString("Get radio frequency"));

        retCode = getAndSendFrequency(rigStateDetails->curVfo);
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
        retCode = getAndSendMode(rigStateDetails->curVfo);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Mode error %1").arg(QString::number(retCode)));
            radioError(retCode, tr("Request Mode"));

        }
        else
        {
            logMessage(QString("Got Mode = %1").arg(rigcommon::convertModeToQString(rigStateDetails->rmode)));
        }
    }




    if (radioCommsOK && selectedRadioSupportCap.getSupportSetRit() && rigStateDetails->ritEnable)
    {
        logMessage((QString("Poll RIT Info - Get Rit Freq = %1").arg(selectedRadioSupportCap.getSupportGetRit() ? "True" : "False")));


        if (selectedRadioSupportCap.getSupportGetRit())
        {
            retCode = getRitFreq(rigStateDetails->curVfo);
            if (retCode < 0)
            {
                // error
                logMessage(QString("Get radioInfo: Get RIT Freq error").arg(QString::number(retCode)));
                radioError(retCode, tr("Request RIT Freq"));
            }
            else
            {
                logMessage(QString("Get radioInfo: Get RIT Freq = %1").arg(convertRitFreqToStr(rigStateDetails->rRitFreq, rigStateDetails->ritKHzFlag)));
            }

        }



        if (selectedRadioSupportCap.getSupportGetRit())
        {

            bool ritStatus = false;
            if (selectedRadioSupportCap.getSupportGetRitState())
            {
                logMessage(QString("Get Rit"));
                retCode = getRitRadioStatus(rigStateDetails->curVfo, &ritStatus);
                if (retCode < 0)
                {
                    //error
                    logMessage(QString("Get radioInfo: Get RIT state error").arg(QString::number(retCode)));
                    radioError(retCode, tr("Request RIT State"));
                }
                else
                {
                    if (ritStatus != rigStateDetails->radioRitOn)
                    {
                        rigStateDetails->radioRitOn = ritStatus;
                        logMessage(QString("Get radioInfo: Radio Rit Status = %1").arg(rigStateDetails->radioRitOn ? "On" : "Off"));
                        ritIndicatorToggle(rigStateDetails->radioRitOn);
                        sendRadioRitStatusLogger(rigStateDetails->radioRitOn);
                    }

                }


            }



        }


    }


    if (radioCommsOK && selectedRadioSupportCap.getSupportVolume() && currentRadio.enableDisableCatFeature.volumeEnable)
    {
        logMessage(QString("Get Radio Volume"));
        retCode = getVolume(rigStateDetails->curVfo);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Volume error %1").arg(QString::number(retCode)));
            radioError(retCode, tr("Request Volume"));
        }

    }


    if (radioCommsOK && selectedRadioSupportCap.getSupportSMeter() && currentRadio.enableDisableCatFeature.sMeterEnable)
    {

        logMessage(QString("Get Signal Strength"));
        retCode = getSignalStrength(rigStateDetails->curVfo);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get signal strength error %1").arg(QString::number(retCode)));
            radioError(retCode, tr("Request Signal Strength"));
        }
   }




    // test CW Message
    if (radioCommsOK && isTestCwMessageRunning())
    {

    }


    if (pubNow)
    {
       msg->rigCache.publish();
    }

}


void RigControlMainWindow::onNewVfo(QString omniRigVfo)
{
    rigStateDetails->curVfo = strToVfo(omniRigVfo);
    ui->vfo_state_label->setText(omniRigVfo);

}



void RigControlMainWindow::onSelectRadio(PubSubName s, QString band, Frequency freq, QString mode)
{
    trace(QString("RigControlMainWindow::onSelectRadio closeApp is %1").arg(closeApp));
    if (closeApp)
        return;

    logMessage(QString("**** Received SelectRadio from Logger = %1, band = %2, freq = %3, mode = %4 ****").arg(s.toString(), band, freq.traceStr(), mode));


    loggerRequests->selRadioMode = mode;

    loggerRequests->selRadioFreq = freq;

    loggerRequests->selBand = band;

    QString oldRadio = currentRadioName;
    //currentRadioName = s.key();

    if (!s.isEmpty() && (s.key() == oldRadio))
    {
        trace(QString("Selected Radio - %1 is the same as previous radio - %2, refresh radio").arg(s.key(), oldRadio));
        refreshRadio();
    }
    else
    {
        trace(QString("Selected Radio - %1 is different to previous radio - %2, update radio").arg(s.key(), oldRadio));
        upDateRadio(s.key());
        msg->rigCache.invalidate();
    }

}


void RigControlMainWindow::loggerSetFreq(Frequency freq)
{
    if (closeApp)
        return;

    logMessage(QString("Received Freq from Logger = %1").arg(freq.traceStr()));
    if (radioCommsOK && !rigStateDetails->rigErrorFlag)
    {
        logMessage(QString("new freq %1, old freq %2").arg(freq.traceStr(), loggerRequests->logger_freq.traceStr()));


        loggerRequests->logger_freq = freq;

        if (!freq.isClear())
        {
           setFreq(freq, rigStateDetails->curVfo);
        }
        else
        {
            logMessage(QString("loggerSetFreq: freq empty ignore"));
        }


    }

}


void RigControlMainWindow::loggerSetBand(QString band)
{
    logMessage((QString("loggerSetBand: band requested = %1").arg(band)));

    Frequency f(0); // not used

    if (band != rigStateDetails->selTvBand && currentRadio.transVertEnable && currentRadio.transVertSettings.count() != 0)
    {

        logMessage(QString("loggerSetBand: Look for transverter for band %1").arg(band));
        selectTransverter(band, f);

    }


}



void RigControlMainWindow::setFreq(Frequency freq, VFO vfo)
{
    logMessage(QString("SetFreq: Change to Freq = %1").arg(freq.traceStr()));
    cmdLockOn();    // lock get radio info

    int retCode = 0;
    QString cb;
    //int tvNum = 0;
    //bool usingTransVert = false;


    if (freq.operator long long() == 0)
    {
        cb = rigStateDetails->curBand;
    }
    else
    {
        BandList &blist = BandList::getBandList();
        QSharedPointer<BandInfo>  bi;

        bool bandOK = blist.findBand(freq, bi);
        if (bandOK)
        {
            cb = bi->uk;
            logMessage(QString("SetFreq: Band found = %1").arg(cb));
        }
        else
        {
            logMessage((QString("SetFreq: No band found for this freq")));
            return;
        }
    }

    Frequency f(freq);

    if ((cb != rigStateDetails->selTvBand) && currentRadio.transVertEnable && (currentRadio.transVertSettings.count() != 0))
    {
        logMessage(QString("SetFreq: Look for transverter for band %1").arg(cb));
        selectTransverter(cb, f);
    }
    else if ((cb == rigStateDetails->selTvBand) && currentRadio.transVertEnable && (currentRadio.transVertSettings.count() != 0))
    {
        logMessage(QString("SetFeq: Transverter Selected for %1 band, calculate frequency").arg(rigStateDetails->selTvBand));
        // now calculate the freq
        f = f - currentRadio.transVertSettings.value(rigStateDetails->selTvBand)->transVertOffset;
        logMessage(QString("SetFreq: Transvert Enabled Freq = %1").arg(f.traceStr()));
    }

    if (radioCommsOK)
    {

        if (radio)
        {
            //xxxxxxxxxxxCorrect rigstate frequency to rig
            Frequency mf = f;
            if (rigStateDetails->RTTYModeFlag)
            {
                mf = f + Frequency(currentRadio.rttyOffset);
            }
            else if (rigStateDetails->PSKModeFlag)
            {
                mf = f - Frequency(currentRadio.pskOffset);
            }

           // rigStateDetails->rfrequency will be set when we poll the rig later
           retCode = radio->setFrequency(mf, vfo);
        }
        else
        {
            logMessage(QString("setFreq - radio = nullptr"));
            return;
        }

        if (retCode != Rig_OK)
        {
            if (retCode == -9)
            {
                logMessage(QString("SetFreq: Invalid Tx Freq for Radio, Freq = %1").arg(QString::number(static_cast<qint64>(f))));
                cmdLockOff();
                return;
            }

            logMessage(QString("SetFreq: Error Setting Freq Code = %1").arg(retCode));
            radioError(retCode, tr("SetFreq"));
        }
        else
        {
            logMessage(QString("SetFreq: Rig set to Freq = %1").arg(QString::number(static_cast<qint64>(f))));
        }

        if (!rigFactory->supported_rigs()->value(currentRadio.rigModel).getPollData()) // non polling radio
        {
            // when not polling radio for information
            trace(QString("SetFreq: Check Transvert Freq at startup"));
            cmdLockOff();
            getRadioInfo(DONT_PUBLISH_NOW);

         }

    }
    else
    {
        logMessage(QString("SetFreq: Radio is not connected"));
    }


    cmdLockOff();
}


bool RigControlMainWindow::selectTransverter(QString &band, Frequency &f)
{
    rigStateDetails->selTransverterNum = NO_TRANSVERTER_NUM;
    if (findTransverter(rigStateDetails->selTvBand, band))
    {
        logMessage(QString("SelectTransverter: transverter found for band %1").arg(band));
        getAndSendTransVertSwNum(rigStateDetails->selTvBand);
        // now calculate the freq
        f = f - currentRadio.transVertSettings.value(band)->transVertOffset;
        logMessage(QString("SelectTransverter: Transvert Enabled Freq = %1").arg(f.traceStr()));

        return true;
    }

    logMessage(QString("SelectTransverter: No transverter found for band %1").arg(band));

    return false;
}


bool RigControlMainWindow::findTransverter(QString &transVerterBand, QString band)
{
    //int tvNum = 0;

    // does a transverter support this band?
    logMessage((QString("FindTransverter: Looking for Transverters that support this band %1").arg(band)));

    bool b = false;

    QStringList tvList = currentRadio.transVertSettings.keys();
    for(const auto &tv: QASCONST(tvList))
    {
        if (currentRadio.transVertSettings.value(tv)->band == band)
        {
            b = true;
            //usingTransVert = true;
            logMessage(QString("FindTransverter: Found Transverter %1 for this band %2").arg(currentRadio.transVertSettings.value(tv)->band, band));
            break;
        }
    }

    if (b)  // found a tranverter supporting this band
    {
        transVerterBand = band;
        logMessage(QString("FindTransverter: Using transverter for band = %1").arg(transVerterBand));
        ui->transVertBandDisp->setText(transVerterBand);
        showActiveTransVertIndicator(transVerterBand);
        sendTransVertStatusToLog(true);

        if (currentRadio.enableTransSwitch)
        {
            if (currentRadio.transVertSettings.value(band)->transSwitchNum != transVertSwNum)
            {
                transVertSwNum = currentRadio.transVertSettings.value(band)->transSwitchNum;
                ui->transVertSwNum->setText(transVertSwNum);
                transVertSwNum = currentRadio.transVertSettings.value(band)->transSwitchNum;
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

        //transVerterNum = tvNum;
        return true;

    }
    else
    {
        // no transverter found for this band
        logMessage(QString("FindTransverter: No transverter found for this band %1").arg(band));
        clearTransVertSupport();
        sendTransVertStatusToLog(false);

        return false;

    }
}


void RigControlMainWindow::getAndSendTransVertSwNum(QString transvertName)
{
    if (currentRadio.enableTransSwitch)
    {
        if (currentRadio.transVertSettings.value(transvertName)->transSwitchNum != transVertSwNum)
        {
            transVertSwNum = currentRadio.transVertSettings.value(transvertName)->transSwitchNum;
            ui->transVertSwNum->setText(transVertSwNum);
            transVertSwNum = currentRadio.transVertSettings.value(transvertName)->transSwitchNum;
            sendTransVertSwitchToLogger(transVertSwNum);
            sendTransVertSwitchToComPort(transVertSwNum);
            logMessage(QString("getAndSendTransVertSwNum: Send TransVert Switch number - %1").arg(transVertSwNum));

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
        logMessage(QString("getAndSendTransVertSwNum: Transvert Switch not enabled - %1").arg(TRANSSW_NUM_DEFAULT));
    }


}


void RigControlMainWindow::clearTransVertSupport()
{
    rigStateDetails->selTvBand.clear();
    displayTransVertVfo(0.0);
    //setTransVertDisplayVisible(false);
    rigStateDetails->curTransVertFreq.clear();
    ui->transVertBandDisp->setText("");
    transVertSwNum = TRANSSW_NUM_DEFAULT;
    ui->transVertSwNum->setText(TRANSSW_NUM_DEFAULT);
    rigStateDetails->selTransVertBandIndicator.clear();
    sendTransVertSwitchToLogger(TRANSSW_NUM_DEFAULT);
    sendTransVertSwitchToComPort(TRANSSW_NUM_DEFAULT);
    updateSupportedRadioIndicators();
}


int RigControlMainWindow::getAndSendVfo()
{
    int retCode = 0;
    if (selectedRadioSupportCap.getSupportGetVfo())
    {
        retCode = radio->getVfo(&rigStateDetails->curVfo);
        trace(QString("Read VFO = %1").arg(vfoToStr(rigStateDetails->curVfo)));
    }
    else
    {
        rigStateDetails->curVfo = VFO::CURRENT_VFO;
        trace(QString("Radio doesn't support read VFO - set Current_Vfo"));
    }

    ui->vfo_state_label->setText(vfoToStr(rigStateDetails->curVfo));

    return retCode;

}

int RigControlMainWindow::getAndSendFrequency(VFO vfo)
{
    int retCode = getRxFreq(vfo);
    if ( retCode == Rig_OK)
    {
        processRxFrequencyForDisplay();
        return retCode;
    }
    else
    {
        logMessage(QString("Get radioInfo: Get Freq error, code = %1, vfo = %2").arg(QString::number(retCode), vfoToStr(rigStateDetails->curVfo)));
        radioError(retCode, tr("Request Frequency"));
    }
    return retCode;
}



int RigControlMainWindow::getRxFreq(VFO /*vfo*/)
{
    int retCode = 0;
    if (radio)
    {
        getFreqFromRadio(retCode);

        logMessage(QString("getRxFreq - freq = %1 mode = %2").arg(rigStateDetails->rfrequency.convertFreqStrDisp(), rigcommon::convertModeToQString(rigStateDetails->rmode)));
    }
    else
    {
        logMessage(QString("getRxFreq - radio = nullptr"));
    }
    return retCode;

}


void RigControlMainWindow::processRxFrequencyForDisplay()
{
    Frequency transVertF;
    //int retCode = 0;
    //int tvNum = 0;
    bool b = false;


    rigStateDetails->curVfoFreq = rigStateDetails->rfrequency;
    logMessage(QString("Get Freq: Read Freq from Radio = %1").arg(rigStateDetails->curVfoFreq.traceStr()));
    if (currentRadio.transVertEnable && currentRadio.transVertSettings.count() > 0)
    {
        if (rigStateDetails->selTvBand != "")
        {
            logMessage(QString("Get Freq: Transvert enabled"));
            // look for supporting transverter

            QStringList tvList = currentRadio.transVertSettings.keys();
            QString tvName;
            for(const auto &tv: QASCONST(tvList))
            {
                if (currentRadio.transVertSettings.value(tv)->band == rigStateDetails->selTvBand)
                {
                    b = true;
                    tvName = tv;
                    break;
                }
            }

            if (b)
            {
                logMessage(QString("Found transverter for band = %1").arg(b));
                logMessage(QString("Transverter Key %1 %2 offset %3 rfreq %4").arg(tvName
                           , currentRadio.transVertSettings.value(tvName)->transVertName
                           , currentRadio.transVertSettings.value(tvName)->transVertOffset.traceStr()
                           , rigStateDetails->rfrequency.traceStr())
                           );

                transVertF = rigStateDetails->rfrequency + currentRadio.transVertSettings.value(tvName)->transVertOffset;
                logMessage(QString("Get Freq: TransvertF = %1").arg(transVertF.traceStr()));
            }

            logMessage(QString("Get Freq: Transvert Freq. = %1").arg(transVertF.traceStr()));
            rigStateDetails->curTransVertFreq = transVertF;
            displayTransVertVfo(transVertF);

        }
        else
        {
            //setTransVertDisplayVisible(false);
            logMessage(QString("GetFreq: No transvert band set for this freq = %1").arg(rigStateDetails->curVfoFreq.traceStr()));
        }
    }

    displayFreqVfo(rigStateDetails->rfrequency);

    if (currentRadio.transVertEnable && b)
    {
        sendFreqToLog(transVertF);
        msg->rigCache.publish();

    }
    else
    {
        sendFreqToLog(rigStateDetails->rfrequency);
        msg->rigCache.publish();
    }

}

void RigControlMainWindow::onNewRxFreq(Frequency f)
{
    double fd = f;
    rigStateDetails->rfrequency = Frequency(fd);
    processRxFrequencyForDisplay();
}

QString RigControlMainWindow::getBand(const Frequency &freq)
{
    for (int i = 0; i < bands.count(); i++)
    {
        if (freq >= bands[i]->fLow && freq <= bands[i]->fHigh)
        {
            return bands[i]->name();
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
    if (currentRadio.mgmMode != mgmMode)
    {
        ui->mgmMode->setText(currentRadio.mgmMode);
    }
}


/******************** Rigctld *************************************/


void RigControlMainWindow::getRigctldNames(QString address, quint16 port)
{



                RigCtldClient *client;
                client = new RigCtldClient();

                if (!client->connectToHost(address, port))
                {
                    logMessage(QString("getRigctldNames - Connect Failed - %1:%2").arg(address).arg(port));
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
                        QObject::connect( client, &RigCtldClient::finished, &loop, &QEventLoop::quit );
                        loop.exec();
                    }

                    if (client->getRetCode() >=0 && client->checkMsgReceived())
                    {
                        rigCtldDetails->rigctld_radioNumber = client->getRadioModel();
                        rigCtldDetails->rigctld_radioName = client->getRadioModelName();
                        rigCtldDetails->rigctld_radioMfg = client->getRadioManufacturerName();
                        logMessage(QString("getrigctld - Got names ok - %1 %2 %3").arg(rigCtldDetails->rigctld_radioNumber, rigCtldDetails->rigctld_radioMfg, rigCtldDetails->rigctld_radioName));
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
    rigCtldDetails->rigctld_radioNumber.clear();
    rigCtldDetails->rigctld_radioName.clear();
    rigCtldDetails->rigctld_radioMfg.clear();

}




void RigControlMainWindow::runRigCtlDaemon(RigCtldParameters &rigctldPar)
{
    QString program = getRigCtldExePath() + getRigCtldExeName();

    QStringList arguments;

#if defined Q_OS_WIN32
    QString serPort = rigctldPar.getComport();
#elif defined Q_OS_LINUX
    QString serPort = "/dev/" + rigctldPar.getComport();
#elif defined Q_OS_MAC
    QString serPort = "/dev/" + rigctldPar.getComport();
#endif

    QStringList parityNames;
    QString parityName;
    QString networkAdd = rigctldPar.getNetworkAddress();
    QString networkPort = rigctldPar.getPortNum();

    if (currentRadio.catPortType == RigCapConstants::PortType::serial)
    {
        //parityNames = radio->getParityCodeNames();
        //parityName = parity;
        arguments << "-m" + rigctldPar.getModelNumber() << "-r" + serPort  << "-s" + rigctldPar.getBaudRate() << "--set-conf=data_bits=" + rigctldPar.getDataBits() << "--set-conf=stop_bits=" + rigctldPar.getStopBits()
                  << "--set-conf=serial_parity=" + rigctldPar.getParity() << "--set-conf=serial_handshake=" + rigctldPar.getHandshake() << "--set-conf=rts_state=" + rigctldPar.getRtsState() << "--set-conf=dtr_state=" + rigctldPar.getDtrState();

        if (rigctldPar.getManufacturer() == "Icom")
        {
            if (!rigctldPar.getCiv().isEmpty())
            {
               arguments << "--set-conf=civaddr=" + rigctldPar.getCiv();

               trace(QString("runRigCtlDaemon:: using icom civ address = %1").arg(rigctldPar.getCiv()));

            }
        }
    }
    else if (currentRadio.catPortType == RigCapConstants::PortType::none)
    {
        // for dummy radio
        arguments << "-m" + rigctldPar.getModelNumber();
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

    if (rigctldPar.getPttEnabled())
    {
        trace(QString("runRigCtlDaemon:: PTT Enabled"));

        if (rigctldPar.getPttType() == serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_DTR
            || rigctldPar.getPttType() == serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_RTS)
        {
            trace(QString("runRigCtlDaemon:: Serial %1 PTT control selected").arg(serialCommonData::pttTypeStr[static_cast<int>(rigctldPar.getPttType())]));
            // we are using serial PTT control
            if (!rigctldPar.getPttComport().isEmpty())
            {
                arguments << QString("--ptt-file=%1").arg(rigctldPar.getPttComport());
                arguments << QString("--ptt-type=%2").arg(serialCommonData::pttTypeStr[static_cast<int>(rigctldPar.getPttType())]);
            }
            else
            {
                trace(QString("runRigCtlDaemon:: Error Serial PTT Comport Empty = %1!").arg(rigctldPar.getPttComport()));
            }

        }
        else
        {
            if (rigctldPar.getPttType() == serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_CAT)
            {
                trace(QString("runRigCtlDaemon:: CAT selected"));
                arguments << QString("--ptt-type=RIG");
            }
            else
            {
                trace(QString("runRigCtlDaemon:: Error PTT Type %1 selected").arg(serialCommonData::pttTypeStr[static_cast<int>(rigctldPar.getPttType())]));
            }
        }
    }





    if (rigctldPar.getTraceCode() != rigCtldTrace::rigCtldTraceCodes::rctNONE)
    {
        arguments << rigCtldTrace::rigCtldTraceStr[rigctldPar.getTraceCode()];
    }


    trace(QString("runRigCtlDaemon:: start rigCtlD - manufacturer = %1, model = %2, comport = %3, baudrate = %4, databits = %5, stopbits = %6, parity = %7, handshake = %8, rtsState = %9, dtrState = %10, civ = %11, netaddress = %12, netPort = %13, pttComport = %14, pttType = %15 ")
          .arg(rigctldPar.getManufacturer()).arg(rigctldPar.getModelNumber())
              .arg(rigctldPar.getComport()).arg(rigctldPar.getBaudRate())
              .arg(rigctldPar.getDataBits()).arg(rigctldPar.getStopBits())
              .arg(parityName).arg(rigctldPar.getHandshake())
              .arg(rigctldPar.getRtsState()).arg(rigctldPar.getDtrState())
              .arg(rigctldPar.getCiv()).arg(rigctldPar.getNetworkAddress())
              .arg(rigctldPar.getPortNum()).arg(rigctldPar.getPttComport())
              .arg(serialCommonData::pttTypeStr[static_cast<int>(rigctldPar.getPttType())]));

//    trace(arguments.join(" ; "));
    rigCtldProcess->start(program, arguments);

}


void RigControlMainWindow::rigCtldMessage()
{
    if (rigCtldProcess->state() == QProcess::Running)
    {
        QString line = rigCtldProcess->readAllStandardOutput();
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


void RigControlMainWindow::setRigCtldIndicator(RIGCTLD_INDICATOR_ID idNum)
{
    if (idNum == RIGCTLD_IND_INT)
    {
        ui->rigCtldIndicator->setStyleSheet(RIGCTLD_INDICATOR_INTERNAL);
    }
    else if (idNum ==RIGCTLD_IND_EXT)
    {
        ui->rigCtldIndicator->setStyleSheet(RIGCTLD_INDICATOR_EXTERNAL);
    }
    else
    {
        ui->rigCtldIndicator->setStyleSheet(RIGCTLD_INDICATOR_OFF);
    }
}



void RigControlMainWindow::rigCtldStatusTimeout()
{
    if (currentRadio.rigCtldEnable)
    {

        if (currentRadio.startMinosRigCtld)
        {
            if (rigCtldProcess->state() == QProcess::Running)
            {
               setRigCtldIndicator(RIGCTLD_IND_INT);
            }
            else
            {
               setRigCtldIndicator(RIGCTLD_IND_OFF);
            }
        }
        else
        {
            // using external rigctld
            setRigCtldIndicator(RIGCTLD_IND_EXT);
        }



    }
}


void RigControlMainWindow::getRigCtldConnectDelay()
{
    QString fileName;
    if (appName.isEmpty())
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOCAL() + MINOS_RADIO_CONFIG_FILE;
    }
    else
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOGGER() + MINOS_RADIO_CONFIG_FILE;
    }


    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(RIGCTLD_GROUP_NAME);
    rigCtldDetails->rigCtldConnectDelay = config.value("RigCtldConnectDelay", DEFAULT_RIGCTLD_CONNECT_DELAY).toInt();
    config.endGroup();

    if (rigCtldDetails->rigCtldConnectDelay > MAX_RIGCTLD_CONNECT_DELAY)
    {
        logMessage(QString("ERROR rigctld connect delay value = %1, too high, setting max delay = %2").arg(rigCtldDetails->rigCtldConnectDelay).arg(MAX_RIGCTLD_CONNECT_DELAY));
        rigCtldDetails->rigCtldConnectDelay = 5;
    }


}



/*****************  cache data ***************/


void RigControlMainWindow::initCacheData(QStringList &availRadios)
{

    if (!availRadios.isEmpty())
    {
        QVector<QSharedPointer<scatParams> >radioDataList;
        scatParams radioData;
        for(auto &radioName: availRadios)
        {
            QSharedPointer<scatParams> radioData = QSharedPointer<scatParams>(new scatParams);
            getRadioConfigData(radioData.data(), radioName);
            radioDataList.append(radioData);
        }

        for (int i = 0; i < radioDataList.count(); i++)
        {
            QStringList supBandList;
            //int radioModelNumber = radioDataList[i]->rigModelNumber;
            buildSupBandList(radioDataList[i].data(), supBandList);

            addBandListToRigCache(radioDataList[i]->radioName, supBandList);

            msg->rigCache.setRttyOffset(radioDataList[i]->radioName, radioDataList[i]->rttyOffset);
            msg->rigCache.setPskOffset(radioDataList[i]->radioName, radioDataList[i]->pskOffset);

            msg->rigCache.invalidate();

        }
    }
}

// this is a bit brutal...updates all rigdetails even if data hasn't changed...

void RigControlMainWindow::updateRigDetailsCache()
{
    // update riglist first
    QStringList availRadios;
    getAvailRadiosList(availRadios);
    sendRadioListLogger(availRadios);
    // now rigdetails available before radio is opened

    initCacheData(availRadios);
    msg->rigCache.publish();

}


/******************** Supported Bands  ********/



void RigControlMainWindow::buildSupBandList(scatParams *radioData, QStringList &bandList)
{
    bandList.clear();

    // find the bands the radio supports
    QStringList supBandsList;
    buildSupportedRadioBands(radioData, supBandsList);


    // merge radio bands and transverter bands
    if(radioData->transVertEnable)
    {

        trace(QString("[buildSupBandList] - Transverters enabled - look for configured transverters"));
        for (auto const &b: QASCONST(bands))
        {
            bool transVertFound = findSupTransBand(b->name(), radioData);
            if (findSupRadioBand(b->name(), supBandsList) ||  transVertFound)
            {
                if (transVertFound)
                {
                    trace(QString("[buildSupBandList] - Transverter Found for band = %1").arg(b->name()));
                }

                bandList.append(b->name());
            }

        }

    }
    else
    {
        // no transverters enabled
        trace(QString("[buildSupBandList] - Transverters not enabled"));
        bandList = supBandsList;
    }



}


// probe radio for supported bands
void RigControlMainWindow::buildSupportedRadioBands(scatParams *radioData, QStringList& supBandList)
{

    if (radioData->rigModelNumber <= RigId::NonHamlibBaseId)
    {
        trace(QString("[buildSupportedRadioBands] - hamlib radios"));
        for (const auto &b: QASCONST(bands))
        {


            if (rigFactory->checkForBands(radioData->rigModelNumber, b->fLow))
            {
                trace(QString("[buildSupportedRadioBands] - Radio supports band = %1").arg(b->uk));
                supBandList.append(b->name());

            }
            else
            {
                trace(QString("[buildSupportedRadioBands] - Radio Does not support band = %1").arg(b->uk));
            }
        }
    }
    else
    {
        trace(QString("[buildSupportedRadioBands] - non hamlib radios"));
        // non hamlib radios
        for (const auto &b: QASCONST(bands))
        {

            if (radioData->supportBands.getSupportBandFlag(b->name()))
            {
               trace(QString("[buildSupportedRadioBands] - Radio supports band = %1").arg(b->uk));
               supBandList.append(b->name());
            }
            else
            {
                trace(QString("[buildSupportedRadioBands] - Radio Does not support band or supported band not set for band = %1").arg(b->uk));
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
bool RigControlMainWindow::findSupTransBand(const QString band, const scatParams* radioData)
{

    QStringList tvList = radioData->transVertSettings.keys();

    for(const auto &tv: QASCONST(tvList))
    {
        if (tv == band)
        {
            return true;
        }
    }

    return false;

    //if (radioData->transVertNames.count() > 0)
   // {
   //     for (int i = 0; i < radioData->transVertNames.count();i++)
   //     {

  //          if (band == radioData->transVertNames[i])
  //          {
  //              return true;
  //          }
  //      }

  //      return false;
  //  }

  //  return false;
}


/************************** Mode  *********************************/


int RigControlMainWindow::getAndSendMode(VFO vfo)
{

    int retCode = 0;
    logMessage(QString("getAndSendMode, vfo = %1").arg(vfoToStr(vfo)));

    if (radio)
    {
        retCode = radio->getMode(vfo, rigStateDetails->rmode);

        if (retCode == Rig_OK)
        {
            rigStateDetails->curMode = rigStateDetails->rmode;

            rigStateDetails->curModeStr = rigcommon::convertModeToQString(rigStateDetails->rmode);

            logMessage(QString("Get Mode: From Rx mode = %1; MGM Mode %2").arg(rigStateDetails->curModeStr, currentRadio.mgmMode));

            if (rigStateDetails->RTTYModeFlag)
            {
                displayModeVfo(RY);
                sendModeToLog(QString("%1:%2").arg(RY, currentRadio.rttyMode));
            }
            else if (rigStateDetails->PSKModeFlag)
            {
                displayModeVfo(PSK);
                sendModeToLog(QString("%1:%2").arg(PSK, currentRadio.pskMode));
            }
            else if (rigStateDetails->mgmModeFlag)
            {
                displayModeVfo(MGM);
                sendModeToLog(QString("%1:%2").arg(MGM, currentRadio.mgmMode));
            }
            else
            {
                displayModeVfo(rigcommon::convertModeToQString(rigStateDetails->rmode));
                sendModeToLog(QString("%1:%2").arg(rigcommon::convertModeToQString(rigStateDetails->rmode), " "));
            }
        }
    }
    else
    {
        logMessage(QString("radio = nullptr"));
    }

    return retCode;
}


void RigControlMainWindow::onNewMode()
{
    getAndSendMode(rigStateDetails->curVfo);
    msg->rigCache.publish();
}

void RigControlMainWindow::loggerSetMode(QString mode)
{
    if (closeApp)
        return;

    logMessage(QString("Log SetMode:: Mode Received from Logger = %1").arg(mode));
    //int retCode = RIG_OK;

    if (radioCommsOK && !rigStateDetails->rigErrorFlag)
    {
        loggerRequests->slogMode = mode;
        //logMode = radio->convertQStrMode(mode);


        if (loggerRequests->slogMode == MGM)
        {
            logMessage(QString("Log SetMode:MGM mode Selected"));
            if (rigStateDetails->mgmModeFlag)
            {
                logMessage(QString("Log SetMode: Mgm flag is set"));
                if (rigStateDetails->curMode !=  rigcommon::convertQStringToMode(currentRadio.mgmMode))
                {
                    setMode(currentRadio.mgmMode, rigStateDetails->curVfo);
                    logMessage((QString("Log SetMode: MgmMode Flag alread set, Send to setmode MGM Mode = %1").arg(currentRadio.mgmMode)));

                }
            }
            else
            {
                rigStateDetails->mgmModeFlag = true;
                rigStateDetails->RTTYModeFlag = false;
                rigStateDetails->PSKModeFlag = false;
                setMode(currentRadio.mgmMode, rigStateDetails->curVfo);

                logMessage((QString("Log SetMode: Set MgmMode Flag, Send to setmode MGM Mode = %1").arg(currentRadio.mgmMode)));
            }
        }
        else if (loggerRequests->slogMode == RY)
        {
            logMessage(QString("Log SetMode:RY mode Selected"));
            if (rigStateDetails->RTTYModeFlag)
            {
                logMessage(QString("Log SetMode: RTTY flag is set"));
                if (rigStateDetails->curMode !=  rigcommon::convertQStringToMode(currentRadio.rttyMode))
                {
                    setMode(currentRadio.rttyMode, rigStateDetails->curVfo);
                    logMessage((QString("Log SetMode: rttyMode Flag alread set, Send to setmode RTTY Mode = %1").arg(currentRadio.mgmMode)));

                }
            }
            else
            {
                rigStateDetails->mgmModeFlag = false;
                rigStateDetails->RTTYModeFlag = true;
                rigStateDetails->PSKModeFlag = false;
                setMode(currentRadio.rttyMode, rigStateDetails->curVfo);

                logMessage((QString("Log SetMode: Set rttyMode Flag, Send to setmode RTTY Mode = %1").arg(currentRadio.rttyMode)));
            }
        }
        else if (loggerRequests->slogMode == PSK)
        {
            logMessage(QString("Log SetMode:PSK mode Selected"));
            if (rigStateDetails->PSKModeFlag)
            {
                logMessage(QString("Log SetMode: PSK flag is set"));
                if (rigStateDetails->curMode !=  rigcommon::convertQStringToMode(currentRadio.pskMode))
                {
                    setMode(currentRadio.pskMode, rigStateDetails->curVfo);
                    logMessage((QString("Log SetMode: PskMode Flag alread set, Send to setmode PSK Mode = %1").arg(currentRadio.pskMode)));

                }
            }
            else
            {
                rigStateDetails->mgmModeFlag = false;
                rigStateDetails->RTTYModeFlag = false;
                rigStateDetails->PSKModeFlag = true;
                setMode(currentRadio.pskMode, rigStateDetails->curVfo);

                logMessage((QString("Log SetMode: Set PskMode Flag, Send to setmode PSK Mode = %1").arg(currentRadio.pskMode)));
            }
        }
        else
        {
            rigStateDetails->mgmModeFlag = false;
            rigStateDetails->RTTYModeFlag = false;
            rigStateDetails->PSKModeFlag = false;
            logMessage(QString("Log SetMode: Clear mgm, rtty, psk Mode Flags, Set mode = %1").arg(mode));
            setMode(mode, rigStateDetails->curVfo);

        }
    }
    // mode won't have changed yet
    //msg->rigCache.publish();
}

void RigControlMainWindow::setMode(QString mode, VFO vfo)
{
    int retCode = 0;
    logMessage(QString("setMode"));

    if (radio)
    {
        getRadioInfo(false);

        cmdLockOn();      // lock get radio info
        logMessage(QString("SetMode: Mode Requested = %1, vfo = %2").arg(mode, vfoToStr(vfo)));
        mode = mode.left(mode.indexOf(":"));
        if (mode == PH)
        {
            Frequency modeTestFreq;
            if (rigStateDetails->curTransVertFreq.isClear())
            {
                modeTestFreq = rigStateDetails->curVfoFreq;
            }
            else
            {
                modeTestFreq = rigStateDetails->curTransVertFreq;
            }
            if (modeTestFreq > Frequency(10000000))
            {
                trace(QString("Switching to USB as frequency is %1").arg(modeTestFreq.traceStr()));
                mode = "USB";
            }
            else
            {
                trace(QString("Switching to LSB as frequency is %1").arg(modeTestFreq.traceStr()));
                mode = "LSB";
            }
        }

        MODE mCode = rigcommon::convertQStringToMode(mode);

        if (radioCommsOK)
        {
            retCode = radio->setMode(vfo, mCode);
            if (retCode == Rig_OK)
            {
                logMessage(QString("SetMode: changed! Mode = %1 , vfo = %2").arg(rigcommon::convertModeToQString(mCode), vfoToStr(vfo)));

            }
            else
            {
                logMessage(QString("SetMode: Change Error Code = %1, Mode = %2").arg(QString::number(retCode), rigcommon::convertModeToQString(mCode), rigcommon::convertModeToQString(mCode)));

                if (radio->modeSupported(mCode, rigStateDetails->rfrequency))
                {
                    radioError(retCode, tr("Set Mode"));
                }
                else
                {
                    logMessage(QString("Mode not supported by radio"));
                    sendStatusToLogError(tr("%1 not supported by radio").arg(mode));
                }
            }
        }
        else
        {
            logMessage(QString("Set Mode: radio not connected"));
        }
        cmdLockOff();
    }


}

void RigControlMainWindow::loggerSetVolume(int level)
{
    if (closeApp)
        return;

    logMessage(QString("Set Volume: From Logger, level = %1").arg(level));
    setVolume(rigStateDetails->curVfo, level);

}

/************************** RIT *********************************/


void RigControlMainWindow::clearSupportRitFlags()
{
    rigStateDetails->radioRitOn = false;
    rigStateDetails->ritEnable = false;
    rigStateDetails->ritKHzFlag = false;
    rigStateDetails->ritMaxKHzFreq = MAX_RITFREQ;
}

void RigControlMainWindow::getRitSupportStatus()
{

    if (radio != nullptr)
    {

        rigStateDetails->ritMaxKHzFreq = radio->getMaxRitFreq(currentRadio.rigModelNumber);

        // load rit test spinner
        ui->setRitSpinner->setRange(rigStateDetails->ritMaxKHzFreq * -1, rigStateDetails->ritMaxKHzFreq);


        logMessage(QString("Rit MaxFreq = %1").arg(rigStateDetails->ritMaxKHzFreq));
        if (rigStateDetails->ritMaxKHzFreq != 0)
        {

            if (rigStateDetails->ritMaxKHzFreq <= MAX_RITFREQ)
            {
                rigStateDetails->ritKHzFlag = false;
            }
            else
            {
                rigStateDetails->ritKHzFlag = true;      // rx rit > +/-10Khz
            }

            sendMaxRitFreqLogger();

            if (rigStateDetails->rigCap.getSupportGetRit())
            {
                // Does radio support getting Rit Freq?
                logMessage(QString("Get Rit Support Status - getRit support is  = %1").arg(selectedRadioSupportCap.getSupportGetRit() ? "True" : "False"));

            }

            if (rigStateDetails->rigCap.getSupportSetRit())
            {
                // Does radio support setting Rit Freq?
                logMessage(QString("Get Rit Support Status - setRit support is  = %1").arg(selectedRadioSupportCap.getSupportSetRit() ? "True" : "False"));

            }

            if (selectedRadioSupportCap.getSupportSetRit())
            {
                rigStateDetails->ritEnable = currentRadio.enableDisableCatFeature.ritEnable;
                logMessage(QString("Get Rit Support - Rit Enabled = %1").arg(rigStateDetails->ritEnable ? "True" : "False"));

            }

            // Does radio support turning Rit on/off

            if (rigStateDetails->rigCap.getSupportSetRitState())
            {
                logMessage(QString("Get Rit Support Status - set Rit on/off support is  = %1").arg(selectedRadioSupportCap.getSupportSetRitState() ? "True" : "False"));

            }

            if (rigStateDetails->rigCap.getSupportGetRitState())
            {
                // Does radio support getting Rit on/off state?
                logMessage(QString("Get Rit Support Status - Rit On/Off state support is  = %1").arg(selectedRadioSupportCap.getSupportGetRitState() ? "True" : "False"));

            }


        }
        else
        {
            logMessage(QString("Read Rit Max Freq = 0, disable Rit"));
        }

    }
    else
    {
        logMessage(QString("Get Rit Support Status - radio ptr is null"));

    }

}



void RigControlMainWindow::setRitLogStatus(bool status)
{


    if (closeApp)
        return;


    loggerRequests->logRitOn = status;
    logMessage(QString("Logger RIT Status received = %1").arg(status ? "True" : "False"));
    int retCode = 0;
    if (radio)
    {
        ritIndicatorToggle(loggerRequests->logRitOn);

        if (selectedRadioSupportCap.getSupportSetRitState())
        {
            logMessage(QString("Radio Support RIT On/off switching - turn off"));
            if (radioCommsOK)
            {
                // radio supports turning RIT on and off
                retCode = radio->setRitState(rigStateDetails->curVfo, loggerRequests->logRitOn);
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
            if (radioCommsOK && selectedRadioSupportCap.getSupportSetRit() && rigStateDetails->ritEnable)
            {
                setRitFreq(ShortFreq());
                sendRitFreqLogger(ShortFreq());
            }


        }

        if (!selectedRadioSupportCap.getSupportGetRitState())
        {
            ritIndicatorToggle(loggerRequests->logRitOn);
        }
    }
    else
    {
        logMessage(QString("radio = nullptr"));
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
    //ui->ritEnableChk->setVisible(state);
    ui->ritGroupBox->setVisible(state);
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



int RigControlMainWindow::getRitFreq(VFO vfo)
{
    logMessage(QString("getRitFreq"));
    int retCode = 0;

    if (radio)
    {

        ShortFreq ritFreq;
        static ShortFreq oldritFreq(50000);

        retCode = radio->getRit(vfo, ritFreq);
        if (retCode == Rig_OK)
        {
            //ShortFreq iRitFreq = ritFreq;
            if (oldritFreq != ritFreq)
            {
               rigStateDetails->rRitFreq = ritFreq;
               oldritFreq = ritFreq;
               ui->ritFreq->setText(convertRitFreqToStr(rigStateDetails->rRitFreq, rigStateDetails->ritKHzFlag));
               logMessage(QString("GetRitFreq from radio = %1").arg(rigStateDetails->rRitFreq.traceStr()));
               sendRitFreqLogger(rigStateDetails->rRitFreq);


            }
        }
    }



    return retCode;
}


void RigControlMainWindow::doSetRitFreq(const ShortFreq &ritFreq)
{

    if (closeApp)
        return;
    logMessage(QString("setRitFreq"));
    if (radio)
    {
        if (rigStateDetails->ritEnable)
        {
            int retCode = 0;
            logMessage(QString("Set Rit = %1").arg(ritFreq.traceStr()));
            cmdLockOn();
            retCode = radio->setRit(rigStateDetails->curVfo, ritFreq);
            cmdLockOff();
            if (retCode < 0)
            {
                // error
                logMessage(QString("Set RIT freq error").arg(QString::number(retCode)));
                radioError(retCode, tr("Set RIT Freq."));
            }
            else
            {
                if (!selectedRadioSupportCap.getSupportGetRit())
                {
                    // get rit is not available, update local rit display
                    // and send to logger to update logger
                    logMessage(QString("Get Rit not available - update display %1").arg(ritFreq.traceStr()));
                    ui->ritFreq->setText(convertRitFreqToStr(ritFreq, rigStateDetails->ritKHzFlag));
                    sendRitFreqLogger(ritFreq);
                }
            }
        }
    }
    else
    {
        logMessage(QString("radio = nullptr"));
    }

}

void RigControlMainWindow::setRitFreq(const ShortFreq &freq)
{

    logMessage(QString("SetRit Freq from logger = %1").arg(freq.traceStr()));
    doSetRitFreq(freq);
}


int  RigControlMainWindow::getRitRadioStatus(VFO vfo, bool *status)
{
    logMessage(QString("Get Rit RadioStatus"));

    cmdLockOn();
    bool s;
    int retCode = 0;
    if (radio)
    {
        retCode = radio->getRitState(vfo, s);
        cmdLockOff();

        if (retCode == Rig_OK)
        {
            *status = s;
            logMessage(QString("Get Rit RadioStatus = %1").arg(s ? "True" : "False"));
        }
        else
        {
            logMessage(QString("Get Rit RadioStatus - failed with errorcode = %1").arg(QString::number(retCode)));
        }
    }
    else
    {
        logMessage(QString("radio = nullptr"));
    }

    return retCode;
}


void RigControlMainWindow::sendRadioRitStatusLogger(bool status)
{
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setRadioRitStatus(psname, status);
    logMessage(QString("Send Radio Rit status to logger = %1 psn=%2").arg(status ? "On" : "Off", psname.toString()));
}

void RigControlMainWindow::sendRitFreqLogger(const ShortFreq &ritFreq)
{
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setRadioRitFreq(psname, ritFreq);
    logMessage(QString("Send Rit freq to logger = %1 psn=%2")
               .arg(convertRitFreqToStr(ritFreq, rigStateDetails->ritKHzFlag), psname.toString()));
}

// non polling Rit slots

void RigControlMainWindow::onRitOn()
{

}

void RigControlMainWindow::onRitOff()
{

}

void RigControlMainWindow::onRitOffset()
{

}

void RigControlMainWindow::onRit0()
{

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
        value = value * VOLMULT;
        vol = qRound(value);
        if (vol > 200)
        {
            vol = 200;
        }
        if (vol < 0)
        {
            vol = 0;
        }

        if (vol != rigStateDetails->curVol)
        {
            rigStateDetails->curVol = vol;
            sendVolToLog(rigStateDetails->curVol);
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
    if (radio)
    {
       retCode = radio->setVolume(vfo, volLevel);
    }
    else
    {
        logMessage(QString("radio = nullptr"));
    }

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

    if (radio)
    {

        retCode = radio->getSignalStrength(vfo, &value);

        if (retCode >= 0)
        {
            if (rigStateDetails->curSignalStrength != value)
            {
                rigStateDetails->curSignalStrength = value;
                displaySignalStrength(rigStateDetails->curSignalStrength);
            }
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
void RigControlMainWindow::displayFreqVfo(Frequency frequency)
{

    ui->radioFreqA->setText(frequency.convertFreqStrDisp());
}


void RigControlMainWindow::displayTransVertVfo(Frequency frequency)
{
    ui->transVertFreqA->setText(frequency.convertFreqStrDisp());

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
        msg->rigCache.publish();
    }
}

void RigControlMainWindow::radioError(int errorCode, QString cmd)
{

    pollTimer->stop();


    if ( errorCode >= 0)
    {
        return;
    }


    rigStateDetails->rigErrorFlag = true;

    errorCode *= -1;
     QString errorMsg;
    if (radio != nullptr)
    {
        errorMsg = radio->getErrorMsgText(errorCode);

        sendStatusToLogError(errorMsg);

        if (radio)
        {
            logMessage(QString("%1 library Error - Code = %2 - %3").arg(rigStateDetails->rigCap.getLibraryName()).arg(errorCode).arg(errorMsg));
            QMessageBox::critical(this, tr("RigControl %1 library Error").arg(rigStateDetails->rigCap.getLibraryName()), tr("%1\n%2 - %3\nCommand: %4").arg(currentRadio.radioName).arg(errorCode).arg(errorMsg, cmd));
        }

    }
    else
    {
        errorMsg = QString("No radio defined to obtain error message");
    }


    closeRadio();
    rigStateDetails->rigErrorFlag = false;
    sendStatusToLogDisConnected();
}


/********************* PTT ****************************************/


// Omnirig
void RigControlMainWindow::onPttState(bool pttState)
{
    if (pttState != rigStateDetails->curPttStatus)
    {
        trace(QString("Omnirig onPttState = %1").arg(pttState ? "On" : "Off"));
        rigStateDetails->curPttStatus = pttState;
        setRigControlPttState(rigStateDetails->curPttStatus);
    }
}



int RigControlMainWindow::getTXStatus(VFO vfo)
{

   bool pttStatus = false;
   int retCode = radio->getPttStatus(vfo, pttStatus);

   if (retCode >= 0)
   {
       trace(QString("TXStatus = %1").arg(pttStatus ? "On" : "Off"));

       if (pttStatus != rigStateDetails->curPttStatus)
       {
           rigStateDetails->curPttStatus = pttStatus;
           trace(QString("TXStatus cxhanged to %1").arg(pttStatus ? "On" : "Off"));
           setRigControlPttState(rigStateDetails->curPttStatus);
       }
   }

   return retCode;

}


void RigControlMainWindow::setRigControlPttState(bool state)
{

    sendPttStateLogger();
    setTxRxIndOnOff(state);
}


int RigControlMainWindow::setTxState(VFO vfo, bool txState)
{

    trace(QString("setTxState = %1").arg(txState ? "On" : "Off"));
    int retCode = radio->setPtt(vfo, txState);
    return retCode;
}

/*
bool RigControlMainWindow::readTestStandAloneFlag()
{
    QString fileName;
    if (appName.isEmpty())
    {
       fileName = RIG_CONFIGURATION_FILEPATH_LOCAL() + MINOS_RADIO_CONFIG_FILE;
    }
    else
    {
       fileName = RIG_CONFIGURATION_FILEPATH_LOGGER() + MINOS_RADIO_CONFIG_FILE;
    }


    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("TestStandAlone");
    bool state = config.value("TestStandAlone", false).toBool();
    config.endGroup();

    return state;
}
*/




void RigControlMainWindow::readTraceLogFlag()
{
    QString fileName;
    if (appName.isEmpty())
    {
       fileName = RIG_CONFIGURATION_FILEPATH_LOCAL() + MINOS_RADIO_CONFIG_FILE;
    }
    else
    {
       fileName = RIG_CONFIGURATION_FILEPATH_LOGGER() + MINOS_RADIO_CONFIG_FILE;
    }


    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("TraceLog");
    rigStateDetails->traceCommsFlag = config.value("TraceLog", false).toBool();
    config.endGroup();

    if (radio)
    {
        radio->setTraceComms(rigStateDetails->traceCommsFlag);
    }


    ui->traceDataComms->setChecked(rigStateDetails->traceCommsFlag);
}

void RigControlMainWindow::saveTraceLogFlag(bool state)
{

    // set state of hamlib commms tracing

    rigStateDetails->traceCommsFlag = state;
    if (radio)
    {
        radio->setTraceComms(state);
    }


    // save to ini for restart

    QString fileName;
    if (appName.isEmpty())
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOCAL() + MINOS_RADIO_CONFIG_FILE;
    }
    else
    {
        fileName = RIG_CONFIGURATION_FILEPATH_LOGGER() + MINOS_RADIO_CONFIG_FILE;
    }

    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("TraceLog");

    config.setValue("TraceLog", state);

    config.endGroup();
    trace("Tracelog Changed in " + fileName + " = " + QString::number(state));
}




void RigControlMainWindow::about()
{
    QMessageBox::about(this, tr("Minos RigControl"), tr("Minos QT RigControl\nCopyright D Balharrie G8FKH/M0DGB 2024"));
}




void RigControlMainWindow::sendRadioListLogger(const QStringList &availRadios)
{
    if (!testMode)
    {
        QStringList radioList;
        for(const auto &rn: QASCONST(availRadios))
        {
            if (!rn.isEmpty())
            {
                PubSubName r(rn);
                radioList.append(r.toString());
            }
        }
        logMessage(QString("Sending radiolist to logger"));
        for (const auto &radio: QASCONST(radioList))
        {
            logMessage(QString("Send radioname %1").arg(radio));
        }
        logMessage(QString("radiolist complete"));
        msg->publishRadioNames(radioList);
    }

}

void RigControlMainWindow::addBandListToRigCache(const QString radioName, const QStringList& supBandList)
{
    if (!supBandList.isEmpty())
    {

        PubSubName psname(radioName);
        QString bandList = supBandList.join(":");
        logMessage(QString("Add bandlist to rigcache for radio %1 = %2").arg(radioName, bandList));
        msg->rigCache.setBandList(psname, bandList);

    }
    else
    {
        logMessage(QString("error radio bandlist empty"));
    }

}

void RigControlMainWindow::sendStatusLogger(const QString &message )
{
    logMessage(QString("Send status to logger = %1").arg(message));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setStatus(psname, message);
}


void RigControlMainWindow::sendStatusToLogConnected()
{
    logMessage(QString("Send status to logger connected"));
    sendStatusLogger(tr(RIG_STATUS_CONNECTED));
    ui->reconnectButton->setVisible(false);

}

void RigControlMainWindow::sendStatusToLogDisConnected()
{
    logMessage(QString("Send status to logger disconnected"));
    sendStatusLogger(tr(RIG_STATUS_DISCONNECTED));

    if (!currentRadioName.isEmpty())
    {
        ui->reconnectButton->setVisible(true);
    }

}


void RigControlMainWindow::sendStatusToLogError(QString errMsg)
{
    logMessage(QString("Send error status to logger - %1").arg(errMsg));
    sendStatusLogger(QString("%1:%2").arg(tr(RIG_STATUS_ERROR),errMsg));
}

void RigControlMainWindow::sendRadioSwitchCompleteToLogger()
{
    logMessage(QString("Send status to logger radio switch complete"));
    sendStatusLogger(RIG_SWITCH_COMPLETED);
}


void RigControlMainWindow::sendFreqToLog(const Frequency &freq)
{
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setRadioFreq(psname, freq);
    logMessage(QString("Send freq to logger = %1 psn=%2").arg(freq.traceStr(), psname.toString()));
}

void RigControlMainWindow::sendModeToLog(QString mode)
{
    logMessage(QString("Send mode to logger = %1").arg(mode));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setRadioMode(psname, mode);
}

void RigControlMainWindow::sendVolToLog(int level)
{
    logMessage(QString("Send volume to logger = %1").arg(QString::number(level)));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setRadioVolume(psname, level);
}


void RigControlMainWindow::addVolStatusToRigCache(bool status)
{
    logMessage(QString("Add Volume Status to rigcache = %1").arg(status  ? "True" : "False"));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setVolumeStatus(psname, status);
}


void RigControlMainWindow::addRigModelToRigCache(QString rigModel)
{
    logMessage(QString("Add RigModel to rigcache = %1").arg(rigModel));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setRigModel(psname, rigModel);
}


void RigControlMainWindow::addVoiceMemStatusToRigCache(bool status)
{
    logMessage(QString("Add Voice Memory Status to rigcache = %1").arg(status  ? "True" : "False"));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setVoiceMemAvail(psname, status);
}

void RigControlMainWindow::addVoiceNumberMessagesToRigCache(int numMessages)
{
    logMessage(QString("Add Number of Voice Memory Messages to rigcache = %1").arg(numMessages));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setNumVoiceMessages(psname, numMessages);
}

void RigControlMainWindow::addVoiceKeyerSupportStopCmdToRigCache(bool supportStopCmd)
{
    logMessage(QString("Add Voice Keyer Support Stop message cmd to rigcache = %1").arg(supportStopCmd ? "True" :"False"));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setRigVoiceKeyerSupportStopFlag(psname, supportStopCmd);
}


// as we are now sending rigmodel to logger and it includes manufacturer and model number
// we could despense with cwMemType and derive it from rigModel??
void RigControlMainWindow::addCwKeyerTypeToRigCache(int cwMemType)
{
    logMessage(QString("Add CW Keyer Type to rigcache = %1").arg(getCwRadioManufacturer(cwMemType)));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setCwMemType(psname, cwMemType);
}

void RigControlMainWindow::addCwKeyerSupportStopCmdToRigCache(bool supportStopCmd)
{
    logMessage(QString("Add CW Keyer Support Support Stop message cmd to rigcache = %1").arg(supportStopCmd ? "True" :"False"));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setRigCwKeyerSupportStopFlag(psname, supportStopCmd);
}


void RigControlMainWindow::addPTTEnabledStatusToRigCache(bool status)
{
    logMessage(QString("Add PTT Enabled Status to rigcache = %1").arg(status  ? "True" : "False"));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setPttEnabled(psname, status);
}




void RigControlMainWindow::sendTransVertEnabled(bool status)
{
    logMessage(QString("Send Transvert Enabled to logger = %1").arg(status  ? "True" : "False"));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setTransverterEnabled(psname, status);
}



void RigControlMainWindow::sendTransVertStatusToLog(bool status)
{
    logMessage(QString("Send Transvert Status to logger = %1").arg(status  ? "True" : "False"));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setTransverterStatus(psname, status);
}



void RigControlMainWindow::sendTransVertSwitchToLogger(const QString &swNum)
{
    if (swNum == "")
    {
        logMessage(QString("Send Transvert Switch Number to Logger - Switch Number Empty - Nothing Sent"));
        return;
    }

    logMessage(QString("Send Transvert Switch Number to logger = %1").arg(swNum));
    PubSubName psname(currentRadioName);
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


    if (currentRadio.transVertEnable
            && currentRadio.enableTransSwitch
            && currentRadio.enableLocTVSwMsg
            && serialTVSw->getOpenFlag())      // com port should be open!
    {
        logMessage(QString("Send Transvert Switch Number to Comport = %1, message = %2").arg(currentRadio.locTVSwComport, QString::fromLocal8Bit(msg)));
        serialTVSw->sendTVSwMessage(msg);
    }


}


void RigControlMainWindow::sendRitEnableStatusLogger()
{
    if (selectedRadioSupportCap.getSupportSetRit() & rigStateDetails->ritEnable)
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
    logMessage(QString("Send Rit Enable Status to logger = %1").arg(status  ? "True" : "False"));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setRitEnableStatus(psname, status);
}

void RigControlMainWindow::sendMaxRitFreqLogger()
{
    logMessage(QString("Send RitMaxFreq = %1 to logger").arg(rigStateDetails->ritMaxKHzFreq));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setRitMaxKHzFreq(psname, rigStateDetails->ritMaxKHzFreq);
}

void RigControlMainWindow::sendPttTypeLogger()
{
    logMessage(QString("Send PTT Type = %1 to logger").arg(serialCommonData::pttTypeStr[static_cast<int>(currentRadio.pttType)]));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setPttType(psname, static_cast<int>(currentRadio.pttType));
}

void RigControlMainWindow::sendPttEnabledLogger()
{
    logMessage(QString("Send PTT Enabled = %1 to logger").arg(currentRadio.enablePTT ? "Yes" : "No"));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setPttEnabled(psname, currentRadio.enablePTT);
}

void RigControlMainWindow::sendPttStateLogger()
{
    logMessage(QString("Send PTT State = %1 to logger").arg(rigStateDetails->curPttStatus ? "TX" : "RX"));
    PubSubName psname(currentRadio.radioName);
    msg->rigCache.setPttState(psname, rigStateDetails->curPttStatus);
}



void RigControlMainWindow::onLaunchSetup()
{

    RigSetupDialog setupRadio(rigFactory, bands);
    setupRadio.setCurrentRadioName(currentRadioName);
    setupRadio.setTabToCurrentRadio();
    setupRadio.loadAvailComports();
    if (setupRadio.exec() == QDialog::Accepted)
    {
        if (!setupRadio.listOfRadioNameChanges.isEmpty() ||
            !setupRadio.listOfRadiosDataChanged.isEmpty())
        {
            updateRigDetailsCache();
            msg->publishListChangedRadioNames(setupRadio.listOfRadioNameChanges, setupRadio.listOfRadiosDataChanged);
        }

        // if we don't update, we dont see radios added/deleted in test mode
        if (testMode)
        {
            updateSelectRadioBox();
        }



        for(const auto &r: QASCONST(setupRadio.listOfRadiosDataChanged))
        {
            if (r == currentRadioName)
            {
               currentRadioSettingChanged(currentRadioName);
            }
        }

    }




}
void RigControlMainWindow::onConfigureRigctld()
{
//    QString filepath = getRigCtldExePath();
//    QString filename = getRigCtldExeName();

//    QDir cdir(GetCurrentDir());
    QString InitialDir = getRigCtldExePath();

    QFileDialog dialog(this, tr("Select Rigctld Program"), InitialDir);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    const QStringList schemes = QStringList(QStringLiteral("file"));

    dialog.setSupportedSchemes(schemes);
#endif

#ifdef Q_OS_WIN
    QString Filter = QString("Executable Files") + " (*.exe);;"
                     + QString(tr("All Files")) + " (*.*)" ;
    dialog.setNameFilter(Filter);
#else
    dialog.setFilter(QDir::AllDirs | QDir::Files | QDir::Dirs /*| QDir::Executable*/); //executable doesn't seem to work
#endif

    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.selectFile( getRigCtldExeName());

    if (dialog.exec() == QDialog::Accepted)
    {
        // need to make path relative
        QString progName = dialog.selectedUrls().value(0).toLocalFile();
        QString rpath = ExtractFileDir(progName);
        progName = ExtractFileName(progName);
        setRigCtldExePath(rpath);
        setRigCtldExeName(progName);

    }


}
void RigControlMainWindow::readCurrentRadio(QString &currentRadioName)
{

    QString fileName = RADIO_PATH_LOGGER() + appName + FILENAME_CURRENT_RADIO;
    QSettings config(fileName, QSettings::IniFormat);

    config.beginGroup("CurrentRadio");
    currentRadioName = config.value("radioName", "").toString();
    config.endGroup();


}

void RigControlMainWindow::saveCurrentRadio(const QString currentRadioName)
{
    QString fileName = RADIO_PATH_LOGGER() + appName + FILENAME_CURRENT_RADIO;
    QSettings config(fileName, QSettings::IniFormat);


    config.beginGroup("CurrentRadio");
    config.setValue("radioName", currentRadioName);
    config.endGroup();




}



// we really only have one of these!

void RigControlMainWindow::getRadioConfigData(scatParams *radioData, QString radioName)
{


    QString fileName = RADIO_PATH_LOGGER() + FILENAME_AVAIL_RADIOS;
    QSettings  config(fileName, QSettings::IniFormat);

    config.beginGroup(radioName);
    radioData->radioName = config.value("radioName", "").toString();
    radioData->rigModel = config.value("radioModel", "").toString();
    radioData->civAddress = config.value("civAddress", "").toString();
    radioData->catPortType = config.value("portType", RigCapConstants::PortType::serial).toInt();
    radioData->advancedCommsFlag = config.value("advancedComms", false).toBool();
    radioData->comport = config.value("comport", "").toString();
    radioData->baudrate = config.value("baudrate", 9600).toInt();
    radioData->databits = config.value("databits", 8).toInt();
    radioData->parity = static_cast<serialCommonData::serialParityCodes>(config.value("parity", 0).toInt());
    radioData->stopbits = config.value("stopbits", 1).toInt();
    radioData->handshake = static_cast<serialCommonData::s_handshakeCodes>(config.value("handshake", 0).toInt());
    radioData->forceDtr = static_cast<serialCommonData::s_forceLinesCodes>(config.value("forceDTR", 0).toInt());
    radioData->forceRts= static_cast<serialCommonData::s_forceLinesCodes>(config.value("forceRTS", 0).toInt());
    radioData->enablePTT = config.value("enablePtt", false).toBool();
    radioData->pttType = static_cast<serialCommonData::MINOS_PTT_TYPES>(config.value("pttType", 0).toInt());
    radioData->pttSerialPort = config.value("pttSerialPort", "").toString();
    radioData->pollInterval = config.value("radioPollInterval", "1").toString();
    radioData->rigCtldEnable = config.value("rigCtldEnable", false).toBool();
    radioData->startMinosRigCtld = config.value("startMinosRigCtld", true).toBool();
    radioData->rigCtldNetworkAdd = config.value("rigCtldNetworkAddress", "").toString();
    radioData->rigCtldNetworkPort = config.value("rigCtldPortNumber", "").toString();
    radioData->transVertEnable = config.value("transVertEnable", false).toBool();
    radioData->antSwitchAvail = config.value("antSwitchAvail", false).toBool();
    radioData->networkAdd = config.value("netAddress", "").toString();
    radioData->networkPort = config.value("netPort", "").toString();
    radioData->mgmMode = config.value("mgmMode", hamlibData::USB).toString();
    radioData->rttyMode = config.value("rttyMode", hamlibData::LSB).toString();
    radioData->pskMode = config.value("pskMode", hamlibData::USB).toString();
    radioData->rttyOffset = config.value("rttyOffset", RTTY_MARK_OFFSET).toInt();
    radioData->pskOffset = config.value("pskOffset", PSK_OFFSET).toInt();
    radioData->enableDisableCatFeature.enableDisplay = config.value("enableShowCatFeatures", false).toBool();
    radioData->enableDisableCatFeature.ritEnable = config.value("ritEnable", false).toBool();
    radioData->enableDisableCatFeature.sMeterEnable = config.value("sMeterEnable", true).toBool();
    radioData->enableDisableCatFeature.volumeEnable = config.value("volumeEnable", true).toBool();
    radioData->enableDisableCatFeature.voiceMemEnable = config.value("voiceMemEnable", true).toBool();
    radioData->enableDisableCatFeature.cWMemEnable = config.value("cWMemEnable", true).toBool();
    radioData->enableDisableCatFeature.catPttEnable = config.value("catPttEnable", true).toBool();


    for (const auto &b: QASCONST(bands))
    {
        if (b->getType() == HF_BANDTYPE)
        {
            QString name = b->normalisedName();
            radioData->supportBands.setSupportBandFlag(b->name(), config.value("support" + name, false).toBool());
        }
        else
        {
            if (b->getType() != HF_BANDTYPE)
            {
                QString name = b->normalisedName();
                radioData->supportBands.setSupportBandFlag(b->name(), config.value("support" + name, false).toBool());
            }
        }


    }
    radioData->enableTransSwitch = config.value("enableTransVertSw", false).toBool();
    radioData->enableLocTVSwMsg = config.value("locTransSwEnable", false).toBool();
    radioData->locTVSwComport = config.value("locTransVertSwComport", "").toString();
    config.endGroup();

    // now read transverter settings
    QString fileNameTransVert = TRANSVERT_PATH_LOGGER() + radioData->radioName + FILENAME_TRANSVERT_RADIOS;
    QSettings  configTransVert(fileNameTransVert, QSettings::IniFormat);

    QStringList tvList = configTransVert.childGroups();
    if (!tvList.isEmpty())
    {
        for(const auto &tv: QASCONST(tvList))
        {
            readTranVerterSetting(radioData, tv, configTransVert);
        }
    }
    else
    {
        // no transverters
        radioData->transVertSettings.clear();

    }



}

void RigControlMainWindow::readTranVerterSetting(scatParams *radioData, QString transvertName, QSettings  &config)
{
    config.beginGroup(transvertName);
    QSharedPointer<TransVertParams> tvp = QSharedPointer<TransVertParams>(new TransVertParams());
    tvp->transVertName = config.value("name", "").toString();
    tvp->band = config.value("band", "").toString();
    tvp->radioFreq = Frequency(config.value("radioFreq", 0).toString());
    tvp->targetFreq = Frequency(config.value("targetFreq", 0).toString());
    tvp->transVertOffset = Frequency(config.value("offsetDouble", 0).toString());
    tvp->antSwitchNum = config.value("antSwNumber", "0").toString();
    tvp->transSwitchNum = config.value("transVertSw", "0").toString();
    radioData->transVertSettings.insert(transvertName, tvp);
    config.endGroup();
 }


QString RigControlMainWindow::getRigCtldExeName()
{
#if defined Q_OS_WIN32
    QString progname = RIGCTL_WIN32_EXE_FILENAME;
#elif defined Q_OS_LINUX
    QString progname = RIGCTL_LINUX_EXE_FILENAME;
#elif defined Q_OS_MAC
    QString progname = RIGCTL_MAC_EXE_FILENAME;
#endif

    QString fileName;
    fileName = RIG_CONFIGURATION_FILEPATH_LOGGER() + MINOS_RADIO_CONFIG_FILE;
    QSettings  settings(fileName, QSettings::IniFormat);
    settings.beginGroup(RIGCTLD_GROUP_NAME);

    QString rigCtldExeName = settings.value(RIGCTLD_NAME_SETTING_NAME, progname).toString();

    settings.endGroup();

    rigCtldExeName = rigCtldExeName.trimmed();

    trace("getRigCtldExeName is " + rigCtldExeName);

    return rigCtldExeName;
}

QString RigControlMainWindow::getRigCtldExePath()
{
    QString fileName;
    fileName = RIG_CONFIGURATION_FILEPATH_LOGGER() + MINOS_RADIO_CONFIG_FILE;
    QSettings  settings(fileName, QSettings::IniFormat);
    settings.beginGroup(RIGCTLD_GROUP_NAME);

    QString rigCtldExePath = settings.value(RIGCTLD_PATH_SETTING_NAME, DEFAULT_RIGCTLD_PATH()).toString();

    settings.endGroup();

    rigCtldExePath = rigCtldExePath.trimmed();
    if (rigCtldExePath.right(1) != "/")
    {
        rigCtldExePath += "/";
    }

    trace("getRigCtldExePath is " + rigCtldExePath);

    return rigCtldExePath;

}
void RigControlMainWindow::setRigCtldExePath(const QString &path)
{
    QString fileName;
    fileName = RIG_CONFIGURATION_FILEPATH_LOGGER() + MINOS_RADIO_CONFIG_FILE;
    QSettings  settings(fileName, QSettings::IniFormat);
    settings.beginGroup(RIGCTLD_GROUP_NAME);

    settings.setValue(RIGCTLD_PATH_SETTING_NAME, path);

    settings.endGroup();

}

void RigControlMainWindow::setRigCtldExeName(const QString &progname)
{
    QString fileName;
    fileName = RIG_CONFIGURATION_FILEPATH_LOGGER() + MINOS_RADIO_CONFIG_FILE;
    QSettings  settings(fileName, QSettings::IniFormat);
    settings.beginGroup(RIGCTLD_GROUP_NAME);

    settings.setValue(RIGCTLD_NAME_SETTING_NAME, progname);

    settings.endGroup();
}



void RigControlMainWindow::getAvailRadiosList(QStringList &availRadios)
{
    QString fileName = RADIO_PATH_LOGGER() + FILENAME_AVAIL_RADIOS;
    QSettings  settings(fileName, QSettings::IniFormat);
    availRadios = settings.childGroups();
    for (int i = 0; i < availRadios.count(); i++)
    {
        if (availRadios[i].contains("Version"))
        {
            availRadios.removeAt(i);
            return;
        }
    }

}

void RigControlMainWindow::checkIniFileVersion()
{

    QString fileName = RADIO_PATH_LOGGER() + FILENAME_AVAIL_RADIOS;
    QSettings  settings(fileName, QSettings::IniFormat);

    QStringList availRadios = settings.childGroups();
    int numAvailRadios = availRadios.count();

    if (numAvailRadios > 0)
    {
        QString version = settings.value("Version/version", QString()).toString();
        if (version == "1" || version == "2")
        {
            updateAvailRadiosToVersion3(version, settings, availRadios, numAvailRadios);
        }

        // check version again
        version = settings.value("Version/version", QString()).toString();

        if (version != "3")
        {
            mShowMessage(tr("The Radio configuration files in %1 are from an old incompatible version of Minos.\n\n"
                            "Please delete them and set up the radios again").arg(RADIO_PATH_LOGGER()), parentWidget());
            exit(10);
        }

        int v = availRadios.indexOf("Version"); // the section name
        availRadios.removeAt(v);

    }
}


void RigControlMainWindow::updateAvailRadiosToVersion3(QString version, QSettings& settings, QStringList &availRadios, int numAvailRadios)
{
        QString radioModel;
        QStringList spList;
        QString radio;

        if (version == "1")
        {
            for (int i = 0; i < numAvailRadios; i++)
            {
                radio = availRadios[i] + "/radioModel";
                radioModel = settings.value(radio, QString()).toString();
                if (radioModel.contains(','))
                {
                    spList = radioModel.split(',');
                    if (spList.count() == 3)
                    {
                        radioModel = spList[1].trimmed() + " " + spList[2].trimmed();
                        settings.setValue(radio, radioModel);

                    }

                }

                // remove redundant settings
                radio = availRadios[i];
                settings.remove(radio + "/radioMfgName");
                settings.remove(radio + "/radioModelName");
                settings.remove(radio + "/radioModelName");
                settings.remove(radio + "/radioModelNumber");
            }
        }
        else if (version == "2")
        {
            bool catPttEnable;

            for (int i = 0; i < numAvailRadios; i++)
            {
                radio = availRadios[i];

                if (settings.contains(radio + "/catEnable"))
                {
                    catPttEnable = settings.value(radio + "/catEnable", true).toBool();
                    settings.remove(radio + "/catEnable");
                    settings.setValue(radio + "/catPttEnable", catPttEnable);
                }



            }


        }

        settings.setValue("Version/version", "3");
}


void RigControlMainWindow::aboutRigConfig()
{
    RigCapabilities rigCap = rigFactory->supported_rigs()->value(currentRadio.rigModel);

    QString msg = QString("*** Rig Configuration ***\n\n");

    msg.append(tr("App Instance Name  = %1\n\n").arg(appName));
    if (radio != nullptr)
    {
        if (radio != nullptr)
        {
            msg.append(tr("Hamlib Version = %1\n").arg(rigStateDetails->rigCap.getDetailedLibraryVersion()));
        }
        else
        {
            msg.append(tr("Unable to determine library version at this time!\n"));
        }

        msg.append(tr("Radio Name = %1\n").arg(currentRadio.radioName));
        msg.append(tr("Radio Number = %1\n").arg(currentRadio.radioNumber));
        msg.append(tr("Rig Model = %1\n").arg(rigCap.getRigModelName()));
        msg.append(tr("Rig Number = %1\n").arg(rigCap.getRigModelNumber()));
        msg.append(tr("Rig Manufacturer = %1\n").arg(rigCap.getRigManufacturer()));
        if (rigCap.getRigManufacturer() == "Icom")
        {
            if (currentRadio.civAddress == "")
            {
                msg.append(tr("Icom CIV address = Using Default CIV Address\n"));
            }
            else
            {
                msg.append(tr("Icom CIV address = %1\n").arg(currentRadio.civAddress));
            }

        }
        msg.append(QString("\n"));

        //msg.append(tr("Rig PortType = %1\n").arg(hamlibData::portTypeList[setupRadio->currentRadio.portType]));

        if (currentRadio.catPortType == RigCapConstants::PortType::network)
        {
            msg.append(tr("Network Address = %1\n").arg(currentRadio.networkAdd));
            msg.append(tr("Network Port = %1\n").arg(currentRadio.networkPort));

        }


        if (currentRadio.rigCtldEnable)
        {
            if (currentRadio.startMinosRigCtld)
            {
               msg.append(tr("Using Minos RigCtld Daemon\n"));
            }
            else
            {
                msg.append(tr("Using external Hamlib rigctld daemon"));
            }



            if (currentRadio.rigCtldNetworkAdd.isEmpty())
            {
               msg.append(tr("Using rigctld default network address.\n"));
            }
            else
            {
               msg.append(tr("Rigctld network address = %1\n").arg(currentRadio.rigCtldNetworkAdd));
            }

            if (currentRadio.rigCtldNetworkPort.isEmpty())
            {
                msg.append(tr("Using rigctld default network address.\n"));
            }
            else
            {
                msg.append(tr("Rigctld port address = %1\n").arg(currentRadio.rigCtldNetworkPort));
            }

            msg.append(tr("Rigctld path = %1\n").arg(getRigCtldExePath()));
            msg.append(tr("Rigctld Connect delay = %1\n").arg(rigCtldDetails->rigCtldConnectDelay));
        }
        else
        {

            if (currentRadio.catPortType == RigCapConstants::PortType::serial)
            {
                msg.append(tr("Comport = %1\n").arg(currentRadio.comport));
                msg.append(tr("Baudrate = %1\n").arg(currentRadio.baudrate));
                msg.append(tr("Data bits = %1\n").arg(currentRadio.databits));
                msg.append(tr("Stop bits = %1\n").arg(QString::number(currentRadio.stopbits)));
                msg.append(tr("Parity = %1\n").arg(serialCommonData::parityStr[static_cast<int>(currentRadio.parity)]));
                msg.append(tr("Handshake = %1\n").arg(serialCommonData::handshakeStr[static_cast<int>(currentRadio.handshake)]));
                msg.append(tr("ForceDTR = %1\n").arg(serialCommonData::forceLinesStr[static_cast<int>(currentRadio.forceDtr)]));
                msg.append(tr("ForceRTS = %1\n").arg(serialCommonData::forceLinesStr[static_cast<int>(currentRadio.forceRts)]));

            }
            else if (currentRadio.catPortType == RigCapConstants::PortType::network)
            {
                    msg.append(tr("Communication Port Type = %1\n").arg("network"));
                    msg.append(tr("Network Address = %1\n").arg(currentRadio.networkAdd));
                    msg.append(tr("Network Port = %1\n").arg(currentRadio.networkPort));

            }
            else if (currentRadio.catPortType == RigCapConstants::PortType::usb)
            {
                    msg.append(tr("Communication Port Type = %1\n").arg("usb"));
            }



        }

        msg.append(QString("\n"));
        msg.append(tr("TransVert Enable = %1\n").arg(currentRadio.transVertEnable ? tr("True") : tr("False")));
        msg.append(tr("Number of TransVerters = %1\n").arg(currentRadio.transVertSettings.count()));

        QStringList tvList = currentRadio.transVertSettings.keys();
        for(const auto &tv: QASCONST(tvList))
        {
            msg.append(QString("\n"));
            msg.append(tr("Transverter %1\n").arg(tv));
            msg.append(tr("Transverter Name = %1\n").arg(currentRadio.transVertSettings.value(tv)->transVertName));
            msg.append(tr("Transverter Band = %1\n").arg(currentRadio.transVertSettings.value(tv)->band));
            msg.append(tr("Transverter Offset = %1\n").arg(currentRadio.transVertSettings.value(tv)->transVertOffset.traceStr()));
            msg.append(tr("Transverter Switch num = %1\n").arg(currentRadio.transVertSettings.value(tv)->transSwitchNum));
            msg.append(tr("Transverter Switch enable = %1\n").arg(currentRadio.enableTransSwitch  ? tr("True") : tr("False")));
        }

        msg.append(QString("\n"));
        msg.append(tr("Radio Supports RIT = %1\n").arg(selectedRadioSupportCap.getSupportSetRit() ? tr("True") : tr("False")));
        if (selectedRadioSupportCap.getSupportSetRit())
        {
            msg.append(tr("Rit Enable On = %1\n").arg(rigStateDetails->ritEnable  ? tr("True") : tr("False")));
            msg.append(tr("Radio Supports Get RIT Freq = %1\n").arg(selectedRadioSupportCap.getSupportGetRit() ? tr("True") : tr("False")));
            msg.append(tr("Radio Supports Set RIT Freq = %1\n").arg(selectedRadioSupportCap.getSupportSetRit() ? tr("True") : tr("False")));
            //msg.append(tr("Radio Supports Get RIT State On/Off = %1\n").arg(radioSupGetRitState ? tr("True") : tr("False")));
            //msg.append(tr("Radio Supports Set RIT State On/Off = %1\n").arg(radioSupRitOnOff ? tr("True") : tr("False")));
        }

        QString pttPortTypeStr = "none";
        if (selectedRadioSupportCap.getSupportPttPortType() == RigCapConstants::RigPttPortType::RIG_PTT_NONE)
        {
            pttPortTypeStr = "serial hardware control lines";
        }
        else if (selectedRadioSupportCap.getSupportPttPortType() == RigCapConstants::RigPttPortType::RIG_PTT_RIG
                 || selectedRadioSupportCap.getSupportPttPortType() == RigCapConstants::RigPttPortType::RIG_PTT_RIG_MICDATA)
        {
            pttPortTypeStr = "cat";     // Note A radio may support CAT, but a user may decide to use RTS or DTR instead
        }
        msg.append(tr("PTT control port Type = %1\n").arg((pttPortTypeStr)));
        msg.append(tr("PTT serial comport = %1").arg(currentRadio.pttSerialPort));
        msg.append(tr("Get PTT = %1\n").arg((selectedRadioSupportCap.getSupportGetPtt() ? "True" : "False")));
        msg.append(tr("Set PTT = %1\n").arg((selectedRadioSupportCap.getSupportSetPtt() ? "True" : "False")));
        msg.append(tr("Get Vox = %1\n").arg((selectedRadioSupportCap.getSupportGetVox() ? "True" : "False")));
        msg.append(tr("Set Vox = %1\n").arg((selectedRadioSupportCap.getSupportSetVox() ? "True" : "False")));
        msg.append(tr("Get Vox = %1\n").arg((selectedRadioSupportCap.getSupportGetVox() ? "True" : "False")));
        msg.append(tr("Get and Set Volume = %1\n").arg((selectedRadioSupportCap.getSupportVolume() ? "True" : "False")));
        msg.append(tr("Get and Set Antenna Switch = %1\n").arg((selectedRadioSupportCap.getSupportAntSw() ? "True" : "False")));
        msg.append(tr("Send Voice Memory = %1\n").arg((selectedRadioSupportCap.getSupportVoiceMemory() ? "True" : "False")));
        if (selectedRadioSupportCap.getSupportVoiceMemory())
        {
            msg.append(tr("Start Voice Memory Message Number = %1\n").arg(selectedRadioSupportCap.getStartVoiceMemoryNumber()));
            msg.append(tr("End Voice Memory Message Number = %1\n").arg(selectedRadioSupportCap.getEndVoiceMemoryNumber()));

        }
        msg.append(tr("Stop Voice Memory = %1\n").arg((selectedRadioSupportCap.getSupportStopVoiceMemory() ? "True" : "False")));
        msg.append(tr("Send CW Memory = %1\n").arg((selectedRadioSupportCap.getSupportCwMemory() ? "True" : "False")));
        if (selectedRadioSupportCap.getSupportCwMemory())
        {
            msg.append(tr("Start CW Memory Message Number = %1\n").arg(selectedRadioSupportCap.getStartCwMemoryNumber()));
            msg.append(tr("End CW Memory Message Number = %1\n").arg(selectedRadioSupportCap.getEndCwMemoryNumber()));
        }
        msg.append(tr("Stop CW Memory = %1\n").arg((selectedRadioSupportCap.getSupportCwMemoryStop() ? "True" : "False")));
        msg.append(tr("wait CW Memory = %1\n").arg((selectedRadioSupportCap.getSupportCwMemoryWait() ? "True" : "False")));


        if (rigCap.getPollData())
        {
            msg.append(tr("Radio Polltime = %1\n").arg(currentRadio.pollInterval));
        }

        msg.append(tr("Tracelog = %1\n").arg(ui->traceDataComms->isChecked() ? tr("True") : tr("False")));
    }
    else
    {
        msg.append(tr("No Radio selected\n"));
    }
    QMessageBox::about(this, tr("Minos RigControl"), msg);
}

void RigControlMainWindow::dumpRadioToTraceLog()
{
    RigCapabilities rigCap = rigFactory->supported_rigs()->value(currentRadio.rigModel);


    if (currentRadio.radioName != "" )
    {
        trace("*** Settings Dump ***");
        trace(QString("App Instance Name  = %1").arg(appName));
        if (radio != nullptr)
        {
           trace(QString("Library Version = %1").arg(rigStateDetails->rigCap.getDetailedLibraryVersion()));
        }
        else
        {
            trace(QString("Radio is not created so not able to determine library version at this time"));
        }

        trace(QString("Radio Name = %1").arg(currentRadio.radioName));
        trace(QString("Radio Number = %1").arg(currentRadio.radioNumber));
        trace(QString("Rig Model = %1").arg(rigCap.getRigModelName()));
        trace(QString("Rig Number = %1").arg(rigCap.getRigModelNumber()));
        trace(QString("Rig Manufacturer = %1").arg(rigCap.getRigManufacturer()));
        if (rigCap.getRigManufacturer() == "Icom")
        {
            if (currentRadio.civAddress == "")
            {
                trace(QString("Icom CIV address = Using Default CIV Address"));
            }
            else
            {
                trace(QString("Icom CIV address = %1").arg(currentRadio.civAddress));
            }

        }

        if (currentRadio.rigCtldEnable)
        {
            if (currentRadio.startMinosRigCtld)
            {
               trace(QString("Using Minos RigCtld Daemon"));
            }
            else
            {
                trace(QString("Using external hamlib rigctld daemon"));
            }



            trace(QString("Using rigctld daemon = %1").arg(currentRadio.rigCtldEnable ? "True" : "False"));
            if (currentRadio.rigCtldNetworkAdd.isEmpty())
            {
               trace(QString("Using rigctld default network address."));
            }
            else
            {
               trace(QString("Rigctld network address = %1").arg(currentRadio.rigCtldNetworkAdd));
            }

            if (currentRadio.rigCtldNetworkPort.isEmpty())
            {
                trace(QString("Using rigctld default network address."));
            }
            else
            {
                trace(QString("Rigctld port address = %1").arg(currentRadio.rigCtldNetworkPort));
            }

            trace(QString("Rigctld path = %1").arg(getRigCtldExePath()));
            trace(QString("Rigctld Connect delay = %1").arg(rigCtldDetails->rigCtldConnectDelay));
        }
        else
        {
            if (currentRadio.catPortType == RigCapConstants::PortType::network)
            {
                trace(QString("Communication Port Type = %1").arg("network"));
                trace(QString("Network Address = %1").arg(currentRadio.networkAdd));
                trace(QString("Network Port = %1").arg(currentRadio.networkPort));

            }
            else if (currentRadio.catPortType == RigCapConstants::PortType::serial)
            {
                trace(QString("Communication Port Type = %1").arg("serial"));
                trace(QString("Comport = %1").arg(currentRadio.comport));
                trace(QString("Data bits = %1").arg(currentRadio.databits));
                trace(QString("Baudrate = %1").arg(currentRadio.baudrate));
                trace(QString("Stop bits = %1").arg(QString::number(currentRadio.stopbits)));
                trace(QString("Parity = %1").arg(serialCommonData::parityStr[static_cast<int>(currentRadio.parity)]));
                trace(QString("Handshake = %1").arg(serialCommonData::handshakeStr[static_cast<int>(currentRadio.handshake)]));
                trace(QString("ForceDTR = %1").arg(serialCommonData::forceLinesStr[static_cast<int>(currentRadio.forceDtr)]));
                trace(QString("ForceRTS = %1").arg(serialCommonData::forceLinesStr[static_cast<int>(currentRadio.forceRts)]));

            }
            else if (currentRadio.catPortType == RigCapConstants::PortType::usb)
            {
                trace(QString("Communication Port Type = %1").arg("usb"));
            }

        }




        trace(QString("Get Supported Bands = %1").arg((selectedRadioSupportCap.getSupportGetSupBands() ? "True" : "False")));
        trace(QString("Get Vfo = %1").arg((selectedRadioSupportCap.getSupportGetVfo() ? "True" : "False")));
        trace(QString("Set Vfo = %1").arg((selectedRadioSupportCap.getSupportSetVfo() ? "True" : "False")));
        trace(QString("Get Rit = %1").arg((selectedRadioSupportCap.getSupportGetRit() ? "True" : "False")));
        trace(QString("Set Rit = %1").arg((selectedRadioSupportCap.getSupportSetRit() ? "True" : "False")));
        trace(QString("Get RitState = %1").arg((selectedRadioSupportCap.getSupportGetRitState() ? "True" : "False")));
        trace(QString("Set RitState = %1").arg((selectedRadioSupportCap.getSupportSetRitState() ? "True" : "False")));
        trace(QString("Get Rit Max Freq = %1").arg((selectedRadioSupportCap.getSupportGetRitMax() ? "True" : "False")));
        trace(QString("Get Signal Strength = %1").arg((selectedRadioSupportCap.getSupportSMeter() ? "True" : "False")));
        QString pttPortTypeStr = "none";
        if (selectedRadioSupportCap.getSupportPttPortType() == RigCapConstants::RigPttPortType::RIG_PTT_NONE)
        {
            pttPortTypeStr = "serial hardware control lines";
        }
        else if (selectedRadioSupportCap.getSupportPttPortType() == RigCapConstants::RigPttPortType::RIG_PTT_RIG
                 || selectedRadioSupportCap.getSupportPttPortType() == RigCapConstants::RigPttPortType::RIG_PTT_RIG_MICDATA)
        {
            pttPortTypeStr = "cat";     // Note A radio may support CAT, but a user may decide to use RTS or DTR instead
        }
        trace(QString("PTT control port Type = %1").arg((pttPortTypeStr)));
        trace(QString("Get PTT = %1").arg((selectedRadioSupportCap.getSupportGetPtt() ? "True" : "False")));
        trace(QString("Set PTT = %1").arg((selectedRadioSupportCap.getSupportSetPtt() ? "True" : "False")));
        trace(QString("Ptt control port Type = %1").arg((pttPortTypeStr)));
        trace(QString("Ptt serial comport = %1").arg(currentRadio.pttSerialPort));
        trace(QString("Get Ptt = %1").arg((selectedRadioSupportCap.getSupportGetPtt() ? "True" : "False")));
        trace(QString("Set Ptt = %1").arg((selectedRadioSupportCap.getSupportSetPtt() ? "True" : "False")));
        trace(QString("Get Vox = %1").arg((selectedRadioSupportCap.getSupportGetVox() ? "True" : "False")));
        trace(QString("Set Vox = %1").arg((selectedRadioSupportCap.getSupportSetVox() ? "True" : "False")));
        trace(QString("Get Vox = %1").arg((selectedRadioSupportCap.getSupportGetVox() ? "True" : "False")));
        trace(QString("Get and Set Volume = %1").arg((selectedRadioSupportCap.getSupportVolume() ? "True" : "False")));
        trace(QString("Get and Set Antenna Switch = %1").arg((selectedRadioSupportCap.getSupportAntSw() ? "True" : "False")));
        trace(QString("Send Voice Memory = %1").arg((selectedRadioSupportCap.getSupportVoiceMemory() ? "True" : "False")));
        if (selectedRadioSupportCap.getSupportVoiceMemory())
        {
            trace(QString("Start Voice Memory Message Number = %1").arg(selectedRadioSupportCap.getStartVoiceMemoryNumber()));
            trace(QString("End Voice Memory Message Number = %1").arg(selectedRadioSupportCap.getEndVoiceMemoryNumber()));

        }
        trace(QString("Stop Voice Memory = %1").arg((selectedRadioSupportCap.getSupportStopVoiceMemory() ? "True" : "False")));
        trace(QString("Send CW Memory = %1").arg((selectedRadioSupportCap.getSupportCwMemory() ? "True" : "False")));
        if (selectedRadioSupportCap.getSupportCwMemory())
        {
            trace(QString("Start Cw Memory Message Number = %1").arg(selectedRadioSupportCap.getStartCwMemoryNumber()));
            trace(QString("End Cw Memory Message Number = %1").arg(selectedRadioSupportCap.getEndCwMemoryNumber()));
        }
        trace(QString("Stop Cw Memory = %1").arg((selectedRadioSupportCap.getSupportCwMemoryStop() ? "True" : "False")));
        trace(QString("wait Cw Memory = %1").arg((selectedRadioSupportCap.getSupportCwMemoryWait() ? "True" : "False")));
        trace(QString("Poll for Radio Data = %1").arg((selectedRadioSupportCap.getPollData() ? "True" : "False")));



        trace(QString("MGM mode = %1").arg(currentRadio.mgmMode));
        trace(QString("RTTY mode = %1").arg(currentRadio.rttyMode));
        trace(QString("PSK mode = %1").arg(currentRadio.pskMode));
        trace(QString("TransVert Enable = %1").arg(currentRadio.transVertEnable ? "True" : "False"));
        trace(QString("Number of TransVerters = %1").arg(currentRadio.transVertSettings.count()));

        QStringList tvList = currentRadio.transVertSettings.keys();
        for(const auto &tv: QASCONST(tvList))
        {
            trace(QString("Transverter %1").arg(tv));
            trace(QString("Transverter Name = %1").arg(currentRadio.transVertSettings.value(tv)->transVertName));
            trace(QString("Transverter Band = %1").arg(currentRadio.transVertSettings.value(tv)->band));
            trace(QString("Transverter Offset = %1").arg(currentRadio.transVertSettings.value(tv)->transVertOffset.traceStr()));
            trace(QString("Transverter Switch num = %1").arg(currentRadio.transVertSettings.value(tv)->transSwitchNum));
            trace(QString("Transverter Switch enable = %1").arg(currentRadio.enableTransSwitch  ? "True" : "False"));
        }
        trace(QString("Radio Supports RIT = %1").arg(selectedRadioSupportCap.getSupportSetRit() ? "True" : "False"));
        if (selectedRadioSupportCap.getSupportSetRit())
        {
            trace(QString("Rit Enable On = %1").arg(rigStateDetails->ritEnable  ? "True" : "False"));
            trace(QString("Radio Supports Get RIT Freq = %1").arg(selectedRadioSupportCap.getSupportGetRit() ? "True" : "False"));
            trace(QString("Radio Supports Set RIT Freq = %1").arg(selectedRadioSupportCap.getSupportSetRit() ? "True" : "False"));
            //trace(QString("Radio Supports Get RIT State On/Off = %1").arg(radioSupGetRitState ? "True" : "False"));
            //trace(QString("Radio Supports Set RIT State On/Off = %1").arg(radioSupRitOnOff ? "True" : "False"));
        }

        if (rigCap.getPollData())
        {
            trace(QString("Radio Polltime = %1").arg(currentRadio.pollInterval));
        }

        trace(QString("Tracelog = %1").arg(ui->traceDataComms->isChecked() ? "True" : "False"));
        trace("*** ***** ***");
    }
    else
    {
        trace(QString("No Radio selected"));
    }
}

/********************************* Supported Radio Display *****************************/

void RigControlMainWindow::initialiseSupportedRadioDisplay()
{
    // table of indicators
    int row = 0;
    int col = 0;
    for (int i = 0; i < bands.count(); i++)
    {
        QLabel *l = new QLabel();
        QPushButton *p = new QPushButton();

        l->setText(bands[i].data()->uk);
        l->setAlignment(Qt::AlignRight);
        p->setMaximumSize(16, 16);
        p->setEnabled(false);

        allSupRadioInd << p;
        allSupRadioIndLabels << l;

        ui->gridLayout->addWidget(l, row, col++);
        ui->gridLayout->addWidget(p, row, col++);

        if (col % 8 == 0)
        {
            row++;
            col = 0;
        }
        QSharedPointer<SupIndicatorDetails> supInd = QSharedPointer<SupIndicatorDetails>(new SupIndicatorDetails());
        supInd->supIndicator = allSupRadioInd[i];
        supInd->supIndicatorLabel = allSupRadioIndLabels[i];
        supInd->bandType = bands[i].data()->getType();
        allBandSupRadioInd.insert(bands[i].data()->uk, supInd);
    }


    turnOffAllsupRadioIndicators();

}


void RigControlMainWindow::showActiveTransVertIndicator(QString cb)
{

    if (cb != selTransVertBandIndicator)
    {
        // turn off previous active transverter indicator
        if (selTransVertBandIndicator != "")
        {
            supRadioIndToggle(selTransVertBandIndicator, displayIndicator::TRANSVERT);
        }

        // turn on new indicator
        supRadioIndToggle(cb, displayIndicator::TRANSVERT_ON);
        selTransVertBandIndicator = cb;

    }
}



void RigControlMainWindow::updateSupportedRadioIndicators()
{
    turnOffAllsupRadioIndicators();

    // turn on supported bands
    if (!currentRadio.radioTransSupBands.isEmpty())
    {
        trace(QString("[updateSupportedRadioIndicators] - bands available for indicators"));
        for (int i = 0; i < currentRadio.radioTransSupBands.count(); i++)
        {
            trace(QString("[updateSupportedRadioIndicators] - turn on Radio Indicator for band = %1").arg(currentRadio.radioTransSupBands[i]));
            supRadioIndToggle(currentRadio.radioTransSupBands[i], displayIndicator::RADIO);
        }
    }
    else
    {
        trace(QString("[updateSupportedRadioIndicators] - radioTransSupBands is empty"));
    }


    // turn on supported transverters
    if (!currentRadio.transVertSettings.isEmpty())
    {
        trace(QString("[updateSupportedRadioIndicators] - transverters available"));
        QStringList tvList = currentRadio.transVertSettings.keys();
        for(const auto &tv: QASCONST(tvList))
        {
            trace(QString("[updateSupportedRadioIndicators] - turn on Transverter Indicator for band = %1").arg(currentRadio.transVertSettings.value(tv)->band));
            supRadioIndToggle(currentRadio.transVertSettings.value(tv)->band, displayIndicator::TRANSVERT);

        }
    }
    else
    {
        trace(QString("[updateSupportedRadioIndicators] - No transverters available"));
    }


}


void RigControlMainWindow::turnOffAllsupRadioIndicators()
{

    for (int i = 0; i < bands.count(); i++)
    {
        supRadioIndToggle(bands[i].data()->uk, displayIndicator::OFF);
    }

}


void RigControlMainWindow::setIndicatorVisible(const QString bandType, const bool visible)
{
    for (QMap<QString, QSharedPointer<SupIndicatorDetails>>::const_iterator i = allBandSupRadioInd.constBegin(); i != allBandSupRadioInd.constEnd(); i++)
    {

        if (i.value()->bandType == bandType)
        {
            i.value()->supIndicator->setVisible(visible);
            i.value()->supIndicatorLabel->setVisible(visible);
        }
    }
}



void RigControlMainWindow::supRadioIndToggle(QString band, displayIndicator::indicatorType type)
{

    if (allBandSupRadioInd.contains(band))
    {
        if (type == displayIndicator::OFF)
        {
            allBandSupRadioInd[band]->supIndicator->setStyleSheet(SUP_RADIO_INDICATOR_OFF_STYLE);
        }
        else if (type == displayIndicator::RADIO)
        {
           allBandSupRadioInd[band]->supIndicator->setStyleSheet(SUP_RADIO_INDICATOR_RADIO_STYLE);
        }
        else if (type == displayIndicator::TRANSVERT)
        {
           allBandSupRadioInd[band]->supIndicator->setStyleSheet(SUP_RADIO_INDICATOR_TRANSVERT_STYLE);
        }
        else if (type == displayIndicator::TRANSVERT_ON)
        {
           allBandSupRadioInd[band]->supIndicator->setStyleSheet(SUP_RADIO_INDICATOR_TRANSVERT_ON_STYLE);
        }
    }
    else
    {
        trace(QString("supRadioIndToggle - band missing from indicator table = %1").arg(band));
    }




}



void RigControlMainWindow::setMemoryGroupVisible(bool visible)
{
    ui->memGroupBox->setVisible(visible);

}

void RigControlMainWindow::setPttGroupItemsVisible(bool visible)
{
    ui->pttGroupBox->setVisible(visible);
    setpttIndVisible(visible);

}

void RigControlMainWindow::setVoiceMemIndVisible(bool visible)
{
    ui->voiceMemInd->setVisible(visible);
    ui->voiceMemLbl->setVisible(visible);
}

void RigControlMainWindow::setVoiceMemIndOnOff(bool state)
{
    if (state)
    {
        ui->voiceMemInd->setStyleSheet(VOICEMEM_INDICATOR_ON);
    }
    else
    {
        ui->voiceMemInd->setStyleSheet(VOICEMEM_INDICATOR_OFF);
    }
}


void RigControlMainWindow::setCwMemIndVisible(bool visible)
{
    ui->cwMemInd->setVisible(visible);
    ui->cwMemLbl->setVisible(visible);
}

void RigControlMainWindow::setCwMemIndOnOff(bool state)
{
    if (state)
    {
        ui->cwMemInd->setStyleSheet(CWMEM_INDICATOR_ON);
    }
    else
    {
        ui->cwMemInd->setStyleSheet(CWMEM_INDICATOR_OFF);
    }
}

void RigControlMainWindow::setpttIndVisible(bool visible)
{
    ui->pttInd->setVisible(visible);
    ui->pttLbl ->setVisible(visible);
    ui->txRxInd->setVisible(visible);
    ui->txRxLbl->setVisible(visible);
}

void RigControlMainWindow::setPttIndOnOff(bool state)
{
    if (state)
    {
        ui->pttInd->setStyleSheet(PTT_INDICATOR_ON);
    }
    else
    {
        ui->pttInd->setStyleSheet(PTT_INDICATOR_OFF);
    }
}

void RigControlMainWindow::setTxRxIndOnOff(bool state)
{
    if (state)
    {
        ui->txRxInd->setStyleSheet(TX_RX_INDICATOR_ON);
    }
    else
    {
        ui->txRxInd->setStyleSheet(TX_RX_INDICATOR_OFF);
    }
}






/************************ Voice Message ************************************************/



// hamlib sendVoiceMessage() funtion uses numbers 0 to max number of voicememory on the radio
// activate voicemessage on most radios. Note! 0 is stop message, and 1 is the first message memory.
// hamlib TS890S, if msgNum is 0, need to call seperate stop_voice_mem function.

// 2023 added a seperate stopVoiceMessage from logger

void RigControlMainWindow::onSetVoiceMessageNum(QString msgNum)
{
    bool ok = false;
    int vmNum = msgNum.toInt(&ok);

    if (radio && ok)
    {
        trace(QString("Send Voice Message number = %1").arg(msgNum));
        radio->sendVoiceMessage(rigStateDetails->curVfo, vmNum);

    }
    else
    {
        trace(QString("send Voice Memory - radio empty, msgNum invalid"));
    }

}

// also use this for stop CW Message
void RigControlMainWindow::onSetStopVoiceMessage(QString msg)
{
    trace(QString("onSetStopVoiceMessage = %1").arg(msg));

    if (radio)
    {

        if (msg == voiceKeyerCommon::STOP_VOICE_MESSAGE)
        {
            if (selectedRadioSupportCap.getSupportStopVoiceMemory())
            {
                if (currentRadio.rigMfg_Name == "YAESU")
                {
                   // hamlib does not have a Yaesu stop_voice_mem function
                   // use sendVoiceMessage with msg number = 0 to stop message.
                   trace(QString("This is a Yaesu radio, send a 0 to stop voice message"));
                   radio->sendVoiceMessage(rigStateDetails->curVfo, 0);

                }
                else
                {
                    trace(QString("This radio supports hamlib stop_voice_mem function"));
                    radio->stop_voice_mem(rigStateDetails->curVfo);
                }

            }
            else
            {
                trace(QString("This radio does not support stop voice mem - send 0 to sendvoicemessage"));
                radio->sendVoiceMessage(rigStateDetails->curVfo, 0);
            }
        }
        else if (msg == voiceKeyerCommon::STOP_CW_MESSAGE)
        {
            if (selectedRadioSupportCap.getSupportStopVoiceMemory())
            {
                trace(QString("This radio supports hamlib stop_morse function"));
                radio->stopMorse(rigStateDetails->curVfo);
            }
        }


    }
}



/**************** CW Message ***************************************************/


void RigControlMainWindow::onSetCwTxMessage(QString cwMsg)
{

    if (selectedRadioSupportCap.getSupportCwMemory() && currentRadio.enableDisableCatFeature.cWMemEnable)
    {
        if (radio && !cwMsg.isEmpty())
        {
           trace(QString("Cw Tx Message Received from logger = %1 for radio %2").arg(cwMsg, currentRadio.rigMfg_Name));
           radio->sendMorse(rigStateDetails->curVfo, cwMsg);
        }
        else
        {
            trace(QString("Cw Tx Message is empty or radio not defined"));
            return;
        }
    }

}



void RigControlMainWindow::handleIcomCwMessage(QString cwMsg)
{

    if (cwMsg.length() == 1)
    {
        trace(QString("Stop Message is = %1").arg(cwMsg));
        QChar c = cwMsg.at(0);
        if (c == QChar(65533))  // QChar value of '\xff'
        {
            // send stop CW
            trace(QString("Icom Cw Tx Message Stop received from logger"));
            radio->stopMorse(rigStateDetails->curVfo);
        }
        else
        {
            // error stop command incorrect!
            trace(QString("Icom Cw Tx Message Stop Char incorrect = %1").arg(c));
            return;
        }


    }
    else
    {
        trace(QString("Icom Cw Tx Message Received from logger = %1").arg(cwMsg));
        radio->sendMorse(rigStateDetails->curVfo, cwMsg);
    }

}



void RigControlMainWindow::handleYaesuCwMessage(QString cwMsg)
{
    // Yaesu does not support a CW Message stop command
    trace(QString("Yaesu Cw Tx Message Received from logger = %1").arg(cwMsg));
    radio->sendMorse(rigStateDetails->curVfo, cwMsg);
}







/*********************************** test *********************************************/


void RigControlMainWindow::setTestMode(bool test)
{
    trace(QString("testMode is %1 test parameter is %2").arg(testMode).arg(test));
    if (test)
    {
        ui->testActionsGroupBox->setVisible(true);

        if (!testMode)
        {
            testMode = true;
            liveRadio = currentRadioName;
            trace("save liveRadio " + liveRadio);

            if (hamlibOk)
            {
                updateSelectRadioBox();     // we don't want to update radiolist if hamlib version is incorrect
            }

            if (appName.isEmpty())
            {
                // we are running local
                ui->selectRadioFromLoggerRb->setVisible(false);
                ui->selectRadioFromRigControlRb->setVisible(false);
                logMessage((QString("Test Radio StandAlone Mode")));
            }
            else
            {
                ui->selectRadioFromLoggerRb->setChecked(true);
                setSelectRadioBoxVisible(false);
                upDateRadio(liveRadio);
                logMessage((QString("Test Radio Logger Mode")));
            }
        }



    }
    else
    {
        if (testMode)
        {
            trace("restore liveRadio " + liveRadio);;
            testMode = false;
            upDateRadio(liveRadio);
            msg->rigCache.invalidate();
            ui->testActionsGroupBox->setVisible(false);
        }

        logMessage((QString("Radio Selection for Current Radio, for AppName %1, will be from logger").arg(appName)));


    }
    //setSelectRadioBoxVisible(testMode);
    //testBoxesVisible(testMode);
    //setRadioNameLabelVisible(!testMode);





}

void RigControlMainWindow::on_testRadioButton_clicked()
{
    setTestMode(!testMode);
}


void RigControlMainWindow::loadTestModeCombo()
{
    QString fileName = RADIO_PATH_LOGGER() + FILENAME_RIGCONTROL_TEST_DATA;
    QSettings  config(fileName, QSettings::IniFormat);
    config.beginGroup("MODES");

    QString modes = config.value("modeComboList", "USB,LSB,CW,FM,MGM,RY,PS").toString();
    config.endGroup();

    QStringList modeList = modes.split(',');
    ui->testModeComboBox->addItems(modeList);

}


void RigControlMainWindow::selFreqClicked()
{
    // check freq valid format
    static QRegularExpression re("^[0]*");
    QString f = ui->freqInputBox->text().trimmed().remove( re);

    if (valInputFreq(f, tr("Invalid freq!")))
    {
        // convert radio freq
        f = convertSinglePeriodFreqToFullDigit(f).remove('.');
        setFreq(f, rigStateDetails->curVfo);
    }
}

void RigControlMainWindow::setTestControlsVisible(bool visible)
{

    setRitTestControlsVisible(visible);
    setPttTestControlsVisible(visible);
    setCwMemTestControlsVisible(visible);
    setVoiceMemTestControlsVisible(visible);
}


void RigControlMainWindow::setTestModeControlsVisible(bool visible)
{
    ui->testModeComboBox->setVisible(visible);
    ui->modeTestLabel->setVisible(visible);
}


void RigControlMainWindow::setRitTestControlsVisible(bool visible)
{
    ui->setRitSpinner->setVisible(visible);
    ui->testRitButton->setVisible(visible);
}


void RigControlMainWindow::setPttTestControlsVisible(bool visible)
{
    ui->txPttTestPb->setVisible(visible);
}

void RigControlMainWindow::setCwMemTestControlsVisible(bool visible)
{
    ui->cwKeyerPb->setVisible(visible);
    if (visible)
    {
        if (!selectedRadioSupportCap.getSupportCwMemoryStop())
        {
            ui->cwKeyerStopPb->setVisible(visible);
        }
    }
    else
    {
       ui->cwKeyerStopPb->setVisible(visible);
    }



}


void RigControlMainWindow::setVoiceMemTestControlsVisible(bool visible)
{
    ui->voiceMessageTestLabel->setVisible(visible);
    ui->voiceMessageSpinBox->setVisible(visible);
    ui->voiceMessagePlayPB->setVisible(visible);
    ui->voiceMessageStopPB->setVisible(visible);
}


// Test CW Message Playback on radio.


void RigControlMainWindow::onCwKeyerPbClicked()
{
    if (selectedRadioSupportCap.getSupportCwMemory() && currentRadio.enableDisableCatFeature.cWMemEnable)
    {
        if (radioCommsOK)
        {
            // we probably don't need to check the manfacturer as we have checked the cw memory capability
            QString rigManufacturer = currentRadio.rigMfg_Name;
            if (rigManufacturer == "Yaesu"
                || rigManufacturer ==  "Kenwood"
                || rigManufacturer == "Icom"
                || rigManufacturer == "Elecraft"
                || rigManufacturer == "Flex-radio"
                || rigManufacturer == "OpenHPSDR"
                || rigManufacturer == "Flex-radio/Apache"
                || rigManufacturer == "QRPLabs"
                || rigManufacturer == "Thetis")
            {
                // This is an optional ini to allow manufacturer specific messages
                QString fileName = RADIO_PATH_LOGGER() + FILENAME_RIGCONTROL_TEST_DATA;
                QSettings  config(fileName, QSettings::IniFormat);

                rigManufacturer.replace('/', '_');  // can't have / in group name

                config.beginGroup(rigManufacturer);

                QString cwMsg = config.value("cwTestMessage", "CQ CQ DE M0ABC").toString();
                config.endGroup();

                //int cwTXTimeoutDur = config.value("cwTxMessageTimeout", 25).toInt();

                if (cwMsg.isEmpty())
                {
                    trace("Test CW message empty for radio " + rigManufacturer);
                    return;
                }

                if (radio)
                {
                    if (rigStateDetails->curModeStr != hamlibData::CW)
                    {
                        setMode(hamlibData::CW, rigStateDetails->curVfo);
                    }

                    trace("Sending test CW Message: " + cwMsg);
                    radio->sendMorse(rigStateDetails->curVfo, cwMsg);
                    return;
                }
            }

        }
    }

    trace("Error sending test CW Message! Radio not connected or CW Message not supported");

}


void RigControlMainWindow::onCwKeyerStopPbClicked()
{
    if(radio)
    {
        radio->stopMorse(rigStateDetails->curVfo);
    }
}


bool RigControlMainWindow::isTestCwMessageRunning()
{
    if (cwMessageTestTimer->isActive())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void RigControlMainWindow::onCWMessageTimerTimeout()
{

}

void RigControlMainWindow::onTxPttTestPbClicked()
{

    if (radio)
    {
        if (rigStateDetails->curPttStatus)
        {
            rigStateDetails->curPttStatus = false;

        }
        else
        {
            rigStateDetails->curPttStatus = true;

        }

        onSetPttOnOff(rigStateDetails->curPttStatus);
        setTestPttButtonIndOnOff(rigStateDetails->curPttStatus);
        setRigControlPttState(rigStateDetails->curPttStatus);
    }
}

void RigControlMainWindow::setTestPttButtonIndOnOff(bool state)
{
    if (state)
    {
        ui->txPttTestPb->setStyleSheet(TX_RX_INDICATOR_ON);
    }
    else
    {
        ui->txPttTestPb->setStyleSheet(BACKGROUND_GREY);
    }
}

/*
void RigControlMainWindow::testIncRit(int )
{

        bool ok;
        QString sFreq = ui->setRitSpinner->text();
        ShortFreq dFreq = sFreq.toShort(&ok);
        if (ok)
        {
            setRitFreq(curVfo, dFreq);
        }
        else
        {
            trace(QString("testIncRit: Invalid incRIT value = %1").arg(sFreq));
        }


}
*/

void RigControlMainWindow::testIncRit(int value)
{


        setRitFreq(value);

}


void RigControlMainWindow::ritbuttontoggle()
{
    if (radio)
    {
        bool status = !rigStateDetails->radioRitOn;
        int retCode = 0;
        retCode = radio->setRitState(rigStateDetails->curVfo, status);
        if (retCode < 0)
        {
            trace(QString("testRitButtontoggle: Rit on/off toggle error, code = %1").arg(retCode));
        }
    }

}


void RigControlMainWindow::onSelectRadioFromLoggerClicked()
{
    if (ui->selectRadioFromLoggerRb->isChecked())
    {
       setSelectRadioBoxVisible(false);
       closeRadio();
    }

}

void RigControlMainWindow::onTestModeComboBoxTextChanged(const QString testMode)
{
    trace(QString("**** test mode combo selected - %1 ****").arg(testMode));
    trace(QString("Calling loggerSetMode"));
    loggerSetMode(testMode);
}

void RigControlMainWindow::onSelectRadioFromRigControlClicked()
{
    if (ui->selectRadioFromRigControlRb->isChecked())
    {
        setSelectRadioBoxVisible(true);
        closeRadio();
    }

}

void RigControlMainWindow::onVoiceMessageSpinBoxTextChanged()
{

}

void RigControlMainWindow::onVoiceMessagePlayClicked()
{

}


void RigControlMainWindow::onVoiceMessageStopClicked()
{

}

void RigControlMainWindow::on_reconnectButton_clicked()
{
    upDateRadio(currentRadioName);
}

void RigControlMainWindow::on_traceDataComms_stateChanged(int /*arg1*/)
{
    saveTraceLogFlag(ui->traceDataComms->isChecked());
}
