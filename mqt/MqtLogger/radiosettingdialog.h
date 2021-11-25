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

#include <QFrame>
#include <QSettings>
#include <QLineEdit>
#include "BandList.h"
#include "rigcontrolcommonconstants.h"
#include "rigutils.h"


namespace Ui {
class RadioSettingDialog;
}

class BandSwDetails
{

public:
    QLineEdit *bandSwLineEdit;
    QLabel *bandSwLabel;
    QString bandType;

};


class RadioSettingDialog : public QFrame
{
    Q_OBJECT

public:

    explicit RadioSettingDialog(QWidget *parent = nullptr);
    ~RadioSettingDialog();

    void initialise();
    void finalise();

    void freqPresetReadSettings(PresetFreq &presetFreq, const QVector<QSharedPointer<BandInfo> > &band);

    bool getFreqChanged(){return freqChanged;}
    PresetFreq& getPresetSettings(){return presetFreq;}
    void saveSettings();
    bool check();
    void cancel();

private slots:

    void on_radioSettingsTabWidget_currentChanged(int index);

    void onCwPresetLineEditingFinished(QString bandName, QLineEdit *le);
    void onPhonePresetLineEditingFinished(QString bandName, QLineEdit *le);
    void onMgmPresetLineEditingFinished(QString bandName, QLineEdit *le);



    void onIgnorePresetFreqChecked(bool checked);
    void onIgnorePreviousFreqChecked(bool checked);
    void onTurnOffColourRadioFreqDialChkChanged(bool checked);
    void onRestoreContestModeChecked(bool checked);
    void onBandSwLineEditingFinished();
    void onEnableBandSwChkBox();
    void onEnableSerialBandSwChkBox();

    void on_PresetTabWidget_currentChanged(int index);
private:
    Ui::RadioSettingDialog *ui;
    //QStringList presetFreq;
    PresetFreq presetFreq;
    QVector<QSharedPointer<BandInfo> > bands;
    bool freqChanged = false;
    bool* freqPresetChanged;

    bool checking = false;
    bool errorShown = false;
    bool cancelled = false;

    QList<QLineEdit*> cwPresetLineEditList;

    QList<QLineEdit*> phonePresetLineEditList;

    QList<QLineEdit*> mgmPresetLineEditList;

    QList<QLabel*> hfLabels;
    QList<QLineEdit*> hfLineEdits;

    QList<QLineEdit*> bandSwLineEdits;


    QMap<QString, BandSwDetails> bandSwDetails;



    bool hfFlag;

    QSharedPointer<RadioSettingsDialogChangeFlag> logRadioSettingsChangeFlag;


    //bool checkInBand(Frequency freq, freqPresetData::bandOffSet band);
    //void getFreq(QLineEdit* f_box, freqPresetData::bandOffSet band);

    bool checkInBand(Frequency freq, const QString &band, const QString &mode);
    void getFreq(QLineEdit* f_box, int band);

    void loadSettingsToDialog();

    void saveModePresetFreqSettings(QString mode, QSettings &config);





    bool readRadioSettingsCheckBox(LOGGERPROFILE profile);
    void saveRadioSettingsCheckBox(QCheckBox *chkbox, LOGGERPROFILE profile);
    void saveRadioSettingsCheckBoxes();
    void saveBandSwData();
    void enableBandSwLineEdits(bool enabled);
    void saveBandSwComport();
    void saveBandSwCheckBoxes();
};

#endif // RADIOSETTINGDIALOG_H
