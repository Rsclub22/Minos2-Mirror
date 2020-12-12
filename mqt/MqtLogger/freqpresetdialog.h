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




#ifndef FREQPRESETDIALOG_H
#define FREQPRESETDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QLineEdit>
#include "BandList.h"
#include "rigcontrolcommonconstants.h"
#include "rigutils.h"



namespace Ui {
class FreqPresetDialog;
}


QString convertBandKey(QString band);


class FreqPresetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FreqPresetDialog(bool hfFlag, const QVector<QSharedPointer<BandInfo> > &_bands, QWidget *parent = nullptr);
    ~FreqPresetDialog();


    static void readSettings(PresetFreq &presetFreq, const QVector<QSharedPointer<BandInfo> > &band);
    static void checkPreviousVersionIniFile(PresetFreq& presetFreq, const QVector<QSharedPointer<BandInfo> > &bands);


    bool getFreqChanged(){return freqChanged;}
    PresetFreq& getPresetSettings(){return presetFreq;}
    void saveSettings();



private slots:


    void onbandCheckBoxStateChanged(int i);

private:
    Ui::FreqPresetDialog *ui;
    //QStringList presetFreq;
    PresetFreq presetFreq;
    QVector<QSharedPointer<BandInfo> > bands;
    bool freqChanged = false;
    bool* freqPresetChanged;

    QList<QLineEdit*> presetFreqLineEditList;


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



};

#endif // FREQPRESETDIALOG_H
