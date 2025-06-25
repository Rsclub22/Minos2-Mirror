/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018 - 2024
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include <QMessageBox>

#include <QListWidgetItem>
#include <QList>
#include "regsettings.h"
#include "rigcommon.h"
#include "rigcontrolcommonconstants.h"

#include "ContestApp.h"
#include "AppStartup.h"
#include "rigutils.h"
#include "tlogcontainer.h"
#include "MTrace.h"
#include "MShowMessageDlg.h"

#include "radiosettingdialog.h"
#include "deletedradioforvoicecwmemorybuttonsdialog.h"
#include "ui_radiosettingdialog.h"



RadioSettingDialog::RadioSettingDialog( QWidget *parent) :
    QFrame(parent),
    ui(new Ui::RadioSettingDialog)
{
    ui->setupUi(this);

    RegSettings settings;
    int curTabNo = settings.getSettings().value("OptionsLogRadioSettingsDialog/curTab").toInt();
    ui->radioSettingsTabWidget->setCurrentIndex(curTabNo);
    int presetCurTabNo = settings.getSettings().value("OptionsPresetFreqDialog/curTab").toInt();
    ui->PresetTabWidget->setCurrentIndex(presetCurTabNo);
}

RadioSettingDialog::~RadioSettingDialog()
{
    delete ui;
}

