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
#include "rigcommon.h"


QString convertBandKey(QString band)
{
    //return band.remove('\x20').replace('.', '_');
    return band;
}


FreqPresetDialog::FreqPresetDialog(bool hfFlag_, const QVector<QSharedPointer<BandInfo> > &band, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FreqPresetDialog)
{
    ui->setupUi(this);
    bands = band;
    hfFlag = hfFlag_;


    cwPresetLineEditList << ui->cwLineEdit_1_8mhz << ui->cwLineEdit_3_5mhz << ui->cwLineEdit_7mhz
                         << ui->cwLineEdit_14mhz << ui->cwLineEdit_21mhz << ui->cwLineEdit_28mhz
                         << ui->cwLineEdit_50mhz << ui->cwLineEdit_70mhz << ui->cwLineEdit_144mhz
                         << ui->cwLineEdit_432mhz << ui->cwLineEdit_1296mhz << ui->cwLineEdit_2300mhz
                         <<ui->cwLineEdit_3_4ghz << ui->cwLineEdit_5_6ghz << ui->cwLineEdit_10ghz;


    for (int i = 0; i < cwPresetLineEditList.count(); i++)
    {
        connect(cwPresetLineEditList[i], &QLineEdit::editingFinished, [=]() {onCwPresetLineEditingFinished(i);});

    }

    phonePresetLineEditList << ui->phoneLineEdit_1_8mhz << ui->phoneLineEdit_3_5mhz << ui->phoneLineEdit_7mhz
                            << ui->phoneLineEdit_14mhz << ui->phoneLineEdit_21mhz << ui->phoneLineEdit_28mhz
                            << ui->phoneLineEdit_50mhz << ui->phoneLineEdit_70mhz << ui->phoneLineEdit_144mhz
                            << ui->phoneLineEdit_432mhz << ui->phoneLineEdit_1296mhz << ui->phoneLineEdit_2300mhz
                            <<ui->phoneLineEdit_3_4ghz << ui->phoneLineEdit_5_6ghz << ui->phoneLineEdit_10ghz;

    for (int i = 0; i < phonePresetLineEditList.count(); i++)
    {
        connect(phonePresetLineEditList[i], &QLineEdit::editingFinished, [=]() {onPhonePresetLineEditingFinished(i);});

    }

    mgmPresetLineEditList << ui->mgmLineEdit_1_8mhz << ui->mgmLineEdit_3_5mhz << ui->mgmLineEdit_7mhz
                          << ui->mgmLineEdit_14mhz << ui->mgmLineEdit_21mhz << ui->mgmLineEdit_28mhz
                          << ui->mgmLineEdit_50mhz << ui->mgmLineEdit_70mhz << ui->mgmLineEdit_144mhz
                          << ui->mgmLineEdit_432mhz << ui->mgmLineEdit_1296mhz << ui->mgmLineEdit_2300mhz
                          << ui->mgmLineEdit_3_4ghz << ui->mgmLineEdit_5_6ghz << ui->mgmLineEdit_10ghz;

    for (int i = 0; i < mgmPresetLineEditList.count(); i++)
    {
        connect(mgmPresetLineEditList[i], &QLineEdit::editingFinished, [=]() {onMgmPresetLineEditingFinished(i);});

    }

    hfLineEdits << ui->cwLineEdit_1_8mhz << ui->cwLineEdit_3_5mhz << ui->cwLineEdit_7mhz
                << ui->cwLineEdit_14mhz << ui->cwLineEdit_21mhz << ui->cwLineEdit_28mhz
                << ui->phoneLineEdit_1_8mhz << ui->phoneLineEdit_3_5mhz << ui->phoneLineEdit_7mhz
                << ui->phoneLineEdit_14mhz << ui->phoneLineEdit_21mhz << ui->phoneLineEdit_28mhz
                << ui->mgmLineEdit_1_8mhz << ui->mgmLineEdit_3_5mhz << ui->mgmLineEdit_7mhz
                << ui->mgmLineEdit_14mhz << ui->mgmLineEdit_21mhz << ui->mgmLineEdit_28mhz;





    hfLabels << ui->cw_1_8MHzLbl << ui->cw_3_5MHzLbl << ui->cw_7MHzLbl
             << ui->cw_14MHzLbl  << ui->cw_21MHzLbl << ui->cw_28MHzLbl
             << ui->phone_1_8MHzLbl << ui->phone_3_5MHzLbl << ui->phone_7MHzLbl
             << ui->phone_14MHzLbl  << ui->phone_21MHzLbl << ui->phone_28MHzLbl
             << ui->mgm_1_8MHzLbl << ui->mgm_3_5MHzLbl << ui->mgm_7MHzLbl
             << ui->mgm_14MHzLbl  << ui->mgm_21MHzLbl << ui->mgm_28MHzLbl;




    setHf(hfFlag);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [=](){accept();});
    connect(ui->buttonBox, &QDialogButtonBox::rejected, [=](){reject();});

    readSettings(presetFreq, bands); // static

    loadSettingsToDialog();



}


