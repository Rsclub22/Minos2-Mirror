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

#include "base_pch.h"

#include "rigmemcommondata.h"
#include "rigutils.h"
//#include "rigcontrolcommonconstants.h"
#include "rigcommon.h"
#include "rotatorcommon.h"

#include "runbuttondialog.h"
#include "ui_runbuttondialog.h"

RunButtonDialog::RunButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RunButtonDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    for (auto const &sm: supModeList)
    {
        ui->modecb->addItem(sm);
    }


    ui->freqLineEdit->setFocus();
    // validate the input
    connect(ui->freqLineEdit, SIGNAL(editingFinished()), this, SLOT(onFreqEditFinish()));

}




RunButtonDialog::~RunButtonDialog()
{
    delete ui;
}


void RunButtonDialog::onFreqEditFinish()
{
    QString f = ui->freqLineEdit->text().trimmed().remove( QRegularExpression("^[0]*"));
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
        msgBox.setText(tr(FREQ_EDIT_ERR_MSG));
        msgBox.exec();

    }
}

void RunButtonDialog::setLogData(memoryData::memData* ldata, int buttonNumber)
{

    memoryNumber = buttonNumber;
    logdata = ldata;

    //setWindowTitle(QString("Run%1 - Edit").arg(QString::number(buttonNumber + 1)));

    ui->modecb->setCurrentText(ldata->mode);


    if (logdata->freq.isClear())
    {
        ui->freqLineEdit->setText("");
    }
    else
    {
        ui->freqLineEdit->setText(ldata->freq.convertFreqStrDispSingleNoTrailZero());
    }


}

void RunButtonDialog::on_okButton_clicked()
{
    // update run data
    //logdata->freq = ui->freqLineEdit->text().remove('.');
    QString f = ui->freqLineEdit->text().remove( QRegularExpression("^[0]*")); //remove periods and leading zeros
    if (f.isEmpty())
    {
        logdata->freq = f;
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

        logdata->freq = Frequency(fl[0] + fl[1]);

    }
    logdata->mode = ui->modecb->currentText();

    accept();
}

void RunButtonDialog::on_cancelbutton_clicked()
{
    reject();
}


