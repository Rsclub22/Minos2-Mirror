/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include <QMessageBox>
#include "rotpresetdialog.h"
#include "ui_rotpresetdialog.h"


RotPresetDialog::RotPresetDialog(QWidget *parent, int buttonNumber, RotPresetData* _editData, RotPresetData* _curData, QString name) :
    QDialog(parent),
    ui(new Ui::RotPresetDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    editData = _editData;
    curData = _curData;
    setWindowTitle(tr("Rotator Preset %1 - %2").arg(QString::number(buttonNumber + 1)).arg(name));
    ui->nameEdit->setText(curData->name);
    ui->bearingEdit->setText(curData->bearing);
    ui->nameEdit->setFocus();

    //connect(ui->bearingEdit, SIGNAL(editingFinished()), this, SLOT(bearingEditFinished()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(editAccepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(editRejected()));
}

RotPresetDialog::~RotPresetDialog()
{
    delete ui;
}




bool RotPresetDialog::bearingEditFinished()
{
/*

    int b = ui->bearingEdit->text().toInt();
    if (b < 0 || b > 360)
    {
        QMessageBox::information(this, tr("Bearing Out of range"),
                                 tr("Bearing: %1, out of range, should be 0-360 \nPlease enter another bearing").arg(b),
                                  QMessageBox::Ok|QMessageBox::Default,
                                  QMessageBox::NoButton, QMessageBox::NoButton);
        return false;
    }
    return true;

*/

    if (ui->bearingEdit->isValid())
    {
        return true;
    }

    QMessageBox::information(this, tr("Bearing Out of range"),
                             tr("Bearing: %1, out of range, should be 0-360 \nPlease enter another bearing").arg(ui->bearingEdit->text().trimmed()),
                              QMessageBox::Ok|QMessageBox::Default,
                              QMessageBox::NoButton, QMessageBox::NoButton);
    return false;




}


void RotPresetDialog::editAccepted()
{

    if (ui->bearingEdit->text().trimmed() != curData->bearing)
    {
        if (bearingEditFinished())
        {
            editData->bearing = ui->bearingEdit->text().trimmed();
        }
        else
        {
            return;
        }
    }

    if (ui->nameEdit->text().trimmed() != curData->name)
    {
        editData->name = ui->nameEdit->text().trimmed();
    }
}


void RotPresetDialog::editRejected()
{

    copyPresetData(editData, curData);

}


void RotPresetDialog::copyPresetData(RotPresetData* src, RotPresetData* dest)
{
    dest->name = src->name;
    dest->bearing = src->bearing;
    dest->number = src->number;

}
