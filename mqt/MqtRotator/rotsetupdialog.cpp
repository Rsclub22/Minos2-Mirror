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

#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QMessageBox>
#include <QInputDialog>

#include "regsettings.h"
#include "MShowMessageDlg.h"
#include "addantennadialog.h"
#include "rotsetupdialog.h"
#include "ui_rotsetupdialog.h"

RotSetupDialog::RotSetupDialog(RotatorFactory* rotFactory_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RotSetupDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    rotatorFactory = rotFactory_;

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("RotControlSetup/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &RotSetupDialog::saveButtonPushed);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &RotSetupDialog::cancelButtonPushed);
    connect(ui->addAntenna, &QPushButton::clicked, this, &RotSetupDialog::addAntenna);
    connect(ui->removeAntenna, &QPushButton::clicked, this, &RotSetupDialog::removeAntenna);
    connect(ui->editAntName, &QPushButton::clicked, this, &RotSetupDialog::editAntennaName);

    initSetup();

}



RotSetupDialog::~RotSetupDialog()
{
    delete ui;
}



void RotSetupDialog::initSetup()
{

    // get the number of available antennas

    QString fileName = ANTENNA_PATH_LOGGER() + FILENAME_AVAIL_ANTENNAS;
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
            mShowMessage(tr("The Rotator configuration files in %1 are from an old incompatible version of Minos.\n\n"
                            "Please delete them and set up the rotators again").arg(ANTENNA_PATH_LOGGER()), parentWidget());
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

            RotCapabilities rotCap = rotatorFactory->supported_rotators()->value(availAntData[i]->rotatorModel);

            availAntData[i]->rotatorModelName = rotCap.getRotatorModelName();
            availAntData[i]->rotatorModelNumber = rotCap.getModelNumber();
            availAntData[i]->rotatorManufacturer = rotCap.getRotatorManufacturer();
            availAntData[i]->rotatorCWEndStop = rotCap.getMaxRot();
            availAntData[i]->rotatorCCWEndStop = rotCap.getMinRot();

            //bool overLapState = availAntData[i]->overLapFlag;   // save as it is changed in setEndStopType
            antennaTab[i]->getRotatorType(availAntData[i], availAntData[i]->rotatorCCWEndStop, availAntData[i]->rotatorCWEndStop);
            availAntData[i]->overLapFlag = availAntData[i]->overLapFlag;
            antennaTab[i]->setEndStopType(availAntData[i], availAntData[i]->rotatorCCWEndStop, availAntData[i]->rotatorCWEndStop);
            //availAntData[i]->overLapFlag = availAntData[i]->overLapFlag;  //restore
            //antennaTab[i]->setOverlapEndStop(availAntData[i], overLapState);

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

    RotCapabilities rotCap = rotatorFactory->supported_rotators()->value(availAntData[tabNum]->rotatorModel);

    ui->antennaTab->setTabText(tabNum, availAntData[tabNum]->antennaName);

    if (availAntData[tabNum]->rotatorModel != "")
    {
        antennaTab[tabNum]->setRotatorModel(availAntData[tabNum]->rotatorModel);
        antennaTab[tabNum]->setPollInterval(availAntData[tabNum]->pollInterval);
        antennaTab[tabNum]->pollIntervalVisible(true);
        //antennaTab[tabNum]->setCheckStop(availAntData[tabNum]->southStopType);

        antennaTab[tabNum]->setCheckOverLap(availAntData[tabNum]->overLapFlag);
        antennaTab[tabNum]->setSimCW_CCWcmdChecked(availAntData[tabNum]->simCwCcwCmd);

        // set southstop visible if rotator is 0 - 360


        if (availAntData[tabNum]->rotType == ROT_0_360)
        {
           if (rotCap.getAllowSouthStopConfig())
           {
              antennaTab[tabNum]->sStopButtonsVisible(true);
              antennaTab[tabNum]->setSStopButtons(availAntData[tabNum]->southStopType);

           }
           else
           {
               antennaTab[tabNum]->sStopButtonsVisible(false);
               antennaTab[tabNum]->setSStopButtons(availAntData[tabNum]->southStopType);
           }

           antennaTab[tabNum]->setOverLapFlagVisible(false);
        }
        else if (availAntData[tabNum]->rotType == ROT_NEG180_450)
        {
           antennaTab[tabNum]->setOverLapFlagVisible(true);
           if ((availAntData[tabNum]->endStopType == ROT_0_360
                || availAntData[tabNum]->endStopType == ROT_0_450
                ||availAntData[tabNum]->endStopType == ROT_NEG179_180)
               && !availAntData[tabNum]->overLapFlag  )
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
           antennaTab[tabNum]->setOverLapFlagVisible(false);
        }

        //antennaTab[tabNum]->setCheckOverLap(availAntData[tabNum]->overLapFlag);
        antennaTab[tabNum]->setAntennaOffset(QString::number(availAntData[tabNum]->antennaOffset));
        antennaTab[tabNum]->antennaOffSetVisible(true);
        //antennaTab[tabNum]->setComport(availAntData[tabNum]->comport);
        loadAvailComportsToTab(tabNum);
        antennaTab[tabNum]->setDataSpeed(QString::number(availAntData[tabNum]->baudrate));
        antennaTab[tabNum]->setDataBits(QString::number(availAntData[tabNum]->databits));
        antennaTab[tabNum]->setStopBits(QString::number(availAntData[tabNum]->stopbits));
        antennaTab[tabNum]->setParityBits(static_cast<int>(availAntData[tabNum]->parity));
        antennaTab[tabNum]->setHandshake(static_cast<int>(availAntData[tabNum]->handshake));
        antennaTab[tabNum]->setForceDTR(static_cast<int>(availAntData[tabNum]->forceDtr));
        antennaTab[tabNum]->setForceRTS(static_cast<int>(availAntData[tabNum]->forceRts));

        antennaTab[tabNum]->setNetAddress(availAntData[tabNum]->networkAdd);
        antennaTab[tabNum]->setNetPortNum(availAntData[tabNum]->networkPort);

        if (availAntData[tabNum]->portType == RotCapConstants::PortType::network)
        {
            antennaTab[tabNum]->serialDataEntryVisible(false);
            antennaTab[tabNum]->advancedSerialDataEntryVisible(false);
            antennaTab[tabNum]->setAdvancedCommsChkBoxVisible(false);
            antennaTab[tabNum]->networkDataEntryVisible(true);
            antennaTab[tabNum]->advancedSerialDataEntryVisible(false);
        }
        else if (availAntData[tabNum]->portType == RotCapConstants::PortType::none)
        {
            antennaTab[tabNum]->serialDataEntryVisible(false);
            antennaTab[tabNum]->advancedSerialDataEntryVisible(false);
            antennaTab[tabNum]->setAdvancedCommsChkBoxVisible(false);
            antennaTab[tabNum]->networkDataEntryVisible(false);
            antennaTab[tabNum]->advancedSerialDataEntryVisible(false);
        }
        else if (availAntData[tabNum]->portType == RotCapConstants::PortType::serial)
        {
            antennaTab[tabNum]->serialDataEntryVisible(true);
            antennaTab[tabNum]->advancedSerialDataEntryVisible(availAntData[tabNum]->advancedCommsFlag);
            antennaTab[tabNum]->setAdvancedCommsChkBoxVisible(true);
            antennaTab[tabNum]->checkAdvancedCommsCheckBox(availAntData[tabNum]->advancedCommsFlag);
            antennaTab[tabNum]->advancedSerialDataEntryVisible(true);
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

        if (!rotCap.getSupportStopCommand())
        {
            antennaTab[tabNum]->setSimCW_CCWcmdVisible(false);

        }


        if (rotCap.getEnableSelectDisplayDial())
        {
            antennaTab[tabNum]->setCompassDialChkBoxVisible(true);
            antennaTab[tabNum]->checkCompassDialChkBox(availAntData[tabNum]->showCompassDialFlag);
        }
        else
        {
            antennaTab[tabNum]->setCompassDialChkBoxVisible(false);
        }

        if (rotCap.getAllowSkyScan())
        {
            antennaTab[tabNum]->setSkyScanTabChkBoxVisible(true);
            antennaTab[tabNum]->checkSkyScanChkBox(availAntData[tabNum]->showSkyScanFlag);
        }

    }
    else
    {
        // no rotator model selected, hide elements on tab
        antennaTab[tabNum]->setRotatorModel(availAntData[tabNum]->rotatorModel);
        antennaTab[tabNum]->networkDataEntryVisible(false);
        antennaTab[tabNum]->setOverLapFlagVisible(false);
        antennaTab[tabNum]->sStopButtonsVisible(false);
        antennaTab[tabNum]->setSStopButtons(S_STOPOFF);
        antennaTab[tabNum]->pollIntervalVisible(false);
        antennaTab[tabNum]->antennaOffSetVisible(false);
        antennaTab[tabNum]->serialDataEntryVisible(false);
        antennaTab[tabNum]->setSimCW_CCWcmdVisible(false);
        antennaTab[tabNum]->setCompassDialChkBoxVisible(false);
        antennaTab[tabNum]->setSkyScanTabChkBoxVisible(false);

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
    RegSettings settings;
    settings.getSettings().setValue("RotControlSetup/geometry", saveGeometry());
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
    fileName = ANTENNA_PATH_LOGGER() + FILENAME_AVAIL_ANTENNAS;
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
            config.setValue("supportCwCcwCmd", availAntData[i]->supportCwCcwCmd);
            config.setValue("rotatorPollInterval", availAntData[i]->pollInterval);
            config.setValue("simulateCwCCw", availAntData[i]->simCwCcwCmd);
            config.setValue("overRun", availAntData[i]->overLapFlag);           // leave name as overrun in ini file
            config.setValue("southStopType", availAntData[i]->southStopType);
            config.setValue("antennaOffset", availAntData[i]->antennaOffset);
            config.setValue("portType", availAntData[i]->portType);
            config.setValue("comport", availAntData[i]->comport);
            config.setValue("baudrate", availAntData[i]->baudrate);
            config.setValue("databits", availAntData[i]->databits);
            config.setValue("parity", static_cast<int>(availAntData[i]->parity));
            config.setValue("stopbits", availAntData[i]->stopbits);
            config.setValue("handshake", static_cast<int>(availAntData[i]->handshake));
            config.setValue("forceDTR", static_cast<int>(availAntData[i]->forceDtr));
            config.setValue("forceRTS", static_cast<int>(availAntData[i]->forceRts));
            config.setValue("advancedComms", availAntData[i]->advancedCommsFlag);
            config.setValue("netAddress", availAntData[i]->networkAdd);
            config.setValue("netPort", availAntData[i]->networkPort);
            config.setValue("showCompassDial", availAntData[i]->showCompassDialFlag);
            config.setValue("showSkyScan", availAntData[i]->showSkyScanFlag);
            config.setValue("nearStopTolerance", availAntData[i]->nearStopTolerance);
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
    availAntData[antNum]->supportCwCcwCmd = config.value("supportCwCcwCmd", false).toBool();
    availAntData[antNum]->simCwCcwCmd = config.value("simulateCwCCw", true).toBool();
    availAntData[antNum]->overLapFlag = config.value("overRun", false).toBool();
    availAntData[antNum]->southStopType = static_cast<southStop> (config.value("southStopType", southStop::S_STOPOFF).toInt());
    availAntData[antNum]->antennaOffset = config.value("antennaOffset", "").toInt();
    availAntData[antNum]->portType = (config.value("portType", RotCapConstants::PortType::none).toInt());
    availAntData[antNum]->comport = config.value("comport", "").toString();
    availAntData[antNum]->baudrate = config.value("baudrate", 9600).toInt();
    availAntData[antNum]->databits = config.value("databits", 8).toInt();
    availAntData[antNum]->parity = static_cast<serialCommonData::serialParityCodes>(config.value("parity", 0).toInt());
    availAntData[antNum]->stopbits = config.value("stopbits", 1).toInt();
    availAntData[antNum]->handshake = static_cast<serialCommonData::s_handshakeCodes>(config.value("handshake", 0).toInt());
    availAntData[antNum]->forceDtr = static_cast<serialCommonData::s_forceLinesCodes>(config.value("forceDTR", 0).toInt());
    availAntData[antNum]->forceRts= static_cast<serialCommonData::s_forceLinesCodes>(config.value("forceRTS", 0).toInt());
    availAntData[antNum]->networkAdd = config.value("netAddress", "").toString();
    availAntData[antNum]->networkPort = config.value("netPort", "").toString();
    availAntData[antNum]->advancedCommsFlag = config.value("advancedComms", false).toBool();
    availAntData[antNum]->showCompassDialFlag = config.value("showCompassDial",true).toBool();
    availAntData[antNum]->showSkyScanFlag = config.value("showSkyScan", false).toBool();
    availAntData[antNum]->nearStopTolerance = config.value("nearStopTolerance", 0).toBool();

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

void RotSetupDialog::saveCurrentAntenna()
{

    QString fileName;
    fileName = ANTENNA_PATH_LOGGER() + appName + FILENAME_CURRENT_ANTENNA;
    QSettings config(fileName, QSettings::IniFormat);


    config.beginGroup("CurrentAntenna");
    config.setValue("antennaName", currentAntennaName);
    config.endGroup();




}

void RotSetupDialog::readCurrentAntenna()
{

    QString fileName;
    fileName = ANTENNA_PATH_LOGGER() + appName + FILENAME_CURRENT_ANTENNA;
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

  // initial settings
  antennaTab[tabNum]->setDataSpeed("9600");
  antennaTab[tabNum]->comDataSpeedSelected();

  antennaTab[tabNum]->setDataBits("8");
  antennaTab[tabNum]->comDataBitsSelected();

  antennaTab[tabNum]->setStopBits("1");
  antennaTab[tabNum]->comStopBitsSelected();

  antennaTab[tabNum]->setParityBits(0);
  antennaTab[tabNum]->comParityBitsSelected();


  antennaTab[tabNum]->setForceRTS(1);
  antennaTab[tabNum]->on_forceRTSSelected();


  ui->antennaTab->setCurrentIndex(tabNum);
  emit antennaTabChanged();


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
                            QMessageBox::Yes|QMessageBox::No|QMessageBox::Escape,
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

void RotSetupDialog::updateAvailAntennasToVersion2()
{

    QString fileName = ANTENNA_PATH_LOGGER() + FILENAME_AVAIL_ANTENNAS;
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


