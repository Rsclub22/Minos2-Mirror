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

    QSettings settings;
    int curTabNo = settings.value("OptionsLogRadioSettingsDialog/curTab").toInt();
    ui->radioSettingsTabWidget->setCurrentIndex(curTabNo);
    int presetCurTabNo = settings.value("OptionsPresetFreqDialog/curTab").toInt();
    ui->PresetTabWidget->setCurrentIndex(presetCurTabNo);
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

    //===========================================================================================================
    {
        QGridLayout *layout = new QGridLayout();
        ui->cwPresetFrame->setLayout(layout);

        int row = 0;
        int col = 0;

        for (const auto &b: qAsConst(bands))
        {
            QLineEdit *qle = new QLineEdit();
            cwPresetLineEditList << qle;
            connect(qle, &QLineEdit::editingFinished, this, [=]() {onCwPresetLineEditingFinished(b->name(), qle);});


            QLabel *qlel = new QLabel();
            qlel->setText(b->uk);

            layout->addWidget(qlel, row, col);
            layout->addWidget(qle, row, col + 1);
            col += 2;
            if (col%6 == 0)
            {
                row++;
                col = 0;
            }
        }
    }
    //===========================================================================================================
    {
        QGridLayout *layout = new QGridLayout();
        ui->phonePresetFrame->setLayout(layout);

        int row = 0;
        int col = 0;

        for (const auto &b: qAsConst(bands))
        {
            QLineEdit *qle = new QLineEdit();
            phonePresetLineEditList << qle;
            connect(qle, &QLineEdit::editingFinished, this, [=]() {onPhonePresetLineEditingFinished(b->name(), qle);});


            QLabel *qlel = new QLabel();
            qlel->setText(b->uk);

            layout->addWidget(qlel, row, col);
            layout->addWidget(qle, row, col + 1);
            col += 2;
            if (col%6 == 0)
            {
                row++;
                col = 0;
            }
        }
    }

    //===========================================================================================================
    {
        QGridLayout *layout = new QGridLayout();
        ui->mgmPresetFrame->setLayout(layout);

        int row = 0;
        int col = 0;

        for (const auto &b: qAsConst(bands))
        {
            QLineEdit *qle = new QLineEdit();
            mgmPresetLineEditList << qle;
            connect(qle, &QLineEdit::editingFinished, this, [=]() {onMgmPresetLineEditingFinished(b->name(), qle);});


            QLabel *qlel = new QLabel();
            qlel->setText(b->uk);

            layout->addWidget(qlel, row, col);
            layout->addWidget(qle, row, col + 1);
            col += 2;
            if (col%6 == 0)
            {
                row++;
                col = 0;
            }
        }
    }
    //===========================================================================================================
    QList<QLabel*> bandSwLabels;
    {
        QGridLayout *layout = new QGridLayout();
        ui->bandswitchMessageFrame->setLayout(layout);

        int row = 0;
        int col = 0;

        for (const auto &b: qAsConst(bands))
        {
            QLineEdit *qle = new QLineEdit();
            connect(qle, &QLineEdit::editingFinished, this, [=]() {onBandSwLineEditingFinished();});
            bandSwLineEdits << qle;

            QLabel *qlel = new QLabel();
            qlel->setText(b->uk);
            bandSwLabels << qlel;

            layout->addWidget(qlel, row, col);
            layout->addWidget(qle, row, col + 1);
            col += 2;
            if (col%6 == 0)
            {
                row++;
                col = 0;
            }
            BandSwDetails bswd;
            bswd.bandSwLineEdit = qle;
            bswd.bandSwLabel = qlel;
            bswd.bandType = b->getType();
            bandSwDetails.insert(b->uk, bswd);
        }
    }

    //===========================================================================================================
    fillPortsInfo(ui->bandSwCombo);
    connect(ui->enableBandSwChkBox, &QCheckBox::stateChanged, this, [=]() {onEnableBandSwChkBox();});
    connect(ui->enableSerialBandSwChkBox, &QCheckBox::stateChanged, this, [=]() {onEnableSerialBandSwChkBox();});

    freqPresetReadSettings(presetFreq, bands); // static

    ui->turnOffColourRadioFreqDialChkBox->setChecked(readRadioSettingsCheckBox(elpContestTurnOffOperatingFreqColorRadioDial));
    ui->contestStartIgnorePresetFreqChkBox->setChecked(readRadioSettingsCheckBox(elpContestStartIgnorePresetFreq));
    ui->contestChangeIgnorePreviousFreqChkBox->setChecked(readRadioSettingsCheckBox(elpContestChangeIgnorePreviousFreq));
    ui->constestChangeRestoreContestModeChkBox->setChecked(readRadioSettingsCheckBox(elpContestChangeRestoreContestMode));
    ui->CQRitChkBox->setChecked(readRadioSettingsCheckBox(elpCQRit));

    ui->enableBandSwChkBox->setChecked(readEnableBandSwitchFromIni());
    enableBandSwLineEdits(ui->enableBandSwChkBox->isChecked());

    ui->enableSerialBandSwChkBox->setChecked(readEnableSerialBandSwitchFromIni());
    ui->bandSwCombo->setVisible(ui->enableSerialBandSwChkBox->isChecked());
    ui->comportLabel->setVisible(ui->enableSerialBandSwChkBox->isChecked());

    loadSettingsToDialog();
}


