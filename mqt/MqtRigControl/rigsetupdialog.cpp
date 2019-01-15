/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
/////////////////////////////////////////////////////////////////////////////





#include "rigsetupdialog.h"
#include "rigsetupform.h"
#include "transvertsetupform.h"
#include "ui_rigsetupdialog.h"
#include "rigcontrolcommonconstants.h"
#include "addradiodialog.h"
#include "rigcontrol.h"
#include "rigutils.h"
#include <QSignalMapper>
#include <QComboBox>
#include <QMessageBox>
#include <QLineEdit>
#include <QCheckBox>
//#include <QtSerialPort/QSerialPort>
//#include <QSerialPortInfo>
#include <QSettings>
#include <QInputDialog>

#include <QDebug>



RigSetupDialog::RigSetupDialog(RigControl* _radio, const QVector<BandDetail*> _bands, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RigSetupDialog),
    radioRemoved(false)

{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    radio = _radio;
    bands = _bands;

    QSettings settings;
    QByteArray geometry = settings.value("RigControl/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveButtonPushed()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancelButtonPushed()));
    connect(ui->addRadio, SIGNAL(clicked()), this, SLOT(addRadio()));
    connect(ui->removeRadio, SIGNAL(clicked()), this, SLOT(removeRadio()));
    connect(ui->editRadioName, SIGNAL(clicked()), this, SLOT(editRadioName()));


    initSetup();

}


RigSetupDialog::~RigSetupDialog()
{
    delete ui;
}




void RigSetupDialog::initSetup()
{

    // radio settings ini file
    QString fileName;
    fileName = RADIO_PATH_LOGGER + FILENAME_AVAIL_RADIOS;
    QSettings  settings(fileName, QSettings::IniFormat);

    availRadios = settings.childGroups();
    numAvailRadios = availRadios.count();

    if (numAvailRadios > 0)
    {
        QString version = settings.value("Version/version", QString()).toString();
        if (version != "1")
        {
            mShowMessage(QString("The Radio configuration files in %1 are from an old incompatible version of Minos.\r\n\r\n"
                         "Please delete them and set up the radios again").arg(RADIO_PATH_LOGGER), parentWidget());
            exit(10);
        }

        int v = availRadios.indexOf("Version"); // the section name
        availRadios.removeAt(v);
        numAvailRadios--;

        chkloadflg = true;      // stop loading check values tiggering mapper signals
        for (int i = 0; i < numAvailRadios; i++)
        {
            addTab(i, availRadios[i]);

            // find transverters
            QString fileName;
            fileName = TRANSVERT_PATH_LOGGER + availRadios[i] + FILENAME_TRANSVERT_RADIOS;
            QSettings  configTransvert(fileName, QSettings::IniFormat);

            radioTab[i]->getRadioData()->transVertNames = configTransvert.childGroups();  // get transvert names for this radio
            radioTab[i]->getRadioData()->numTransverters =  radioTab[i]->getRadioData()->transVertNames.count();

            if (radioTab[i]->getRadioData()->numTransverters > 0)
            {
                for (int t = 0; t < availRadioData[i]->numTransverters; t++)
                {
                   radioTab[i]->addTransVertTab(t, radioTab[i]->getRadioData()->transVertNames[t], false);   // adding and existing tab, set change flag = N0CHANGE
                }
            }

            getRadioSetting(i, settings);
            loadSettingsToTab(i);
        }
        chkloadflg = false;
    }
    else
    {
        settings.setValue("Version/version", "1");
    }

}

void RigSetupDialog::addTab(int tabNum, QString tabName)
{
    availRadioData.append(new scatParams);
    availRadioData[tabNum]->radioName = tabName;
    if (!availRadios.contains(tabName))
    {
       availRadios.append(tabName);
    }

    radioTab.append(new RigSetupForm(radio, availRadioData[tabNum], bands, ui->radioTab));
    ui->radioTab->insertTab(tabNum, radioTab[tabNum], tabName);
    ui->radioTab->setTabColor(tabNum, Qt::darkBlue);      // radioTab promoted to QLogTabWidget


}


void RigSetupDialog::loadSettingsToTab(int tabNum)
{

    ui->radioTab->setTabText(tabNum, radioTab[tabNum]->getRadioData()->radioName);

    radioTab[tabNum]->setRadioModel(radioTab[tabNum]->getRadioData()->radioModel);

    radioTab[tabNum]->setCIVAddress(radioTab[tabNum]->getRadioData()->civAddress);
    loadAvailComportsToTab(tabNum);                                                 // finds comports first
    //radioTab[tabNum]->setComport(radioTab[tabNum]->getRadioData()->comport);
    radioTab[tabNum]->setDataSpeed(QString::number(radioTab[tabNum]->getRadioData()->baudrate));
    radioTab[tabNum]->setDataBits(QString::number(radioTab[tabNum]->getRadioData()->databits));
    radioTab[tabNum]->setStopBits(QString::number(radioTab[tabNum]->getRadioData()->stopbits));
    radioTab[tabNum]->setParityBits(radioTab[tabNum]->getRadioData()->parity);
    radioTab[tabNum]->setHandshake(radioTab[tabNum]->getRadioData()->handshake);
    radioTab[tabNum]->setNetAddress(radioTab[tabNum]->getRadioData()->networkAdd);
    radioTab[tabNum]->setNetPortNum(radioTab[tabNum]->getRadioData()->networkPort);
    radioTab[tabNum]->setPollInterval(radioTab[tabNum]->getRadioData()->pollInterval);
    radioTab[tabNum]->setTransVertSelected(radioTab[tabNum]->getRadioData()->transVertEnable);
    radioTab[tabNum]->setEnableTransVertSw(radioTab[tabNum]->getRadioData()->enableTransSwitch);
    radioTab[tabNum]->setEnableLocalTransVertSw(radioTab[tabNum]->getRadioData()->enableLocTVSwMsg);

    if (rig_port_e(radioTab[tabNum]->getRadioData()->portType) == RIG_PORT_NETWORK || rig_port_e(radioTab[tabNum]->getRadioData()->portType) == RIG_PORT_UDP_NETWORK)
    {
        radioTab[tabNum]->serialDataEntryVisible(false);
        radioTab[tabNum]->networkDataEntryVisible(true);
    }
    else if (rig_port_e(radioTab[tabNum]->getRadioData()->portType) == RIG_PORT_SERIAL)
    {
        radioTab[tabNum]->serialDataEntryVisible(true);
        radioTab[tabNum]->networkDataEntryVisible(false);
    }
    else if (rig_port_e(radioTab[tabNum]->getRadioData()->portType) == RIG_PORT_NONE)
    {
        radioTab[tabNum]->serialDataEntryVisible(false);
        radioTab[tabNum]->networkDataEntryVisible(false);
    }
    radioTab[tabNum]->setMgmMode(radioTab[tabNum]->getRadioData()->mgmMode);



    // now load transverter settings
    if (radioTab[tabNum]->getRadioData()->numTransverters > 0 )
    {
        for (int t = 0; t < radioTab[tabNum]->getRadioData()->numTransverters; t++)
        {
            radioTab[tabNum]->setTransVertTabText(t, radioTab[tabNum]->getRadioData()->transVertNames[t]);
            radioTab[tabNum]->loadTransVertTab(t);
        }

    }

   // display the correct transverter settings

    radioTab[tabNum]->setLocTVSwComport(radioTab[tabNum]->getRadioData()->locTVSwComport);

    radioTab[tabNum]->setTransVertSelected(radioTab[tabNum]->getRadioData()->transVertEnable);
    if (radioTab[tabNum]->getRadioData()->transVertEnable)
    {
        radioTab[tabNum]->setTransVertSwVisible(true);
        radioTab[tabNum]->setEnableLocalTransVertSwVisible(false);
    }
    else
    {
        radioTab[tabNum]->setTransVertSwVisible(false);
        radioTab[tabNum]->setEnableLocalTransVertSwVisible(false);
        radioTab[tabNum]->setLocTVSWComportVisible(false);
    }

    if (radioTab[tabNum]->getRadioData()->transVertEnable && radioTab[tabNum]->getRadioData()->enableTransSwitch)
    {
        radioTab[tabNum]->setTransVertSwVisible(true);
        radioTab[tabNum]->setEnableLocalTransVertSwVisible(true);
        for (int i = 0; i < radioTab[tabNum]->getRadioData()->numTransverters; i++)
        {
            radioTab[tabNum]->transVertTab[i]->setEnableTransVertSwBoxVisible(true);
        }
    }

    if (radioTab[tabNum]->getRadioData()->transVertEnable && radioTab[tabNum]->getRadioData()->enableTransSwitch && radioTab[tabNum]->getRadioData()->enableLocTVSwMsg)
    {
        radioTab[tabNum]->setLocTVSWComportVisible(true);
    }
    else
    {
        radioTab[tabNum]->setLocTVSWComportVisible(false);
    }

    //radioTab[tabNum]->buildSupBandList();

}




void RigSetupDialog::addRadio()
{

    AddRadioDialog getRadioName_Rig(availRadios, radio);
    getRadioName_Rig.setWindowTitle("Add Radio and Radio Model");
    if (getRadioName_Rig.exec() != QDialog::Accepted)
    {
        return;
    }

    QString radioName = getRadioName_Rig.getRadioName();
    QString radioModel = getRadioName_Rig.getRadioModel();


    if (radioName.isEmpty() || radioModel.isEmpty())
    {
          return;
    }



    // add the new radio
    int tabNum = numAvailRadios;
    addTab(tabNum, radioName);
    numAvailRadios++;
    radioTab[tabNum]->setupRadioModel(radioModel);
    radioTab[tabNum]->setPollInterval(RIG_DEFAULT_POLLINTERVAL);
    loadAvailComportsToTab(tabNum);

    ui->radioTab->setCurrentIndex(tabNum);

    //emit radioTabChanged();

}


bool RigSetupDialog::checkRadioNameMatch(QString radioName)
{
    for (int i = 0; i < numAvailRadios; i++)
    {
        if (ui->radioTab->tabText(i) == radioName)
            return true;
    }

    return false;
}


void RigSetupDialog::removeRadio()
{

    int currentIndex = ui->radioTab->currentIndex();
    QString currentName = ui->radioTab->tabText(currentIndex);

    if (currentRadioName == ui->radioTab->tabText(currentIndex))
    {
        // can't remove current RadioName
        QMessageBox msgBox;
        msgBox.setText("You can not remove the current radio!");
        msgBox.exec();
        return;

    }

    int status = QMessageBox::question( this,
                            tr("Remove Radio"),
                            tr("Do you really want to remove radio - %1?")
                            .arg(currentName),
                            QMessageBox::Yes|QMessageBox::Default,
                            QMessageBox::No|QMessageBox::Escape,
                            QMessageBox::NoButton);

    if (status != QMessageBox::Yes)
    {
        return;
    }

    // remove this radio
    ui->radioTab->removeTab(currentIndex);
    availRadioData.remove(currentIndex);
    availRadios.removeAt(currentIndex);
    numAvailRadios--;
    radioRemoved = true;

    //emit radioTabChanged();



}




void RigSetupDialog::editRadioName()
{
    int tabNum = ui->radioTab->currentIndex();
    QString radioName = ui->radioTab->tabText(tabNum);
    QString oldName = radioName;
    if (currentRadioName == radioName)
    {
        // can't change current antennaName
        QMessageBox msgBox;
        msgBox.setText(QString("You can not change the name of the current radio - %1!").arg(radioName));
        msgBox.exec();
        return;
    }

    bool ok;
    QString text = QInputDialog::getText(this, QString("Edit Radio Name - %1").arg(radioName),
                                         tr("Edit Radio Name:"), QLineEdit::Normal,
                                         radioName, &ok);
    if (ok && !text.isEmpty())
    {
        ui->radioTab->setTabText(tabNum, text);
        for (int i = 0; i < numAvailRadios; i++)
        {
            if (radioName == availRadioData[i]->radioName)
            {
                availRadioData[i]->radioName = text;  // update with new name
                availRadios[i] = text;
                radioTab[i]->radioNameChanged = true;
                radioTab[i]->radioValueChanged = true;

            }
        }
    }
    else
    {
        return;
    }

}


void RigSetupDialog::setTabToCurrentRadio()
{

    for (int i = 0; i < numAvailRadios; i++)
    {
        if (currentRadioName == availRadioData[i]->radioName)
        {
            ui->radioTab->setTabColor(i, Qt::red);
            ui->radioTab->setCurrentIndex(i);
        }
        else
        {
            ui->radioTab->setTabColor(i,Qt::darkBlue);
        }


    }
}

int RigSetupDialog::comportAvial(int radioNum, QString comport)
{
    if (radioTab.count() > 0)
    {
        return radioTab[radioNum]->comportAvial(comport);
    }

    return -1;
}


void RigSetupDialog::loadAvailComports()
{
    for (int i = 0; i <radioTab.count(); i++)
    {
        loadAvailComportsToTab(i);
    }


}


void RigSetupDialog::loadAvailComportsToTab(int tabNum)
{
    radioTab[tabNum]->loadRadioComports();
    radioTab[tabNum]->setComport(availRadioData[tabNum]->comport);
}

void RigSetupDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("RigControl/geometry", saveGeometry());
}



