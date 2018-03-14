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
#include "rigutils.h"
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
    // validate the input
    connect(ui->freqLineEdit, SIGNAL(editingFinished()), this, SLOT(onFreqEditFinish()));
}

RigMemDialog::~RigMemDialog()
{
    delete ui;
}



void RigMemDialog::onFreqEditFinish()
{
    QString f = ui->freqLineEdit->text().trimmed().remove( QRegExp("^[0]*"));
    if (f.contains('.'))
    {
        QStringList fl = f.split('.');
        if (fl[1].count() > 6)
        {
            fl[1].truncate(6);
            f = fl[0] + "." + fl[1];
        }

    }
    if (!f.isEmpty() && !validateFreqTxtInput(f))
    {
        // error
        QMessageBox msgBox;
        msgBox.setText(FREQ_EDIT_ERR_MSG);
        msgBox.exec();

    }
}



void RigMemDialog::setLogData(memoryData::memData* ldata, int buttonNumber, LoggerContestLog *ct)
{
    memoryNumber = buttonNumber;
    logData = ldata;
    ui->modecb->setCurrentText(ldata->mode);

    ui->workedCB->setChecked(ldata->worked);

    ui->callSignLineEdit->setText(ldata->callsign);
    ui->locatorLineEdit->setText(ldata->locator);

    int brg = ldata->bearing;
    if (!ldata->locator.isEmpty())
    {
        double lon = 0.0;
        double lat = 0.0;
        int lres = lonlat( ldata->locator, lon, lat );
        if ( lres == LOC_OK )
        {
            double dist;
            ct->disbear( lon, lat, dist, brg );
        }
    }

    if (brg == COMPASS_ERROR)
    {
        ui->bearingLineEdit->setText("000");
    }
    else
    {
        QString number = QString("%1").arg(brg, 3, 10, QChar('0'));
        ui->bearingLineEdit->setText(number);
    }

    if (ldata->freq.isEmpty())
    {
        ui->freqLineEdit->setText("");
    }
    else
    {
        ui->freqLineEdit->setText(convertFreqStrDispSingle(ldata->freq).remove( QRegExp("0+$"))); //remove trailing zeros);
    }

    ui->timeLineEdit->setText(ldata->time);
}

void RigMemDialog::on_okButton_clicked()
{
    logData->callsign = ui->callSignLineEdit->text().trimmed();
    if (logData->callsign.isEmpty())
        logData->callsign = "??";

    logData->worked = ui->workedCB->isChecked();

    //QString f = convertSinglePeriodFreqToMultiPeriod(ui->freqLineEdit->text());
    QString f = ui->freqLineEdit->text().remove( QRegExp("^[0]*")); //remove periods and leading zeros
    if (f.isEmpty())
    {
        logData->freq = f;
    }
    else
    {
        QStringList fl = f.split('.');
        if (fl.count() == 0)
        {
            trace(QString("Memory Freq Edit - Missing Period - %1").arg(f));
            return;
        }

        fl[1] = fl[1] + "0000000";
        fl[1].truncate(6);

        logData->freq = fl[0] + fl[1];
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


