/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2021
//
//
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef RADIOSETTINGDIALOG_H
#define RADIOSETTINGDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QLineEdit>
#include "BandList.h"
#include "rigcontrolcommonconstants.h"
#include "rigutils.h"


namespace Ui {
class RadioSettingDialog;
}




class RadioSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RadioSettingDialog(bool hfFlag, const QVector<QSharedPointer<BandInfo> > &_bands, QWidget *parent = nullptr);
    ~RadioSettingDialog();


    static void freqPresetReadSettings(PresetFreq &presetFreq, const QVector<QSharedPointer<BandInfo> > &band);
    static void checkPreviousVersionIniFile(PresetFreq& presetFreq, const QVector<QSharedPointer<BandInfo> > &bands);


    bool getFreqChanged(){return freqChanged;}
    PresetFreq& getPresetSettings(){return presetFreq;}
    void saveSettings();

private slots:

    void onCwPresetLineEditingFinished(int i);
    void onPhonePresetLineEditingFinished(int i);
    void onMgmPresetLineEditingFinished(int i);



    void onIgnorePresetFreqChecked(bool checked);
    void onIgnorePreviousFreqChecked(bool checked);
    void onTurnOffColourRadioFreqDialChkChanged(bool checked);
    void onRestoreContestModeChecked(bool checked);
    void onAccepted();
private:
    Ui::RadioSettingDialog *ui;
    //QStringList presetFreq;
    PresetFreq presetFreq;
    QVector<QSharedPointer<BandInfo> > bands;
    bool freqChanged = false;
    bool* freqPresetChanged;

    QList<QLineEdit*> cwPresetLineEditList;

    QList<QLineEdit*> phonePresetLineEditList;

    QList<QLineEdit*> mgmPresetLineEditList;

    QList<QLabel*> hfLabels;
    QList<QLineEdit*> hfLineEdits;

    bool hfFlag;


    //bool checkInBand(Frequency freq, freqPresetData::bandOffSet band);
    //void getFreq(QLineEdit* f_box, freqPresetData::bandOffSet band);

    bool checkInBand(Frequency freq, int band);
    void getFreq(QLineEdit* f_box, int band);

    void loadSettingsToDialog();
    void setHf(bool hfFlag);

    void saveModePresetFreqSettings(QString mode, QSettings &config);





    bool readRadioSettingsCheckBox(LOGGERPROFILE profile);
    void saveRadioSettingsCheckBox(QCheckBox *chkbox, LOGGERPROFILE profile);
    void saveRadioSettingsCheckBoxes();
};

#endif // RADIOSETTINGDIALOG_H
