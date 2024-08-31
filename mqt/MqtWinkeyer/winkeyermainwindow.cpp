/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////
#include <QPushButton>
#include <QSettings>
#include "winkeyermainwindow.h"
#include "ui_winkeyermainwindow.h"
#include "winkeyersetupdialog.h"
#include "winKeyerCommon.h"
#include "regsettings.h"
#include "AppStartup.h"
#include "LogEvents.h"
#include "MinosRPC.h"
#include "MTrace.h"
#include "winkeyerrpc.h"


WinkeyerMainWindow::WinkeyerMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WinkeyerMainWindow)
    , winkeyerControl(new WinkeyerControl(this))
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    trace("Connect to commandRead");  // This connect doesn't appear to work for some time!
    connect(commandReader.data(), &CommandReader::commandLine, this, &WinkeyerMainWindow::onCommandRead);


    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    createCloseEvent();

    connect(&LogTimer, &QTimer::timeout, this, &WinkeyerMainWindow::LogTimerTimer);
    LogTimer.start(100);

    QString appName = getAppStartupName();
    trace(QString("AppName = %1").arg(appName));
    MinosRPC *rpc = MinosRPC::getMinosRPC(appName);
    Q_UNUSED(rpc)

    addWinkeyerStatusObjectsToStatusBar();

   // connect (WinkeyerRpc::getWinKeyerRpc(), &WinkeyerRpc::winkeyerMsg, this, &WinkeyerMainWindow::onwinkeyerMessage);
   // connect (WinkeyerRpc::getWinkeyerRpcc(), &WinkeyerRpc::loggerWinkeyerMsg, this, &WinkeyerMainWindow::onLoggerWinkeyerMsg);

    connect(winkeyerControl, &WinkeyerControl::winKeyerOpenStatus, this, &WinkeyerMainWindow::onHandleWinKeyerOpenStatus);
    connect(winkeyerControl, &WinkeyerControl::wk_XoffStatus, this, [=](const QString &status) {onHandleXoffStatus(status);});
    connect(winkeyerControl, &WinkeyerControl::wk_BreakInStatus, this, [=](const QString &status) {onHandleBreakInStatus(status);});
    connect(winkeyerControl, &WinkeyerControl::wk_KBusyStatus, this, [=](const QString &status) {onHandleKBusyStatus(status);});
    connect(winkeyerControl, &WinkeyerControl::wk_KWaitStatus, this, [=](const QString &status) {onHandleKWaitStatus(status);});


    connect(ui->setupPushButton, &QPushButton::clicked, this, &WinkeyerMainWindow::onHandleSetupPushButton);
    connect(ui->sendTextLineEdit, &QLineEdit::textChanged, this, &WinkeyerMainWindow::onTextChanged);


    if (!winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport().isEmpty())
    {
        QString baudrate = QString(DEFAULT_BAUDRATE);

        if (winkeyerControl->initComport(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport(), baudrate.toInt()) == SERIAL_OK)
        {

            trace(QString("Winkeyer App Start - Serial port %1, baudrate %3 Opened OK").arg(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport()).arg(baudrate));
            updateStatusBarMessage("", "");
            winkeyerControl->start();
        }
        else
        {
            trace(QString("Winkeyer App Start - Serial port %1, baudrate %3 Failed to Open Error %3").arg(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport())
                      .arg(baudrate).arg(winkeyerControl->getSerialPortErrorMsg()));
            updateStatusBarMessage(winkeyerControl->getSerialPortErrorMsg(), "");
        }
    }



}

WinkeyerMainWindow::~WinkeyerMainWindow()
{
    delete ui;
    delete winkeyerControl;
}


void WinkeyerMainWindow::resizeEvent(QResizeEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void WinkeyerMainWindow::moveEvent(QMoveEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void WinkeyerMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        RegSettings settings;
        settings.getSettings().setValue("geometry", saveGeometry());
    }
}
void WinkeyerMainWindow::closeEvent(QCloseEvent *event)
{
    trace("WinkeyerServerMainWindow::closeEvent");

    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());

    //saveWinkeyerSettings();

    QWidget::closeEvent(event);
}

void WinkeyerMainWindow::LogTimerTimer()
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

void WinkeyerMainWindow::onCommandRead(QString cmd)
{
    trace(QString("onCommandRead %1").arg(cmd));
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        close();
    }
}


void WinkeyerMainWindow::on_sendPushButton_clicked()
{
    // Retrieve data from a QLineEdit or any other input widget
    QString inputData = ui->sendTextLineEdit->text();
    winkeyerControl->enqueueData(inputData.toUtf8());
}


void WinkeyerMainWindow::on_openPushButton_clicked()
{
    if (winkeyerControl->getIsWkOpen())
    {
        // open so lets close
        saveWinkeyerSettings(winkeyerControl->getCurrentWinkeyStateStoragePtr());
        //winkeyerControl->setWk1Mode();
        winkeyerControl->closeWinKeyer();
        winkeyerControl->setIsWKOpen(false);
        ui->openPushButton->setText("Open");
        updateStatusBarMessage("", "Closed");
    }
    else
    {
        if (winkeyerControl->isSerialPortOpen())
        {
            winkeyerControl->openWinKeyer();
        }
    }


}







void WinkeyerMainWindow::onHandleWinKeyerOpenStatus(bool open)
{


    if (open)
    {

        ui->openPushButton->setText("Close");
        updateStatusBarMessage("", "Open");
        winkeyerControl->setIsWKOpen(true);

        winkeyerControl->wkSendDefaults(winkeyerControl->getCurrentWinkeyStateStoragePtr());
        winkeyerControl->wakeUpTxThread();
    }
    else
    {

        ui->openPushButton->setText("Open");
        updateStatusBarMessage("", "Closed");
        winkeyerControl->setIsWKOpen(false);
    }

}


