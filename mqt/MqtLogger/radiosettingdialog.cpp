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
#include "rigcommon.h"
#include "rigcontrolcommonconstants.h"
#include "ContestApp.h"
#include "tlogcontainer.h"

#include "radiosettingdialog.h"
#include "ui_radiosettingdialog.h"

RadioSettingDialog::RadioSettingDialog( QWidget *parent) :
    QFrame(parent),
    ui(new Ui::RadioSettingDialog)
{
    ui->setupUi(this);
}

RadioSettingDialog::~RadioSettingDialog()
{
    delete ui;
}

void RadioSettingDialog::initialise()
{
    BandList::getBandList().loadAllBands(bands);
    hfFlag = true;
    logRadioSettingsChangeFlag = QSharedPointer<RadioSettingsDialogChangeFlag>(new RadioSettingsDialogChangeFlag());

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


    bandSwLineEdits << ui->bandSwLineEdit_1_8mhz << ui->bandSwLineEdit_3_5mhz << ui->bandSwLineEdit_7mhz
                    << ui->bandSwLineEdit_14mhz << ui->bandSwLineEdit_21mhz << ui->bandSwLineEdit_28mhz
                    << ui->bandSwLineEdit_50mhz << ui->bandSwLineEdit_70mhz << ui->bandSwLineEdit_144mhz
                    << ui->bandSwLineEdit_432mhz << ui->bandSwLineEdit_1296mhz << ui->bandSwLineEdit_2300mhz
                    << ui->bandSwLineEdit_3_4ghz << ui->bandSwLineEdit_5_6ghz << ui->bandSwLineEdit_10ghz;

    for (int i = 0; i < bandSwLineEdits.count(); i++)
    {
        connect(bandSwLineEdits[i], &QLineEdit::editingFinished, this, [=]() {onBandSwLineEditingFinished(i);});
    }

    bandSwLabels << ui->bandSw_1_8MHzLbl << ui->bandSw_3_5MHzLbl << ui->bandSw_7MHzLbl
                 << ui->bandSw_14MHzLbl << ui->bandSw_21MHzLbl << ui->bandSw_28MHzLbl
                 << ui->bandSw_50MHzLbl << ui->bandSw_70MHzLbl << ui->bandSw_144MHzLbl
                 << ui->bandSw_432MHzLbl << ui->bandSw_1296MHzLbl <<ui->bandSw_2300MHzLbl
                 << ui->bandSw_3_4GHzLbl << ui->bandSw_5_6GHzLbl << ui->bandSw_10GHzLbl;



    for (int i = 0; i < bands.count(); i++)
    {
        BandSwDetails bswd;
        bswd.bandSwLineEdit = bandSwLineEdits[i];
        bswd.bandSwLabel = bandSwLabels[i];
        bswd.bandType = bands[i]->getType();
        bandSwDetails.insert(bands[i].data()->uk, bswd);
    }

    fillPortsInfo(ui->bandSwCombo);
    connect(ui->enableBandSwChkBox, &QCheckBox::stateChanged, this, [=]() {onEnableBandSwChkBox();});
    connect(ui->enableSerialBandSwChkBox, &QCheckBox::stateChanged, this, [=]() {onEnableSerialBandSwChkBox();});


    setHf(hfFlag);

    //connect(ui->turnOffColourRadioFreqDialChkBox, &QCheckBox::clicked, this, [=](bool checked){onTurnOffColourRadioFreqDialChkChanged(checked);});
    //connect(ui->contestStartIgnorePresetFreqChkBox, &QCheckBox::clicked, this, [=](bool checked){onIgnorePreviousFreqChecked(checked);});
    //connect(ui->contestChangeIgnorePreviousFreqChkBox, &QCheckBox::clicked, this, [=](bool checked){onIgnorePresetFreqChecked(checked);});
    //connect(ui->constestChangeRestoreContestModeChkBox, &QCheckBox::clicked, this, [=](bool checked){onRestoreContestModeChecked(checked);});

    freqPresetReadSettings(presetFreq, bands); // static



    ui->turnOffColourRadioFreqDialChkBox->setChecked(readRadioSettingsCheckBox(elpContestTurnOffOperatingFreqColorRadioDial));
    ui->contestStartIgnorePresetFreqChkBox->setChecked(readRadioSettingsCheckBox(elpContestStartIgnorePresetFreq));
    ui->contestChangeIgnorePreviousFreqChkBox->setChecked(readRadioSettingsCheckBox(elpContestChangeIgnorePreviousFreq));
    ui->constestChangeRestoreContestModeChkBox->setChecked(readRadioSettingsCheckBox(elpContestChangeRestoreContestMode));

    ui->enableBandSwChkBox->setChecked(readEnableBandSwitchFromIni());
    enableBandSwLineEdits(ui->enableBandSwChkBox->isChecked());

    ui->enableSerialBandSwChkBox->setChecked(readEnableSerialBandSwitchFromIni());
    ui->bandSwCombo->setVisible(ui->enableSerialBandSwChkBox->isChecked());
    ui->comportLabel->setVisible(ui->enableSerialBandSwChkBox->isChecked());


    loadSettingsToDialog();
}

