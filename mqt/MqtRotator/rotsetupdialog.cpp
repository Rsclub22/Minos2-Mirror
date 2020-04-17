/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016-2020
//
//
// Hamlib Library
//
/////////////////////////////////////////////////////////////////////////////





#include "rotsetupdialog.h"
#include "ui_rotsetupdialog.h"
//#include "rotcontrol.h"
#include "addantennadialog.h"

#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>

#include <QMessageBox>
#include <QInputDialog>

#include <QDebug>




RotSetupDialog::RotSetupDialog(RotatorFactory* rotFactory_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RotSetupDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    rotatorFactory = rotFactory_;

    QSettings settings;
    QByteArray geometry = settings.value("RotControlSetup/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveButtonPushed()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancelButtonPushed()));
    connect(ui->addAntenna, SIGNAL(clicked()), this, SLOT(addAntenna()));
    connect(ui->removeAntenna, SIGNAL(clicked()), this, SLOT(removeAntenna()));
    connect(ui->editAntName, SIGNAL(clicked()), this, SLOT(editAntennaName()));

    initSetup();

}



RotSetupDialog::~RotSetupDialog()
{
    delete ui;
}



void RotSetupDialog::initSetup()
{

    // get the number of available antennas

    QString fileName = ANTENNA_PATH_LOGGER + FILENAME_AVAIL_ANTENNAS;
    QSettings settings(fileName, QSettings::IniFormat);

    availAntennas = settings.childGroups();
    numAvailAntennas = availAntennas.count();
    if (numAvailAntennas > 0)
    {
        QString version = settings.value("Version/version", QString()).toString();
        if (version == "1")
        {
            updateAvailAntennasToVersion2();
        }

        // check version again
        version = settings.value("Version/version", QString()).toString();

        if (version != "2")
        {
            mShowMessage(tr("The Rotator configuration files in %1 are from an old incompatible version of Minos.\r\n\r\n"
                         "Please delete them and set up the rotators again").arg(ANTENNA_PATH_LOGGER), parentWidget());
            exit(10);
        }

        numAvailAntennas--;
        int v = availAntennas.indexOf("Version");   // the section name
        availAntennas.removeAt(v);

        chkloadflg = true;      // stop loading check values tiggering mapper signals
        for (int i = 0; i < numAvailAntennas; i++)
        {
            addTab(i, availAntennas[i]);

            getAvailAntenna(i, settings);   // get settings from ini file

            availAntData[i]->rotatorModelName = rotatorFactory->supported_rotators()->value(availAntData[i]->rotatorModel).rotatorModelName;
            availAntData[i]->rotatorModelNumber = rotatorFactory->supported_rotators()->value(availAntData[i]->rotatorModel).modelNumber;
            availAntData[i]->rotatorManufacturer = rotatorFactory->supported_rotators()->value(availAntData[i]->rotatorModel).rotatorManufacturer;
            availAntData[i]->rotatorCWEndStop = rotatorFactory->supported_rotators()->value(availAntData[i]->rotatorModel).maxRot;
            availAntData[i]->rotatorCCWEndStop = rotatorFactory->supported_rotators()->value(availAntData[i]->rotatorModel).minRot;

            bool overrunState = availAntData[i]->overRunFlag;   // save as it is changed in setEndStopType
            antennaTab[i]->setEndStopType(availAntData[i], availAntData[i]->rotatorCCWEndStop, availAntData[i]->rotatorCWEndStop);
            availAntData[i]->overRunFlag = overrunState;  //restore
            antennaTab[i]->setOverlapEndStop(availAntData[i], overrunState);

            loadSettingsToTab(i);
        }
        chkloadflg = false;
    }
    else
    {
        settings.setValue("Version/version", "2");
    }

}







void RotSetupDialog::addTab(int tabNum, QString tabName)
{
    availAntData.append(new srotParams);
    availAntData[tabNum]->antennaName = tabName;
    if (!availAntennas.contains(tabName))
    {
        availAntennas.append(tabName);
    }


    antennaTab.append(new rotSetupForm(rotatorFactory, availAntData[tabNum]));
    ui->antennaTab->insertTab(tabNum, antennaTab[tabNum], tabName);
    ui->antennaTab->setTabColor(tabNum, Qt::darkBlue);


}



void RotSetupDialog::loadSettingsToTab(int tabNum)
{


    ui->antennaTab->setTabText(tabNum, availAntData[tabNum]->antennaName);

    if (availAntData[tabNum]->rotatorModel != "")
    {
        antennaTab[tabNum]->setRotatorModel(availAntData[tabNum]->rotatorModel);
        antennaTab[tabNum]->setPollInterval(availAntData[tabNum]->pollInterval);
        antennaTab[tabNum]->pollIntervalVisible(true);
        //antennaTab[tabNum]->setCheckStop(availAntData[tabNum]->southStopType);

        antennaTab[tabNum]->setCheckOverrun(availAntData[tabNum]->overRunFlag);
        antennaTab[tabNum]->setSimCW_CCWcmdChecked(availAntData[tabNum]->simCwCcwCmd);

        // set southstop visible if rotator is 0 - 360

        if (availAntData[tabNum]->rotType == ROT_0_360)
        {
           antennaTab[tabNum]->sStopButtonsVisible(true);
           antennaTab[tabNum]->setSStopButtons(availAntData[tabNum]->southStopType);
           antennaTab[tabNum]->setOverRunFlagVisible(false);
        }
        else if (availAntData[tabNum]->rotType == ROT_0_450)
        {
           antennaTab[tabNum]->setOverRunFlagVisible(true);
           if ((availAntData[tabNum]->endStopType == ROT_0_360 || availAntData[tabNum]->endStopType == ROT_180_180) && !availAntData[tabNum]->overRunFlag  )
           {

               antennaTab[tabNum]->sStopButtonsVisible(true);
               antennaTab[tabNum]->setSStopButtons(availAntData[tabNum]->southStopType);
           }
           else
           {
              antennaTab[tabNum]->sStopButtonsVisible(false);
           }
        }
        else
        {
           antennaTab[tabNum]->sStopButtonsVisible(false);
           antennaTab[tabNum]->setOverRunFlagVisible(false);
        }

        antennaTab[tabNum]->setCheckOverrun(availAntData[tabNum]->overRunFlag);
        antennaTab[tabNum]->setAntennaOffset(QString::number(availAntData[tabNum]->antennaOffset));
        antennaTab[tabNum]->antennaOffSetVisible(true);
        //antennaTab[tabNum]->setComport(availAntData[tabNum]->comport);
        loadAvailComportsToTab(tabNum);
        antennaTab[tabNum]->setDataSpeed(QString::number(availAntData[tabNum]->baudrate));
        antennaTab[tabNum]->setDataBits(QString::number(availAntData[tabNum]->databits));
        antennaTab[tabNum]->setStopBits(QString::number(availAntData[tabNum]->stopbits));
        antennaTab[tabNum]->setParityBits(availAntData[tabNum]->parity);
        antennaTab[tabNum]->setHandshake(availAntData[tabNum]->handshake);
        antennaTab[tabNum]->setNetAddress(availAntData[tabNum]->networkAdd);
        antennaTab[tabNum]->setNetPortNum(availAntData[tabNum]->networkPort);

        if (availAntData[tabNum]->portType == RotCapConstants::PortType::network)
        {
            antennaTab[tabNum]->serialDataEntryVisible(false);
            antennaTab[tabNum]->networkDataEntryVisible(true);
        }
        else if (availAntData[tabNum]->portType == RotCapConstants::PortType::none)
        {
            antennaTab[tabNum]->serialDataEntryVisible(false);
            antennaTab[tabNum]->networkDataEntryVisible(false);
        }
        else if (availAntData[tabNum]->portType == RotCapConstants::PortType::serial)
        {
            antennaTab[tabNum]->serialDataEntryVisible(true);
            antennaTab[tabNum]->networkDataEntryVisible(false);
        }


        if (availAntData[tabNum]->supportCwCcwCmd)
        {
            antennaTab[tabNum]->setSimCW_CCWcmdVisible(false);
            antennaTab[tabNum]->setSimCW_CCWcmdChecked(availAntData[tabNum]->simCwCcwCmd);
        }
        else
        {
            antennaTab[tabNum]->setSimCW_CCWcmdVisible(true);
            antennaTab[tabNum]->setSimCW_CCWcmdChecked(availAntData[tabNum]->simCwCcwCmd);

        }




    }
    else
    {
        // no rotator model selected, hide elements on tab
        antennaTab[tabNum]->setRotatorModel(availAntData[tabNum]->rotatorModel);
        antennaTab[tabNum]->networkDataEntryVisible(false);
        antennaTab[tabNum]->setOverRunFlagVisible(false);
        antennaTab[tabNum]->sStopButtonsVisible(false);
        antennaTab[tabNum]->setSStopButtons(S_STOPOFF);
        antennaTab[tabNum]->pollIntervalVisible(false);
        antennaTab[tabNum]->antennaOffSetVisible(false);
        antennaTab[tabNum]->serialDataEntryVisible(false);
        antennaTab[tabNum]->setSimCW_CCWcmdVisible(false);

    }


}


void RotSetupDialog::setTabToCurrentAntenna()
{

    for (int i = 0; i < numAvailAntennas; i++)
    {
        if (currentAntennaName == availAntData[i]->antennaName)
        {
            ui->antennaTab->setTabColor(i, Qt::red);
            ui->antennaTab->setCurrentIndex(i);
        }
        else
        {
            ui->antennaTab->setTabColor(i,Qt::darkBlue);
        }


    }
}





void RotSetupDialog::saveButtonPushed()
{

    saveSettings();

}


void RotSetupDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("RotControlSetup/geometry", saveGeometry());
}


