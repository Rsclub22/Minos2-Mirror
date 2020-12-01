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

FreqPresetDialog::FreqPresetDialog(const QVector<QSharedPointer<BandInfo> > &band, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FreqPresetDialog)
{
    ui->setupUi(this);
    bands = band;




    presetFreqLineEditList << ui->lineEdit_1_8mhz << ui->lineEdit_3_5mhz << ui->lineEdit_7mhz
                           << ui->lineEdit_14mhz << ui->lineEdit_21mhz << ui->lineEdit_28mhz
                           << ui->lineEdit_50mhz << ui->lineEdit_70mhz << ui->lineEdit_144mhz
                           << ui->lineEdit_432mhz << ui->lineEdit_1296mhz << ui->lineEdit_2300mhz
                           <<ui->lineEdit_3_4ghz << ui->lineEdit_5_6ghz << ui->lineEdit_10ghz;





    for (int i = 0; i < presetFreqLineEditList.count(); i++)
    {
        connect(presetFreqLineEditList[i], &QLineEdit::editingFinished, [=]() {onbandCheckBoxStateChanged(i);});

    }

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){accept();});
    connect(ui->buttonBox, &QDialogButtonBox::rejected, [=](){reject();});

    readSettings(presetFreq, bands); // static

    loadSettingsToDialog();




}

void FreqPresetDialog::onbandCheckBoxStateChanged(int i)
{
    getFreq(presetFreqLineEditList[i], i);
}

FreqPresetDialog::~FreqPresetDialog()
{
    delete ui;
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
          // *freqPresetChanged = true;
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


    QString fileName = RADIO_PATH_LOGGER + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("FreqPresets");
    for (int i = 0; i < bands.count(); i++)
    {
        QString name = bands[i].data()->uk;
        name.remove('\x20').replace('.', '_');
        config.setValue(name, presetFreq[i]);
    }


    config.endGroup();




}


void FreqPresetDialog::readSettings(QStringList& presetFreq, const QVector<QSharedPointer<BandInfo> > &bands)  // static
{

    QString fileName = RADIO_PATH_LOGGER + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);

    presetFreq.clear();

    config.beginGroup("FreqPresets");

    for (int i = 0; i < bands.count(); i++)
    {
        QString name = bands[i].data()->uk;
        name.remove('\x20').replace('.', '_');
        presetFreq.append(config.value(name, freqPresetData::bandFreq[i]).toString());

    }

    config.endGroup();

}





void FreqPresetDialog::loadSettingsToDialog()
{

    for (int i = 0; i < presetFreq.count(); i++)
    {
        presetFreqLineEditList[i]->setText(Frequency(presetFreq[i]).convertFreqStrDispSingleNoTrailZero());
    }
}