void RadioSettingDialog::finalise()
{
    saveSettings();

    if (logRadioSettingsChangeFlag->isChanged())
    {
        if (logRadioSettingsChangeFlag->serialComport)
        {
            QString comport = readSerialComportBandSwitchFromIni();
            if (!comport.isEmpty())
            {
                trace(QString("Bandswitch comport changed to %1").arg(comport));
                if (LogContainer->serialTVSw->getOpenFlag())
                {
                    trace(QString("Bandswitch comport open - closing"));
                    LogContainer->serialTVSw->closeComport();
                }

                if (LogContainer->serialTVSw->openComport(comport))
                {
                    trace(QString("Bandswitch comport %1 opened OK").arg(comport));

                }
                else
                {
                    QString errMsg = LogContainer->serialTVSw->error();
                    trace(QString("Bandswitch Comport failed to open = %1 Error = %2").arg(comport).arg(errMsg));
                }
            }
            else
            {
                trace(QString("Bandswitch comport changed, but comport is empty!"));
            }
        }

        emit LogContainer->logRadioSettingsChanged(logRadioSettingsChangeFlag);
    }

}

void RadioSettingDialog::setHf(bool hfFlag)
{
    foreach(auto &lbl, hfLabels)
    {
       lbl->setVisible(hfFlag);
    }
    foreach(auto &ledt, hfLineEdits)
    {
        ledt->setVisible(hfFlag);
    }


    foreach(auto &bswd, bandSwDetails)
    {
        if (bswd.bandType == HF_BANDTYPE)
        {
            bswd.bandSwLineEdit->setVisible(hfFlag);
            bswd.bandSwLabel->setVisible(hfFlag);
        }

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


void RadioSettingDialog::onBandSwLineEditingFinished(int i)
{
    Q_UNUSED(i)
}

void RadioSettingDialog::onEnableBandSwChkBox()
{
    enableBandSwLineEdits(ui->enableBandSwChkBox->isChecked());
}

void RadioSettingDialog::onEnableSerialBandSwChkBox()
{
    ui->bandSwCombo->setVisible(ui->enableSerialBandSwChkBox->isChecked());
    ui->comportLabel->setVisible(ui->enableSerialBandSwChkBox->isChecked());

}
void RadioSettingDialog::saveSettings()
{

    QString fileName = RADIO_PATH_LOGGER + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);

    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_CW, config);
    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_PHONE, config);
    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_MGM, config);

    saveRadioSettingsCheckBoxes();
    saveBandSwComport();
    saveBandSwData();
    saveBandSwCheckBoxes();

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

void RadioSettingDialog::saveBandSwData()
{
    foreach(auto &b, bands)
    {
        QString bandIni;
        bandIni = b.data()->uk;
        bandIni = bandIni.remove('\x20').replace('.', '_');
        QString storedData = readBandSwitchDataFromIni(bandIni);
        if (bandSwDetails.value(b.data()->uk).bandSwLineEdit->text() != storedData)
        {
            writeBandSwitchDataToIni(bandIni, bandSwDetails.value(b.data()->uk).bandSwLineEdit->text().trimmed());
        }
    }
}

void RadioSettingDialog::saveBandSwComport()
{
    if (readSerialComportBandSwitchFromIni() != ui->bandSwCombo->currentText())
    {
        logRadioSettingsChangeFlag->serialComport = true;
        writeSerialComportBandSwitchDataToIni(ui->bandSwCombo->currentText());
    }
}


void RadioSettingDialog::saveBandSwCheckBoxes()
{
    if (readEnableBandSwitchFromIni() != ui->enableBandSwChkBox->isChecked())
    {
        writeEnableBandSwitchDataToIni(ui->enableBandSwChkBox->isChecked());
    }

    if (readSerialComportBandSwitchFromIni() != ui->enableSerialBandSwChkBox->isChecked())
    {
        writeEnableSerialBandSwitchDataToIni(ui->enableSerialBandSwChkBox->isChecked());
    }
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



void RadioSettingDialog::freqPresetReadSettings(PresetFreq  &presetFreq, const QVector<QSharedPointer<BandInfo> > &bands)
{

    QStringList listOfBands;
    for(auto &b:bands)
    {
        listOfBands.append(b->uk);
    }
    presetFreq.readSettings(listOfBands);


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

    foreach(auto &b, bands)
    {
        QString iniBand = b.data()->uk;
        iniBand = iniBand.remove('\x20').replace('.', '_');
        QString bandData = readBandSwitchDataFromIni(iniBand);
        if (bandSwDetails.contains(b.data()->uk))
        {
            bandSwDetails.value(b.data()->uk).bandSwLineEdit->setText(bandData);
        }

    }

    QString comport = readSerialComportBandSwitchFromIni();
    if (!comport.isEmpty())
    {
        ui->bandSwCombo->setCurrentText(comport);
    }


}

void RadioSettingDialog::enableBandSwLineEdits(bool enabled)
{
    foreach (auto &b, bands)
    {
        bandSwDetails.value(b.data()->uk).bandSwLineEdit->setEnabled(enabled);
        bandSwDetails.value(b.data()->uk).bandSwLabel->setEnabled(enabled);
        ui->bandSwMsgsLabel->setEnabled(enabled);
    }

    ui->enableSerialBandSwChkBox->setEnabled(enabled);
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
       // flag change
       if (chkbox == ui->turnOffColourRadioFreqDialChkBox)
       {
           logRadioSettingsChangeFlag->operatingFreqColor = true;
       }
       else if (chkbox == ui->contestStartIgnorePresetFreqChkBox)
       {
           logRadioSettingsChangeFlag->ignorePresetFreq = true;
       }
       else if (chkbox == ui->contestChangeIgnorePreviousFreqChkBox)
       {
           logRadioSettingsChangeFlag->ignorePreviousFreq = true;
       }
       else if (chkbox == ui->constestChangeRestoreContestModeChkBox)
       {
           logRadioSettingsChangeFlag->restoreContestMode = true;
       }
    }

}


void RadioSettingDialog::onRestoreContestModeChecked(bool checked)
{
    TContestApp::getContestApp()->loggerBundle.setBoolProfile(elpContestChangeRestoreContestMode, checked);

}
