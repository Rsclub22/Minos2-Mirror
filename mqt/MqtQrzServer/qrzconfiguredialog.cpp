/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Qrz Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2021
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////




#include "qrzconfiguredialog.h"
#include "ui_qrzconfiguredialog.h"


QrzConfigureDialog::QrzConfigureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QrzConfigureDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->logonCallsignLineEdit->setValidator(&ucValidator);
}

QrzConfigureDialog::~QrzConfigureDialog()
{
    delete ui;
}

int QrzConfigureDialog::exec()
{
    ui->logonCallsignLineEdit->setText(logCallsign);
    ui->qrzPasswordLineEdit->setText(logPassword);

    return QDialog::exec();
}

void QrzConfigureDialog::on_resetDBButton_clicked()
{
    resetDB = true;
    accept();
}


void QrzConfigureDialog::on_OKButton_clicked()
{
    logCallsign = ui->logonCallsignLineEdit->text().trimmed();
    logPassword = ui->qrzPasswordLineEdit->text().trimmed();

    accept();
}


void QrzConfigureDialog::on_cancelButton_clicked()
{
    reject();
}