void RigSetupDialog::closeEvent (QCloseEvent *event)
{
    cancelButtonPushed();
    doCloseEvent();
    QWidget::closeEvent(event);
}

void RigSetupDialog::saveButtonPushed()
{

    saveSettings();
    doCloseEvent();

}



void RigSetupDialog::cancelButtonPushed()
{
    bool change = false;
    for (int i = 0; i < radioTab.count(); i++)
    {
        if (radioTab[i]->radioValueChanged || radioRemoved || radioTab[i]->getTransVertRemovedFlag())
        {
            radioRemoved = false;
            change = true;
            break;
        }
    }



    if (change)
    {
        availRadios.clear();
        numAvailRadios = 0;
        availRadioData.clear();
        radioTab.clear();
        ui->radioTab->clear();
        initSetup();                // load data from file
    }

    doCloseEvent();
}


// remove ??? *********************************
/*
void RigSetupDialog::saveRadio(int i)
{

    QString fileName;
    fileName = RADIO_PATH_LOGGER + FILENAME_AVAIL_RADIOS;
    QSettings config(fileName, QSettings::IniFormat);


    saveRadioData(i, config);

    if (availRadioData[i]->transVertEnable)
    {
        fileName = RADIO_PATH_LOGGER + availRadioData[i]->radioModelName + FILENAME_TRANSVERT_RADIOS;
        QSettings  config(fileName, QSettings::IniFormat);

        for (int trv = 0; trv < availRadioData[i]->numTransverters; trv++)
        {
            saveTranVerterSetting(i, trv, config);
        }

    }

}

*/

