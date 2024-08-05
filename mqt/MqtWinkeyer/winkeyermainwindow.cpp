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



    if (!winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport().isEmpty())
    {
        if (winkeyerControl->initComport(winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport()) == SERIAL_OK)
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
    QString inputData = ui->textLineEdit->text();
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
    }
    else
    {
        ui->openStatusLabel->setText("Closed");
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
            QString oldComport = winkeyerControl->getCurrentWinkeyStateStoragePtr()->getComport();
            QString oldBaudRate = winkeyerControl->getCurrentWinkeyStateStoragePtr()->getBaudrate();

            winkeyerControl->getNewWinkeyStateStoragePtr() = dialog.getSetupData();

            //save these when closing ???????????????
            QString fileName = "winkeyerSettings.ini";
            QSettings  winkeyerConfig(fileName, QSettings::IniFormat);
            currentWinkeyStateStoragePtr->saveWinkeyerStateStorageToFile(winkeyerConfig);
        }
    }
}





