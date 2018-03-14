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
#include "rigcontrol.h"
#include "setupdialog.h"
#include "rigcontrolrpc.h"
#include "rigutils.h"
#include <QTimer>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QDebug>




RigControlMainWindow::RigControlMainWindow(QWidget *parent) :
    QMainWindow(parent)
    , msg(0)
    , ui(new Ui::RigControlMainWindow)
    , rigErrorFlag(false)
    , mgmModeFlag(false)
    , supRitFlag(false)
    , cmdLockFlag(false)
    , curVfoFrq(0)
    , curTransVertFrq(0)
    , rRitFreq(0)
    , radioIndex(0)

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

    QSettings settings;
    geoStr = "geometry";
    if (appName.length() > 0)
    {
        geoStr = geoStr + appName;
    }

    QByteArray geometry = settings.value(geoStr).toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    radio = new RigControl();
    radio->getRigList();

    selectRig = new SetupDialog(radio);
    selectRig->setAppName(appName);

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

    setTransVertDisplayVisible(selectRig->currentRadio.transVertEnable);

    enableRitDisplay(false);

    if (appName.length() > 0)
    {
        logMessage((QString("Read Current Radio for AppName %1 from logger").arg(appName)));

    }
    else
    {
        logMessage((QString("Read Current Radio for Local selection")));

        selectRig->readCurrentRadio();

        if (selectRig->currentRadio.radioName == "")
        {
            logMessage(QString("No radio selected for this appName, %1").arg(appName));
            QString errmsg = "<font color='Red'>Please select a radio!</font>";
            showStatusMessage(errmsg);
            sendStatusLogger(errmsg);
        }
        else
        {
            ui->selectRadioBox->setCurrentIndex(ui->selectRadioBox->findText(selectRig->currentRadio.radioName));
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
   if (ui->actionTraceLog->isChecked())
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
    connect(ui->selectRadioBox, SIGNAL(activated(int)), this, SLOT(upDateRadio()));

    connect(ui->actionSetup_Radios, SIGNAL(triggered()), selectRig, SLOT(show()));
    connect(ui->actionTraceLog, SIGNAL(changed()), this, SLOT(saveTraceLogFlag()));

    connect(pollTimer, SIGNAL(timeout()), this, SLOT(getRadioInfo()));


    // configure antenna dialog
    connect(selectRig, SIGNAL(currentRadioSettingChanged(QString)), this, SLOT(currentRadioSettingChanged(QString)));
    connect(selectRig, SIGNAL(radioNameChange()), this, SLOT(updateSelectRadioBox()));

    // Message from Logger
    connect(msg, SIGNAL(setFreq(QString)), this, SLOT(loggerSetFreq(QString)));
    connect(msg, SIGNAL(setMode(QString)), this, SLOT(loggerSetMode(QString)));
    connect(msg, SIGNAL(selectLoggerRadio(QString)), this, SLOT(onSelectRadio(QString)));



    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionAbout_Radio_Config, SIGNAL(triggered()), this, SLOT(aboutRigConfig()));

    connect(radio, SIGNAL(debug_protocol(QString)), this, SLOT(logMessage(QString)));

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
                ui->selectRadioBox->setCurrentIndex(selectRig->currentRadio.radioNumber.toInt(&ok, 10));
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
    for (int i= 0; i < NUM_RADIOS; i++)
    {
        ui->selectRadioBox->addItem(selectRig->availRadios[i].radioName);
    }

    if (appName.length() > 0)
    {
        sendRadioListLogger();
    }
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
    radioIndex = ui->selectRadioBox->currentIndex();
    ridx = radioIndex;
    if (ridx > 0)
    {
        if (radio->get_serialConnected())
        {
                closeRadio();
        }


        ridx -= 1;
        if (selectRig->availRadios[ridx].radioModelNumber == 0)
        {
            closeRadio();
            QMessageBox::critical(this, tr("Radio Error"), "Please configure a radio name and model");
            return;
        }


        selectRig->currentRadio.radioName = selectRig->availRadios[ridx].radioName;
        selectRig->currentRadio.radioNumber = selectRig->availRadios[ridx].radioNumber;
        selectRig->currentRadio.radioMfg_Name = selectRig->availRadios[ridx].radioMfg_Name;
        selectRig->currentRadio.radioModel = selectRig->availRadios[ridx].radioModel;
        selectRig->currentRadio.radioModelNumber = selectRig->availRadios[ridx].radioModelNumber;
        selectRig->currentRadio.civAddress = selectRig->availRadios[ridx].civAddress;
        selectRig->currentRadio.portType = selectRig->availRadios[ridx].portType;
        selectRig->currentRadio.networkAdd = selectRig->availRadios[ridx].networkAdd;
        selectRig->currentRadio.networkPort = selectRig->availRadios[ridx].networkPort;
        selectRig->currentRadio.comport = selectRig->availRadios[ridx].comport;
        selectRig->currentRadio.baudrate = selectRig->availRadios[ridx].baudrate;
        selectRig->currentRadio.databits = selectRig->availRadios[ridx].databits;
        selectRig->currentRadio.stopbits = selectRig->availRadios[ridx].stopbits;
        selectRig->currentRadio.parity = selectRig->availRadios[ridx].parity;
        selectRig->currentRadio.handshake = selectRig->availRadios[ridx].handshake;
        selectRig->currentRadio.transVertEnable = selectRig->availRadios[ridx].transVertEnable;

        // only show transvert freq box is enabled
        setTransVertDisplayVisible(selectRig->currentRadio.transVertEnable);
        sendTransVertStatus(selectRig->currentRadio.transVertEnable);   // send to logger
        selectRig->currentRadio.transVertNegative = selectRig->availRadios[ridx].transVertNegative;
        selectRig->currentRadio.transVertOffset = selectRig->availRadios[ridx].transVertOffset;
        selectRig->currentRadio.transVertOffsetStr = selectRig->availRadios[ridx].transVertOffsetStr;
        selectRig->currentRadio.mgmMode = selectRig->availRadios[ridx].mgmMode;

        selectRig->saveCurrentRadio();

        openRadio();

        if (radio->get_serialConnected())
        {

            ui->radioNameDisp->setText(selectRig->currentRadio.radioName);


            logMessage(QString("Update - Get radio frequency"));
            /*int retCode =*/ getAndSendFrequency(RIG_VFO_CURR);   // also sends it
            /*
            if (retCode < 0)
            {
                // error
                logMessage(QString("Radio Update: Get Freq error, code = %1").arg(QString::number(retCode)));
                hamlibError(retCode, "Request Frequency");
            }
            else
            {
                logMessage(QString("Radio Update: Got Frequency = %1").arg(QString::number(rfrequency)));

                sendFreqToLog(rfrequency, true);// force a freq update
            }
            */



            if (selectRig->currentRadio.radioModelNumber != 135) // don't send USB if Ft991
            {
                if (radio->get_serialConnected())
                {
                    if (appName.count() > 0)
                    {
                        logMessage(QString("Update Radio: Logger Set Mode to %1").arg(slogMode));
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
            }


            if (radio->get_serialConnected())
            {

                writeWindowTitle(appName);
                sendStatusToLogConnected();
                sendRadioNameLogger(selectRig->currentRadio.radioName);
                dumpRadioToTraceLog();

            }
            else
            {
                trace(QString("#### Radio Failed to connect ####"));
                sendStatusToLogDisConnected();
            }

        }
    }
    else
    {   // no radio selected
        trace("No radio selected");
        ui->radioNameDisp->setText("");
        closeRadio();
        if (appName.length() > 0)
        {
            writeWindowTitle(appName);
            sendRadioNameLogger("");
        }
        else
        {
            writeWindowTitle(appName);
        }

    }

    if (radio->get_serialConnected())
    {
        pollTimer->start(pollTime);             // start timer to send poll radio
    }


}

void RigControlMainWindow::refreshRadio()
{
    logMessage(QString("refreshRadio: Index Selected = %1").arg(QString::number(ui->selectRadioBox->currentIndex())));

    int ridx = 0;
    radioIndex = ui->selectRadioBox->currentIndex();
    ridx = radioIndex;
    if (ridx > 0)
    {
        if (radio->get_serialConnected())
        {
            openRadio();    // do everything except init it

            logMessage(QString("Update - Get radio frequency"));
            getAndSendFrequency(RIG_VFO_CURR);   // also sends

            if (selectRig->currentRadio.radioModelNumber != 135) // don't send USB if Ft991
            {
                logMessage(QString("Update Radio: Logger Set Mode to %1").arg(slogMode));
                loggerSetMode(selRadioMode);
            }
            writeWindowTitle(appName);
            sendStatusToLogConnected();
            sendRadioNameLogger(selectRig->currentRadio.radioName);
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

        if (selectRig->currentRadio.radioName == "")
        {
            logMessage(QString("Open Radio: No radio name!"));
            showStatusMessage("Please select a Radio");
            return;
        }

        logMessage(QString("Open Radio: Opening Radio %1 PortType %2").arg(selectRig->currentRadio.radioName).arg(hamlibData::portTypeList[selectRig->currentRadio.portType]));
        showStatusMessage(QString("Opening Radio: %1").arg(selectRig->currentRadio.radioName));

        if (rig_port_e(selectRig->currentRadio.portType) == RIG_PORT_SERIAL)
        {
            if(selectRig->comportAvial(selectRig->currentRadio.radioNumber.toInt(), selectRig->currentRadio.comport) == -1)
            {
                logMessage(QString("Open Radio: Check comport - defined port %1 not available on computer").arg(selectRig->currentRadio.comport));
                showStatusMessage(QString("Comport %1 no longer configured on computer?").arg(selectRig->currentRadio.comport));
                return;
            }

            if (selectRig->currentRadio.comport == "")
            {
                logMessage(QString("Open Radio: No comport"));
                showStatusMessage("Please select a Comport");
                return;
            }

        }

        if (rig_port_e(selectRig->currentRadio.portType) == RIG_PORT_NETWORK || rig_port_e(selectRig->currentRadio.portType == RIG_PORT_UDP_NETWORK))
        {
            if (selectRig->currentRadio.networkAdd == "" || (selectRig->currentRadio.networkPort == ""))
            {
                logMessage(QString("Open Radio: No network or Port Number"));
                showStatusMessage("Please enter a network Address and Port Number");
                return;
            }

        }
        if (selectRig->currentRadio.radioModel == "")
        {
            logMessage(QString("Open Radio: No radio model"));
            showStatusMessage("Please select a radio model");
            return;
        }

        scatParams p = selectRig->getCurrentRadio();

        if (!radio->get_serialConnected())
        {
            // if radio is already open, don't reinit it
            retCode = radio->init(selectRig->currentRadio);
            if (retCode < 0)
            {
                logMessage(QString("Error Opening Radio Error Code = %1").arg(QString::number(retCode)));
                hamlibError(retCode, "Open Radio");
            }
        }

        if (radio->get_serialConnected())
        {
            logMessage(QString("Open Radio: Radio Opened %1").arg(selectRig->currentRadio.radioName));
            showStatusMessage(QString("Radio Opened: %1").arg(selectRig->currentRadio.radioName));



            if (rig_port_e(selectRig->currentRadio.portType) == RIG_PORT_SERIAL)
            {
                    showStatusMessage(QString("Connected: %1 - %2, %3, %4, %5, %6, %7, %8")
                                      //.arg(selectRig->currentRadio.radioName, selectRig->currentRadio.radioModel, selectRig->currentRadio.comport, selectRig->currentRadio.baudrate, selectRig->currentRadio.databits, selectRig->currentRadio.stopbits, radio->getParityCodeNames()[selectRig->currentRadio.parity], radio->getHandShakeNames()[selectRig->currentRadio.handshake]));
                                      .arg(p.radioName).arg(p.radioModel).trimmed().arg(p.comport).arg(p.baudrate).arg(p.databits)
                                      .arg(p.stopbits).arg(radio->getParityCodeNames()[p.parity]).arg(radio->getHandShakeNames()[p.handshake]));


            }
            else if (rig_port_e(selectRig->currentRadio.portType) == RIG_PORT_NETWORK || rig_port_e(selectRig->currentRadio.portType) == RIG_PORT_UDP_NETWORK)
            {
                    showStatusMessage(QString("Connected: %1 - %2, %3:%4").arg(selectRig->currentRadio.radioName).arg(selectRig->currentRadio.radioModel.trimmed()).arg(selectRig->currentRadio.networkAdd).arg(selectRig->currentRadio.networkPort));
            }
            else if (rig_port_e(selectRig->currentRadio.portType) == RIG_PORT_NONE)
            {
                    showStatusMessage(QString("Connected: %1 - %2").arg(selectRig->currentRadio.radioName).arg(selectRig->currentRadio.radioModel.trimmed()));
            }



        }
        else
        {

            logMessage(QString("Radio Open Error"));
            showStatusMessage(tr("Radio Open error"));
        }


    }

    void RigControlMainWindow::closeRadio()
    {

        if (radio->get_serialConnected())
        {
           radio->closeRig();
        }

        showStatusMessage("Disconnected");
        sendStatusToLogDisConnected();
        logMessage(QString("Radio Closed"));



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
            /*
            if (retCode < 0)
            {
                // error
                logMessage(QString("Get radioInfo: Get Freq error, code = %1").arg(QString::number(retCode)));
                hamlibError(retCode, "Request Frequency");
            }
            else
            {
                logMessage(QString("Got Frequency = %1").arg(QString::number(rfrequency)));
            }
            */
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



    /*
            if (supRitFlag)
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

    */

    }


    void RigControlMainWindow::onSelectRadio(QString s)
    {

        logMessage(QString("Recieved SelectRadio from Logger = %1").arg(s));


        // the first time rigcontrolframe uses this message the mode is appended to the name
        if (s.contains(':'))
        {
            QStringList sl = s.split(':');
            if (sl.count() != 2)
            {
                logMessage(QString("OnSelectRadio Error Splitting radioName and mode %1").arg(s));
                return;     // error
            }
            s = sl[0];
            if (sl[1] == "")
            {
                selRadioMode = "USB";
            }
            else
            {
                selRadioMode = sl[1];
            }
        }

        QString oldRadio = ui->selectRadioBox->currentText();

        int index = ui->selectRadioBox->findText(s, Qt::MatchFixedString);
        if (index >= 0)
        {
            ui->selectRadioBox->setCurrentIndex(index);
        }
        else
        {
            ui->selectRadioBox->setCurrentText(s);
        }

        s = ui->selectRadioBox->currentText();
        if (!s.isEmpty() && s == oldRadio)
        {
            refreshRadio();
        }
        else
        {
            upDateRadio();
        }
    }

    void RigControlMainWindow::loggerSetFreq(QString freq)
    {
        logMessage(QString("Recieved Freq from Logger = %1").arg(freq));
        if (radio->get_serialConnected() && !rigErrorFlag)
        {
            logger_freq = freq;
            setFreq(freq, RIG_VFO_CURR);
        }

    }


    void RigControlMainWindow::setFreq(QString freq, vfo_t vfo)
    {
        cmdLockOn();    // lock get radio info
        bool ok = false;
        int retCode = 0;
        QString sfreq = freq;

        double f = sfreq.toDouble(&ok);
        logMessage(QString("SetFreq: Change to Freq = %1").arg(QString::number(f)));

        if (ok)
        {
            if (selectRig->currentRadio.transVertEnable)
            {
                if (selectRig->currentRadio.transVertNegative)
                {
                    f = f + selectRig->currentRadio.transVertOffset;
                    logMessage(QString("SetFreq: Negative Transvert Enabled Freq = %1").arg(QString::number(f)));
                }
                else
                {
                    f = f - selectRig->currentRadio.transVertOffset;
                    logMessage(QString("SetFreq: Transvert Enabled Freq = %1").arg(QString::number(f)));
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


        retCode = radio->getFrequency(vfo, &rfrequency);
        if (retCode == RIG_OK)
        {

            curVfoFrq = rfrequency;
            logMessage(QString("Trans Enable = %1").arg(QString::number(selectRig->currentRadio.transVertEnable)));
            if (selectRig->currentRadio.transVertEnable)
            {
                logMessage(QString("Transvert enabled"));
                if (selectRig->currentRadio.transVertNegative)
                {
                    logMessage(QString("Negative Transvert"));
                    transVertF = rfrequency - selectRig->currentRadio.transVertOffset;
                    logMessage(QString("Transvert F = %1").arg(QString::number(transVertF)));
                }
                else
                {
                    logMessage(QString("Positive Transvert"));
                    transVertF = rfrequency + selectRig->currentRadio.transVertOffset;

                }
                logMessage(QString("Transvert Freq. = %1").arg(QString::number(transVertF)));
                curTransVertFrq = transVertF;
                displayTransVertVfo(transVertF);

            }
            displayFreqVfo(rfrequency);

            if (selectRig->currentRadio.transVertEnable)
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



    void RigControlMainWindow::setTransVertDisplayVisible(bool visible)
    {
        ui->transVertFreqA->setVisible(visible);
        ui->TVertTitleA->setVisible(visible);
    }




    void RigControlMainWindow::chkRadioMgmModeChanged()
    {
        static QString mgmMode;
        if (selectRig->currentRadio.mgmMode != mgmMode)
        {
            ui->mgmMode->setText(selectRig->currentRadio.mgmMode);
        }
    }


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

            if (mgmModeFlag && sCurMode != selectRig->currentRadio.mgmMode) // has mode been changed on the radio?
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
                sendModeToLog(QString("%1:%2").arg(hamlibData::MGM).arg(selectRig->currentRadio.mgmMode));
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
                    if (curMode !=  radio->convertQStrMode(selectRig->currentRadio.mgmMode))
                    {
                        setMode(selectRig->currentRadio.mgmMode, RIG_VFO_CURR);
                    }
                }
                else
                {
                    mgmModeFlag = true;
                    setMode(selectRig->currentRadio.mgmMode, RIG_VFO_CURR);
                    logMessage((QString("Log SetMode: Set MgmMode Flag, Selected MGM Mode = %1").arg(selectRig->currentRadio.mgmMode)));
                }
            }
            else
            {
                mgmModeFlag = false;
                logMessage(QString("Log SetMode: Clear mgmModeFlag, Set mode = %1").arg(mode));
                setMode(mode, RIG_VFO_CURR);
            }
        }

    }




    void RigControlMainWindow::setMode(QString mode, vfo_t vfo)
    {
        int retCode = 0;

        cmdLockOn();      // lock get radio info

        logMessage(QString("Setmode: Requested Mode = %1").arg(mode));
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

    void RigControlMainWindow::enableRitDisplay(bool state)
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
            sRitFreq = QString::number(rRitFreq);
            ui->ritFreq->setText(sRitFreq);
        }
        return retCode;
    }


    int RigControlMainWindow::setRitFreq(vfo_t vfo, shortfreq_t ritFreq)
    {
        int retCode = 0;
        cmdLockOn();
        retCode = radio->setRit(vfo, ritFreq);

        cmdLockOff();
        return retCode;

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


    /*
    QString RigControlMainWindow::convertFreqString(double frequency)
    {
        double freq = frequency;
        sfreq = QString::number(freq,'f', 0);
        int len = sfreq.length();


        switch(len)
        {
            case 11:
                sfreq = sfreq.insert(8, '.');
                sfreq = sfreq.insert(5, '.');
                sfreq = sfreq.insert(2, '.');
                break;
            case 10:
                sfreq = sfreq.insert(7, '.');
                sfreq = sfreq.insert(4, '.');
                sfreq = sfreq.insert(1, '.');
                break;
            case 9:
                sfreq = sfreq.insert(3, '.');
                sfreq = sfreq.insert(7, '.');
                break;
            case 8:
                sfreq = sfreq.insert(2, '.');
                sfreq = sfreq.insert(6, '.');
                break;
            case 7:
                sfreq = sfreq.insert(4, '.');
                sfreq = sfreq.insert(1, '.');
                break;
            case 6:
                sfreq = sfreq.insert(3,'.');
                break;
            case 5:
                sfreq = sfreq.insert(2,'.');
                break;
            case 4:
                sfreq = sfreq.insert(1,'.');
                break;
            default:
                sfreq = "??.???.???.???";    // error

        }


        return sfreq;
    }

    */


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
        if(appName > 0)
        {
            sendStatusToLogError();
        }

        rigErrorFlag = true;

        errorCode *= -1;
        QString errorMsg = radio->gethamlibErrorMsg(errorCode);
        logMessage(QString("Hamlib Error - Code = %1 - %2").arg(QString::number(errorCode)).arg(errorMsg));

        QMessageBox::critical(this, "RigControl hamlib Error - " + selectRig->currentRadio.radioName, QString::number(errorCode) + " - " + errorMsg + "\n" + "Command - " + cmd);

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


        ui->actionTraceLog->setChecked(config.value("TraceLog", false).toBool());

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

        config.setValue("TraceLog", ui->actionTraceLog->isChecked());

        config.endGroup();
        trace("Tracelog Changed = " + QString::number(ui->actionTraceLog->isChecked()));
    }





    void RigControlMainWindow::about()
    {
        QMessageBox::about(this, "Minos RigControl", "Minos QT RigControl\nCopyright D Balharrie G8FKH/M0DGB 2017");
    }

    void RigControlMainWindow::sendRadioListLogger()
    {
        QStringList radioList;
        for (int i= 0; i < NUM_RADIOS; i++)
        {
            if (!selectRig->availRadios[i].radioName.isEmpty())
            {
                radioList.append(selectRig->availRadios[i].radioName);
            }
        }
        msg->publishRadioNames(radioList);
    }


    void RigControlMainWindow::sendRadioNameLogger(QString radioName)
    {
        logMessage(QString("Send radioName %1 to Logger").arg(radioName));
        msg->publishRadioName(radioName);
    }

    void RigControlMainWindow::sendStatusLogger(const QString &message )
    {
        if (appName.length() > 0)
        {
            logMessage(QString("Send status to logger = %1").arg(message));
            msg->publishState(message);
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
        msg->publishErrorMsg(errMsg);

    }

    void RigControlMainWindow::sendFreqToLog(freq_t freq)
    {

        if (appName.length() > 0)
        {
            logMessage(QString("Send freq to logger = %1").arg(convertFreqToStr(freq)));
            msg->publishFreq(convertFreqToStr(freq));
        }
    }

    void RigControlMainWindow::sendModeToLog(QString mode)
    {
        if (appName.length() > 0)
        {
            logMessage(QString("Send mode to logger = %1").arg(mode));
            msg->publishMode(mode);
        }
    }

    void RigControlMainWindow::sendTransVertStatus(bool status)
    {
        QString flag;
        if (appName.length() > 0)
        {
            if (status)
            {
                flag = TXVERT_ON;
            }
            else
            {
                flag = TXVERT_OFF;
            }
            logMessage(QString("Send Transvert Status to logger = %1").arg(flag));
            msg->publishTransVertStatus(flag);
        }
    }

    void RigControlMainWindow::sendTransVertOffsetToLogger()
    {
        QString f = convertFreqToStr(selectRig->currentRadio.transVertOffset);
        logMessage(QString("Send Transvert Offset to logger = %1%2").arg(selectRig->currentRadio.transVertEnable ? f = "-" : f = "+").arg(f));
        msg->publishTransVertOffSetFreq(selectRig->currentRadio.transVertNegative, f);

    }

    void RigControlMainWindow::sendTransVertSwitchToLogger(const QString &swNum)
    {
        logMessage(QString("Send Transvert Switch Number to logger = %1").arg(swNum));
        msg->publishTransVertSwitch(swNum);

    }



    void RigControlMainWindow::aboutRigConfig()
    {
        QString msg = QString("*** Rig Configuration ***\n\n");
        msg.append(QString("App Instance Name  = %1\n").arg(appName));
        msg.append(QString("Hamlib Version = %1\n").arg(radio->gethamlibVersion()));
        msg.append(QString("Radio Name = %1\n").arg(selectRig->currentRadio.radioName));
        msg.append(QString("Radio Number = %1\n").arg(selectRig->currentRadio.radioNumber));
        msg.append(QString("Rig Model = %1\n").arg(selectRig->currentRadio.radioModel));
        msg.append(QString("Rig Number = %1\n").arg(QString::number(selectRig->currentRadio.radioModelNumber)));
        msg.append(QString("Rig Manufacturer = %1\n").arg(selectRig->currentRadio.radioMfg_Name));
        if (selectRig->currentRadio.radioMfg_Name == "Icom")
        {
            if (selectRig->currentRadio.civAddress == "")
            {
                msg.append(QString("Icom CIV address = Using Default CIV Address\n"));
            }
            else
            {
                msg.append(QString("Icom CIV address = %1\n").arg(selectRig->currentRadio.civAddress));
            }

        }
        msg.append(QString("Rig PortType = %1\n").arg(hamlibData::portTypeList[selectRig->currentRadio.portType]));
        msg.append(QString("Network Address = %1\n").arg(selectRig->currentRadio.networkAdd));
        msg.append(QString("Network Port = %1\n").arg(selectRig->currentRadio.networkPort));
        msg.append(QString("Comport = %1\n").arg(selectRig->currentRadio.comport));
        msg.append(QString("Baudrate = %1\n").arg(selectRig->currentRadio.baudrate));
        msg.append(QString("Stop bits = %1\n").arg(QString::number(selectRig->currentRadio.stopbits)));
        msg.append(QString("Parity = %1\n").arg(radio->getParityCodeNames()[selectRig->currentRadio.parity]));
        msg.append(QString("Handshake = %1\n").arg(radio->getHandShakeNames()[selectRig->currentRadio.handshake]));
        QString f = "";
        selectRig->currentRadio.transVertEnable ? f = "True" : f = "False";
        msg.append(QString("TransVert Enable = %1\n").arg(f));
        selectRig->currentRadio.transVertNegative ? f = "True" : f = "False";
        msg.append(QString("TransVert Negative = %1\n").arg(f));
        msg.append(QString("TransVert Offset = %1\n").arg(convertFreqStrDisp(convertFreqToStr(selectRig->currentRadio.transVertOffset))));
        msg.append(QString("Use RX Passband = %1\n").arg(f));
        supRitFlag ? f = "True" : f = "False";
        msg.append(QString("Radio Supports RIT = %1\n").arg(f));
        ui->actionTraceLog->isChecked() ? f = "True" : f = "False";
        msg.append(QString("Tracelog = %1\n").arg(f));



        QMessageBox::about(this, "Minos RigControl", msg);
    }




    void RigControlMainWindow::dumpRadioToTraceLog()
    {
        trace("*** Radio Update ***");
        trace(QString("App Instance Name  = %1").arg(appName));
        trace(QString("Hamlib Version = %1").arg(radio->gethamlibVersion()));
        trace(QString("Radio Name = %1").arg(selectRig->currentRadio.radioName));
        trace(QString("Radio Number = %1").arg(selectRig->currentRadio.radioNumber));
        trace(QString("Rig Model = %1").arg(selectRig->currentRadio.radioModel).trimmed());
        trace(QString("Rig Number = %1").arg(QString::number(selectRig->currentRadio.radioModelNumber)));
        trace(QString("Rig Manufacturer = %1").arg(selectRig->currentRadio.radioMfg_Name));
        if (selectRig->currentRadio.radioMfg_Name == "Icom")
        {
            if (selectRig->currentRadio.civAddress == "")
            {
                trace(QString("Icom CIV address = Using Default CIV Address"));
            }
            else
            {
                trace(QString("Icom CIV address = %1").arg(selectRig->currentRadio.civAddress));
            }

        }
        trace(QString("Rig PortType = %1").arg(hamlibData::portTypeList[selectRig->currentRadio.portType]));
        trace(QString("Network Address = %1").arg(selectRig->currentRadio.networkAdd));
        trace(QString("Network Port = %1").arg(selectRig->currentRadio.networkPort));
        trace(QString("Comport = %1").arg(selectRig->currentRadio.comport));
        trace(QString("Baudrate = %1").arg(selectRig->currentRadio.baudrate));
        trace(QString("Stop bits = %1").arg(QString::number(selectRig->currentRadio.stopbits)));
        trace(QString("Parity = %1").arg(radio->getParityCodeNames()[selectRig->currentRadio.parity]));
        trace(QString("Handshake = %1").arg(radio->getHandShakeNames()[selectRig->currentRadio.handshake]));
        QString f = "";
        selectRig->currentRadio.transVertEnable ? f = "True" : f = "False";
        trace(QString("TransVert Enable = %1").arg(f));
        selectRig->currentRadio.transVertNegative ? f = "True" : f = "False";
        trace(QString("TransVert Negative = %1").arg(f));
        trace(QString("TransVert Offset = %1").arg(convertFreqStrDisp(convertFreqToStr(selectRig->currentRadio.transVertOffset))));
        trace(QString("Use RX Passband = %1").arg(f));
        supRitFlag ? f = "True" : f = "False";
        trace(QString("Radio Supports RIT = %1").arg(f));

        ui->actionTraceLog->isChecked() ? f = "True" : f = "False";
        trace(QString("Tracelog = %1").arg(f));
    }



    void delay(int sec)
    {
        QTime dieTime= QTime::currentTime().addSecs(sec);
        while (QTime::currentTime() < dieTime)
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