void RotSetupDialog::closeEvent (QCloseEvent *event)
{
    cancelButtonPushed();
    doCloseEvent();
    QWidget::closeEvent(event);
}



void RotSetupDialog::cancelButtonPushed()
{

    bool change = false;
    for (int i = 0; i < antennaTab.count(); i++)
    {
        if (antennaTab[i]->getAntennaValueChanged())
        {
            change = true;
            break;
        }
    }

    if (change)
    {

        availAntennas.clear();
        numAvailAntennas = 0;
        availAntData.clear();
        antennaTab.clear();
        ui->antennaTab->clear();
        initSetup();
    }

    doCloseEvent();
}



void RotSetupDialog::saveSettings()
{


    bool antennaNameChg = false;
    bool currentAntennaChanged = false;

    QString fileName;
    fileName = ANTENNA_PATH_LOGGER + FILENAME_AVAIL_ANTENNAS;
    QSettings config(fileName, QSettings::IniFormat);

    // get current list of saved antennas
    QStringList savedAntNames = config.childGroups();
    int v = savedAntNames.indexOf("Version");   // the section name
    savedAntNames.removeAt(v);

    if (savedAntNames.count() > 0)
    {
        for (int i = 0; i < savedAntNames.count(); i++)
        {
            if (!availAntennas.contains(savedAntNames[i]))
            {
                config.beginGroup(savedAntNames[i]);        // entry no longer exists
                config.remove("");      // remove all keys for this group
                config.endGroup();
            }
        }
    }




    for (int i = 0; i < numAvailAntennas; i++)
    {

        if (antennaTab[i]->getAntennaValueChanged())
        {
            config.beginGroup(availAntData[i]->antennaName);
            if (currentAntennaName == availAntData[i]->antennaName)
            {
                // settings changed in current antenna
                currentAntennaChanged = true;
            }


            config.setValue("antennaName", availAntData[i]->antennaName);
            if (antennaTab[i]->getAntennaNameChanged())
            {
                antennaNameChg = true;
                antennaTab[i]->setAntennaNameChanged(false);
            }
            config.setValue("antennaNumber", i+1);
            config.setValue("rotatorModel", availAntData[i]->rotatorModel);
            //config.setValue("rotatorModelName", availAntData[i]->rotatorModelName);
            //config.setValue("rotatorModelNumber", availAntData[i]->rotatorModelNumber);
            //config.setValue("rotatorManufacturer", availAntData[i]->rotatorManufacturer);
            //config.setValue("rotatorCWEndStop", double(availAntData[i]->rotatorCWEndStop));
            //config.setValue("rotatorCCWEndStop", double(availAntData[i]->rotatorCCWEndStop));
            //config.setValue("rotatorType", availAntData[i]->rotType);
            //config.setValue("endStopType", availAntData[i]->endStopType);
            config.setValue("supportCwCcwCmd", availAntData[i]->supportCwCcwCmd);
            config.setValue("rotatorPollInterval", availAntData[i]->pollInterval);
            //config.setValue("maxAzimuth", double(availAntData[i]->max_azimuth));
            //config.setValue("minAzimuth", double(availAntData[i]->min_azimuth));
            config.setValue("simulateCwCCw", availAntData[i]->simCwCcwCmd);
            //config.setValue("southStopType", availAntData[i]->southStopType);
            config.setValue("overRun", availAntData[i]->overRunFlag);
            config.setValue("antennaOffset", availAntData[i]->antennaOffset);
            config.setValue("portType", availAntData[i]->portType);
            config.setValue("comport", availAntData[i]->comport);
            config.setValue("baudrate", availAntData[i]->baudrate);
            config.setValue("databits", availAntData[i]->databits);
            config.setValue("parity", availAntData[i]->parity);
            config.setValue("stopbits", availAntData[i]->stopbits);
            config.setValue("handshake", availAntData[i]->handshake);
            config.setValue("netAddress", availAntData[i]->networkAdd);
            config.setValue("netPort", availAntData[i]->networkPort);
            config.endGroup();
            antennaTab[i]->setAntennaValueChanged(false);

        }

    }




   if (antennaNameChg)
   {
       emit antennaNameChange();
   }

   if (currentAntennaChanged)
   {

      emit currentAntennaSettingChanged(currentAntennaName);

   }

    doCloseEvent();
}




