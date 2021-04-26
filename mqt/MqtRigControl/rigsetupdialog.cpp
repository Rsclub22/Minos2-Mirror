/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018 - 2021
//
//
/////////////////////////////////////////////////////////////////////////////





#include "rigsetupdialog.h"
#include "rigsetupform.h"
#include "transvertsetupform.h"
#include "ui_rigsetupdialog.h"
#include "rigcontrolcommonconstants.h"
#include "addradiodialog.h"
#include "rigutils.h"
#include <QComboBox>
#include <QMessageBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSettings>
#include <QInputDialog>

#include <QDebug>



RigSetupDialog::RigSetupDialog(RigFactory* rigFactory_, const QVector<QSharedPointer<BandInfo> > &_bands, const bool hfFlag_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RigSetupDialog),
    hfFlag(hfFlag_)

{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    rigFactory = rigFactory_;
    bands = _bands;

    QSettings settings;
    QByteArray geometry = settings.value("RigControl/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    //connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &RigSetupDialog::saveButtonPushed);
    //connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &RigSetupDialog::cancelButtonPushed);
    connect(ui->addRadio, &QPushButton::clicked, this, &RigSetupDialog::addRadio);
    connect(ui->removeRadio, &QPushButton::clicked, this, &RigSetupDialog::removeRadio);
    connect(ui->editRadioName, &QPushButton::clicked, this, &RigSetupDialog::editRadioName);


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

    // get current settings
    availRadios = settings.childGroups();
    for (int i = 0; i < availRadios.count(); i++)
    {
        if (availRadios[i].contains("Version"))
        {
            availRadios.removeAt(i);
            break;
        }
    }

    //numAvailRadios = availRadios.count();

    for (int i = 0; i < availRadios.count(); i++)
    {
        addTab(i, availRadios[i]);

        // find transverters

        QString fileName = TRANSVERT_PATH_LOGGER + availRadios[i] + FILENAME_TRANSVERT_RADIOS;
        QSettings  configTransvert(fileName, QSettings::IniFormat);

        QStringList listOfTransverters = configTransvert.childGroups();  // get transvert names for this radio
        //availRadioData.value(availRadios[i])->transVertSettings.count() =  availRadioData.value(availRadios[i])->transVertNames.count();

        if (listOfTransverters.count() > 0)
        {
            for (int t = 0; t < listOfTransverters.count(); t++)
            {
               radioTab.value(availRadios[i])->addTransVertTab(t, listOfTransverters[t], false);   // adding and existing tab, set change flag = N0CHANGE
            }
        }

        getRadioSetting(availRadioData.value(availRadios[i]), availRadios[i], settings);


        loadSettingsToTab(i, availRadios[i]);
    }


}




void RigSetupDialog::addTab(int tabNum, QString tabName)
{
    availRadioData.insert(tabName, QSharedPointer<scatParams>(new scatParams));
    availRadioData.value(tabName)->radioName = tabName;
    if (!availRadios.contains(tabName))
    {
       availRadios.append(tabName);
    }

    radioTab.insert(tabName, new RigSetupForm(rigFactory, availRadioData.value(tabName), bands, ui->radioTab, hfFlag));
    ui->radioTab->insertTab(tabNum, radioTab.value(tabName), tabName);
    ui->radioTab->setTabColor(tabNum, Qt::darkBlue);      // radioTab promoted to QLogTabWidget


}


void RigSetupDialog::loadSettingsToTab(int tabNum, QString tabName)
{

    ui->radioTab->setTabText(tabNum, availRadioData.value(tabName)->radioName);

    radioTab.value(tabName)->setRadioModel(availRadioData.value(tabName)->rigModel);

    radioTab.value(tabName)->setCIVAddress(availRadioData.value(tabName)->civAddress);
    loadAvailComportsToTab(tabName);                                                 // finds comports first
    //radioTab[tabNum]->setComport(radioTab[tabNum]->getRadioData()->comport);
    radioTab.value(tabName)->setDataSpeed(QString::number(availRadioData.value(tabName)->baudrate));
    radioTab.value(tabName)->setDataBits(QString::number(availRadioData.value(tabName)->databits));
    radioTab.value(tabName)->setStopBits(QString::number(availRadioData.value(tabName)->stopbits));
    radioTab.value(tabName)->setParityBits(availRadioData.value(tabName)->parity);
    radioTab.value(tabName)->setHandshake(availRadioData.value(tabName)->handshake);
    radioTab.value(tabName)->setForceDTRComboBox(availRadioData.value(tabName)->forceDtr);
    radioTab.value(tabName)->setForceRTSComboBox(availRadioData.value(tabName)->forceRts);
    radioTab.value(tabName)->setNetAddress(availRadioData.value(tabName)->networkAdd);
    radioTab.value(tabName)->setNetPortNum(availRadioData.value(tabName)->networkPort);

    RigCapabilities rigCap = rigFactory->supported_rigs()->value(availRadioData.value(tabName)->rigModel);


    radioTab.value(tabName)->setPollInterval(availRadioData.value(tabName)->pollInterval);
    if (rigCap.pollData)
    {
        radioTab.value(tabName)->pollIntervalVisible(true);
    }
    else
    {
        radioTab.value(tabName)->pollIntervalVisible(false);
    }

    radioTab.value(tabName)->setTransVertSelected(availRadioData.value(tabName)->transVertEnable);
    radioTab.value(tabName)->setEnableTransVertSw(availRadioData.value(tabName)->enableTransSwitch);
    radioTab.value(tabName)->setEnableLocalTransVertSw(availRadioData.value(tabName)->enableLocTVSwMsg);



    if (rigCap.portType == RigCapConstants::PortType::network)
    {
        radioTab.value(tabName)->serialDataEntryVisible(false);
        radioTab.value(tabName)->advancedSerialDataEntryVisible(false);
        radioTab.value(tabName)->setAdvancedCommsChkBoxVisible(false);
        radioTab.value(tabName)->networkDataEntryVisible(true);
        radioTab.value(tabName)->setRigctldCheckBoxVisible(false);
        radioTab.value(tabName)->getRadioData()->rigCtldEnable = false;
    }
    else if (rigCap.portType == RigCapConstants::PortType::serial)
    {
        radioTab.value(tabName)->serialDataEntryVisible(true);
        radioTab.value(tabName)->advancedSerialDataEntryVisible(availRadioData.value(tabName)->advancedCommsFlag);
        radioTab.value(tabName)->setAdvancedCommsChkBoxVisible(true);
        radioTab.value(tabName)->checkAdvancedCommsCheckBox(availRadioData.value(tabName)->advancedCommsFlag);
        radioTab.value(tabName)->networkDataEntryVisible(false);
        if (radioTab.value(tabName)->getRadioData()->handshake == serialCommonData::handshakeCodes::HANDSHAKE_HARDWARE) // CTS/RTS enabled
        {
            radioTab.value(tabName)->setForceRTSDisabled(true);
        }
        else
        {
            radioTab.value(tabName)->setForceRTSDisabled(false);
        }

    }
    else if (rigCap.portType == RigCapConstants::PortType::none)
    {
        radioTab.value(tabName)->serialDataEntryVisible(false);
        radioTab.value(tabName)->advancedSerialDataEntryVisible(false);
        radioTab.value(tabName)->setAdvancedCommsChkBoxVisible(false);
        radioTab.value(tabName)->networkDataEntryVisible(false);
        radioTab.value(tabName)->setRigctldCheckBoxVisible(false);
        radioTab.value(tabName)->getRadioData()->rigCtldEnable = false;
    }

    // serial ptt comport loaded with other comports
    radioTab.value(tabName)->setPttTypeRadioButtons(availRadioData.value(tabName)->pttType);


    if (rigCap.supportGetPtt && rigCap.supportSetPtt)
    {
        radioTab.value(tabName)->setPTTCheckBoxDisabled(false);
        if (radioTab.value(tabName)->getRadioData()->enablePTT)
        {
            radioTab.value(tabName)->setPttControlsVisible(true);
            radioTab.value(tabName)->setPTTCheckBoxChecked(true);
            if (radioTab.value(tabName)->getRadioData()->pttType == serialCommonData::PTT_METHOD_CAT)
            {

            }

        }
        else
        {
           radioTab.value(tabName)->setPttControlsVisible(false);
           radioTab.value(tabName)->setPTTCheckBoxChecked(false);

        }
    }
    else
    {
        radioTab.value(tabName)->setPttControlsVisible(false);
        radioTab.value(tabName)->setPTTCheckBoxChecked(false);
        radioTab.value(tabName)->setPTTCheckBoxDisabled(true);
    }




    radioTab.value(tabName)->setMgmMode(availRadioData.value(tabName)->mgmMode);

    //for (int i =0; i < radioTab[tabNum]->getRadioData()->supportBands.count(); i++)
    foreach(auto &b, bands)
    {
       radioTab.value(tabName)->setSupportBandChkBox(b.data()->name(), availRadioData.value(tabName)->supportBands.getSupportBandFlag(b.data()->name()));
    }


    if (rigCap.supportGetSupBands)
    {
        radioTab.value(tabName)->setSupportBandCheckBoxVisible(false);
    }
    else
    {
        radioTab.value(tabName)->setSupportBandCheckBoxVisible(true);
    }

    radioTab.value(tabName)->setUseRigctldCheckbox(availRadioData.value(tabName)->rigCtldEnable);
    radioTab.value(tabName)->setStartMinosRigctldCheckbox(availRadioData.value(tabName)->startMinosRigCtld);
    radioTab.value(tabName)->rigCtldItemsVisible(availRadioData.value(tabName)->rigCtldEnable);
    radioTab.value(tabName)->setRigctldNetworkAddress(availRadioData.value(tabName)->rigCtldNetworkAdd);
    radioTab.value(tabName)->setRigctldPortNumber(availRadioData.value(tabName)->rigCtldNetworkPort);

    // now load transverter settings

    if (availRadioData.value(tabName)->transVertSettings.count() > 0 )
    {
        //for (int t = 0; t < availRadioData.value(tabName)->numTransverters; t++)
        QStringList tvList = availRadioData.value(tabName)->transVertSettings.keys();
        foreach(const auto &tv, tvList)
        {
            //radioTab.value(tabName)->setTransVertTabText(tv, tvList);
            radioTab.value(tabName)->loadTransVertTab(tv);
        }

    }

   // display the correct transverter settings

    radioTab.value(tabName)->setLocTVSwComport(availRadioData.value(tabName)->locTVSwComport);

    radioTab.value(tabName)->setTransVertSelected(availRadioData.value(tabName)->transVertEnable);
    if (availRadioData.value(tabName)->transVertEnable)
    {
        radioTab.value(tabName)->setTransVertSwVisible(true);
        radioTab.value(tabName)->setEnableLocalTransVertSwVisible(false);
    }
    else
    {
        radioTab.value(tabName)->setTransVertSwVisible(false);
        radioTab.value(tabName)->setEnableLocalTransVertSwVisible(false);
        radioTab.value(tabName)->setLocTVSWComportVisible(false);
    }

    if (availRadioData.value(tabName)->transVertEnable && availRadioData.value(tabName)->enableTransSwitch)
    {
        radioTab.value(tabName)->setTransVertSwVisible(true);
        radioTab.value(tabName)->setEnableLocalTransVertSwVisible(true);
        //for (int i = 0; i < availRadioData.value(tabName)->numTransverters; i++)
        QStringList tvList = availRadioData.value(tabName)->transVertSettings.keys();
        foreach(const auto &tv, tvList)
        {
            radioTab.value(tabName)->transVertTab.value(tv)->setEnableTransVertSwBoxVisible(true);
        }
    }

    if (availRadioData.value(tabName)->transVertEnable && availRadioData.value(tabName)->enableTransSwitch && availRadioData.value(tabName)->enableLocTVSwMsg)
    {
        radioTab.value(tabName)->setLocTVSWComportVisible(true);
    }
    else
    {
        radioTab.value(tabName)->setLocTVSWComportVisible(false);
    }

    radioTab.value(tabName)->loadEnableShowCatFeaturesBox(rigCap);



}




void RigSetupDialog::addRadio()
{
    //QStringList availRadios;
    //getAvailRadiosList(availRadios);
    QString windowTitle = tr("Add Radio and Radio Model");
    AddRadioDialog getRadioName_Rig(&availRadioData, rigFactory, windowTitle);
    //getRadioName_Rig.setWindowTitle(tr("Add Radio and Radio Model"));
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
    int tabNum = availRadios.count();
    addTab(tabNum, radioName);
    numAvailRadios++;
    radioTab.value(radioName)->setAdvancedCommsFlag(false);
    radioTab.value(radioName)->setupRadioModel(radioModel);
    radioTab.value(radioName)->setPollInterval(RIG_DEFAULT_POLLINTERVAL);

    loadAvailComportsToTab(radioName);

    // initial settings
    radioTab.value(radioName)->setDataSpeed("9600");
    radioTab.value(radioName)->comSpeedSelected();

    radioTab.value(radioName)->setDataBits("8");
    radioTab.value(radioName)->comDataBitsSelected();

    radioTab.value(radioName)->setStopBits("1");
    radioTab.value(radioName)->comStopBitsSelected();

    radioTab.value(radioName)->setParityBits(0);
    radioTab.value(radioName)->comParitySelected(true);


    radioTab.value(radioName)->setForceRTSComboBox(1);
    radioTab.value(radioName)->on_forceRTSSelected();



    ui->radioTab->setCurrentIndex(tabNum);



}


void RigSetupDialog::getAvailRadiosList(QStringList &availRadios)
{
    foreach (const auto &rd, availRadioData)
    {
        if (!rd->markForDeletion)
        {
            availRadios.append(rd->radioName);
        }
    }
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
        msgBox.setText(tr("You can not remove the current radio!"));
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
    availRadioData.value(currentName)->markForDeletion = true;
    //availRadios.removeAt(currentIndex);
    radioTab.remove(currentName);
    //numAvailRadios--;
    //setupChangeFlags.setRadioRemoved(true);

    //emit radioTabChanged();



}




void RigSetupDialog::editRadioName()
{
    int tabNum = ui->radioTab->currentIndex();
    QString radioName = ui->radioTab->tabText(tabNum);

    if (currentRadioName == radioName)
    {
        // can't change current antennaName
        QMessageBox msgBox;
        msgBox.setText(tr("You can not change the name of the current radio - %1!").arg(radioName));
        msgBox.exec();
        return;
    }

    //bool ok;

    AddRadioDialog editRadioName(&availRadioData, rigFactory, tr("Edit Radio Name - %1").arg(radioName));
    //editRadioName.setWindowTitle(tr("Edit Radio Name - %1").arg(radioName));
    editRadioName.hideRadioSelection(false);
    if (editRadioName.exec() != QDialog::Accepted)
    {
        return;
    }
    //QString newName = QInputDialog::getText(this, tr("Edit Radio Name - %1").arg(radioName),
    //                                     tr("Edit Radio Name:"), QLineEdit::Normal,
    //                                     radioName, &ok).trimmed();

    QString newName = editRadioName.getRadioName();

    if (!newName.isEmpty())
    {

        if (newName != radioName)
        {
            //for (int i = 0; i < numAvailRadios; i++)
            QStringList avrList = availRadioData.keys();

            // does the name already exist
            foreach (const auto &r, avrList)
            {
                if (newName == availRadioData.value(r)->radioName)
                {
                    if (availRadioData.value(r)->markForDeletion)
                    {
                        QMessageBox msgBox;
                        msgBox.setText(tr("%1 has been marked for deletion, please try another name").arg(newName));
                        msgBox.exec();
                    }
                    else
                    {
                        QMessageBox msgBox;
                        msgBox.setText(tr("%1 already exists, please try another name").arg(newName));
                        msgBox.exec();
                    }

                    return;
                }
            }


            foreach (const auto &r, avrList)
            {
                if (radioName == availRadioData.value(r)->radioName)
                {
                    QSharedPointer<scatParams> radioData = QSharedPointer<scatParams>(new scatParams());
                    radioData = availRadioData.value(radioName);
                    radioData->radioName = newName;
                    radioData->previousRadioName =radioName;
                    ui->radioTab->setTabText(tabNum, newName);

                    //update data tables
                    availRadioData.remove(radioName);
                    availRadioData.insert(newName, radioData);

                    RigSetupForm *rsf = radioTab.value(radioName);
                    radioTab.remove(radioName);
                    radioTab.insert(newName, rsf);
               }
            }
        }



    }


}


void RigSetupDialog::setTabToCurrentRadio()
{

   for (int i = 0; i < ui->radioTab->count(); i++)
   {

       if (availRadioData.contains(ui->radioTab->tabText(i)))
       {
           if (currentRadioName == availRadioData.value(ui->radioTab->tabText(i))->radioName)
            {
                ui->radioTab->setTabColor(i, Qt::red);
                ui->radioTab->setCurrentIndex(i);
            }
            else
            {
                ui->radioTab->setTabColor(i,Qt::darkBlue);
            }
       }
       else
       {
           trace(QString("SetTabToCurrentRadio - radio %1 is not in availRadioData").arg(ui->radioTab->tabText(i)));
       }



   }

}

int RigSetupDialog::comportAvial(QString radioName, QString comport)
{
    if (radioTab.count() > 0)
    {
        return radioTab.value(radioName)->comportAvial(comport);
    }

    return -1;
}


void RigSetupDialog::loadAvailComports()
{
    QStringList lk = radioTab.keys();
    foreach (auto &k, lk)
    {
        loadAvailComportsToTab(k);
        loadAvailPttComportsToTab(k);
    }


}


void RigSetupDialog::loadAvailComportsToTab(QString radioName)
{
    radioTab.value(radioName)->loadRadioComports();
    radioTab.value(radioName)->setComport(availRadioData.value(radioName)->comport);
}

void RigSetupDialog::loadAvailPttComportsToTab(QString radioName)
{
    radioTab.value(radioName)->loadAvailPttComports();
    radioTab.value(radioName)->setPttComport(availRadioData.value(radioName)->pttSerialPort);
}

void RigSetupDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("RigControl/geometry", saveGeometry());
}