void RigSetupDialog::saveSettings()
{


    QString fileNameTransVert;

    QString fileNameRadio = RADIO_PATH_LOGGER + FILENAME_AVAIL_RADIOS;
    QSettings configRadio(fileNameRadio, QSettings::IniFormat);

    // get current list of saved radios, remove those that no longer exist
    QStringList savedRadioNames = configRadio.childGroups();
    int v = savedRadioNames.indexOf("Version");   // the section name
    savedRadioNames.removeAt(v);

    if (savedRadioNames.count() > 0 && radioRemoved)
    {
        radioRemoved = false;
        for (int i = 0; i < savedRadioNames.count(); i++)
        {
            if (!availRadios.contains(savedRadioNames[i]))
            {
                   configRadio.beginGroup(savedRadioNames[i]);        // entry no longer exists
                    configRadio.remove("");      // remove all keys for this group
                    configRadio.endGroup();
                    // remove transverters for this radio
                    fileNameTransVert = TRANSVERT_PATH_LOGGER + savedRadioNames[i] + FILENAME_TRANSVERT_RADIOS;
                    if (QFile::exists(fileNameTransVert))
                    {
                        QFile::remove(fileNameTransVert);
                    }
            }
        }
    }



    bool currRadioChanged = false;
    bool radioSettingChanged = false;
    bool transVertSettingChanged = false;
    bool transVertNameChanged = false;


    for (int i = 0; i < numAvailRadios; i++)
    {

        if (radioTab[i]->radioValueChanged)
        {
            if (radioTab[i]->radioNameChanged)
            {
                emit radioNameChange();
                radioTab[i]->radioNameChanged = false;
            }

            if (currentRadioName == radioTab[i]->getRadioData()->radioName)
            {
                // settings changed in current radio
                currRadioChanged = true;
            }

            saveRadioData(i, configRadio);

        }

        radioTab[i]->radioValueChanged = false;
        radioSettingChanged = true;

    }


    for (int i = 0; i < numAvailRadios; i++)
    {

        // now save transvert settings
        if (radioTab[i]->getRadioData()->transVertEnable)
        {
            radioTab[i]->setTransVertRemovedFlag(false);

            fileNameTransVert = TRANSVERT_PATH_LOGGER + radioTab[i]->getRadioData()->radioName + FILENAME_TRANSVERT_RADIOS;
            QSettings  configTransVert(fileNameTransVert, QSettings::IniFormat);

            // does a transvert file exist for this radio
            if (QFile::exists(fileNameTransVert))
            {
                QStringList savedTransVertNames = configTransVert.childGroups();

                if(savedTransVertNames.count() > 0)
                {
                    for (int t = 0; t < savedTransVertNames.count(); t++)
                    {
                        if (!radioTab[i]->getRadioData()->transVertNames.contains(savedTransVertNames[t]))
                        {
                            configTransVert.beginGroup(savedTransVertNames[t]);        // entry no longer exists
                            configTransVert.remove("");      // remove all keys for this group
                            configTransVert.endGroup();
                            if (currentRadioName == radioTab[i]->getRadioData()->radioName)
                            {
                                // settings changed in current radio
                                currRadioChanged = true;
                            }
                        }
                    }
                }
            }
            else
            {
                // radio renamed... need to recreate the file
                for (int t = 0; t < radioTab[i]->getRadioData()->numTransverters; t++)
                {
                    if (radioTab[i]->transVertTab[t]->transVertValueChanged)
                    {
                        if (radioTab[i]->transVertTab[t]->transVertNameChanged)
                        {
                            radioTab[i]->transVertTab[t]->transVertNameChanged = false;
                            transVertNameChanged = true;

                        }

                        if (currentRadioName == radioTab[i]->getRadioData()->radioName)
                        {
                            // settings changed in current radio
                            currRadioChanged = true;
                        }
                    }

                    saveTranVerterSetting(i, t, configTransVert);
                }

            }


            if (radioTab[i]->getRadioData()->numTransverters > 0)
            {


                /*
                // look for transverters marked for deletion
                int t = radioTab[i]->getRadioData()->numTransverters - 1;
                while (t > 0)
                {
                    if (radioTab[i]->transVertTab[t]->transVertValueChanged  == Tvert_Config_Status::DELETE)
                    {
                        // remove this transverter
                        radioTab[i]->transVertTabRemove(t);
                        radioTab[i]->getRadioData()->transVertNames.removeAt(t);
                        radioTab[i]->getRadioData()->transVertSettings.removeAt(t);
                        radioTab[i]->getRadioData()->numTransverters--;
                        if (currentRadioName == radioTab[i]->getRadioData()->radioName)
                        {
                            // settings changed in current radio
                            currRadioChanged = true;
                        }

                    }
                    t--;
                }
    */
                // look for transverters that have changed

                for (int t = 0; t < radioTab[i]->getRadioData()->numTransverters; t++)
                {


                    if (radioTab[i]->transVertTab[t]->transVertValueChanged)
                    {
                        if (radioTab[i]->transVertTab[t]->transVertNameChanged)
                        {
                            radioTab[i]->transVertTab[t]->transVertNameChanged = false;
                            transVertNameChanged = true;

                        }

                        if (currentRadioName == radioTab[i]->getRadioData()->radioName)
                        {
                            // settings changed in current radio
                            currRadioChanged = true;
                        }

                        saveTranVerterSetting(i, t, configTransVert);


                        radioTab[i]->transVertTab[t]->transVertValueChanged = false;
                        transVertSettingChanged = true;
                    }
                }
                //radioTab[i]->buildSupBandList();
            }
        }



    }

    if (radioSettingChanged || transVertSettingChanged || transVertNameChanged)
    {
        emit upDateRadioDetailsCache();
    }



    if (radioSettingChanged)
    {
       emit radioSettingsSaved();
    }

    if (transVertSettingChanged)
    {
        emit transVertSettingHasChanged();
    }

    if (transVertNameChanged)
    {
        emit transVertNameHasChanged();
    }

    if (currRadioChanged)
    {
        emit currentRadioSettingChanged(currentRadioName);
    }



}