void RotSetupDialog::getAvailAntenna(int antNum, QSettings& config)
{

    config.beginGroup(availAntennas[antNum]);
    availAntData[antNum]->antennaName = config.value("antennaName", "").toString();
    availAntData[antNum]->antennaNumber = config.value("antennaNumber", QString::number(antNum+1)).toString();
    availAntData[antNum]->rotatorModel = config.value("rotatorModel", "").toString();
    availAntData[antNum]->pollInterval = config.value("rotatorPollInterval", ROT_DEFAULT_POLLINTERVAL).toString();
    //availAntData[antNum]->rotType = endStop(config.value("rotatorType", int(ROT_0_360)).toInt());
    //availAntData[antNum]->endStopType = endStop(config.value("endStopType", int(ROT_0_360)).toInt());
    //availAntData[antNum]->max_azimuth = config.value("maxAzimuth", 360).toInt();
    //availAntData[antNum]->min_azimuth = config.value("minAzimuth", 0).toInt();
    availAntData[antNum]->supportCwCcwCmd = config.value("supportCwCcwCmd", false).toBool();
    availAntData[antNum]->simCwCcwCmd = config.value("simulateCwCCw", true).toBool();
    //availAntData[antNum]->southStopType = southStop(config.value("southStopType", S_STOPOFF).toInt());
    availAntData[antNum]->overRunFlag = config.value("overRun", false).toBool();
    availAntData[antNum]->antennaOffset = config.value("antennaOffset", "").toInt();
    availAntData[antNum]->portType = (config.value("portType", RotCapConstants::PortType::none).toInt());
    availAntData[antNum]->comport = config.value("comport", "").toString();
    availAntData[antNum]->baudrate = config.value("baudrate", 9600).toInt();
    availAntData[antNum]->databits = config.value("databits", 8).toInt();
    availAntData[antNum]->parity = config.value("parity", 0).toInt();
    availAntData[antNum]->stopbits = config.value("stopbits", 1).toInt();
    availAntData[antNum]->handshake = config.value("handshake", 0).toInt();
    availAntData[antNum]->networkAdd = config.value("netAddress", "").toString();
    availAntData[antNum]->networkPort = config.value("netPort", "").toString();
    config.endGroup();

}