bool RadioSettingDialog::check()
{
    if (cancelled)
        return true;

    checking = true;
    errorShown = false;

    presetFreq.clearDirty();

    for (int i = 0; i < cwPresetLineEditList.size(); i++)
    {
        getFreq(cwPresetLineEditList[i], i);

    }
    for (int i = 0; i < phonePresetLineEditList.size(); i++)
    {
        getFreq(phonePresetLineEditList[i], i);

    }
    for (int i = 0; i < mgmPresetLineEditList.size(); i++)
    {
        getFreq(mgmPresetLineEditList[i], i);

    }
    checking = false;

    if (errorShown)
    {
        errorShown = false;
        return false;
    }

    return true;
}
void RadioSettingDialog::cancel()
{
    cancelled = true;
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

void RadioSettingDialog::on_radioSettingsTabWidget_currentChanged(int index)
{
    QSettings settings;
    settings.setValue("OptionsLogRadioSettingsDialog/curTab", index);
}


void RadioSettingDialog::on_PresetTabWidget_currentChanged(int index)
{
    QSettings settings;
    settings.setValue("OptionsPresetFreqDialog/curTab", index);
}

void RadioSettingDialog::onCwPresetLineEditingFinished(QString bandName, QLineEdit *le)
{
    if (!checking)
    {
        QString freq = le->text().trimmed().remove( QRegularExpression("^[0]*"));
        if (valInputFreq(freq, QString("Invalid CW Preset Frequency for %1").arg(bandName)))
        {
           freq = convertFreqToFullDigit(freq).remove('.');

           // check in band
           QString mode = freqPresetData::PRESET_MODE_CW;
           checkInBand(Frequency(freq), bandName, mode);
        }
    }
}

void RadioSettingDialog::onPhonePresetLineEditingFinished(QString bandName, QLineEdit *le)
{
    if (!checking)
    {
        QString freq = le->text().trimmed().remove( QRegularExpression("^[0]*"));
        if (valInputFreq(freq, tr("Invalid Phone Preset Frequency for %1").arg(bandName)))
        {
           freq = convertFreqToFullDigit(freq).remove('.');

           // check in band
           QString mode = freqPresetData::PRESET_MODE_PHONE;
           checkInBand(Frequency(freq), bandName, mode);
        }
    }
}

void RadioSettingDialog::onMgmPresetLineEditingFinished(QString bandName, QLineEdit *le)
{
    if (!checking)
    {
        QString freq = le->text().trimmed().remove( QRegularExpression("^[0]*"));
        if (valInputFreq(freq, tr("Invalid MGM Preset Frequency for %1").arg(bandName)))
        {
           freq = convertFreqToFullDigit(freq).remove('.');

           // check in band
           QString mode = freqPresetData::PRESET_MODE_MGM;
           checkInBand(Frequency(freq), bandName, mode);
        }
    }
}




void RadioSettingDialog::getFreq(QLineEdit* f_box, int band)
{
    if (cancelled)
        return;
    QString freq = f_box->text().trimmed().remove( QRegularExpression("^[0]*"));
    if (valInputFreq(freq, tr(RADIO_FREQ_EDIT_ERR_MSG)))
    {
       freq = convertFreqToFullDigit(freq).remove('.');


       // check in band
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

       if (checkInBand(Frequency(freq), bands[band]->name(), mode))
       {
           presetFreq.setPresetFreq(mode, bands[band].data()->uk, freq);


           freqChanged = true;
          // *freqPresetChanged = true;
       }
    }

}




// check in band

bool RadioSettingDialog::checkInBand(Frequency freq, const QString &band, const QString &mode)
{
    if (BandList::getBandList().isFreqOK(freq, band, mode))
    {
        return true;
    }
    errorShown = true;
    QMessageBox msgBox;
    msgBox.setText(tr("Frequency %1 is out of band/mode for %2/%3").arg(freq.convertFreqStrDispSingle(), band, mode));
    msgBox.exec();
    return false;
}


void RadioSettingDialog::onBandSwLineEditingFinished()
{
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

    presetFreq.clearDirty();

    saveRadioSettingsCheckBoxes();
    saveBandSwComport();
    saveBandSwData();
    saveBandSwCheckBoxes();

}


void RadioSettingDialog::saveModePresetFreqSettings(QString mode, QSettings &config)
{
    config.beginGroup(mode);
    for (const auto &b: qAsConst(bands))
    {
        if (presetFreq.isDirty(mode, b->uk))
        {
            QString newFreq = presetFreq.getPresetFreq(mode, b->uk).str();
            config.setValue(b->uk, newFreq);
        }
    }
    config.endGroup();
}


void RadioSettingDialog::saveRadioSettingsCheckBoxes()
{
    saveRadioSettingsCheckBox(ui->turnOffColourRadioFreqDialChkBox, elpContestTurnOffOperatingFreqColorRadioDial);
    saveRadioSettingsCheckBox(ui->contestStartIgnorePresetFreqChkBox, elpContestStartIgnorePresetFreq);
    saveRadioSettingsCheckBox(ui->contestChangeIgnorePreviousFreqChkBox, elpContestChangeIgnorePreviousFreq);
    saveRadioSettingsCheckBox(ui->constestChangeRestoreContestModeChkBox, elpContestChangeRestoreContestMode);
    saveRadioSettingsCheckBox(ui->CQRitChkBox, elpCQRit);


}

void RadioSettingDialog::saveBandSwData()
{
    foreach(auto &b, bands)
    {
        QString bandIni;
        bandIni = b.data()->uk;
        bandIni.remove('\x20').replace('.', '_');
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


void RadioSettingDialog::freqPresetReadSettings(PresetFreq  &presetFreq, const QVector<QSharedPointer<BandInfo> > &bands)
{
    presetFreq.readSettings(bands);
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
        iniBand.remove('\x20').replace('.', '_');
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
