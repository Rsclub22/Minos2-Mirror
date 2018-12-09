/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef ROTSETUPDIALOG_H
#define ROTSETUPDIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>
#include <QStringList>
#include <QLabel>
#include <QVector>
#include <QSettings>
#include "rotcontrol.h"
#include "rotsetupform.h"
#include "rotatorcommon.h"




class QIntValidator;
class QComboBox;
class QLineEdit;
class QCheckBox;



namespace Ui {
class RotSetupDialog;
}




class RotSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RotSetupDialog(RotControl* _rotator, QWidget *parent = nullptr);
    ~RotSetupDialog() override;



    QString currentAntennaName;
    srotParams currentAntenna;

    QVector<srotParams*> availAntData;
    QStringList availAntennas;
    int numAvailAntennas;


    QString getCurrentAntenna() const;
    int getRotatorId(QString rotator);


    void readCurrentAntenna();
    void saveCurrentAntenna();
    QString getRotatorComPort(QString);

    void readSettings();
    void copyAntennaToCurrent(int antennaNumber);

    void setAppName(QString name);
    int comportAvial(QString comport);
    int findCurrentAntenna(QString currentAntName);
    void setTabToCurrentAntenna();
    void loadAvailComports();

signals:

    void currentAntennaSettingChanged(QString);
    void antennaNameChange();
    void antennaTabChanged();


private slots:

    void saveButtonPushed();
    void cancelButtonPushed();

    void addAntenna();
    void removeAntenna();
    void editAntennaName();
private:
    Ui::RotSetupDialog *ui;

    RotControl *rotator;

    QVector<rotSetupForm*> antennaTab;



    bool chkloadflg = false;
    QString appName = "";

    void saveSettings();

    void clearAvailRotators();
    void clearCurrentRotator();



    void clearAntennaValueChanged();
    void clearAntennaNameChanged();
    void serialDataEntryVisible(int antNumber, bool visible);
    void networkDataEntryVisible(int antNumber, bool visible);

    void addTab(int tabNum, QString tabName);
    bool checkAntNameMatch(QString antName);
//    void saveAntenna(int i);


    void loadSettingsToTab(int tabNum);
    void getAvailAntenna(int antNum, QSettings &config);
    void initSetup();
    void closeEvent(QCloseEvent *event) override;
    void doCloseEvent();
};

#endif // SETUPDIALOG_H