void WinkeyerMainWindow::onHandleSetupPushButton()
{
    openWinKeyerSetupDialog();
}

void WinkeyerMainWindow::openWinKeyerSetupDialog()
{
    WinKeyerSetupDialog dialog(this);
    dialog.setWindowTitle("Winkeyer Setup");
    if (dialog.exec() == QDialog::Accepted)
    {
        if (dialog.isSetupDirty())
        {
            // check if comport params have changed as we need to restart comport
            QString oldComport = winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport();
            QString oldBaudRate = winkeyerControl->getCurrentWinkeyStateStoragePtr()->getBaudrate();


            winkeyerControl->setNewWinkeyStateStoragePtr(QSharedPointer<WinkeyerStateStorage>::create(*dialog.getSetupData()));

            if (oldComport != winkeyerControl->getNewWinkeyStateStoragePtr()->getComport())
            {
                winkeyerControl->getCurrentWinkeyStateStoragePtr()->setComport(winkeyerControl->getNewWinkeyStateStoragePtr()->getComport());
                if (winkeyerControl->isSerialPortOpen())
                {
                    QString baudrate = QString(DEFAULT_BAUDRATE);
                    winkeyerControl->serialPortClose();
                    if (winkeyerControl->initComport(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport(), baudrate.toInt()) == SERIAL_OK)
                    {
                        trace(QString("Winkeyer Setup - Serial port %1, baudrate %3 Opened OK").arg(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport()).arg(baudrate));
                        updateStatusBarMessage("", "");

                    }
                    else
                    {
                        trace(QString("Winkeyer Setup - Serial port %1, baudrate %3 Failed to Open Error %3").arg(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport())
                                  .arg(baudrate).arg(winkeyerControl->getSerialPortErrorMsg()));
                        updateStatusBarMessage(winkeyerControl->getSerialPortErrorMsg(), "");
                    }
                }
            }

            // current keyer settings are updated as data sent to Winkeyer.
            winkeyerControl->wakeUpTxThread();  // send changes to Winkeyer


            saveWinkeyerSettings(winkeyerControl->getNewWinkeyStateStoragePtr());
        }
    }
}



void WinkeyerMainWindow::onTextChanged(const QString &text)
{
    static QString oldText;
    if (text.length() > oldText.length()) {
        QChar newChar = text[text.length() - 1];
        handleKeyboardChar(newChar);
    }
    oldText = text;
}

void WinkeyerMainWindow::onHandleXoffStatus(QString status)
{
    xoffStatus->setText(status);

}

void WinkeyerMainWindow::onHandleBreakInStatus(QString status)
{
    breakInStatus->setText(status);

}

void WinkeyerMainWindow::onHandleKBusyStatus(QString status)
{
    busyStatus->setText(status);

}


void WinkeyerMainWindow::onHandleKWaitStatus(QString status)
{
    waitStatus->setText(status);

}

void WinkeyerMainWindow::handleKeyboardChar(QChar kbdChar)
{


    if (kbdChar == KBD_BACKSPACE)
    {

    }
    else if (kbdChar == KBD_CR)
    {
        // ignore CR
    }
    else if (kbdChar == KBD_ESC)
    {
        // ignore ESC

    }

    winkeyerControl->wkSendBufferedChar(kbdChar.toLatin1());

}



void WinkeyerMainWindow::saveWinkeyerSettings(QSharedPointer<WinkeyerStateStorage> winkeySettings)
{
    QString fileName = WINKEYER_PATH_LOGGER() + WINKEYER_CONFIG_FILENAME;
    QSettings  winkeyerConfig(fileName, QSettings::IniFormat);
    winkeySettings->saveWinkeyerStateStorageToFile(winkeyerConfig);
}


void WinkeyerMainWindow::addWinkeyerStatusObjectsToStatusBar()
{
    // create labels and set initial text
    xoffStatus = new QLabel();
    xoffStatus->setText("    ");
    breakInStatus = new QLabel();
    breakInStatus->setText("     ");
    busyStatus = new QLabel();
    busyStatus->setText("    ");
    waitStatus = new QLabel();
    waitStatus->setText("    ");

    ui->statusbar->addPermanentWidget(xoffStatus);
    ui->statusbar->addPermanentWidget(breakInStatus);
    ui->statusbar->addPermanentWidget(busyStatus);
    ui->statusbar->addPermanentWidget(waitStatus);


}

void WinkeyerMainWindow::updateStatusBarMessage(QString serialErrorMsg, QString wkStatusMsg)
{
    QString serialOpen;
    QString serialErrorMessage;

    if (winkeyerControl->isSerialPortOpen())
    {
        serialOpen = "Open";
    }
    else
    {
        serialOpen = "Closed";
    }

    if (!serialErrorMsg.isEmpty())
    {
        serialErrorMessage = "";
    }
    else
    {
        serialErrorMessage = QString("Error: %1").arg(serialErrorMessage);
    }

    ui->statusbar->showMessage(QString("Serial: %1, %2, %3 %4 | Keyer Status: %5")
                                   .arg(serialOpen)
                                   .arg(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport())
                                   .arg(QString(DEFAULT_BAUDRATE))
                                   .arg(serialErrorMsg)
                                   .arg(wkStatusMsg));
}