void RigSetupDialog::saveRadioData(int radNum, QSettings& config)
{

    config.beginGroup(radioTab[radNum]->getRadioData()->radioName);
    config.setValue("radioName", radioTab[radNum]->getRadioData()->radioName);
    config.setValue("radioMfgName", radioTab[radNum]->getRadioData()->radioMfg_Name);
    config.setValue("radioModel", radioTab[radNum]->getRadioData()->radioModel);
    config.setValue("radioModelName", radioTab[radNum]->getRadioData()->radioModelName);
    config.setValue("radioModelNumber", radioTab[radNum]->getRadioData()->radioModelNumber);
    config.setValue("civAddress", radioTab[radNum]->getRadioData()->civAddress);
    config.setValue("portType", radioTab[radNum]->getRadioData()->portType);
    config.setValue("comport", radioTab[radNum]->getRadioData()->comport);
    config.setValue("baudrate", radioTab[radNum]->getRadioData()->baudrate);
    config.setValue("databits", radioTab[radNum]->getRadioData()->databits);
    config.setValue("parity", radioTab[radNum]->getRadioData()->parity);
    config.setValue("stopbits", radioTab[radNum]->getRadioData()->stopbits);
    config.setValue("handshake", radioTab[radNum]->getRadioData()->handshake);
    config.setValue("radioPollInterval", radioTab[radNum]->getRadioData()->pollInterval);
    config.setValue("antSwitchAvail", radioTab[radNum]->getRadioData()->antSwitchAvail);
    config.setValue("transVertEnable", radioTab[radNum]->getRadioData()->transVertEnable);
    config.setValue("netAddress", radioTab[radNum]->getRadioData()->networkAdd);
    config.setValue("netPort", radioTab[radNum]->getRadioData()->networkPort);
    config.setValue("mgmMode", radioTab[radNum]->getRadioData()->mgmMode);
    config.setValue("enableTransVertSw", radioTab[radNum]->getRadioData()->enableTransSwitch);
    config.setValue("locTransSwEnable", radioTab[radNum]->getRadioData()->enableLocTVSwMsg);
    config.setValue("locTransVertSwComport", radioTab[radNum]->getRadioData()->locTVSwComport);
    config.endGroup();



}



