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
#include <QMessageBox>
#include "rigmemcommondata.h"
#include "rigutils.h"
#include "tsinglelogframe.h"
#include "tlogcontainer.h"
#include "MTrace.h"

#include "runbuttondialog.h"
#include "ui_runbuttondialog.h"

RunButtonDialog::RunButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RunButtonDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->freqLineEdit->setFocus();
    // validate the input
    connect(ui->freqLineEdit, &QLineEdit::editingFinished, this, &RunButtonDialog::onFreqEditFinish);

}




RunButtonDialog::~RunButtonDialog()
{
    delete ui;
}


void RunButtonDialog::onFreqEditFinish()
{
    static QRegularExpression qre("^[0]*");
    QString f = ui->freqLineEdit->text().trimmed().remove( qre);
    if (f.contains('.'))
    {
        QStringList fl = f.split('.');
        if (fl[1].size() > 6)
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

void RunButtonDialog::setLogData(memoryData::memData* ldata, int buttonNumber, LoggerContestLog *ct)
{
    ui->modecb->clear();
    QString mlist = ct->modeList.getValue();
    QStringList ml = mlist.split('|');
    ui->modecb->insertItems(0, ml);

    memoryNumber = buttonNumber;
    logdata = ldata;

    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    ScreenContact sc = tslf->getScreenEntry();

    QString m = ldata->mode;
    if (m.isEmpty())
    {
        logdata->mode = sc.mode.getValue();
    }

    ui->modecb->setCurrentText(ldata->mode);


    if (logdata->freq.isClear())
    {
        logdata->freq = sc.getFrequency().getValue();
    }
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
    static QRegularExpression qre("^[0]*");
    QString f = ui->freqLineEdit->text().remove( qre); //remove periods and leading zeros
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