int RotSetupDialog::comportAvial(QString comport)
{
    if (antennaTab.count() > 0)
    {
        return antennaTab[0]->comportAvial(comport);
    }
    else
    {
        return -1;
    }
}


void RotSetupDialog::loadAvailComports()
{
    for (int i = 0; i <antennaTab.count(); i++)
    {
        loadAvailComportsToTab(i);
    }
}


void RotSetupDialog::loadAvailComportsToTab(int tabNum)
{
    antennaTab[tabNum]->fillPortsInfo();
    antennaTab[tabNum]->setComport(availAntData[tabNum]->comport);
}

//void RotSetupDialog::clearAvailRotators()
//{
//    srotParams nulParam;

//    for (int i = 0; i < numAvailAntennas; i++)
//    {
//        availAntData[i] = nulParam;
//    }




//void RotSetupDialog::clearCurrentRotator()
//{
//    srotParams nulParam;
//    currentAntenna = nulParam;
//}


//void RotSetupDialog::copyAntennaToCurrent(int antennaNumber)
//{
//    currentAntenna = availAntennas[antennaNumber];
//}

//void RotSetupDialog::clearAntennaValueChanged()
//{
//    for (int i = 0; i < NUM_ANTENNAS; i++)
//    {
//        antennaValueChanged[i] = false;
//    }
//}

