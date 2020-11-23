/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////






#include <QMessageBox>
#include "freqpresetdialog.h"
#include "ui_freqpresetdialog.h"

FreqPresetDialog::FreqPresetDialog(QStringList& _presetFreq, const QVector<QSharedPointer<BandInfo> > &band, bool* _freqPresetChanged, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FreqPresetDialog)
{
    ui->setupUi(this);
    bands = band;
    presetFreq = _presetFreq;
    freqPresetChanged = _freqPresetChanged;


    loadSettingsToDialog();

 //   connect (ui->lineEdit_28mhz, SIGNAL(editingFinished()), this, SLOT(b_28mhzSelected()));
    presetFreqLineEditList << ui->lineEdit_1_8mhz << ui->lineEdit_3_5mhz << ui->lineEdit_7mhz
                           << ui->lineEdit_14mhz << ui->lineEdit_21mhz << ui->lineEdit_28mhz
                           << ui->lineEdit_50mhz << ui->lineEdit_70mhz << ui->lineEdit_144mhz
                           << ui->lineEdit_432mhz << ui->lineEdit_1296mhz << ui->lineEdit_2300mhz
                           <<ui->lineEdit_3_4ghz << ui->lineEdit_5_6ghz << ui->lineEdit_10ghz;





    for (int i = 0; i < presetFreqLineEditList.count(); i++)
    {
        connect(presetFreqLineEditList[i], &QLineEdit::editingFinished, [=]() {onbandCheckBoxStateChanged(i);});

    }




/*
    connect (ui->lineEdit_50mhz, SIGNAL(editingFinished()), this, SLOT(b_50mhzSelected()));
    connect (ui->lineEdit_70mhz, SIGNAL(editingFinished()), this, SLOT(b_70mhzSelected()));
    connect (ui->lineEdit_144mhz, SIGNAL(editingFinished()), this, SLOT(b_144mhzSelected()));
    connect (ui->lineEdit_432mhz, SIGNAL(editingFinished()), this, SLOT(b_432mhzSelected()));

    connect (, SIGNAL(editingFinished()), this, SLOT(b_1296mhzSelected()));
    connect (, SIGNAL(editingFinished()), this, SLOT(b_2300mhzSelected()));
    connect (, SIGNAL(editingFinished()), this, SLOT(b_3_4ghzSelected()));
    connect (, SIGNAL(editingFinished()), this, SLOT(b_5_6ghzSelected()));
    connect (, SIGNAL(editingFinished()), this, SLOT(b_10ghzSelected()));
*/
    connect (ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect (ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancelSettings()));




}

void FreqPresetDialog::onbandCheckBoxStateChanged(int i)
{
    getFreq(presetFreqLineEditList[i], i);
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




void FreqPresetDialog::getFreq(QLineEdit* f_box, int band)
{

    QString freq = f_box->text().trimmed().remove( QRegularExpression("^[0]*"));
    if (valInputFreq(freq, tr(RADIO_FREQ_EDIT_ERR_MSG)))
    {
       freq = convertFreqToFullDigit(freq).remove('.');


       // check in band
       if (checkInBand(Frequency(freq), band))
       {
           presetFreq[band] = freq;
           freqChanged = true;
           *freqPresetChanged = true;
       }
    }

}




// check in band

bool FreqPresetDialog::checkInBand(Frequency freq, int band)
{
    if (freq >= bands[band]->fLow && freq <= bands[band]->fHigh)
    {
        return true;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Freq. is out of band for %1").arg(bands[band]->name()));
        msgBox.exec();
        return false;
    }
    return false;
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
    config.setValue("1_8MHz", presetFreq[freqPresetData::_1_8MHZ]);
    config.setValue("3_5MHz", presetFreq[freqPresetData::_3_5MHZ]);
    config.setValue("7MHz", presetFreq[freqPresetData::_7MHZ]);
    config.setValue("14MHz", presetFreq[freqPresetData::_14MHZ]);
    config.setValue("21MHz", presetFreq[freqPresetData::_21MHZ]);
    config.setValue("28MHz", presetFreq[freqPresetData::_28MHZ]);
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
    presetFreq.append(config.value("1_8MHz", freqPresetData::bandFreq[freqPresetData::_1_8MHZ]).toString());
    presetFreq.append(config.value("3_5MHz", freqPresetData::bandFreq[freqPresetData::_3_5MHZ]).toString());
    presetFreq.append(config.value("7MHz", freqPresetData::bandFreq[freqPresetData::_7MHZ]).toString());
    presetFreq.append(config.value("14MHz", freqPresetData::bandFreq[freqPresetData::_14MHZ]).toString());
    presetFreq.append(config.value("21MHz", freqPresetData::bandFreq[freqPresetData::_21MHZ]).toString());
    presetFreq.append(config.value("28MHz", freqPresetData::bandFreq[freqPresetData::_28MHZ]).toString());
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
    ui->lineEdit_50mhz->setText(Frequency(presetFreq[freqPresetData::_50MHZ]).convertFreqStrDispSingleNoTrailZero());
    ui->lineEdit_70mhz->setText(Frequency(presetFreq[freqPresetData::_70MHZ]).convertFreqStrDispSingleNoTrailZero());
    ui->lineEdit_144mhz->setText(Frequency(presetFreq[freqPresetData::_144MHZ]).convertFreqStrDispSingleNoTrailZero());
    ui->lineEdit_432mhz->setText(Frequency(presetFreq[freqPresetData::_432MHZ]).convertFreqStrDispSingleNoTrailZero());
    ui->lineEdit_1296mhz->setText(Frequency(presetFreq[freqPresetData::_1296MHZ]).convertFreqStrDispSingleNoTrailZero());
    ui->lineEdit_2300mhz->setText(Frequency(presetFreq[freqPresetData::_2300MHZ]).convertFreqStrDispSingleNoTrailZero());
    ui->lineEdit_3_4ghz->setText(Frequency(presetFreq[freqPresetData::_3_4GHZ]).convertFreqStrDispSingleNoTrailZero());
    ui->lineEdit_5_6ghz->setText(Frequency(presetFreq[freqPresetData::_5_6GHZ]).convertFreqStrDispSingleNoTrailZero());
    ui->lineEdit_10ghz->setText(Frequency(presetFreq[freqPresetData::_10GHZ]).convertFreqStrDispSingleNoTrailZero());
}