void RigSetupDialog::closeEvent (QCloseEvent *event)
{
    //cancelButtonPushed();
    doCloseEvent();
    QWidget::closeEvent(event);
}


void RigSetupDialog::done(int r)
{
    bool supportedBandsOK = false;
    bool transvertFreqInBand = false;

    if(QDialog::Accepted == r)  // ok was pressed
    {
        supportedBandsOK = checkOmniRigSupportedBands();
        transvertFreqInBand = checkTransvertFreqInBand();

        if (supportedBandsOK && transvertFreqInBand)
        {
            saveSettings();
        }
        else
        {
            return;
        }

   }
   // else    // cancel, close or exc was pressed
  //  {
        QDialog::done(r);

  //  }


}

bool RigSetupDialog::checkTransvertFreqInBand()
{
    QStringList radList = availRadioData.keys();
    QStringList outOfBandTransverts;

    foreach(const auto &r, radList)
    {
        if (availRadioData.value(r)->transVertEnable)
        {
            QStringList transvertList = availRadioData.value(r)->transVertSettings.keys();
            foreach(const auto &tv, transvertList)
            {
                QString transVertBand;
                if (!transVerterInBand(availRadioData.value(r)->transVertSettings.value(tv), transVertBand ))
                {
                    outOfBandTransverts.append(r + ':' + transVertBand);
                }

            }
        }
    }

    if (outOfBandTransverts.isEmpty())
    {
        // transvert settings ok
        return true;
    }
    else
    {
        QString outofBandTransvertMsg;
        foreach(const auto &obt, outOfBandTransverts)
        {
            QStringList radioTransVert = obt.split(':');
            if (radioTransVert.count() == 2)
            {
                outofBandTransvertMsg.append(QString("Radio = %1, Band = %2\n").arg(radioTransVert[0], radioTransVert[1]));
            }

        }

        QMessageBox::critical(this, tr("Transvert Settings Out of Band"),
                                       tr("The Transvert settings are out of band for the\n"
                                          "following:\n"
                                          "%1").arg(outofBandTransvertMsg),
                                       QMessageBox::Ok);

        return false;
    }
}