void RadioSettingDialog::initialise()
{
    BandList::getBandList().loadAllBands(bands);
    logRadioSettingsChangeFlag = QSharedPointer<RadioSettingsDialogChangeFlag>(new RadioSettingsDialogChangeFlag());

    //===========================================================================================================
    {
        QGridLayout *layout = new QGridLayout();
        ui->cwPresetFrame->setLayout(layout);

        int row = 0;
        int col = 0;

        for (const auto &b: QASCONST(bands))
        {
            QLineEdit *qle = new QLineEdit();
            qle->setClearButtonEnabled(true);
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

        for (const auto &b: QASCONST(bands))
        {
            QLineEdit *qle = new QLineEdit();
            qle->setClearButtonEnabled(true);
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
        ui->RTTYPresetFrame->setLayout(layout);

        int row = 0;
        int col = 0;

        for (const auto &b: QASCONST(bands))
        {
            QLineEdit *qle = new QLineEdit();
            qle->setClearButtonEnabled(true);
            RTTYPresetLineEditList << qle;
            connect(qle, &QLineEdit::editingFinished, this, [=]() {onRTTYPresetLineEditingFinished(b->name(), qle);});


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
        ui->PSKPresetFrame->setLayout(layout);

        int row = 0;
        int col = 0;

        for (const auto &b: QASCONST(bands))
        {
            QLineEdit *qle = new QLineEdit();
            qle->setClearButtonEnabled(true);
            PSKPresetLineEditList << qle;
            connect(qle, &QLineEdit::editingFinished, this, [=]() {onPSKPresetLineEditingFinished(b->name(), qle);});


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

        for (const auto &b: QASCONST(bands))
        {
            QLineEdit *qle = new QLineEdit();
            qle->setClearButtonEnabled(true);
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

        for (const auto &b: QASCONST(bands))
        {
            QLineEdit *qle = new QLineEdit();
            qle->setClearButtonEnabled(true);
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

    //===========================================================================================================

    connect(ui->radioReadOnlyCheckBox, &QCheckBox::clicked, this, [=]() {onRadioReadOnlyCheckBoxClicked();});

    turnOffColourRadioFreqDial.initialise(&TContestApp::getContestApp() ->loggerBundle, elpContestTurnOffOperatingFreqColorRadioDial, ui->turnOffColourRadioFreqDialChkBox);

    contestRadioReadOnly.initialise(&TContestApp::getContestApp()->loggerBundle, elpContestRadioReadOnly, ui->radioReadOnlyCheckBox);
    contestStartIgnorePresetFreq.initialise(&TContestApp::getContestApp() ->loggerBundle,elpContestStartIgnorePresetFreq, ui->contestStartIgnorePresetFreqChkBox );
    contestChangeIgnorePreviousFreq.initialise(&TContestApp::getContestApp() ->loggerBundle, elpContestChangeIgnorePreviousFreq, ui->contestChangeIgnorePreviousFreqChkBox );
    constestChangeRestoreContestMode.initialise(&TContestApp::getContestApp() ->loggerBundle, elpContestChangeRestoreContestMode, ui->constestChangeRestoreContestModeChkBox );
    CQRit.initialise(&TContestApp::getContestApp() ->loggerBundle, elpCQRit, ui->CQRitChkBox);
    if (ui->radioReadOnlyCheckBox->isChecked())
    {
        ui->radioWriteSettingsGroupBox->setDisabled(true);
    }
    //===========================================================================================================
    ui->enableBandSwChkBox->setChecked(readEnableBandSwitchFromIni());


    // handle Voice and CW Save Memory Buttons by Name Checkboxes
    ui->saveVoiceMemoryButtonByRadioName->setChecked(readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::RigControl));
    connect(ui->saveVoiceMemoryButtonByRadioName, &QCheckBox::stateChanged, this, [=]() {onSaveVoiceMemoryButtonByRadioNameChkBoxClicked();});

    ui->saveCWMemoryButtonByRadioName->setChecked(readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::CW_RigControl));
    connect(ui->saveCWMemoryButtonByRadioName, &QCheckBox::stateChanged, this, [=]() {onSaveCwMemoryButtonByRadioNameChkBoxClicked();});


    ui->deleteVoiceMemoryButtonRadiosPushButton->setVisible(readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::RigControl));
    connect(ui->deleteVoiceMemoryButtonRadiosPushButton, &QPushButton::clicked, this, [=]() {onDeleteVoiceMemoryButtonRadiosPushButtonClicked();});

    ui->deleteCWMemoryButtonRadiosPushButton->setVisible(readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::CW_RigControl));
    connect(ui->deleteCWMemoryButtonRadiosPushButton, &QPushButton::clicked, this, [=]() {onDeleteCwMemoryButtonRadiosPushButtonClicked();});

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
    for (int i = 0; i < RTTYPresetLineEditList.size(); i++)
    {
        getFreq(RTTYPresetLineEditList[i], i);

    }
    for (int i = 0; i < PSKPresetLineEditList.size(); i++)
    {
        getFreq(PSKPresetLineEditList[i], i);

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
    if (turnOffColourRadioFreqDial.finalise())
    {
        logRadioSettingsChangeFlag->operatingFreqColor = true;
    }
    if (contestRadioReadOnly.finalise())
    {
        logRadioSettingsChangeFlag->radioReadOnly = true;
    }
    if (contestStartIgnorePresetFreq.finalise( ))
    {
        logRadioSettingsChangeFlag->ignorePresetFreq = true;
    }
    if (contestChangeIgnorePreviousFreq.finalise( ))
    {
        logRadioSettingsChangeFlag->ignorePreviousFreq = true;
    }
    if (constestChangeRestoreContestMode.finalise( ))
    {
        logRadioSettingsChangeFlag->restoreContestMode = true;
    }
    if (CQRit.finalise())
    {
        logRadioSettingsChangeFlag->cqRit = true;
    }

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
                    trace(QString("Bandswitch Comport failed to open = %1 Error = %2").arg(comport, errMsg));
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
    RegSettings settings;
    settings.getSettings().setValue("OptionsLogRadioSettingsDialog/curTab", index);
}


void RadioSettingDialog::on_PresetTabWidget_currentChanged(int index)
{
    RegSettings settings;
    settings.getSettings().setValue("OptionsPresetFreqDialog/curTab", index);
}

void RadioSettingDialog::onCwPresetLineEditingFinished(QString bandName, QLineEdit *le)
{
    if (!checking)
    {
        static QRegularExpression qre("^[0]*");
        QString freq = le->text().trimmed().remove( qre);
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
        static QRegularExpression qre("^[0]*");
        QString freq = le->text().trimmed().remove( qre);
        if (valInputFreq(freq, tr("Invalid Phone Preset Frequency for %1").arg(bandName)))
        {
           freq = convertFreqToFullDigit(freq).remove('.');

           // check in band
           QString mode = freqPresetData::PRESET_MODE_PHONE;
           checkInBand(Frequency(freq), bandName, mode);
        }
    }
}

void RadioSettingDialog::onRTTYPresetLineEditingFinished(QString bandName, QLineEdit *le)
{
    if (!checking)
    {
        static QRegularExpression qre("^[0]*");
        QString freq = le->text().trimmed().remove( qre);
        if (valInputFreq(freq, tr("Invalid RTTY Preset Frequency for %1").arg(bandName)))
        {
           freq = convertFreqToFullDigit(freq).remove('.');

           // check in band
           QString mode = freqPresetData::PRESET_MODE_RTTY;
           checkInBand(Frequency(freq), bandName, mode);
        }
    }
}

void RadioSettingDialog::onPSKPresetLineEditingFinished(QString bandName, QLineEdit *le)
{
    if (!checking)
    {
        static QRegularExpression qre("^[0]*");
        QString freq = le->text().trimmed().remove( qre);
        if (valInputFreq(freq, tr("Invalid PSK Preset Frequency for %1").arg(bandName)))
        {
           freq = convertFreqToFullDigit(freq).remove('.');

           // check in band
           QString mode = freqPresetData::PRESET_MODE_PSK;
           checkInBand(Frequency(freq), bandName, mode);
        }
    }
}

void RadioSettingDialog::onMgmPresetLineEditingFinished(QString bandName, QLineEdit *le)
{
    if (!checking)
    {
        static QRegularExpression qre("^[0]*");
        QString freq = le->text().trimmed().remove( qre);
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
    static QRegularExpression qre("^[0]*");
    QString freq = f_box->text().trimmed().remove( qre);
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
       else if (RTTYPresetLineEditList.contains(f_box))
       {
           mode = freqPresetData::PRESET_MODE_RTTY;
       }
       else if (PSKPresetLineEditList.contains(f_box))
       {
           mode = freqPresetData::PRESET_MODE_PSK;
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
    bool excludedFreq = false;

    if (BandList::getBandList().isFreqOK(freq, band, mode, excludedFreq))
    {
        return true;
    }
    errorShown = true;
    QMessageBox msgBox;

    if (excludedFreq)
    {
       msgBox.setText(tr("Frequency %1 should be avoided as part of the bandplan for %2/%3").arg(freq.convertFreqStrDispSingle(), band, mode));
    }
    else
    {
       msgBox.setText(tr("Frequency %1 is out of band/mode for %2/%3").arg(freq.convertFreqStrDispSingle(), band, mode));
    }

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

void RadioSettingDialog::onRadioReadOnlyCheckBoxClicked()
{
    if (ui->radioReadOnlyCheckBox->isChecked())
    {
        // save current settings
        writeIgnorePresetFreqFlagSavedSettingFlag(ui->contestStartIgnorePresetFreqChkBox->isChecked());
        writeContestChangeIgnorePreviousFreqFlagSavedSettingFlag(ui->contestChangeIgnorePreviousFreqChkBox->isChecked());
        writeContestChangeRestoreContestModeSavedSettingFlag(ui->constestChangeRestoreContestModeChkBox->isChecked());
        writeCqRitSavedSettingFlag(ui->CQRitChkBox->isChecked());

        //set readonly settings
        ui->contestChangeIgnorePreviousFreqChkBox->setChecked(true);
        ui->contestStartIgnorePresetFreqChkBox->setChecked(true);
        ui->constestChangeRestoreContestModeChkBox->setChecked(false);
        ui->CQRitChkBox->setChecked(false);

        ui->radioWriteSettingsGroupBox->setDisabled(true);
    }
    else
    {
        // restore settings prior to read only mode
        ui->contestStartIgnorePresetFreqChkBox->setChecked(readIgnorePresetFreqFlagSavedSettingFlag());
        ui->contestChangeIgnorePreviousFreqChkBox->setChecked(readContestChangeIgnorePreviousFreqFlagSavedSettingFlag());
        ui->constestChangeRestoreContestModeChkBox->setChecked(readContestChangeRestoreContestModeSavedSettingFlag());
        ui->CQRitChkBox->setChecked(readCqRitSavedSettingFlag());

        ui->radioWriteSettingsGroupBox->setDisabled(false);
    }
}



void RadioSettingDialog::onSaveVoiceMemoryButtonByRadioNameChkBoxClicked()
{
    setDeleteMemoryButtonRadiosPushButtonVisible(ui->saveVoiceMemoryButtonByRadioName->isChecked(), TxKeyerId::RigControl);
}

void RadioSettingDialog::onSaveCwMemoryButtonByRadioNameChkBoxClicked()
{
    setDeleteMemoryButtonRadiosPushButtonVisible(ui->saveCWMemoryButtonByRadioName->isChecked(), TxKeyerId::CW_RigControl);
}


void RadioSettingDialog::onDeleteVoiceMemoryButtonRadiosPushButtonClicked()
{
    deleteMemoryButtonRadios(TxKeyerId::RigControl);
}

void RadioSettingDialog::onDeleteCwMemoryButtonRadiosPushButtonClicked()
{
    deleteMemoryButtonRadios(TxKeyerId::CW_RigControl);
}


void RadioSettingDialog::deleteMemoryButtonRadios(TxKeyerId id)
{

    QStringList listOfRadios;

    getListOfRadioNamesForMemoryButtons(listOfRadios, id);

    DeletedRadioForVoiceCwMemoryButtonsDialog vmDeleteRadios(listOfRadios,id, nullptr);

    int ret = vmDeleteRadios.exec();

    QList<QListWidgetItem *> selectedItems;
    QStringList listOfRadiosToDelete;



    if (ret == QDialog::Accepted)
    {
        selectedItems = vmDeleteRadios.getSelectedItems(id);


        if (!selectedItems.isEmpty())
        {

            foreach (QListWidgetItem* item, selectedItems)
            {
                QString itemText = item->text();
                listOfRadiosToDelete.append(itemText);
            }


            QString fileName = VOICE_KEYER_PATH() + VOICE_KEYER_BASE_FILE_NAME + keyerTypes[id] + ".ini";
            QSettings config(fileName, QSettings::IniFormat);

            if (!listOfRadiosToDelete.isEmpty())
            {

                for(const auto &radioName: listOfRadiosToDelete)
                {
                    config.beginGroup(radioName);
                    config.remove("");
                    config.endGroup();
                }
            }

        }





    }
}

void RadioSettingDialog::setDeleteMemoryButtonRadiosPushButtonVisible(bool visible, TxKeyerId id)
{
    if (id == TxKeyerId::RigControl)
    {
        ui->deleteVoiceMemoryButtonRadiosPushButton->setVisible(visible);
    }
    else if (id == TxKeyerId::CW_RigControl)
    {
        ui->deleteCWMemoryButtonRadiosPushButton->setVisible(visible);
    }


}

void RadioSettingDialog::saveSettings()
{

    QString fileName = RADIO_PATH_LOGGER() + FILENAME_FREQ_PRESETS;

    QSettings config(fileName, QSettings::IniFormat);

    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_CW, config);
    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_PHONE, config);
    saveModePresetFreqSettings(freqPresetData::PRESET_MODE_MGM, config);

    presetFreq.clearDirty();

    saveBandSwComport();
    saveBandSwData();
    saveBandSwCheckBoxes();
    saveVoiceMemoryButtonByRadioNameCheckBox();
    saveCwMemoryButtonByRadioNameCheckBox();
}


void RadioSettingDialog::saveModePresetFreqSettings(QString mode, QSettings &config)
{
    config.beginGroup(mode);
    for (const auto &b: QASCONST(bands))
    {
        if (presetFreq.isDirty(mode, b->uk))
        {
            QString newFreq = presetFreq.getPresetFreq(mode, b->uk).str();
            config.setValue(b->uk, newFreq);
        }
    }
    config.endGroup();
}

void RadioSettingDialog::saveBandSwData()
{
    for(const auto &b: QASCONST(bands))
    {
        QString bandIni;
        bandIni = b->normalisedName();
        QString storedData = readBandSwitchDataFromIni(bandIni);
        if (bandSwDetails.value(b->uk).bandSwLineEdit->text() != storedData)
        {
            writeBandSwitchDataToIni(bandIni, bandSwDetails.value(b->uk).bandSwLineEdit->text().trimmed());
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

    if (readSerialComportBandSwitchFromIni() != ui->bandSwCombo->currentText())
    {
        writeEnableSerialBandSwitchDataToIni(ui->enableSerialBandSwChkBox->isChecked());
    }
}

void RadioSettingDialog::saveVoiceMemoryButtonByRadioNameCheckBox()
{
    if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::RigControl) != ui->saveVoiceMemoryButtonByRadioName->isChecked())
    {
        writeSaveVoiceCWMemoryButtonByRadioNameToIni(ui->saveVoiceMemoryButtonByRadioName->isChecked(), TxKeyerId::RigControl);

    }
}

void RadioSettingDialog::saveCwMemoryButtonByRadioNameCheckBox()
{
    if (readSaveVoiceCWMemoryButtonByRadioNameFromIni(TxKeyerId::CW_RigControl) != ui->saveCWMemoryButtonByRadioName->isChecked())
    {
        writeSaveVoiceCWMemoryButtonByRadioNameToIni(ui->saveCWMemoryButtonByRadioName->isChecked(), TxKeyerId::CW_RigControl);

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

    for (int i = 0; i < RTTYPresetLineEditList.count(); i++)
    {

        RTTYPresetLineEditList[i]->setText(presetFreq.getPresetFreq(freqPresetData::PRESET_MODE_RTTY, bands[i].data()->uk).convertFreqStrDispSingleNoTrailZero());
    }

    for (int i = 0; i < PSKPresetLineEditList.count(); i++)
    {

        PSKPresetLineEditList[i]->setText(presetFreq.getPresetFreq(freqPresetData::PRESET_MODE_PSK, bands[i].data()->uk).convertFreqStrDispSingleNoTrailZero());
    }

    for (int i = 0; i < mgmPresetLineEditList.count(); i++)
    {

        mgmPresetLineEditList[i]->setText(presetFreq.getPresetFreq(freqPresetData::PRESET_MODE_MGM, bands[i].data()->uk).convertFreqStrDispSingleNoTrailZero());
    }

    for(const auto &b: QASCONST( bands))
    {
        QString iniBand = b->normalisedName();
        QString bandData = readBandSwitchDataFromIni(iniBand);
        if (bandSwDetails.contains(b->uk))
        {
            bandSwDetails.value(b->uk).bandSwLineEdit->setText(bandData);
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
    for (const auto &b: QASCONST(bands))
    {
        bandSwDetails.value(b->uk).bandSwLineEdit->setEnabled(enabled);
        bandSwDetails.value(b->uk).bandSwLabel->setEnabled(enabled);
        ui->bandSwMsgsLabel->setEnabled(enabled);
    }

    ui->enableSerialBandSwChkBox->setEnabled(enabled);
    ui->bandSwCombo->setEnabled(enabled);
    ui->comportLabel->setEnabled(enabled);
}

/*** Save settings before radio read only enabled, to restore if read only deselected ****/

bool RadioSettingDialog::readIgnorePresetFreqFlagSavedSettingFlag()
{

    return loggerIniSettingsGetBool(savedSettings.CONTEST_START_IGNORE_PRESET_FREQ_SAVED_SETTING_KEY);
}

void RadioSettingDialog::writeIgnorePresetFreqFlagSavedSettingFlag(const bool state)
{
    loggerIniSettingsSetBool(savedSettings.CONTEST_START_IGNORE_PRESET_FREQ_SAVED_SETTING_KEY, state);
}



bool RadioSettingDialog::readContestChangeIgnorePreviousFreqFlagSavedSettingFlag()
{

    return loggerIniSettingsGetBool(savedSettings.CONTEST_CHANGE_IGNORE_PREVIOUS_FREQ_SAVED_SETTING_KEY);

}

void RadioSettingDialog::writeContestChangeIgnorePreviousFreqFlagSavedSettingFlag(const bool state)
{
    loggerIniSettingsSetBool(savedSettings.CONTEST_CHANGE_IGNORE_PREVIOUS_FREQ_SAVED_SETTING_KEY, state);
}

bool RadioSettingDialog::readContestChangeRestoreContestModeSavedSettingFlag()
{
    return loggerIniSettingsGetBool(savedSettings.CONTEST_CHANGE_RESTORE_CONTEST_MODE_SAVED_SETTING_KEY);
}

void RadioSettingDialog::writeContestChangeRestoreContestModeSavedSettingFlag(const bool state)
{
    loggerIniSettingsSetBool(savedSettings.CONTEST_CHANGE_RESTORE_CONTEST_MODE_SAVED_SETTING_KEY, state);
}

bool RadioSettingDialog::readCqRitSavedSettingFlag()
{
    return loggerIniSettingsGetBool(savedSettings.CQ_RIT_SAVED_SETTING_KEY);
}

void RadioSettingDialog::writeCqRitSavedSettingFlag(const bool state)
{
    loggerIniSettingsSetBool(savedSettings.CQ_RIT_SAVED_SETTING_KEY, state);
}


void RadioSettingDialog::loggerIniSettingsSetBool(const QString key, const bool value)
{
    QString filename = CONFIGURATION_FILEPATH_LOGGER() + savedSettings.MINOS_LOGGER_CONFIG_FILENAME;
    QSettings config(filename, QSettings::IniFormat);
    config.beginGroup(savedSettings.LOGGER_INI_DEFAULT_GROUP_NAME);
    config.setValue(key, value);
    config.endGroup();

}

bool RadioSettingDialog::loggerIniSettingsGetBool(const QString key)
{
    bool state = false;
    QString filename = CONFIGURATION_FILEPATH_LOGGER() + savedSettings.MINOS_LOGGER_CONFIG_FILENAME;
    QSettings config(filename, QSettings::IniFormat);
    config.beginGroup(savedSettings.LOGGER_INI_DEFAULT_GROUP_NAME);
    state = config.value(key, false).toBool();
    config.endGroup();

    return state;
}

QString RadioSettingDialog::CONFIGURATION_FILEPATH_LOGGER()
{
    return getDirectoryLocation(dlConfiguration) + "/";
}
