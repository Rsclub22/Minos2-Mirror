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
#include "winkeyermainwindow.h"
#include "ui_winkeyermainwindow.h"
#include "winkeyersetupdialog.h"
#include "winKeyerCommon.h"

WinkeyerMainWindow::WinkeyerMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WinkeyerMainWindow)
    , winkeyerControl(new WinkeyerControl(this))
{
    ui->setupUi(this);



    connect(winkeyerControl, &WinkeyerControl::winKeyerOpenStatus, this, &WinkeyerMainWindow::handleWinKeyerOpenStatus);
    connect(ui->setupPushButton, &QPushButton::clicked, this, &WinkeyerMainWindow::handleSetupPushButton);
    connect(ui->sendTextLineEdit, &QLineEdit::textChanged, this, &WinkeyerMainWindow::onTextChanged);


    if (!winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport().isEmpty())
    {
        if (winkeyerControl->initComport(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport(), QString(DEFAULT_BAUDRATE).toInt()) == SERIAL_OK)
        {
            qDebug() << "open serial port ok";
            winkeyerControl->start();
        }
        else
        {
            qDebug() << "failed to open";
        }
    }



}

WinkeyerMainWindow::~WinkeyerMainWindow()
{
    delete ui;
    delete winkeyerControl;
}



void WinkeyerMainWindow::on_sendPushButton_clicked()
{
    // Retrieve data from a QLineEdit or any other input widget
    QString inputData = ui->sendTextLineEdit->text();
    winkeyerControl->enqueueData(inputData.toUtf8());
}


void WinkeyerMainWindow::on_openPushButton_clicked()
{
    // Retrieve data from a QLineEdit or any other input widget

    winkeyerControl->openWinKeyer();
}







void WinkeyerMainWindow::handleWinKeyerOpenStatus(bool open)
{
    if (open)
    {
        ui->openStatusLabel->setText("Open");
        winkeyerControl->setIsWKOpen(true);
    }
    else
    {
        ui->openStatusLabel->setText("Closed");
        winkeyerControl->setIsWKOpen(false);
    }

}


void WinkeyerMainWindow::handleSetupPushButton()
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
                    winkeyerControl->serialPortClose();
                    if (winkeyerControl->initComport(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport(), QString(DEFAULT_BAUDRATE).toInt()) == SERIAL_OK)
                    {
                        qDebug() << "open serial port ok";

                    }
                    else
                    {
                        qDebug() << "failed to open";
                    }
                }
            }
            // move this to save on close only...also need to update non wkState as well???
            QString fileName = WINKEYER_PATH_LOGGER() + WINKEYER_CONFIG_FILENAME;
            QSettings  winkeyerConfig(fileName, QSettings::IniFormat);
            winkeyerControl->getNewWinkeyStateStoragePtr()->saveWinkeyerStateStorageToFile(winkeyerConfig);
        }
    }
}



void WinkeyerMainWindow::onTextChanged(const QString &text) {
    static QString oldText;
    if (text.length() > oldText.length()) {
        QChar newChar = text[text.length() - 1];
        handleKeyboardChar(newChar);
    }
    oldText = text;
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





