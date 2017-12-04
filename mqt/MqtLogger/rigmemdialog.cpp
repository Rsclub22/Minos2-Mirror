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
//
//
/////////////////////////////////////////////////////////////////////////////

#include "logger_pch.h"

#include "rigmemdialog.h"
#include "ui_rigmemdialog.h"
#include "rigmemcommondata.h"
#include "rigcontrolcommonconstants.h"
#include "rotatorCommonConstants.h"


RigMemDialog::RigMemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RigMemDialog)
{
    ui->setupUi(this);

    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    for (int i = 0; i < hamlibData::supModeList.count(); i++)
    {
        ui->modecb->addItem(hamlibData::supModeList[i]);
    }

    ui->callSignLineEdit->setValidator(new UpperCaseValidator(true));
    ui->locatorLineEdit->setValidator(new UpperCaseValidator(true));

    ui->callSignLineEdit->setFocus();

}

RigMemDialog::~RigMemDialog()
{
    delete ui;
}


void RigMemDialog::setLogData(memoryData::memData* ldata, int buttonNumber)
{
    memoryNumber = buttonNumber;
    logData = ldata;
    ui->modecb->setCurrentText(ldata->mode);


    ui->callSignLineEdit->setText(ldata->callsign);
    ui->locatorLineEdit->setText(ldata->locator);

    if (ldata->freq.remove('.').count() < 4)
    {
        ui->freqLineEdit->setInputMask(maskData::freqMask[7]);
    }
    else
    {
        ui->freqLineEdit->setInputMask(maskData::freqMask[ldata->freq.remove('.').count() - 4]);
    }
    ui->freqLineEdit->setText(ldata->freq);

    if (ldata->bearing == COMPASS_ERROR)
    {
        ui->bearingLineEdit->setText("");
    }
    else
    {
        ui->bearingLineEdit->setText(QString::number(ldata->bearing));
    }
    ui->timeLineEdit->setText(ldata->time);
    //setWindowTitle(QString("M%1 - Edit").arg(QString::number(buttonNumber + 1)));
}

void RigMemDialog::on_okButton_clicked()
{
    logData->callsign = ui->callSignLineEdit->text();
    logData->freq = ui->freqLineEdit->text().remove('.').remove( QRegExp("^[0]*")); //remove periods and leading zeros
    if (logData->freq == "")
    {
        logData->freq = "00000000000";
    }
    logData->mode = ui->modecb->currentText();
    logData->locator = ui->locatorLineEdit->text();
    logData->bearing = ui->bearingLineEdit->text().toInt();
    logData->time = ui->timeLineEdit->text();
    accept();
}

void RigMemDialog::on_cancelButton_clicked()
{
    reject();
}


