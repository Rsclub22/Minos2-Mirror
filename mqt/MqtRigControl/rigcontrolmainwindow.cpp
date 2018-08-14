/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
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
#include <QTimer>
#include <QMessageBox>
#include <QProcessEnvironment>


#include <QDebug>




RigControlMainWindow::RigControlMainWindow(QWidget *parent) :
    QMainWindow(parent)
  , ui(new Ui::RigControlMainWindow)
{
    ui->setupUi(this);

    ui->progressBar->setRange(0, 120);
    ui->progressBar->setTextVisible(false);

    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();


    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appName = env.value("MQTRPCNAME", "") ;

    createCloseEvent();

    connect(&LogTimer, SIGNAL(timeout()), this, SLOT(LogTimerTimer()));
    LogTimer.start(100);

    msg = new RigControlRpc(this);

    QSettings settings;
    geoStr = "geometry";
    if (appName.length() > 0)
    {
        geoStr = geoStr + appName;
    }

    if (appName.length() > 0)
    {
        testBoxesVisible(false);
    }
    else
    {
        testBoxesVisible(true);
    }

    QByteArray geometry = settings.value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    radio = new RigControl();
    radio->getRigList();

    loadBands();
    FreqPresetDialog::readSettings(presetFreq);

    setupRadio = new RigSetupDialog(radio, bands);
    setupRadio->setAppName(appName);

    if (appName.length() > 0)
    {
        // connected to logger don't show radio selectbox
        setSelectRadioBoxVisible(false);
        setRadioNameLabelVisible(true);
    }
    else
    {
        setSelectRadioBoxVisible(true);
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
    sendTransVertSwitchToComPort(TRANSSW_NUM_DEFAULT);

    logRitOn = false;
    setRitOnOffDisplayVisible(logRitOn);
    setRitOnOffDisplay(logRitOn);




    if (appName.length() > 0)
    {
        logMessage((QString("Read Current Radio for AppName %1 from logger").arg(appName)));

    }
    else
    {
        logMessage((QString("Read Current Radio for Local selection")));

        setupRadio->readCurrentRadio();

        if (setupRadio->currentRadioName == "")
        {
            logMessage(QString("No radio selected for this appName, %1").arg(appName));
            QString errmsg = "<font color='Red'>Please select a radio!</font>";
            showStatusMessage(errmsg);
            sendStatusLogger(errmsg);
        }
        else
        {
            ui->selectRadioBox->setCurrentText(setupRadio->currentRadioName);
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
    if (ui->actionTrace_Log->isChecked())
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
    if (cmd.indexOf("ShowServers", Qt::CaseInsensitive) >= 0)
        setShowServers(true);
    if (cmd.indexOf("HideServers", Qt::CaseInsensitive) >= 0)
        setShowServers(false);
}

void RigControlMainWindow::initActionsConnections()
{
    connect(ui->selectRadioBox, SIGNAL(activated(int)), this, SLOT(selectRadio()));
    connect(ui->actionSetup_Radios, SIGNAL(triggered()), this, SLOT(onLaunchSetup()));
    connect(ui->actionSetup_Band_Freq, SIGNAL(triggered(bool)), this, SLOT(setupBandFreq()));
    connect(ui->actionTrace_Log, SIGNAL(changed()), this, SLOT(saveTraceLogFlag()));

    connect(pollTimer, SIGNAL(timeout()), this, SLOT(getRadioInfo()));


    // configure antenna dialog
    connect(setupRadio, SIGNAL(currentRadioSettingChanged(QString)), this, SLOT(currentRadioSettingChanged(QString)));
    connect(setupRadio, SIGNAL(radioNameChange()), this, SLOT(updateSelectRadioBox()));
    connect(setupRadio, SIGNAL(radioTabChanged()), this, SLOT(updateSelectRadioBox()));


    // Message from Logger
    connect(msg, SIGNAL(setFreq(QString)), this, SLOT(loggerSetFreq(QString)));
    connect(msg, SIGNAL(setRitFreq(QString)), this, SLOT(setRitFreqStr(QString)));
    connect(msg, SIGNAL(setRitStatus(bool)), this, SLOT(setRitLogStatus(bool)));
    connect(msg, SIGNAL(setMode(QString)), this, SLOT(loggerSetMode(QString)));
    connect(msg, SIGNAL(selectLoggerRadio(PubSubName, QString)), this, SLOT(onSelectRadio(PubSubName, QString)));
    connect(msg, SIGNAL(setTpm(int, QString)), this, SLOT(setTpm(int, QString)));
    connect(msg, SIGNAL(setVolume(int)), this, SLOT(loggerSetVolume(int)));

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAbout_Radio_Config, SIGNAL(triggered()), this, SLOT(aboutRigConfig()));

    connect(radio, SIGNAL(debug_protocol(QString)), this, SLOT(logMessage(QString)));

    // standalone test
    connect(ui->selFreq, SIGNAL(clicked(bool)), this, SLOT(selFreqClicked()));
    connect(ui->freqInputBox, SIGNAL(editingFinished()), this, SLOT(selFreqClicked()));



}

void RigControlMainWindow::setupBandFreq()
{

    FreqPresetDialog  fPresetDialog(presetFreq, bands, freqPresetChanged);

    fPresetDialog.exec();

    if (!freqPresetChanged)
    {
        logMessage(QString("RigControl: Band Freq Change, send new bandlist to logger"));
        sendBandListLogger();
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

    if (appName.length() > 0)
    {
        sendRadioListLogger();

    }
}

void RigControlMainWindow::selectRadio()
{
    setupRadio->currentRadioName = ui->selectRadioBox->currentText();
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
    logMessage(QString("UpdateRadio: Index Selected = %1").arg(QString::number(ui->selectRadioBox->currentIndex())));

    pollTimer->stop();      // stop updates

    int ridx = 0;
    if (setupRadio->currentRadioName != "")
    {
        radioIndex = setupRadio->findCurrentRadio(setupRadio->currentRadioName);
        ridx = radioIndex;
        if (ridx > -1 && ridx < setupRadio->numAvailRadios)  // find radio and update current radio pointer
        {
            // found antenna, update currentAntenna  to selected antennadata
            scatParams::copyRig(setupRadio->availRadioData[ridx], setupRadio->currentRadio);
            setupRadio->currentRadio.radioNumber = QString::number(ridx);           // save radio number

            if (radio->get_serialConnected())
            {
                closeRadio();
            }

            if (setupRadio->currentRadio.radioModelNumber == 0)
            {
                closeRadio();
                QMessageBox::critical(this, tr("Radio Error"), "Please configure a radio name and model");
                return;
            }

            // setup local serial transvert switch
            if (setupRadio->currentRadio.transVertEnable
                    && setupRadio->currentRadio.enableTransSwitch
                    && setupRadio->currentRadio.enableLocTVSwMsg)
            {
                if (serialTVSw != nullptr)
                {
                    serialTVSw->closeComport();
                }
                serialTVSw = new SerialTVSwitch(setupRadio->currentRadio.locTVSwComport);
            }

            // only show transvert freq box is enabled
            setTransVertDisplayVisible(setupRadio->currentRadio.transVertEnable);
            sendTransVertStatus(setupRadio->currentRadio.transVertEnable);   // send to logger
            sendTransVertSwitchToLogger(TRANSSW_NUM_DEFAULT);                                 // turn off transVerter Sw
            sendTransVertSwitchToComPort(TRANSSW_NUM_DEFAULT);

            //setupRadio->currentRadio.transVertNegative = setupRadio->availRadioData[ridx]->transVertNegative;
            //setupRadio->currentRadio.transVertOffset = setupRadio->availRadioData[ridx]->transVertOffset;
            //setupRadio->currentRadio.transVertOffsetStr = setupRadio->availRadioData[ridx]->transVertOffsetStr;
            //setupRadio->currentRadio.mgmMode = setupRadio->availRadioData[ridx]->mgmMode;

            setupRadio->saveCurrentRadio();

            openRadio();

            if (radio->get_serialConnected())
            {
                ui->radioNameDisp->setText(setupRadio->currentRadio.radioName);

                if (radio->get_serialConnected())
                {
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
                        logMode = radio->convertQStrMode("USB");
                        setMode("USB", RIG_VFO_CURR);

                    }
                }

                if (radio->get_serialConnected())
                {

                    supVolume = radio->supportVolControl() ? true : false;

                    supSignalStrength = radio->supportSignalStrength() ? true : false;


                    writeWindowTitle(appName);
                    sendStatusToLogConnected();
                    sendBandListLogger();
                    if (setupRadio->currentRadio.ritAvail && setupRadio->currentRadio.ritEnable)
                    {
                        setRitDisplayVisible(true);
                        setRitOnOffDisplayVisible(true);
                    }
                    else
                    {
                        setRitDisplayVisible(false);
                        setRitOnOffDisplayVisible(false);
                    }
                    sendRitEnableStatusLogger();
                    dumpRadioToTraceLog();

                }
                else
                {
                    trace(QString("#### Radio Failed to connect ####"));
                    sendStatusToLogDisConnected();
                }

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
        if (radio->get_serialConnected())
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


void RigControlMainWindow::openRadio()
{

    int retCode = 0;

    if (setupRadio->currentRadio.radioName == "")
    {
        logMessage(QString("Open Radio: No radio name!"));
        showStatusMessage("Please select a Radio");
        return;
    }

    logMessage(QString("Open Radio: Opening Radio %1 PortType %2").arg(setupRadio->currentRadio.radioName).arg(hamlibData::portTypeList[setupRadio->currentRadio.portType]));
    showStatusMessage(QString("Opening Radio: %1").arg(setupRadio->currentRadio.radioName));

    if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_SERIAL)
    {
        if(setupRadio->comportAvial(setupRadio->currentRadio.radioNumber.toInt(), setupRadio->currentRadio.comport) == -1)
        {
            logMessage(QString("Open Radio: Check comport - defined port %1 not available on computer").arg(setupRadio->currentRadio.comport));
            showStatusMessage(QString("Comport %1 no longer configured on computer?").arg(setupRadio->currentRadio.comport));
            return;
        }

        if (setupRadio->currentRadio.comport == "")
        {
            logMessage(QString("Open Radio: No comport"));
            showStatusMessage("Please select a Comport");
            return;
        }

    }

    if (rig_port_e(setupRadio->currentRadio.portType) == RIG_PORT_NETWORK || rig_port_e(setupRadio->currentRadio.portType == RIG_PORT_UDP_NETWORK))
    {
        if (setupRadio->currentRadio.networkAdd == "" || (setupRadio->currentRadio.networkPort == ""))
        {
            logMessage(QString("Open Radio: No network or Port Number"));
            showStatusMessage("Please enter a network Address and Port Number");
            return;
        }

    }
    if (setupRadio->currentRadio.radioModel == "")
    {
        logMessage(QString("Open Radio: No radio model"));
        showStatusMessage("Please select a radio model");
        return;
    }


    if (!radio->get_serialConnected())
    {
        // if radio is already open, don't reinit it
        retCode = radio->init(setupRadio->currentRadio);
        if (retCode < 0)
        {
            logMessage(QString("Error Opening Radio Error Code = %1").arg(QString::number(retCode)));
            hamlibError(retCode, "Open Radio");
        }
    }

    if (radio->get_serialConnected())
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
            showStatusMessage(QString("Connected: %1 - %2, %3:%4").arg(setupRadio->currentRadio.radioName).arg(setupRadio->currentRadio.radioModel.trimmed()).arg(setupRadio->currentRadio.networkAdd).arg(setupRadio->currentRadio.networkPort));
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
    }

    PubSubName psname(setupRadio->currentRadio.radioName);

    msg->rigCache.setTpm(psname, 1);
    msg->rigCache.publish();
}

void RigControlMainWindow::closeRadio()
{

    if (radio->get_serialConnected())
    {
        radio->closeRig();
    }
    if (serialTVSw != nullptr)
    {
        serialTVSw->closeComport();
        serialTVSw = nullptr;
    }

    showStatusMessage("Disconnected");
    sendStatusToLogDisConnected();
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
    if (radio->get_serialConnected())
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

    if (radio->get_serialConnected())
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




    if (radio->get_serialConnected() && setupRadio->currentRadio.ritEnable && setupRadio->currentRadio.ritEnable)
    {
        logMessage((QString("Get RIT")));
        retCode = getRitFreq(RIG_VFO_CURR);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get RIT error").arg(QString::number(retCode)));
            hamlibError(retCode, "Request RIT");
        }
    }

    if (radio->get_serialConnected() && supVolume)
    {
        retCode = getVolume(RIG_VFO_CURR);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Volume error").arg(QString::number(retCode)));
            hamlibError(retCode, "Request Volume");
        }

    }


    if (radio->get_serialConnected() && supSignalStrength)
    {
        retCode = getSignalStrength(RIG_VFO_CURR);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Get radioInfo: Get Volume error").arg(QString::number(retCode)));
            hamlibError(retCode, "Request Volume");
        }

    }


    msg->rigCache.publish();
}


void RigControlMainWindow::onSelectRadio(PubSubName s, QString mode)
{

    logMessage(QString("Recieved SelectRadio from Logger = %1, mode = %2").arg(s.toString()).arg(mode));


    if (!mode.isEmpty())
    {
        selRadioMode = mode;
    }

    QString oldRadio = setupRadio->currentRadioName;

    setupRadio->currentRadioName = s.key();

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
    if (radio->get_serialConnected() && !rigErrorFlag)
    {
        logMessage(QString("new freq %1, old freq %2, old tpm %3").arg(freq).arg(logger_freq).arg(tpm));
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
    //msg->rigCache.publish();
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
            // BIG delays this way!
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
                //setTransVertDisplayVisible(true);
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
                selTvBand = "";
                displayTransVertVfo(0.0);
                ui->transVertBandDisp->setText("");
                transVertSwNum = TRANSSW_NUM_DEFAULT;
                ui->transVertSwNum->setText(TRANSSW_NUM_DEFAULT);
                sendTransVertSwitchToLogger(TRANSSW_NUM_DEFAULT);
                sendTransVertSwitchToComPort(TRANSSW_NUM_DEFAULT);
                //setTransVertDisplayVisible(true);
            }

        }

        if (radio->get_serialConnected())
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
            displayModeVfo(radio->convertModeQstr(rmode));
            displayPassband(rwidth);
            sendModeToLog(QString("%1:%2").arg(radio->convertModeQstr(rmode)).arg(" "));
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


    if (radio->get_serialConnected() && !rigErrorFlag)
    {
        slogMode = mode;
        logMode = radio->convertQStrMode(mode);


        if (slogMode == hamlibData::MGM)
        {
            logMessage(QString("Log SetMode:MGM mode Selected"));
            if (mgmModeFlag)
            {
                logMessage(QString("Log SetMode: Mgm flag is set"));
                if (curMode !=  radio->convertQStrMode(setupRadio->currentRadio.mgmMode))
                {
                    setMode(setupRadio->currentRadio.mgmMode, RIG_VFO_CURR);
                }
            }
            else
            {
                mgmModeFlag = true;
                setMode(setupRadio->currentRadio.mgmMode, RIG_VFO_CURR);
                logMessage((QString("Log SetMode: Set MgmMode Flag, Selected MGM Mode = %1").arg(setupRadio->currentRadio.mgmMode)));
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
    if (mode == "")
    {
        logMessage(QString("SetMode: Mode is blank!"));
        return;
    }

    int retCode = 0;

    cmdLockOn();      // lock get radio info

    mode = mode.left(mode.indexOf(":"));
    rmode_t mCode = radio->convertQStrMode(mode);

    if (radio->get_serialConnected())
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

    setVolume(RIG_VFO_CURR, level);

}

/************************** RIT *********************************/

void RigControlMainWindow::setRitDisplayVisible(bool state)
{
    ui->ritLbl->setVisible(state);
    ui->ritFreq->setVisible(state);
}


int RigControlMainWindow::getRitFreq(vfo_t vfo)
{
    int retCode = 0;


    retCode = radio->getRit(vfo, &rRitFreq);
    if (retCode == RIG_OK)
    {
        ui->ritFreq->setText(convertRitFreqToStr(rRitFreq));
    }
    return retCode;
}


void RigControlMainWindow::setRitFreqStr(QString ritFreq)
{
    if (setupRadio->currentRadio.ritEnable)
    {
        int retCode = 0;
        shortfreq_t rFreq = ritFreq.toLong();
        retCode = setRitFreq(RIG_VFO_CURR, rFreq);
        if (retCode < 0)
        {
            // error
            logMessage(QString("Set RIT freq error").arg(QString::number(retCode)));
            hamlibError(retCode, "Set RIT Freq.");
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

void RigControlMainWindow::sendRitFreqLogger(double ritFreq)
{
    if (appName.length() > 0)
    {
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setRitFreq(psname, ritFreq);
        logMessage(QString("Send Rit freq to logger = %1 psn=%2").arg(convertRitFreqToStr(ritFreq)).arg(psname.toString()));

    }
}

void RigControlMainWindow::setRitLogStatus(bool status)
{
    logRitOn = status;
    setRitOnOffDisplay(logRitOn);
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
        qDebug() << "get vol = " << vol;
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
    int retCode = 0;
    float volLevel = level;
    volLevel = volLevel/VOLMULT;
    qDebug() << "level is = " << volLevel;
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
            qDebug() << "Signal Strength = " << curSignalStrength;
            displaySignalStrength(curSignalStrength);
        }
    }

    return retCode;

}


void RigControlMainWindow::displaySignalStrength(int level)
{


    int signal = level + 60;
    qDebug() << "meter level = "  << signal;
    ui->progressBar->setValue(signal);

    // display text

    if (level == 60)
    {
        ui->labelsmeter->setText("S9+60");
        return;
    }
    else if ((level >= 50) & (level < 60))
    {
        ui->labelsmeter->setText("S9+50");
        return;
    }
    else if ((level >= 40) & (level < 50))
    {
        ui->labelsmeter->setText("S9+40");
        return;
    }
    else if ((level >= 30) & (level < 40))
    {
        ui->labelsmeter->setText("S9+30");
        return;
    }
    else if ((level >= 20) & (level < 30))
    {
        ui->labelsmeter->setText("S9+20");
        return;
    }
    else if ((level >= 10) & (level < 20))
    {
        ui->labelsmeter->setText("S9+10");
        return;
    }
    else if ((level >= 0) & (level < 10))
    {
        ui->labelsmeter->setText("S9   ");
        return;
    }
    else if ((level >= -6) & (level < 0))
    {
        ui->labelsmeter->setText("S8   ");
        return;
    }
    else if ((level >= -12) & (level < -6))
    {
        ui->labelsmeter->setText("S7   ");
        return;
    }
    else if ((level >= -18) & (level < -12))
    {
        ui->labelsmeter->setText("S6   ");
        return;
    }
    else if ((level >= -22) & (level < -18))
    {
        ui->labelsmeter->setText("S5   ");
        return;
    }
    else if ((level >= -28) & (level < -22))
    {
        ui->labelsmeter->setText("S4   ");
        return;
    }
    else if ((level >= -32) & (level < -28))
    {
        ui->labelsmeter->setText("S3   ");
        return;
    }
    else if ((level >= -38) & (level < -32))
    {
        ui->labelsmeter->setText("S2   ");
        return;
    }
    else if ((level >= -42) & (level < -38))
    {
        ui->labelsmeter->setText("S1   ");
        return;
    }
    else if ((level >= -48) & (level < -42))
    {
        ui->labelsmeter->setText(QString::number(level) + "  ");
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


    ui->actionTrace_Log->setChecked(config.value("TraceLog", false).toBool());

    config.endGroup();
}

void RigControlMainWindow::saveTraceLogFlag()
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

    config.setValue("TraceLog", ui->actionTrace_Log->isChecked());

    config.endGroup();
    trace("Tracelog Changed in " + fileName + " = " + QString::number(ui->actionTrace_Log->isChecked()));
}

void RigControlMainWindow::about()
{
    QMessageBox::about(this, "Minos RigControl", "Minos QT RigControl\nCopyright D Balharrie G8FKH/M0DGB 2017");
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

void RigControlMainWindow::sendBandListLogger()
{
    QString fileName;
    fileName = RADIO_PATH_LOGGER + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("FreqPresets");

    QStringList bandList;
    if (!setupRadio->currentRadio.radioTransSupBands.isEmpty())
    {
        for (int i = 0; i < setupRadio->currentRadio.radioTransSupBands.count(); i++)
        {
            // get index to band
            int k = 0;
            bool match = false;
            do
            {
                if (setupRadio->currentRadio.radioTransSupBands[i] == freqPresetData::presetBands[k])
                {
                    match = true;
                    break;
                }
                k++;
            }while (k < freqPresetData::presetBands.count() && !match);

            bandList.append(QString("%1-%2").arg(setupRadio->currentRadio.radioTransSupBands[i])
                            .arg(config.value(setupRadio->currentRadio.radioTransSupBands[i], freqPresetData::bandFreq[k]).toString()));
        }


        PubSubName psname(setupRadio->currentRadio.radioName);
        QString bands = bandList.join(":");
        logMessage(QString("Send bandlist to logger - %1").arg(bands));
        msg->rigCache.setBandList(psname, bands);
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
        msg->rigCache.setFreq(psname, freq);
        logMessage(QString("Send freq to logger = %1 psn=%2").arg(convertFreqToStr(freq)).arg(psname.toString()));
    }
}

void RigControlMainWindow::sendModeToLog(QString mode)
{
    if (appName.length() > 0)
    {
        logMessage(QString("Send mode to logger = %1").arg(mode));
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setMode(psname, mode);
    }
}

void RigControlMainWindow::sendVolToLog(int level)
{
    if (appName.length() > 0)
    {
        logMessage(QString("Send volume to logger = %1").arg(QString::number(level)));
        PubSubName psname(setupRadio->currentRadio.radioName);
        msg->rigCache.setVolume(psname, level);
    }
}


void RigControlMainWindow::sendTransVertStatus(bool status)
{
    //QString flag;
    if (appName.length() > 0)
    {
        /*if (status)
        {
            flag = TXVERT_ON;
        }
        else
        {
            flag = TXVERT_OFF;
        }*/
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
    logMessage(QString("Send Transvert Switch Number to logger = %1").arg(swNum));
    PubSubName psname(setupRadio->currentRadio.radioName);
    msg->rigCache.setTransverterSwitch(psname, swNum.toInt());
}


void RigControlMainWindow::sendTransVertSwitchToComPort(const QString &swNum)
{
    QByteArray msg = swNum.toUtf8();
    msg.prepend(TVSWMSG_START);
    msg.append(TVSWMSG_TERM);

    if (setupRadio->currentRadio.transVertEnable
            && setupRadio->currentRadio.enableTransSwitch
            && setupRadio->currentRadio.enableLocTVSwMsg)
    {
        serialTVSw->sendTVSwMessage(msg);
    }
}


void RigControlMainWindow::sendRitEnableStatusLogger()
{
    if (!setupRadio->currentRadio.ritAvail)
    {
        sendRitEnableStatus(setupRadio->currentRadio.ritAvail); // turn off rit in logger if not available
    }
    else
    {
        sendRitEnableStatus(setupRadio->currentRadio.ritEnable);
    }

}
void RigControlMainWindow::sendTpm(int tpm)
{
    logMessage(QString("Send Tpm to logger = %1").arg(tpm));
    PubSubName psname(setupRadio->currentRadio.radioName);
    msg->rigCache.setTpm(psname, tpm);
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


void RigControlMainWindow::setRitOnOffDisplayVisible(bool s)
{
    ui->RitOnOffLbl->setVisible(s);
    ui->RitOnffStatLbl->setVisible(s);
}

void RigControlMainWindow::setRitOnOffDisplay(bool s)
{
    if (s)
    {
        ui->RitOnffStatLbl->setText("On");
    }
    else
    {
        {
            ui->RitOnffStatLbl->setText("Off");
        }
    }
}

void RigControlMainWindow::onLaunchSetup()
{
    setupRadio->setTabToCurrentRadio();

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
        msg.append(QString("Rig PortType = %1\n").arg(hamlibData::portTypeList[setupRadio->currentRadio.portType]));
        msg.append(QString("Network Address = %1\n").arg(setupRadio->currentRadio.networkAdd));
        msg.append(QString("Network Port = %1\n").arg(setupRadio->currentRadio.networkPort));
        msg.append(QString("Comport = %1\n").arg(setupRadio->currentRadio.comport));
        msg.append(QString("Baudrate = %1\n").arg(setupRadio->currentRadio.baudrate));
        msg.append(QString("Stop bits = %1\n").arg(QString::number(setupRadio->currentRadio.stopbits)));
        msg.append(QString("Parity = %1\n").arg(radio->getParityCodeNames()[setupRadio->currentRadio.parity]));
        msg.append(QString("Handshake = %1\n").arg(radio->getHandShakeNames()[setupRadio->currentRadio.handshake]));
        msg.append(QString("TransVert Enable = %1\n").arg(setupRadio->currentRadio.transVertEnable ? "True" : "False"));
        msg.append(QString("Number of TransVerters = %1\n").arg(setupRadio->currentRadio.numTransverters));

        for (int i = 0; i < setupRadio->currentRadio.numTransverters; i++)
        {
            msg.append(QString("Transverter %1\n").arg(i));
            msg.append(QString("Transverter Name = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->transVertName));
            msg.append(QString("Transverter Band = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->band));
            msg.append(QString("Transverter Offset = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->transVertOffsetStr));
            msg.append(QString("Transverter Switch num = %1\n").arg(setupRadio->currentRadio.transVertSettings[i]->transSwitchNum));
            msg.append(QString("Transverter Switch enable = %1\n").arg(setupRadio->currentRadio.enableTransSwitch  ? "True" : "False"));
        }
        msg.append(QString("Radio Supports RIT = %1\n").arg(setupRadio->currentRadio.ritAvail ? "True" : "False"));
        msg.append(QString("Rit Enable On = %1\n").arg(setupRadio->currentRadio.ritEnable  ? "True" : "False"));
        msg.append(QString("Tracelog = %1\n").arg(ui->actionTrace_Log->isChecked() ? "True" : "False"));
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
        trace(QString("Radio Supports RIT = %1").arg( setupRadio->currentRadio.ritAvail ? "True" : "False"));
        trace(QString("Rit Enable On = %1").arg(setupRadio->currentRadio.ritEnable  ? "True" : "False"));
        trace(QString("Tracelog = %1").arg(ui->actionTrace_Log->isChecked() ? "True" : "False"));

    }
    else
    {
        trace(QString("No radio selected"));
    }
}

/************************** Band Information ***********************************/

void RigControlMainWindow::loadBands()
{
    BandList &blist = BandList::getBandList();

    for (int i = 5; i < 15; i++)   // just load VHF/UHF bands
    {
        bands.append(new BandDetail(blist.bandList[i].uk, blist.bandList[i].flow, blist.bandList[i].fhigh));
    }

}

void delay(int sec)
{
    QTime dieTime= QTime::currentTime().addSecs(sec);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
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
