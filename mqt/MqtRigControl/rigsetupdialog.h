/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef RIGSETUPDIALOG_H
#define RIGSETUPDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QVector>
#include "rigsetupform.h"
#include "transvertsetupform.h"
#include "rigfactory.h"




class QIntValidator;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QRadioButton;



namespace Ui {
class RigSetupDialog;
}


class SetupChangeFlags
{
public:

    SetupChangeFlags(){};

    bool getRadioNameChanged(){return radioNameChanged;}
    void setRadioNameChanged(bool state){radioNameChanged = state;}

    bool getRadioRemoved(){return radioRemoved;}
    void setRadioRemoved(bool state){radioRemoved = state;}


    bool getCurrRadioChanged(){return currRadioChanged;}
    void setCurrRadioChanged(bool state){currRadioChanged = state;}

    bool getRadioSettingChanged(){return radioSettingChanged;}
    void setRadioSettingChanged(bool state){radioSettingChanged = state;}

    bool getTransVertSettingChanged(){return transVertSettingChanged;}
    void setTransVertSettingChanged(bool state){transVertSettingChanged = state;}

    bool getTransVertNameChanged(){return transVertNameChanged;}
    void setTransVertNameChanged(bool state){transVertNameChanged = state;}

private:


     bool radioNameChanged = false;
     bool radioRemoved = false;
     bool currRadioChanged = false;
     bool radioSettingChanged = false;
     bool transVertSettingChanged = false;
     bool transVertNameChanged = false;




};



class RigSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RigSetupDialog(RigFactory* rigFactory, const QVector<QSharedPointer<BandInfo> > &_bands, bool hfFlag_, QWidget *parent = nullptr);
    ~RigSetupDialog();



    //scatParams currentRadio;



    QVector<QSharedPointer<BandInfo> > bands;

    void initSetup();
    int getRadioId(QString rotator);


    QString getRadioComPort(QString);


    void copyRadioToCurrent(int radioNumber);

    void setAppName(QString name);
    int comportAvial(QString radioName, QString comport);

    void setCurrentRadioName(QString name);

    void setTabToCurrentRadio();

    void fillPortsInfo();
    void loadAvailComports();

    SetupChangeFlags setupChangeFlags;

    QVector<QSharedPointer<RadioNameChange> > listOfRadioNameChanges;
    QVector<QString> listOfRadiosDataChanged;




signals:

    void radioNameChange();
    void currentRadioSettingChanged(QString);
    void radioTabChanged();
    void transVertNameHasChanged();
    void transVertSettingHasChanged();

    void radioSettingsSaved();
    void upDateRadioDetailsCache();

private slots:

    //void saveButtonPushed();
    //void cancelButtonPushed();

    void addRadio();
    void removeRadio();
    void editRadioName();
private:
    Ui::RigSetupDialog *ui;

    RigFactory *rigFactory;

    void done(int r) override;   // override done function to validate data entry

    //QVector<RigSetupForm*> radioTab;
    QMap<QString, RigSetupForm*> radioTab;

    QStringList storedAvailRadios;
    QVector<QSharedPointer<scatParams> > storedRadioData; // data stored in ini files

    QStringList availRadios;
    int numAvailRadios;
    //QVector<QSharedPointer<scatParams> > availRadioData;   // data loaded to tabs
    QMap<QString, QSharedPointer<scatParams> > availRadioData;

    QSettings availRadioIni;
    //QSettings transVerterIni;


    //bool chkloadflg = false;
    QString appName = "";

    //bool radioRemoved;

    bool hfFlag;

    QString currentRadioName;

    void saveSettings();

    void fillRadioModelInfo();

    void fillSpeedInfo();
    void fillDataBitsInfo();
    void fillStopBitsInfo();
    void fillParityInfo();
    void fillHandShakeInfo();

    void clearAvailRadio();
    void clearCurrentRadio();
    //    void enableCIVbox();
    void clearRadioValueChanged();
    void clearRadioNameChanged();
    void civSetToolTip();
    void networkDataEntryVisible(int radioNumber, bool visible);
    void serialDataEntryVisible(int radioNumber, bool visible);
    void loadMgmModes();
    void saveMgmList();
    void addTab(int tabNum, QString tabName);
    void loadSettingsToTab(int tabNum, QString tabName);
    //void saveRadio(int i);
    bool checkRadioNameMatch(QString radioName);
    void saveTranVerterSetting(QSharedPointer<scatParams> radioData, QString transvertName, QSettings  &config);
    //void readTranVerterSetting(int radioNum, int transVertNum, QSettings &config);
    void loadBands();
    //void getRadioSetting(int radNum, QSettings &config);
    void saveRadioData(QSharedPointer<scatParams> radioData, QSettings &config);


    void closeEvent(QCloseEvent *event)override;
    void doCloseEvent();
    void loadAvailComportsToTab(QString radioName);
    void updateAvailRadiosToVersion2(QSettings &settings);
    void loadAvailPttComportsToTab(QString radioName);
    void readTranVerterSetting(QSharedPointer<scatParams> radioData, QString transvertName, QSettings &config);
    void getRadioSetting(QSharedPointer<scatParams> radioData, QString radioName, QSettings &config);
    void getAvailRadiosList(QStringList &availRadios);
    void isAnySupportedBandsAvailForOmnirig(QString &supRadNames);
    bool checkOmniRigSupportedBands();
    bool transVerterInBand(const QSharedPointer<TransVertParams> tvp, QString &transVertBand);
    bool checkTransvertFreqInBand();
};

#endif // SETUPDIALOG_H