void RigSetupDialog::getRadioSetting(int radNum, QSettings& config)
{
    config.beginGroup(availRadios[radNum]);
    radioTab[radNum]->getRadioData()->radioName = config.value("radioName", "").toString();
    radioTab[radNum]->getRadioData()->radioNumber = config.value("radioNumber", QString::number(radNum)).toString();
    radioTab[radNum]->getRadioData()->radioMfg_Name = config.value("radioMfgName", "").toString();
    radioTab[radNum]->getRadioData()->radioModel = config.value("radioModel", "").toString();
    radioTab[radNum]->getRadioData()->radioModelName = config.value("radioModelName", "").toString();
    radioTab[radNum]->getRadioData()->radioModelNumber = config.value("radioModelNumber", "").toInt();
    radioTab[radNum]->getRadioData()->civAddress = config.value("civAddress", "").toString();
    radioTab[radNum]->getRadioData()->portType = config.value("portType", int(RIG_PORT_SERIAL)).toInt();
    radioTab[radNum]->getRadioData()->comport = config.value("comport", "").toString();
    radioTab[radNum]->getRadioData()->baudrate = config.value("baudrate", 9600).toInt();
    radioTab[radNum]->getRadioData()->databits = config.value("databits", 8).toInt();
    radioTab[radNum]->getRadioData()->parity = config.value("parity", 0).toInt();
    radioTab[radNum]->getRadioData()->stopbits = config.value("stopbits", 1).toInt();
    radioTab[radNum]->getRadioData()->handshake = config.value("handshake", 0).toInt();
    radioTab[radNum]->getRadioData()->pollInterval = config.value("radioPollInterval", "1").toString();
    radioTab[radNum]->getRadioData()->transVertEnable = config.value("transVertEnable", false).toBool();
    radioTab[radNum]->getRadioData()->antSwitchAvail = config.value("antSwitchAvail", false).toBool();
    radioTab[radNum]->getRadioData()->networkAdd = config.value("netAddress", "").toString();
    radioTab[radNum]->getRadioData()->networkPort = config.value("netPort", "").toString();
    radioTab[radNum]->getRadioData()->mgmMode = config.value("mgmMode", hamlibData::USB).toString();
    radioTab[radNum]->getRadioData()->enableTransSwitch = config.value("enableTransVertSw", false).toBool();
    radioTab[radNum]->getRadioData()->enableLocTVSwMsg = config.value("locTransSwEnable", false).toBool();
    radioTab[radNum]->getRadioData()->locTVSwComport = config.value("locTransVertSwComport", "").toString();
    config.endGroup();

    // now read transverter settings
    QString fileNameTransVert;
    fileNameTransVert = TRANSVERT_PATH_LOGGER + radioTab[radNum]->getRadioData()->radioName + FILENAME_TRANSVERT_RADIOS;
    QSettings  configTransVert(fileNameTransVert, QSettings::IniFormat);

    for (int t = 0; t < radioTab[radNum]->getRadioData()->numTransverters; t++)
    {
        readTranVerterSetting(radNum, t, configTransVert);
    }
}