bool RigSetupDialog::transVerterInBand(const QSharedPointer<TransVertParams>tvp, QString &transVertBand)
{

    Frequency targetFreq = tvp->radioFreq + tvp->transVertOffset;

    foreach (const auto &b, bands)
    {
        if (b.data()->uk == tvp->band)
        {
            transVertBand = tvp->band;

            if (targetFreq >= b->fLow && targetFreq <= b->fHigh)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

    }



    return false;
}


bool RigSetupDialog::checkOmniRigSupportedBands()
{
    QString supRadNames;
    isAnySupportedBandsAvailForOmnirig(supRadNames);
    if (supRadNames.isEmpty())  // no radios with missing supportbands found
    {
        return true;
    }

    QMessageBox::critical(this, tr("Radio Supported Bands Missing"),
                                   tr("For Minos to work best with Radios,\n"
                                      "Please add bands or transverters to\n"
                                      "these radio definitions:\n"
                                      "%1").arg(supRadNames),
                                   QMessageBox::Ok);

    return false;

}

/*
void RigSetupDialog::saveButtonPushed()
{
    QString supRadNames;
    isAnySupportedBandsAvail(supRadNames);
    if (supRadNames.isEmpty())
    {
        saveSettings();
    }
    else
    {
        QMessageBox::critical(this, tr("Radio Supported Bands Missing"),
                                       tr("For Minos to work best with Radios,\n"
                                          "Please add bands or transverters to\n"
                                          "these radio definitions:\n"
                                          "%1").arg(supRadNames),
                                       QMessageBox::Ok);
    }

    doCloseEvent();

}
*/

// finds Omnirig radios with no supported bands checked

void RigSetupDialog::isAnySupportedBandsAvailForOmnirig(QString &supRadNames)
{
    QStringList lk = radioTab.keys();
    foreach (auto &k, lk)
    {
        RigCapabilities rigCap = rigFactory->supported_rigs()->value(radioTab.value(k)->getRadioData()->rigModel);
        if (!rigCap.supportGetSupBands)
        {
            if (!radioTab.value(k)->isAnySupportBandChecked() && radioTab.value(k)->getRadioData()->transVertSettings.count() == 0)
            {
                supRadNames.append(radioTab.value(k)->getRadioData()->radioName + '\n');
            }
        }
    }

}
/*
void RigSetupDialog::cancelButtonPushed()
{

    bool change = false;
    for (int i = 0; i < radioTab.count(); i++)
    {
        if (radioTab[i]->rigSetupFlags.getRadioValueChanged() || setupChangeFlags.getRadioRemoved() || radioTab[i]->getTransVertRemovedFlag())
        {
            setupChangeFlags.setRadioRemoved(false);
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
*/




void RigSetupDialog::saveSettings()
{


    QString fileNameTransVert;

    QString fileNameRadio = RADIO_PATH_LOGGER + FILENAME_AVAIL_RADIOS;
    QSettings configRadio(fileNameRadio, QSettings::IniFormat);

    QStringList lk = availRadioData.keys();
    // look for deleted radios or radio name change
    foreach (auto &k, lk)
    {
        if (availRadioData.value(k)->markForDeletion )
        {
            configRadio.beginGroup(k);
            configRadio.remove("");      // remove all keys for this group
            configRadio.endGroup();
            // remove transverters for this radio
            fileNameTransVert = TRANSVERT_PATH_LOGGER + k + FILENAME_TRANSVERT_RADIOS;
            if (QFile::exists(fileNameTransVert))
            {
                QFile::remove(fileNameTransVert);
            }

            availRadioData.remove(k);
        }
    }

    // look for radios with changed name
    lk = availRadioData.keys();
    foreach (auto &k, lk)
    {
        if (!availRadioData.value(k)->previousRadioName.isEmpty())
        {
            configRadio.beginGroup(k);
            configRadio.remove("");      // remove all keys for this group
            configRadio.endGroup();
            // remove transverters for this radio
            fileNameTransVert = TRANSVERT_PATH_LOGGER + k + FILENAME_TRANSVERT_RADIOS;
            if (QFile::exists(fileNameTransVert))
            {
                QFile::remove(fileNameTransVert);
            }
            // gather data to send to other rigcontrols
            QSharedPointer<RadioNameChange> radioNames = QSharedPointer<RadioNameChange>(new RadioNameChange);
            radioNames->oldName = availRadioData.value(k)->previousRadioName;
            radioNames->newName = availRadioData.value(k)->radioName;
            listOfRadioNameChanges.append(radioNames);

        }
    }


    // look for radios with changed data
    foreach (auto &k, lk)
    {
        QString fileName;
        fileName = RADIO_PATH_LOGGER + FILENAME_AVAIL_RADIOS;
        QSettings  settings(fileName, QSettings::IniFormat);

        QSharedPointer<scatParams> savedRadioData = QSharedPointer<scatParams>(new scatParams());

        getRadioSetting(savedRadioData, k, settings);

        bool dataChanged = false;
        if (availRadioData.value(k)->compareNotEqual(savedRadioData))
        {
            dataChanged = true;
            saveRadioData(availRadioData.value(k), settings);
        }

        if (availRadioData.value(k)->transVertEnable)
        {
            QStringList tvKeys = availRadioData.value(k)->transVertSettings.keys();
            QString fileName = TRANSVERT_PATH_LOGGER + k + FILENAME_TRANSVERT_RADIOS;
            QSettings  configTransvert(fileName, QSettings::IniFormat);

            if (availRadioData.value(k)->transVertSettingsNotEqual(savedRadioData->transVertSettings))
            {
                dataChanged = false;
                foreach(const auto &tv ,tvKeys)
                {
                    saveTranVerterSetting(availRadioData.value(k), tv, configTransvert);
                }

            }
        }

        if (dataChanged)
        {
            listOfRadiosDataChanged.append(k);
        }


    }


}


void RigSetupDialog::saveRadioData(QSharedPointer<scatParams> radioData, QSettings& config)
{

    config.beginGroup(radioData->radioName);
    config.setValue("radioName", radioData->radioName);
    config.setValue("radioModel", radioData->rigModel);
    config.setValue("civAddress", radioData->civAddress);
    config.setValue("portType", radioData->portType);
    config.setValue("advancedComms", radioData->advancedCommsFlag);
    config.setValue("comport", radioData->comport);
    config.setValue("baudrate", radioData->baudrate);
    config.setValue("databits", radioData->databits);
    config.setValue("parity", radioData->parity);
    config.setValue("stopbits", radioData->stopbits);
    config.setValue("handshake", radioData->handshake);
    config.setValue("forceDTR", radioData->forceDtr);
    config.setValue("forceRTS", radioData->forceRts);
    config.setValue("enablePtt", radioData->enablePTT);
    config.setValue("pttType", radioData->pttType);
    config.setValue("pttSerialPort", radioData->pttSerialPort);
    config.setValue("radioPollInterval", radioData->pollInterval);
    config.setValue("rigCtldEnable", radioData->rigCtldEnable);
    config.setValue("startMinosRigCtld", radioData->startMinosRigCtld);
    config.setValue("rigCtldNetworkAddress", radioData->rigCtldNetworkAdd);
    config.setValue("rigCtldPortNumber", radioData->rigCtldNetworkPort);
    config.setValue("transVertEnable", radioData->transVertEnable);
    config.setValue("netAddress", radioData->networkAdd);
    config.setValue("netPort", radioData->networkPort);
    config.setValue("mgmMode", radioData->mgmMode);
    config.setValue("enableShowCatFeatures", radioData->enableDisableCatFeature.enableDisplay);
    config.setValue("ritEnable", radioData->enableDisableCatFeature.ritEnable);
    config.setValue("sMeterEnable", radioData->enableDisableCatFeature.sMeterEnable);
    config.setValue("volumeEnable", radioData->enableDisableCatFeature.volumeEnable);
    config.setValue("voiceMemEnable", radioData->enableDisableCatFeature.voiceMemEnable);
    config.setValue("cWMemEnable", radioData->enableDisableCatFeature.cWMemEnable);
    config.setValue("catEnable", radioData->enableDisableCatFeature.catEnable);

    foreach (auto &b, bands)
    {
        if (hfFlag)
        {
            QString name = b.data()->name();
            name.remove('\x20').replace('H', 'h').replace('.', '_');
            config.setValue("support" + name, radioData->supportBands.getSupportBandFlag(b.data()->name()));
        }
        else
        {
            if (b.data()->getType() != HF_BANDTYPE)
            {
                QString name = b.data()->name();
                name.remove('\x20').replace('H', 'h').replace('.', '_');
                config.setValue("support" + name, radioData->supportBands.getSupportBandFlag(b.data()->name()));
            }
        }

    }
    config.setValue("enableTransVertSw", radioData->enableTransSwitch);
    config.setValue("locTransSwEnable", radioData->enableLocTVSwMsg);
    config.setValue("locTransVertSwComport", radioData->locTVSwComport);
    config.endGroup();



}



void RigSetupDialog::getRadioSetting(QSharedPointer<scatParams> radioData, QString radioName, QSettings& config)
{
    config.beginGroup(radioName);
    radioData->radioName = config.value("radioName", "").toString();
    radioData->rigModel = config.value("radioModel", "").toString();
    radioData->civAddress = config.value("civAddress", "").toString();
    radioData->portType = config.value("portType", RigCapConstants::PortType::serial).toInt();
    radioData->advancedCommsFlag = config.value("advancedComms", false).toBool();
    radioData->comport = config.value("comport", "").toString();
    radioData->baudrate = config.value("baudrate", 9600).toInt();
    radioData->databits = config.value("databits", 8).toInt();
    radioData->parity = config.value("parity", 0).toInt();
    radioData->stopbits = config.value("stopbits", 1).toInt();
    radioData->handshake = config.value("handshake", 0).toInt();
    radioData->forceDtr = config.value("forceDTR", 0).toInt();
    radioData->forceRts= config.value("forceRTS", 0).toInt();
    radioData->enablePTT = config.value("enablePtt", false).toBool();
    radioData->pttType = config.value("pttType", serialCommonData::PTTMethodCodes::PTT_METHOD_CAT).toInt();
    radioData->pttSerialPort = config.value("pttSerialPort", "").toString();
    radioData->pollInterval = config.value("radioPollInterval", "1").toString();
    radioData->rigCtldEnable = config.value("rigCtldEnable", false).toBool();
    radioData->startMinosRigCtld = config.value("startMinosRigCtld", true).toBool();
    radioData->rigCtldNetworkAdd = config.value("rigCtldNetworkAddress", "").toString();
    radioData->rigCtldNetworkPort = config.value("rigCtldPortNumber", "").toString();
    radioData->transVertEnable = config.value("transVertEnable", false).toBool();
    radioData->antSwitchAvail = config.value("antSwitchAvail", false).toBool();
    radioData->networkAdd = config.value("netAddress", "").toString();
    radioData->networkPort = config.value("netPort", "").toString();
    radioData->mgmMode = config.value("mgmMode", hamlibData::USB).toString();
    radioData->enableDisableCatFeature.enableDisplay = config.value("enableShowCatFeatures", false).toBool();
    radioData->enableDisableCatFeature.ritEnable = config.value("ritEnable", false).toBool();
    radioData->enableDisableCatFeature.sMeterEnable = config.value("sMeterEnable", true).toBool();
    radioData->enableDisableCatFeature.volumeEnable = config.value("volumeEnable", true).toBool();
    radioData->enableDisableCatFeature.voiceMemEnable = config.value("voiceMemEnable", true).toBool();
    radioData->enableDisableCatFeature.cWMemEnable = config.value("cWMemEnable", true).toBool();
    radioData->enableDisableCatFeature.catEnable = config.value("catEnable", true).toBool();


    foreach (auto &b, bands)
    {
        if (hfFlag)
        {
            QString name = b.data()->uk;
            name.remove('\x20').replace('H', 'h').replace('.', '_');
            radioData->supportBands.setSupportBandFlag(b.data()->name(), config.value("support" + name, false).toBool());
        }
        else
        {
            if (b.data()->getType() != HF_BANDTYPE)
            {
                QString name = b.data()->uk;
                name.remove('\x20').replace('H', 'h').replace('.', '_');
                radioData->supportBands.setSupportBandFlag(b.data()->name(), config.value("support" + name, false).toBool());
            }
        }


    }
    radioData->enableTransSwitch = config.value("enableTransVertSw", false).toBool();
    radioData->enableLocTVSwMsg = config.value("locTransSwEnable", false).toBool();
    radioData->locTVSwComport = config.value("locTransVertSwComport", "").toString();
    config.endGroup();

    // now read transverter settings
    QString fileNameTransVert;
    fileNameTransVert = TRANSVERT_PATH_LOGGER + radioData->radioName + FILENAME_TRANSVERT_RADIOS;
    QSettings  configTransVert(fileNameTransVert, QSettings::IniFormat);

    QStringList tvList = configTransVert.childGroups();
    foreach (const auto &t, tvList)
    {
        readTranVerterSetting(radioData, t, configTransVert);
    }

    radioData->rigMfg_Name = rigFactory->supported_rigs()->value(radioData->rigModel).rigManufacturer;
    radioData->rigModelName = rigFactory->supported_rigs()->value(radioData->rigModel).rigModelName;
    radioData->rigModelNumber = rigFactory->supported_rigs()->value(radioData->rigModel).rigModelNumber;

}



void RigSetupDialog::saveTranVerterSetting(QSharedPointer<scatParams> radioData, QString transvertName, QSettings  &config)
{
    config.beginGroup(transvertName);
    config.setValue("name", radioData->transVertSettings.value(transvertName)->transVertName);
    config.setValue("band", radioData->transVertSettings.value(transvertName)->band);
    config.setValue("radioFreq", radioData->transVertSettings.value(transvertName)->radioFreq.str());
    config.setValue("targetFreq", radioData->transVertSettings.value(transvertName)->targetFreq.str());
    config.setValue("offsetDouble", radioData->transVertSettings.value(transvertName)->transVertOffset.str());
    config.setValue("antSwNumber", radioData->transVertSettings.value(transvertName)->antSwitchNum);
    config.setValue("transVertSw", radioData->transVertSettings.value(transvertName)->transSwitchNum);
    config.endGroup();
}


void RigSetupDialog::readTranVerterSetting(QSharedPointer<scatParams> radioData, QString transvertName, QSettings  &config)
{
    config.beginGroup(transvertName);
    QSharedPointer<TransVertParams> tvp = QSharedPointer<TransVertParams>(new TransVertParams());
    tvp->transVertName = config.value("name", "").toString();
    tvp->band = config.value("band", "").toString();
    tvp->radioFreq = Frequency(config.value("radioFreq", 0).toString());
    tvp->targetFreq = Frequency(config.value("targetFreq", 0).toString());
    tvp->transVertOffset = Frequency(config.value("offsetDouble", 0).toString());
    tvp->antSwitchNum = config.value("antSwNumber", "0").toString();
    tvp->transSwitchNum = config.value("transVertSw", "0").toString();
    radioData->transVertSettings.insert(transvertName, tvp);
    config.endGroup();
}



void RigSetupDialog::updateAvailRadiosToVersion2(QSettings& settings)
{
    QString radioModel;
    QStringList spList;
    QString radio;


    for (int i = 0; i < numAvailRadios; i++)
    {
        radio = availRadios[i] + "/radioModel";
        radioModel = settings.value(radio, QString()).toString();
        if (radioModel.contains(','))
        {
            spList = radioModel.split(',');
            if (spList.count() == 3)
            {
                radioModel = spList[1].trimmed() + " " + spList[2].trimmed();
                settings.setValue(radio, radioModel);

            }

        }

        // remove redundant settings
        radio = availRadios[i];
        settings.remove(radio + "/radioMfgName");
        settings.remove(radio + "/radioModelName");
        settings.remove(radio + "/radioModelName");
        settings.remove(radio + "/radioModelNumber");


    }

    settings.setValue("Version/version", "2");
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
    QStringList lk = availRadioData.keys();
    foreach (const auto &k, lk)
    {
        if (availRadioData.value(k)->radioName == radioName)
        {
            return availRadioData.value(k)->comport;
        }
    }

    return ""; // error, none found

}





void RigSetupDialog::setCurrentRadioName(QString name)
{
    currentRadioName = name;
    // set currentRadioName in radio tab to test remove transverter
    QStringList lk = radioTab.keys();
    foreach (const auto &k, lk)
    {
        radioTab.value(k)->setCurrentRadioName(name);
    }
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