//void RotSetupDialog::clearAntennaNameChanged()
//{
//    for (int i = 0; i < NUM_ANTENNAS; i++)
//    {
//        antennaNameChanged[i] = false;
//    }
//}


QString RotSetupDialog::getRotatorComPort(QString antennaName)
{

    for (int i = 0; i < numAvailAntennas ; i++)
    {
        if (availAntData[i]->antennaName == antennaName)
        {
            return availAntData[i]->comport;
        }
    }

    return ""; // error, none found

}



void RotSetupDialog::saveCurrentAntenna()
{

    QString fileName;
    fileName = ANTENNA_PATH_LOGGER + appName + FILENAME_CURRENT_ANTENNA;
    QSettings config(fileName, QSettings::IniFormat);


    config.beginGroup("CurrentAntenna");
    config.setValue("antennaName", currentAntennaName);
    config.endGroup();




}

void RotSetupDialog::readCurrentAntenna()
{

    QString fileName;
    fileName = ANTENNA_PATH_LOGGER + appName + FILENAME_CURRENT_ANTENNA;
    QSettings config(fileName, QSettings::IniFormat);

    {
        config.beginGroup("CurrentAntenna");
        currentAntennaName = config.value("antennaName", "").toString();

        config.endGroup();
    }

}



int RotSetupDialog::findCurrentAntenna(QString currentAntName)
{
    int err = -1;
    for (int i = 0; i < numAvailAntennas; i++)
    {
        if (currentAntName == availAntData[i]->antennaName)
        {
            // current antenna points to selected available antenna
            //currentAntenna = availAntData[i];
            return i;
        }
    }

    return err;
}


void RotSetupDialog::setAppName(QString name)
{
    appName = name;
}


void RotSetupDialog::addAntenna()
{

    AddAntennaDialog getAntennaName_Rot(availAntennas, rotatorFactory);
    getAntennaName_Rot.setWindowTitle("Add Antenna and Rotator Model");
    if (getAntennaName_Rot.exec() != QDialog::Accepted)
    {
        return;
    }

    QString antName = getAntennaName_Rot.getAntennaName();
    QString rotModel = getAntennaName_Rot.getRotatorModel();


    if (antName.isEmpty() || rotModel.isEmpty())
    {
          return;
    }

  // add the new antenna
  int tabNum = numAvailAntennas;
  addTab(tabNum, antName);
  numAvailAntennas++;
  antennaTab[tabNum]->setupRotatorModel(rotModel);
  antennaTab[tabNum]->setPollInterval(ROT_DEFAULT_POLLINTERVAL);
  loadAvailComportsToTab(tabNum);

  ui->antennaTab->setCurrentIndex(tabNum);
  emit antennaTabChanged();


}