void RigSetupDialog::saveTranVerterSetting(int radioNum, int transVertNum, QSettings  &config)
{
    config.beginGroup(radioTab[radioNum]->getRadioData()->transVertNames[transVertNum]);
    config.setValue("name", radioTab[radioNum]->getRadioData()->transVertSettings[transVertNum]->transVertName);
    config.setValue("band", radioTab[radioNum]->getRadioData()->transVertSettings[transVertNum]->band);
    config.setValue("radioFreqStr", radioTab[radioNum]->getRadioData()->transVertSettings[transVertNum]->radioFreqStr);
    config.setValue("radioFreq", radioTab[radioNum]->getRadioData()->transVertSettings[transVertNum]->radioFreq);
    config.setValue("targetFreqStr", radioTab[radioNum]->getRadioData()->transVertSettings[transVertNum]->targetFreqStr);
    config.setValue("targetFreq", radioTab[radioNum]->getRadioData()->transVertSettings[transVertNum]->targetFreq);
    config.setValue("offsetString", radioTab[radioNum]->getRadioData()->transVertSettings[transVertNum]->transVertOffsetStr);
    config.setValue("offsetDouble", radioTab[radioNum]->getRadioData()->transVertSettings[transVertNum]->transVertOffset);
    config.setValue("antSwNumber", radioTab[radioNum]->getRadioData()->transVertSettings[transVertNum]->antSwitchNum);
    config.setValue("transVertSw", radioTab[radioNum]->getRadioData()->transVertSettings[transVertNum]->transSwitchNum);
    config.endGroup();
}


