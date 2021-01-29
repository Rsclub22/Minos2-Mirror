/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018 - 2021
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#include <QMessageBox>
#include "radiosettingdialog.h"
#include "ui_radiosettingdialog.h"
#include "rigcommon.h"
#include "ContestApp.h"





RadioSettingDialog::RadioSettingDialog(bool hfFlag_, const QVector<QSharedPointer<BandInfo> > &band, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RadioSettingDialog)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Log Radio Settings"));

    bands = band;
    hfFlag = hfFlag_;


    cwPresetLineEditList << ui->cwLineEdit_1_8mhz << ui->cwLineEdit_3_5mhz << ui->cwLineEdit_7mhz
                         << ui->cwLineEdit_14mhz << ui->cwLineEdit_21mhz << ui->cwLineEdit_28mhz
                         << ui->cwLineEdit_50mhz << ui->cwLineEdit_70mhz << ui->cwLineEdit_144mhz
                         << ui->cwLineEdit_432mhz << ui->cwLineEdit_1296mhz << ui->cwLineEdit_2300mhz
                         <<ui->cwLineEdit_3_4ghz << ui->cwLineEdit_5_6ghz << ui->cwLineEdit_10ghz;


    for (int i = 0; i < cwPresetLineEditList.count(); i++)
    {
        connect(cwPresetLineEditList[i], &QLineEdit::editingFinished, this, [=]() {onCwPresetLineEditingFinished(i);});

    }

    phonePresetLineEditList << ui->phoneLineEdit_1_8mhz << ui->phoneLineEdit_3_5mhz << ui->phoneLineEdit_7mhz
                            << ui->phoneLineEdit_14mhz << ui->phoneLineEdit_21mhz << ui->phoneLineEdit_28mhz
                            << ui->phoneLineEdit_50mhz << ui->phoneLineEdit_70mhz << ui->phoneLineEdit_144mhz
                            << ui->phoneLineEdit_432mhz << ui->phoneLineEdit_1296mhz << ui->phoneLineEdit_2300mhz
                            <<ui->phoneLineEdit_3_4ghz << ui->phoneLineEdit_5_6ghz << ui->phoneLineEdit_10ghz;

    for (int i = 0; i < phonePresetLineEditList.count(); i++)
    {
        connect(phonePresetLineEditList[i], &QLineEdit::editingFinished, this, [=]() {onPhonePresetLineEditingFinished(i);});

    }

    mgmPresetLineEditList << ui->mgmLineEdit_1_8mhz << ui->mgmLineEdit_3_5mhz << ui->mgmLineEdit_7mhz
                          << ui->mgmLineEdit_14mhz << ui->mgmLineEdit_21mhz << ui->mgmLineEdit_28mhz
                          << ui->mgmLineEdit_50mhz << ui->mgmLineEdit_70mhz << ui->mgmLineEdit_144mhz
                          << ui->mgmLineEdit_432mhz << ui->mgmLineEdit_1296mhz << ui->mgmLineEdit_2300mhz
                          << ui->mgmLineEdit_3_4ghz << ui->mgmLineEdit_5_6ghz << ui->mgmLineEdit_10ghz;

    for (int i = 0; i < mgmPresetLineEditList.count(); i++)
    {
        connect(mgmPresetLineEditList[i], &QLineEdit::editingFinished, this, [=]() {onMgmPresetLineEditingFinished(i);});

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

    //connect(ui->turnOffColourRadioFreqDialChkBox, &QCheckBox::clicked, this, [=](bool checked){onTurnOffColourRadioFreqDialChkChanged(checked);});
    //connect(ui->contestStartIgnorePresetFreqChkBox, &QCheckBox::clicked, this, [=](bool checked){onIgnorePreviousFreqChecked(checked);});
    //connect(ui->contestChangeIgnorePreviousFreqChkBox, &QCheckBox::clicked, this, [=](bool checked){onIgnorePresetFreqChecked(checked);});
    //connect(ui->constestChangeRestoreContestModeChkBox, &QCheckBox::clicked, this, [=](bool checked){onRestoreContestModeChecked(checked);});

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [=](){onAccepted();});


    freqPresetReadSettings(presetFreq, bands); // static

    ui->turnOffColourRadioFreqDialChkBox->setChecked(readRadioSettingsCheckBox(elpContestTurnOffOperatingFreqColorRadioDial));
    ui->contestStartIgnorePresetFreqChkBox->setChecked(readRadioSettingsCheckBox(elpContestStartIgnorePresetFreq));
    ui->contestChangeIgnorePreviousFreqChkBox->setChecked(readRadioSettingsCheckBox(elpContestChangeIgnorePreviousFreq));
    ui->constestChangeRestoreContestModeChkBox->setChecked(readRadioSettingsCheckBox(elpContestChangeRestoreContestMode));

    loadSettingsToDialog();


}