bool RotSetupDialog::checkAntNameMatch(QString antName)
{
    for (int i = 0; i < numAvailAntennas; i++)
    {
        if (ui->antennaTab->tabText(i) == antName)
            return true;
    }

    return false;
}


void RotSetupDialog::removeAntenna()
{

    int currentIndex = ui->antennaTab->currentIndex();
    QString currentName = ui->antennaTab->tabText(currentIndex);

    if (currentAntennaName == ui->antennaTab->tabText(currentIndex))
    {
        // can't remove current antennaName
        QMessageBox msgBox;
        msgBox.setText(tr("You can not remove the current antenna!"));
        msgBox.exec();
        return;

    }

    int status = QMessageBox::question( this,
                            tr("Remove Antenna"),
                            tr("Do you really want to remove antenna - %1?")
                            .arg(currentName),
                            QMessageBox::Yes|QMessageBox::Default,
                            QMessageBox::No|QMessageBox::Escape,
                            QMessageBox::NoButton);

    if (status != QMessageBox::Yes)
    {
        return;
    }

    // remove this antenna
    ui->antennaTab->removeTab(currentIndex);
    availAntData.remove(currentIndex);
    availAntennas.removeAt(currentIndex);
    antennaTab.removeAt(currentIndex);
    numAvailAntennas--;


    //emit antennaTabChanged();



}


void RotSetupDialog::editAntennaName()
{
    int tabNum = ui->antennaTab->currentIndex();
    QString antName = ui->antennaTab->tabText(tabNum);
    QString oldName = antName;
    if (currentAntennaName == antName)
    {
        // can't change current antennaName
        QMessageBox msgBox;
        msgBox.setText(tr("You can not change the name of the current antenna - %1!").arg(antName));
        msgBox.exec();
        return;
    }

    bool ok;
    QString text = QInputDialog::getText(this, tr("Edit Antenna Name - %1").arg(antName),
                                         tr("New Antenna Name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty())
    {
        ui->antennaTab->setTabText(tabNum, text);
        for (int i = 0; i < numAvailAntennas; i++)
        {
            if (antName == availAntData[i]->antennaName)
            {
                availAntData[i]->antennaName = text;  // update with new name
                availAntennas[i] = text;
                antennaTab[i]->setAntennaNameChanged(true);
                antennaTab[i]->setAntennaValueChanged(true);
            }
        }
    }
    else
    {
        return;
    }

    //emit antennaTabChanged();
}




QString RotSetupDialog::getCurrentAntenna() const
{


    return currentAntennaName;

}

void RotSetupDialog::updateAvailAntennasToVersion2()
{

    QString fileName = ANTENNA_PATH_LOGGER + FILENAME_AVAIL_ANTENNAS;
    QSettings settings(fileName, QSettings::IniFormat);

    QString antennaModel;
    QStringList spList;

    for (int i = 0; i < numAvailAntennas; i++)
    {

        settings.beginGroup(availAntennas[i]);

        antennaModel = settings.value("rotatorModel", QString()).toString();
        if (antennaModel.contains(','))
        {
            spList = antennaModel.split(',');
            if (spList.count() == 3)
            {
                antennaModel = spList[1].trimmed() + " " + spList[2].trimmed();
                settings.setValue("rotatorModel", antennaModel);
            }
        }

        // remove redundant settings

        settings.remove("rotatorModelName");
        settings.remove("rotatorModelName");
        settings.remove("rotatorModelNumber");
        settings.remove("rotatorManufacturer");
        settings.remove("rotatorCWEndStop");
        settings.remove("rotatorCCWEndStop");
        settings.remove("rotatorType");
        settings.remove("endStopType");
        settings.remove("maxAzimuth");
        settings.remove("minAzimuth");
        settings.remove("southStopType");



        settings.endGroup();


    }

    settings.setValue("Version/version", "2");
}