void RigSetupDialog::readTranVerterSetting(int radNum, int transVertNum, QSettings  &config)
{
    config.beginGroup(radioTab[radNum]->getRadioData()->transVertNames[transVertNum]);
    radioTab[radNum]->getRadioData()->transVertSettings[transVertNum]->transVertName = config.value("name", "").toString();
    radioTab[radNum]->getRadioData()->transVertSettings[transVertNum]->band = config.value("band", "").toString();
    radioTab[radNum]->getRadioData()->transVertSettings[transVertNum]->radioFreqStr = config.value("radioFreqStr", "00.000.000.000").toString();
    radioTab[radNum]->getRadioData()->transVertSettings[transVertNum]->radioFreq = config.value("radioFreq", 0.0).toDouble();
    radioTab[radNum]->getRadioData()->transVertSettings[transVertNum]->targetFreqStr = config.value("targetFreqStr", "00.000.000.000").toString();
    radioTab[radNum]->getRadioData()->transVertSettings[transVertNum]->targetFreq = config.value("targetFreq", 0.0).toDouble();
    radioTab[radNum]->getRadioData()->transVertSettings[transVertNum]->transVertOffsetStr = config.value("offsetString", "00.000.000.000").toString();
    radioTab[radNum]->getRadioData()->transVertSettings[transVertNum]->transVertOffset = config.value("offsetDouble", 0.0).toDouble();
    radioTab[radNum]->getRadioData()->transVertSettings[transVertNum]->antSwitchNum = config.value("antSwNumber", "0").toString();
    radioTab[radNum]->getRadioData()->transVertSettings[transVertNum]->transSwitchNum = config.value("transVertSw", "0").toString();
    config.endGroup();
}


