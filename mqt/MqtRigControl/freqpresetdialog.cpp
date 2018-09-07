/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
//
/////////////////////////////////////////////////////////////////////////////






#include <QMessageBox>
#include "freqpresetdialog.h"
#include "ui_freqpresetdialog.h"

FreqPresetDialog::FreqPresetDialog(QStringList& _presetFreq, const QVector<BandDetail*> band, bool* _freqPresetChanged, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FreqPresetDialog)
{
    ui->setupUi(this);
    bands = band;
    presetFreq = _presetFreq;
    freqPresetChanged = _freqPresetChanged;


    loadSettingsToDialog();

 //   connect (ui->lineEdit_28mhz, SIGNAL(editingFinished()), this, SLOT(b_28mhzSelected()));
    connect (ui->lineEdit_50mhz, SIGNAL(editingFinished()), this, SLOT(b_50mhzSelected()));
    connect (ui->lineEdit_70mhz, SIGNAL(editingFinished()), this, SLOT(b_70mhzSelected()));
    connect (ui->lineEdit_144mhz, SIGNAL(editingFinished()), this, SLOT(b_144mhzSelected()));
    connect (ui->lineEdit_432mhz, SIGNAL(editingFinished()), this, SLOT(b_432mhzSelected()));

    connect (ui->lineEdit_1296mhz, SIGNAL(editingFinished()), this, SLOT(b_1296mhzSelected()));
    connect (ui->lineEdit_2300mhz, SIGNAL(editingFinished()), this, SLOT(b_2300mhzSelected()));
    connect (ui->lineEdit_3_4ghz, SIGNAL(editingFinished()), this, SLOT(b_3_4ghzSelected()));
    connect (ui->lineEdit_5_6ghz, SIGNAL(editingFinished()), this, SLOT(b_5_6ghzSelected()));
    connect (ui->lineEdit_10ghz, SIGNAL(editingFinished()), this, SLOT(b_10ghzSelected()));


    connect (ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect (ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancelSettings()));




}

FreqPresetDialog::~FreqPresetDialog()
{
    delete ui;
}


/*
void FreqPresetDialog::b_28mhzSelected()
{

    getFreq(ui->lineEdit_28mhz, _28MHZ);

}
*/

void FreqPresetDialog::b_50mhzSelected()
{
    getFreq(ui->lineEdit_50mhz, freqPresetData::_50MHZ);
}

void FreqPresetDialog::b_70mhzSelected()
{
    getFreq(ui->lineEdit_70mhz, freqPresetData::_70MHZ);
}


void FreqPresetDialog::b_144mhzSelected()
{
    getFreq(ui->lineEdit_144mhz, freqPresetData::_144MHZ);
}

void FreqPresetDialog::b_432mhzSelected()
{
    getFreq(ui->lineEdit_432mhz, freqPresetData::_432MHZ);
}
void FreqPresetDialog::b_1296mhzSelected()
{
    getFreq(ui->lineEdit_1296mhz, freqPresetData::_1296MHZ);
}

void FreqPresetDialog::b_2300mhzSelected()
{
    getFreq(ui->lineEdit_2300mhz, freqPresetData::_2300MHZ);
}


void FreqPresetDialog::b_3_4ghzSelected()
{
    getFreq(ui->lineEdit_3_4ghz, freqPresetData::_3_4GHZ);
}


void FreqPresetDialog::b_5_6ghzSelected()
{
    getFreq(ui->lineEdit_5_6ghz, freqPresetData::_5_6GHZ);
}


void FreqPresetDialog::b_10ghzSelected()
{
    getFreq(ui->lineEdit_10ghz, freqPresetData::_10GHZ);
}




void FreqPresetDialog::getFreq(QLineEdit* f_box, freqPresetData::bandOffSet band)
{

    QString freq = f_box->text().trimmed().remove( QRegExp("^[0]*"));
    if (valInputFreq(freq, RADIO_FREQ_EDIT_ERR_MSG))
    {
       freq = convertFreqToFullDigit(freq).remove('.');


       // check in band
       if (checkInBand(freq.toDouble(), band))
       {
           presetFreq[band] = freq;
           freqChanged = true;
           *freqPresetChanged = true;
       }
    }

}




// check in band

bool FreqPresetDialog::checkInBand(double freq, freqPresetData::bandOffSet band)
{
    if (freq >= bands[band]->fLow && freq <= bands[band]->fHigh)
    {
        return true;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(QString("Freq. is out of band for %1").arg(bands[band]->name));
        msgBox.exec();
        return false;
    }
}






