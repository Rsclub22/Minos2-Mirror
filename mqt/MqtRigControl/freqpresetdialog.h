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




class FreqPresetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FreqPresetDialog(QStringList& _presetFreq, const QVector<BandDetail*> _bands, bool& freqPresetChanged, QWidget *parent = nullptr);
    ~FreqPresetDialog();


    static void readSettings(QStringList &_presetFreq);




private slots:
    //void b_28mhzSelected();
    void b_50mhzSelected();
    void b_70mhzSelected();
    void b_144mhzSelected();
    void b_432mhzSelected();
    void b_1296mhzSelected();
    void b_2300mhzSelected();
    void b_2320mhzSelected();
    void b_3_4ghzSelected();
    void b_5_6ghzSelected();
    void b_10ghzSelected();




    void saveSettings();
    void cancelSettings();



private:
    Ui::FreqPresetDialog *ui;
    QStringList presetFreq;
    QVector<BandDetail*> bands;
    bool freqChanged = false;
    bool freqPresetChanged = false;


    bool checkInBand(double freq, freqPresetData::bandOffSet band);
    void getFreq(QLineEdit* f_box, freqPresetData::bandOffSet band);

    void loadSettingsToDialog();
};

#endif // FREQPRESETDIALOG_H