/*
void RigSetupDialog::clearAvailRadio()
{
    scatParams nulParam;

    for (int i = 0; i < numAvailRadios; i++)
    {
        availRadios[i] = nulParam;
    }


}


void RigSetupDialog::clearCurrentRadio()
{
    scatParams nulParam;
    currentRadio = nulParam;
}


void RigSetupDialog::copyRadioToCurrent(int radioNumber)
{
    currentRadio = availRadios[radioNumber];
}

*/


QString RigSetupDialog::getRadioComPort(QString radioName)
{

    for (int i = 0; i < numAvailRadios; i++)
    {
        if (availRadioData[i]->radioName == radioName)
        {
            return availRadioData[i]->comport;
        }
    }

    return ""; // error, none found

}



void RigSetupDialog::saveCurrentRadio()
{

    QString fileName;
    fileName = RADIO_PATH_LOGGER + appName + FILENAME_CURRENT_RADIO;
    QSettings config(fileName, QSettings::IniFormat);


    config.beginGroup("CurrentRadio");
    config.setValue("radioName", currentRadioName);
    config.endGroup();




}

void RigSetupDialog::setCurrentRadioName(QString name)
{
    currentRadioName = name;
    // set currentRadioName in radio tab to test remove transverter
    for (int i = 0; i < radioTab.count(); i++)
    {
        radioTab[i]->setCurrentRadioName(name);
    }
}

QString RigSetupDialog::getCurrentRadioName()
{
    return currentRadioName;
}


void RigSetupDialog::readCurrentRadio()
{

    QString fileName;
    fileName = RADIO_PATH_LOGGER + appName + FILENAME_CURRENT_RADIO;
    QSettings config(fileName, QSettings::IniFormat);


    {
        config.beginGroup("CurrentRadio");
        currentRadioName = config.value("radioName", "").toString();
        config.endGroup();
    }

}




int RigSetupDialog::findCurrentRadio(QString currentRadioName)
{
    int err = -1;
    for (int i = 0; i < numAvailRadios; i++)
    {
        if (currentRadioName == radioTab[i]->getRadioData()->radioName)
        {
            // current antenna points to selected available antenna
            //currentAntenna = availAntData[i];
            return i;
        }
    }

    return err;
}




void RigSetupDialog::setAppName(QString name)
{
    appName = name;
}






void RigSetupDialog::saveMgmList()
{
    const QStringList  mList = {"USB", "LSB", "RTTY", "PKTLSB", "PKTUSB", "PKTFM" };

    QString fileName;
    fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RADIO_CONFIG_FILE;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("MGM_Modes");

    config.setValue("MgmModes", mList);

    config.endGroup();


}



