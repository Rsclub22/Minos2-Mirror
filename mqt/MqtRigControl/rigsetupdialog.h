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

const QString RADIO_DELETED = "radioDeleted";

class RigSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RigSetupDialog(RigFactory* rigFactory, const QVector<QSharedPointer<BandInfo> > &_bands, bool hfFlag_, QWidget *parent = nullptr);
    ~RigSetupDialog();

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


    void addRadio();
    void removeRadio();
    void editRadioName();
private:
    Ui::RigSetupDialog *ui;

    RigFactory *rigFactory;

    void done(int r) override;   // override done function to validate data entry

    QMap<QString, RigSetupForm*> radioTab;


    QStringList availRadios;
    int numAvailRadios;
    QMap<QString, QSharedPointer<scatParams> > availRadioData;

    QSettings availRadioIni;

    QString appName = "";

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
    void civSetToolTip();
    void networkDataEntryVisible(int radioNumber, bool visible);
    void serialDataEntryVisible(int radioNumber, bool visible);
    void addTab(int tabNum, QString tabName);
    void loadSettingsToTab(int tabNum, QString tabName);

    bool checkRadioNameMatch(QString radioName);
    void saveTranVerterSetting(QSharedPointer<scatParams> radioData, QString transvertName, QSettings  &config);

    void loadBands();

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
    void setToolTips(QString tabName);
};

#endif // SETUPDIALOG_H