void FreqPresetDialog::saveSettings()
{

    if (!freqChanged)
    {
        return;
    }

    freqChanged = false;


    QString fileName = RADIO_PATH_LOGGER + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("FreqPresets");
//    config.setValue("28MHz", presetFreq[_28MHZ]);
    config.setValue("50MHz", presetFreq[freqPresetData::_50MHZ]);
    config.setValue("70MHz", presetFreq[freqPresetData::_70MHZ]);
    config.setValue("144MHz", presetFreq[freqPresetData::_144MHZ]);
    config.setValue("432MHz", presetFreq[freqPresetData::_432MHZ]);
    config.setValue("1296MHz", presetFreq[freqPresetData::_1296MHZ]);
    config.setValue("2300MHz", presetFreq[freqPresetData::_2300MHZ]);
    config.setValue("3_4GHz", presetFreq[freqPresetData::_3_4GHZ]);
    config.setValue("5_6GHz", presetFreq[freqPresetData::_5_6GHZ]);
    config.setValue("10GHz", presetFreq[freqPresetData::_10GHZ]);


    config.endGroup();




}


void FreqPresetDialog::readSettings(QStringList& presetFreq)  // static
{

    QString fileName = RADIO_PATH_LOGGER + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);

    presetFreq.clear();

    config.beginGroup("FreqPresets");

 //   _presetFreq.append(config.value("28MHz", bandFreq[_28MHZ]).toString());
    presetFreq.append(config.value("50MHz", freqPresetData::bandFreq[freqPresetData::_50MHZ]).toString());
    presetFreq.append(config.value("70MHz", freqPresetData::bandFreq[freqPresetData::_70MHZ]).toString());
    presetFreq.append(config.value("144MHz", freqPresetData::bandFreq[freqPresetData::_144MHZ]).toString());
    presetFreq.append(config.value("432MHz", freqPresetData::bandFreq[freqPresetData::_432MHZ]).toString());
    presetFreq.append(config.value("1296MHz", freqPresetData::bandFreq[freqPresetData::_1296MHZ]).toString());
    presetFreq.append(config.value("2300MHZ", freqPresetData::bandFreq[freqPresetData::_2300MHZ]).toString());
    presetFreq.append(config.value("3_4GHz", freqPresetData::bandFreq[freqPresetData::_3_4GHZ]).toString());
    presetFreq.append(config.value("5_6GHz", freqPresetData::bandFreq[freqPresetData::_5_6GHZ]).toString());
    presetFreq.append(config.value("10GHz", freqPresetData::bandFreq[freqPresetData::_10GHZ]).toString());


    config.endGroup();

}


void FreqPresetDialog::cancelSettings()
{

    if (!freqChanged)
    {
        return;
    }

    freqChanged = false;
    *freqPresetChanged = false;
    readSettings(presetFreq);
    loadSettingsToDialog();



}


void FreqPresetDialog::loadSettingsToDialog()
{
//    ui->lineEdit_28mhz->setText(convertFreqStrDispSingleNoTrailZero(presetFreq[_28MHZ]));
    ui->lineEdit_50mhz->setText(convertFreqStrDispSingleNoTrailZero(presetFreq[freqPresetData::_50MHZ]));
    ui->lineEdit_70mhz->setText(convertFreqStrDispSingleNoTrailZero(presetFreq[freqPresetData::_70MHZ]));
    ui->lineEdit_144mhz->setText(convertFreqStrDispSingleNoTrailZero(presetFreq[freqPresetData::_144MHZ]));
    ui->lineEdit_432mhz->setText(convertFreqStrDispSingleNoTrailZero(presetFreq[freqPresetData::_432MHZ]));
    ui->lineEdit_1296mhz->setText(convertFreqStrDispSingleNoTrailZero(presetFreq[freqPresetData::_1296MHZ]));
    ui->lineEdit_2300mhz->setText(convertFreqStrDispSingleNoTrailZero(presetFreq[freqPresetData::_2300MHZ]));
    ui->lineEdit_3_4ghz->setText(convertFreqStrDispSingleNoTrailZero(presetFreq[freqPresetData::_3_4GHZ]));
    ui->lineEdit_5_6ghz->setText(convertFreqStrDispSingleNoTrailZero(presetFreq[freqPresetData::_5_6GHZ]));
    ui->lineEdit_10ghz->setText(convertFreqStrDispSingleNoTrailZero(presetFreq[freqPresetData::_10GHZ]));

}
