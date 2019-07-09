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
#include "rigcontrol.h"
#include "rigsetupform.h"
#include "transvertsetupform.h"





class QIntValidator;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QRadioButton;



namespace Ui {
class RigSetupDialog;
}




class RigSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RigSetupDialog(RigControl* rig, const QVector<BandDetail*> _bands, QWidget *parent = nullptr);
    ~RigSetupDialog();



    scatParams currentRadio;

    QVector<scatParams*> availRadioData;
    QStringList availRadios;
    int numAvailRadios;

    QVector<BandDetail*> bands;

    int getRadioId(QString rotator);

    void readCurrentRadio();
    QString getRadioComPort(QString);
    void saveCurrentRadio();

    void copyRadioToCurrent(int radioNumber);

    void setAppName(QString name);
    int comportAvial(int radioNum, QString comport);
    int findCurrentRadio(QString currentRadioName);
    void setCurrentRadioName(QString name);
    QString getCurrentRadioName();
    void setTabToCurrentRadio();

    void fillPortsInfo();
    void loadAvailComports();




    QString getRigCtldExePath();
    void getRigCtldExePathFromFile();

signals:

    void radioNameChange();
    void currentRadioSettingChanged(QString);
    void radioTabChanged();
    void transVertNameHasChanged();
    void transVertSettingHasChanged();

    void radioSettingsSaved();
    void upDateRadioDetailsCache();

private slots:

    void saveButtonPushed();
    void cancelButtonPushed();

    void addRadio();
    void removeRadio();
    void editRadioName();
private:
    Ui::RigSetupDialog *ui;

    RigControl *radio;

    QVector<RigSetupForm*> radioTab;

    QSettings availRadioIni;
    //QSettings transVerterIni;


    bool chkloadflg = false;
    QString appName = "";

    bool radioRemoved;

    QString currentRadioName;

    QString rigCtldExePath;

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
    void loadSettingsToTab(int tabNum);
    //void saveRadio(int i);
    bool checkRadioNameMatch(QString radioName);
    void saveTranVerterSetting(int radioNum, int transVertNum, QSettings  &config);
    void readTranVerterSetting(int radioNum, int transVertNum, QSettings &config);
    void loadBands();
    void getRadioSetting(int radNum, QSettings &config);
    void saveRadioData(int radNum, QSettings &config);

    void initSetup();
    void closeEvent(QCloseEvent *event);
    void doCloseEvent();
    void loadAvailComportsToTab(int tabNum);
};

#endif // SETUPDIALOG_H