RadioSettingDialog::~RadioSettingDialog()
{
    delete ui;
}


void RadioSettingDialog::setHf(bool hfFlag)
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

void RadioSettingDialog::onCwPresetLineEditingFinished(int i)
{
    getFreq(cwPresetLineEditList[i], i);
}

void RadioSettingDialog::onPhonePresetLineEditingFinished(int i)
{
    getFreq(phonePresetLineEditList[i], i);
}

void RadioSettingDialog::onMgmPresetLineEditingFinished(int i)
{
    getFreq(mgmPresetLineEditList[i], i);
}




void RadioSettingDialog::getFreq(QLineEdit* f_box, int band)
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

bool RadioSettingDialog::checkInBand(Frequency freq, int band)
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


void RadioSettingDialog::onAccepted()
{
    saveSettings();
}



void RadioSettingDialog::saveSettings()
{

    QString fileName = RADIO_PATH_LOGGER + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);

    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_CW, config);
    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_PHONE, config);
    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_MGM, config);

    saveRadioSettingsCheckBoxes();

}


void RadioSettingDialog::saveModePresetFreqSettings(QString mode, QSettings &config)
{
    config.beginGroup(mode);
    for (int i = 0; i < bands.count(); i++)
    {
        config.setValue(bands[i].data()->uk, presetFreq.getPresetFreq(mode, bands[i].data()->uk).str());
    }
    config.endGroup();
}


void RadioSettingDialog::saveRadioSettingsCheckBoxes()
{
    saveRadioSettingsCheckBox(ui->turnOffColourRadioFreqDialChkBox, elpContestTurnOffOperatingFreqColorRadioDial);
    saveRadioSettingsCheckBox(ui->contestStartIgnorePresetFreqChkBox, elpContestStartIgnorePresetFreq);
    saveRadioSettingsCheckBox(ui->contestChangeIgnorePreviousFreqChkBox, elpContestChangeIgnorePreviousFreq);
    saveRadioSettingsCheckBox(ui->constestChangeRestoreContestModeChkBox, elpContestChangeRestoreContestMode);


}


void RadioSettingDialog::checkPreviousVersionIniFile(PresetFreq& presetFreq, const QVector<QSharedPointer<BandInfo> > &bands)  // static
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



void RadioSettingDialog::freqPresetReadSettings(PresetFreq  &presetFreq, const QVector<QSharedPointer<BandInfo> > &bands)  // static
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







void RadioSettingDialog::loadSettingsToDialog()
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




void RadioSettingDialog::onTurnOffColourRadioFreqDialChkChanged(bool checked)
{
    TContestApp::getContestApp()->loggerBundle.setBoolProfile(elpContestTurnOffOperatingFreqColorRadioDial, checked);

}


void RadioSettingDialog::onIgnorePreviousFreqChecked(bool checked)
{
    TContestApp::getContestApp()->loggerBundle.setBoolProfile(elpContestChangeIgnorePreviousFreq, checked);

}

void RadioSettingDialog::onIgnorePresetFreqChecked(bool checked)
{
    TContestApp::getContestApp()->loggerBundle.setBoolProfile(elpContestStartIgnorePresetFreq, checked);

}

bool RadioSettingDialog::readRadioSettingsCheckBox(LOGGERPROFILE profile)
{

    bool state;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( profile, state );
    return state;
}


void RadioSettingDialog::saveRadioSettingsCheckBox(QCheckBox* chkbox, LOGGERPROFILE profile)
{
    if (chkbox->isChecked() != readRadioSettingsCheckBox(profile))
    {
       TContestApp::getContestApp()->loggerBundle.setBoolProfile(profile, chkbox->isChecked());

    }

}


void RadioSettingDialog::onRestoreContestModeChecked(bool checked)
{
    TContestApp::getContestApp()->loggerBundle.setBoolProfile(elpContestChangeRestoreContestMode, checked);

}