void FreqPresetDialog::setHf(bool hfFlag)
{
    for(auto &lbl: hfLabels)
    {
       lbl->setVisible(hfFlag);
    }
    for(auto &ledt: hfLineEdits)
    {
        ledt->setVisible(hfFlag);
    }

}

void FreqPresetDialog::onCwPresetLineEditingFinished(int i)
{
    getFreq(cwPresetLineEditList[i], i);
}

void FreqPresetDialog::onPhonePresetLineEditingFinished(int i)
{
    getFreq(phonePresetLineEditList[i], i);
}

void FreqPresetDialog::onMgmPresetLineEditingFinished(int i)
{
    getFreq(mgmPresetLineEditList[i], i);
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
           QString mode;

           if (cwPresetLineEditList.contains(f_box))
           {
               mode = freqPresetData::PRESET_MODE_CW;
           }
           else if (phonePresetLineEditList.contains(f_box))
           {
               mode = freqPresetData::PRESET_MODE_PHONE;
           }
           else
           {
               mode = freqPresetData::PRESET_MODE_MGM;
           }

           presetFreq.setPresetFreq(mode, bands[band].data()->uk, freq);


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

    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_CW, config);
    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_PHONE, config);
    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_MGM, config);

}


void FreqPresetDialog::saveModePresetFreqSettings(QString mode, QSettings &config)
{
    config.beginGroup(mode);
    for (int i = 0; i < bands.count(); i++)
    {
        config.setValue(bands[i].data()->uk, presetFreq.getPresetFreq(mode, bands[i].data()->uk).str());
    }
    config.endGroup();
}





void FreqPresetDialog::checkPreviousVersionIniFile(PresetFreq& presetFreq, const QVector<QSharedPointer<BandInfo> > &bands)  // static
{
    QString fileName = RADIO_PATH_LOGGER + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);

    QStringList keyList = config.childGroups();
    if (keyList.contains("FreqPresets"))    // first version of freqPresets
    {
        config.beginGroup("FreqPresets");
        for (int i = 0; i < bands.count(); i++)
        {

            presetFreq.setPresetFreq(freqPresetData::PRESET_MODE_PHONE, bands[i].data()->uk, Frequency(config.value(bands[i].data()->uk, freqPresetData::bandFreq[i]).toString()));

        }
        config.endGroup();

        config.beginGroup(freqPresetData::PRESET_MODE_PHONE);
        for (int i = 0; i < bands.count(); i++)
        {
            config.setValue(bands[i].data()->uk, presetFreq.getPresetFreq(freqPresetData::PRESET_MODE_PHONE, bands[i].data()->uk).str());
        }
        config.endGroup();

        config.remove("FreqPresets");
    }
}



void FreqPresetDialog::readSettings(PresetFreq  &presetFreq, const QVector<QSharedPointer<BandInfo> > &bands)  // static
{

    QString fileName = RADIO_PATH_LOGGER + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);

    presetFreq.clear();


    config.beginGroup(freqPresetData::PRESET_MODE_CW);

    for (int i = 0; i < bands.count(); i++)
    {
        presetFreq.setPresetFreq(freqPresetData::PRESET_MODE_CW, bands[i].data()->uk, Frequency(config.value(bands[i].data()->uk, freqPresetData::bandFreq[i]).toString()));

    }

    config.endGroup();

    config.beginGroup(freqPresetData::PRESET_MODE_PHONE);

    for (int i = 0; i < bands.count(); i++)
    {
        presetFreq.setPresetFreq(freqPresetData::PRESET_MODE_PHONE, bands[i].data()->uk, Frequency(config.value(bands[i].data()->uk, freqPresetData::bandFreq[i]).toString()));

    }

    config.endGroup();

    config.beginGroup(freqPresetData::PRESET_MODE_MGM);

    for (int i = 0; i < bands.count(); i++)
    {
        presetFreq.setPresetFreq(freqPresetData::PRESET_MODE_MGM, bands[i].data()->uk, Frequency(config.value(bands[i].data()->uk, freqPresetData::bandFreq[i]).toString()));

    }

    config.endGroup();


}







void FreqPresetDialog::loadSettingsToDialog()
{

    for (int i = 0; i < cwPresetLineEditList.count(); i++)
    {

        cwPresetLineEditList[i]->setText(presetFreq.getPresetFreq(freqPresetData::PRESET_MODE_CW, bands[i].data()->uk).convertFreqStrDispSingleNoTrailZero());
    }

    for (int i = 0; i < phonePresetLineEditList.count(); i++)
    {

        phonePresetLineEditList[i]->setText(presetFreq.getPresetFreq(freqPresetData::PRESET_MODE_PHONE, bands[i].data()->uk).convertFreqStrDispSingleNoTrailZero());
    }

    for (int i = 0; i < mgmPresetLineEditList.count(); i++)
    {

        mgmPresetLineEditList[i]->setText(presetFreq.getPresetFreq(freqPresetData::PRESET_MODE_MGM, bands[i].data()->uk).convertFreqStrDispSingleNoTrailZero());
    }

}

