/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#include "rigsetupform.h"
#include "BandList.h"
#include "addtransverterdialog.h"
#include "rigutils.h"
#include <QHostInfo>
#include <QDebug>
#include <QLineEdit>
#include <QCheckBox>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QHostAddress>
#include <QInputDialog>


//static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

RigSetupForm::RigSetupForm(RigFactory* rigFactory_, scatParams* _radioData,
                           const QVector<QSharedPointer<BandInfo> > &_bands, QLogTabWidget* _ui_RadioTab,
                           bool hfFlag_, QWidget *parent):
    QWidget(parent),
    ui(new Ui::rigSetupForm),
    hfFlag(hfFlag_),
    transverterRemoved(false)

{

    ui->setupUi(this);

    rigFactory = rigFactory_;

    radioData = _radioData;

    bands = _bands;

    ui_RadioTab = _ui_RadioTab;


    fillRadioModelInfo();  // add radio models to drop down
    //fillPortsInfo(ui->comPortBox);     // add comports to drop down
    fillPortsInfo(ui->locTVComPortSel);
    fillSpeedInfo();
    fillDataBitsInfo();
    fillStopBitsInfo();
    fillParityInfo();
    fillHandShakeInfo();
    fillForceLinesInfo();
    fillPollInterValInfo();
    fillMgmModes();
    civSetToolTip();

    ui->useRigCtldChkBox->setCheckState(Qt::CheckState::Unchecked);
    rigCtldItemsVisible(false);

    connect(ui->radioModelBox, SIGNAL(activated(int)), this, SLOT(radioModelSelected()));
    connect(ui->comPortBox, SIGNAL(activated(int)), this, SLOT(comportSelected()));
    connect(ui->comSpeedBox, SIGNAL(activated(int)), this, SLOT(comSpeedSelected()));
    connect(ui->comDataBitsBox, SIGNAL(activated(int)), this, SLOT(comDataBitsSelected()));
    connect(ui->comStopBitsBox, SIGNAL(activated(int)), this, SLOT(comStopBitsSelected()));
    connect(ui->comParityBox, SIGNAL(activated(int)), this, SLOT(comParitySelected()));
    connect(ui->comHandShakeBox, SIGNAL(activated(int)), this, SLOT(comHandShakeSelected()));
    connect(ui->forceDtrBox, SIGNAL(activated(int)), this, SLOT(on_forceDTRSelected()));
    connect(ui->forceRtsBox, SIGNAL(activated(int)), this, SLOT(on_forceRTSSelected()));
    connect(ui->advancedCommsChkBox, SIGNAL(clicked(bool)),this, SLOT(onAdvancedCommsSelected(bool)));
    connect(ui->networkAddBox, SIGNAL(editingFinished()), this, SLOT(networkAddressSelected()));
    connect(ui->netPortBox, SIGNAL(editingFinished()), this, SLOT(networkPortSelected()));
    connect(ui->pollInterval, SIGNAL(activated(int)), this, SLOT(pollIntervalSelected()));
    connect(ui->enableTransVert, SIGNAL(clicked(bool)), this, SLOT(enableTransVertSelected(bool)));
    connect(ui->mgmBox, SIGNAL(activated(int)), this, SLOT(mgmModeSelected()));
    connect(ui->CIVlineEdit, SIGNAL(editingFinished()), this, SLOT(civAddressFinished()));

    connect(ui->useRigCtldChkBox, SIGNAL(clicked(bool)), this, SLOT(useRigCtldSelected(bool)));
    connect(ui->startMinosRigCtldChkBox, SIGNAL(clicked(bool)), this, SLOT(onStartMinosRigCtldChkBox(bool)));
    connect(ui->rigCtldNetworkAddBox, SIGNAL(editingFinished()), this, SLOT(rigCtldNetworkAddressSelected()));
    connect(ui->rigCtldNetPortBox, SIGNAL(editingFinished()), this, SLOT(rigCtldNetworkPortSelected()));

    initSupBandsChkBoxs();


    // transvert
    connect(ui->enableTransVertSw, SIGNAL(clicked(bool)), this, SLOT(enableTransVertSwSel(bool)));
    connect(ui->locTvConChk, SIGNAL(clicked(bool)), this, SLOT(localTransVertSwSel(bool)));
    connect(ui->locTVComPortSel, SIGNAL(activated(int)), this, SLOT(locTVComPortSel(int)));


    connect(ui->addTransvert, SIGNAL(clicked(bool)), this, SLOT(addTransVerter()));
    connect(ui->removeTransvert, SIGNAL(clicked(bool)), this, SLOT(removeTransVerter()));
    connect(ui->changeBand, SIGNAL(clicked(bool)), this, SLOT(changeBand()));

}


RigSetupForm::~RigSetupForm()
{
    delete ui;
}





scatParams* RigSetupForm::getRadioData()
{
    return radioData;
}


void RigSetupForm::setCurrentRadioName(QString name)
{
    currentRadioName = name;
}


/************************ Radio Model Dialogue *********************/


void RigSetupForm::radioModelSelected()
{

    setupRadioModel(ui->radioModelBox->currentText());



}


void RigSetupForm::setupRadioModel(QString radioModel)
{
// need to do something is selection is empty!!!!!!!!!!!!!!!!!!!!!!!!



    if (radioModel != radioData->rigModel)
    {
        radioData->rigModel = radioModel;
        ui->radioModelBox->setCurrentText(radioModel);

        RigCapabilities rigCap = rigFactory->supported_rigs()->value(radioData->rigModel);


        radioData->rigModelNumber = rigFactory->supported_rigs()->value(radioData->rigModel).rigModelNumber;
        radioData->rigModelName = rigFactory->supported_rigs()->value(radioData->rigModel).rigModelName;
        radioData->rigMfg_Name = rigFactory->supported_rigs()->value(radioData->rigModel).rigManufacturer;
        radioData->portType = rigFactory->supported_rigs()->value(radioData->rigModel).portType;


        if (rigCap.pollData)
        {
            pollIntervalVisible(true);
        }
        else
        {
            pollIntervalVisible(false);
        }


        if (rigCap.rigManufacturer == "Icom")
        {
            CIVEditVisible(true);
        }
        else
        {
            CIVEditVisible(false);
        }




        if (rigCap.portType == RigCapConstants::PortType::network)
        {
               serialDataEntryVisible(false);
               advancedSerialDataEntryVisible(false);
               setAdvancedCommsChkBoxVisible(false);
               networkDataEntryVisible(true);
               rigCtldItemsVisible(false);
               setRigctldCheckBoxVisible(false);
               setStartMinosRigctldCheckbox(true);


        }
        else if (rigCap.portType == RigCapConstants::PortType::serial)
        {
                serialDataEntryVisible(true);
                advancedSerialDataEntryVisible(radioData->advancedCommsFlag);
                checkAdvancedCommsCheckBox(radioData->advancedCommsFlag);
                setAdvancedCommsChkBoxVisible(true);
                networkDataEntryVisible(false);

        }
        else // RIG_PORT_NONE
        {
                serialDataEntryVisible(false);
                advancedSerialDataEntryVisible(false);
                setAdvancedCommsChkBoxVisible(false);
                networkDataEntryVisible(false);
                setRigctldCheckBoxVisible(false);
                radioData->rigCtldEnable = false;
        }

        if (rigCap.supportGetSupBands)
        {
            setSupportBandCheckBoxVisible(false);
        }
        else
        {
            setSupportBandCheckBoxVisible(true);
        }


        // does this radio support antenna sw?

        for (int i = 0; i < radioData->numTransverters; i++)
        {
            if (rigCap.supportAntSw)
            {
               //transVertTab[i]->antSwNumVisible(true);
               radioData->antSwitchAvail = true;
            }
            else
            {
               //transVertTab[i]->antSwNumVisible(false);
               radioData->antSwitchAvail = false;
            }
        }


        setTransVertSelected(radioData->transVertEnable);

        // display the correct transverter settings

         setLocTVSwComport(radioData->locTVSwComport);

         setTransVertSelected(radioData->transVertEnable);
         if (radioData->transVertEnable)
         {
             setTransVertSwVisible(true);
             setEnableLocalTransVertSwVisible(false);
         }
         else
         {
             setTransVertSwVisible(false);
             setEnableLocalTransVertSwVisible(false);
             setLocTVSWComportVisible(false);
         }

         if (radioData->transVertEnable && radioData->enableTransSwitch)
         {
             setTransVertSwVisible(true);
             setEnableLocalTransVertSwVisible(true);
             for (int i = 0; i < radioData->numTransverters; i++)
             {
                 transVertTab[i]->setEnableTransVertSwBoxVisible(true);
             }
         }

         if (radioData->transVertEnable && radioData->enableTransSwitch && radioData->enableLocTVSwMsg)
         {
             setLocTVSWComportVisible(true);
         }
         else
         {
             setLocTVSWComportVisible(false);
         }

        //buildSupBandList();
        radioValueChanged = true;
    }


}

QString RigSetupForm::getRadioModel()
{
    return ui->radioModelBox->currentText();
}


void RigSetupForm::setRadioModel(QString m)
{
    ui->radioModelBox->setCurrentIndex(ui->radioModelBox->findText(m));

    RigCapabilities rigCap = rigFactory->supported_rigs()->value(ui->radioModelBox->currentText());

    if (rigCap.rigManufacturer == "Icom")
    {
        CIVEditVisible(true);
    }
    else
    {
        CIVEditVisible(false);
    }


}




/********************** CIV Entry ***********************/


void RigSetupForm::civAddressFinished()
{

    bool Ok;

        QString civNum = ui->CIVlineEdit->text().trimmed();


        if (civNum.isEmpty())
        {
           return;
        }

        if (!civNum.contains("0x", Qt::CaseInsensitive))
        {

            civNum.prepend("0x");
             ui->CIVlineEdit->setText(civNum);

        }

        int hexValue = civNum.toInt(&Ok, 16);
        if (Ok &&  (hexValue < 0 || hexValue > 255))
        {
            QMessageBox::critical(this,tr( "CIV Error"), QString(ui->CIVlineEdit->text()) + tr(" CIV number out of range 0 - FF"));
            //ui->CIVlineEdit->setText("");
            return;
        }
        else
        {
            if (civNum != radioData->civAddress)
            {
                radioData->civAddress = civNum;

                radioValueChanged = true;

            }
        }

}









void RigSetupForm::civSetToolTip()
{
        const char * civToolTip = "Leave field blank for default radio CIV,\n"
                            "or enter in the form 0xnn or nn, where nn is the radio CIV address in Hex.";


        ui->CIVlineEdit->setToolTip(tr(civToolTip));

}

QString RigSetupForm::getCIVAddress()
{
    return ui->CIVlineEdit->text();
}

void RigSetupForm::setCIVAddress(QString v)
{
    ui->CIVlineEdit->setText(v);
}


void RigSetupForm::enableCIVEdit(bool enable)
{
    ui->CIVlineEdit->setEnabled(enable);
}


void RigSetupForm::CIVEditVisible(bool visible)
{
    ui->CIVLabel->setVisible(visible);
    ui->CIVlineEdit->setVisible(visible);
}


/***************** Comports ****************************/



void RigSetupForm::comportSelected()
{

    if (ui->comPortBox->currentText() != radioData->comport)
    {
        radioData->comport = ui->comPortBox->currentText();
        radioValueChanged = true;
    }
}

QString RigSetupForm::getComport()
{
    return ui->comPortBox->currentText();
}

void RigSetupForm::setComport(QString p)
{
    ui->comPortBox->setCurrentIndex(ui->comPortBox->findText(p));
}


void RigSetupForm::loadRadioComports()
{
    fillPortsInfo(ui->comPortBox);
}

/***************************** Data Speed *************************/


void RigSetupForm::comSpeedSelected()
{
    if (ui->comSpeedBox->currentText().toInt() != radioData->baudrate)
    {
        radioData->baudrate = ui->comSpeedBox->currentText().toInt();
        radioValueChanged = true;
    }
}


QString RigSetupForm::getDataSpeed()
{
    return ui->comSpeedBox->currentText();
}

void RigSetupForm::setDataSpeed(QString d)
{
    ui->comSpeedBox->setCurrentIndex(ui->comSpeedBox->findText(d));
}

/***************************** Data Bits *************************/

void RigSetupForm::comDataBitsSelected()
{
    if (ui->comDataBitsBox->currentText().toInt() != radioData->databits)
    {
        radioData->databits = ui->comDataBitsBox->currentText().toInt();
        radioValueChanged = true;
    }
}

QString RigSetupForm::getDataBits()
{
    return ui->comDataBitsBox->currentText();
}


void RigSetupForm::setDataBits(QString d)
{
    ui->comDataBitsBox->setCurrentIndex(ui->comDataBitsBox->findText(d));
}


/***************************** Stop Bits *************************/

void RigSetupForm::comStopBitsSelected()
{
    if (ui->comStopBitsBox->currentText().toInt() != radioData->stopbits)
    {
        radioData->stopbits = ui->comStopBitsBox->currentText().toInt();
        radioValueChanged = true;
    }
}

QString RigSetupForm::getStopBits()
{
    return ui->comStopBitsBox->currentText();
}

void RigSetupForm::setStopBits(QString stop)
{
    ui->comStopBitsBox->setCurrentIndex(ui->comStopBitsBox->findText(stop));
}

/***************************** Parity Bits *************************/


void RigSetupForm::comParitySelected()
{
    if (serialCommonData::parityCodesList[ui->comParityBox->currentIndex()] != radioData->parity)
    {
        radioData->parity = serialCommonData::parityCodesList[ui->comParityBox->currentIndex()];
        radioValueChanged = true;
    }
}

void RigSetupForm::setParityBits(int b)
{
    ui->comParityBox->setCurrentIndex(b);
}

/***************************** Handshake *************************/

void RigSetupForm::comHandShakeSelected()
{
    if (serialCommonData::handshakeCodesList[ui->comHandShakeBox->currentIndex()] != radioData->handshake)
    {
        radioData->handshake = serialCommonData::handshakeCodesList[ui->comHandShakeBox->currentIndex()];
        if (radioData->handshake == serialCommonData::HANDSHAKE_HARDWARE)  // RTS/CTS selected
        {
           setForceRTSDisabled(true);

        }
        else
        {
           setForceRTSDisabled(false);

        }
        radioValueChanged = true;
    }
}


QString RigSetupForm::getHandshake()
{
    return ui->comHandShakeBox->currentText();
}

void RigSetupForm::setHandshake(int h)
{
    ui->comHandShakeBox->setCurrentIndex(h);
}

// returns -1 if no text found
int RigSetupForm::comportAvial(QString comport)
{
    return ui->comPortBox->findText(comport);

}

/**************** Force DTR Line *************************************/

void RigSetupForm::on_forceDTRSelected()
{
    if (serialCommonData::forceLinesCodesList[ui->forceDtrBox->currentIndex()] != radioData->forceDtr)
    {
        radioData->forceDtr = serialCommonData::forceLinesCodesList[ui->forceDtrBox->currentIndex()];
        radioValueChanged = true;
    }
}

void RigSetupForm::setForceDTR(int n)
{
    ui->forceDtrBox->setCurrentIndex(n);
}
/**************** Force RTS Line *************************************/

void RigSetupForm::on_forceRTSSelected()
{
    if (serialCommonData::forceLinesCodesList[ui->forceRtsBox->currentIndex()] != radioData->forceRts)
    {
        radioData->forceRts = serialCommonData::forceLinesCodesList[ui->forceRtsBox->currentIndex()];
        radioValueChanged = true;
    }
}

void RigSetupForm::setForceRTSDisabled(bool state)
{
    ui->forceRtsBox->setDisabled(state);
    ui->forceRtsLbl->setDisabled(state);
}

void RigSetupForm::setForceRTS(int n)
{
    ui->forceRtsBox->setCurrentIndex(n);
}



/***************************** Network Address *************************/

void RigSetupForm::networkAddressSelected()
{

    QString savedAddress = radioData->networkAdd;
    bool addressChanged = false;

    bool addressOk = processNetAddress(ui->networkAddBox, savedAddress, addressChanged);

    if (addressChanged)
    {
        if (addressOk)
        {
            radioData->networkAdd = ui->networkAddBox->text().trimmed();
            radioValueChanged = true;
        }
        else
        {
            QMessageBox messageBox;
            QString msg = tr("Invalid Network Address %1").arg( ui->networkAddBox->text());
            messageBox.critical(this, tr("Network Address Entry Error"), msg);
            ui->networkAddBox->setFocus();
        }
    }


}

QString RigSetupForm::getNetAddress()
{
    return ui->networkAddBox->text();
}

void RigSetupForm::setNetAddress(QString netAdd)
{
    ui->networkAddBox->setText(netAdd);
}
/***************************** Network Port *************************/


void RigSetupForm::networkPortSelected()
{

    processPortNumber(ui->networkAddBox, ui->netPortBox, radioData->networkPort);


}

QString RigSetupForm::getNetPortNum()
{
    return ui->netPortBox->text();
}

void RigSetupForm::setNetPortNum(QString p)
{
    ui->netPortBox->setText(p);
}

/************************* MGM Mode *********************************/



void RigSetupForm::mgmModeSelected()
{
    if (ui->mgmBox->currentText() != radioData->mgmMode)
    {
        radioData->mgmMode = ui->mgmBox->currentText();
        radioValueChanged = true;
    }
}

QString RigSetupForm::getMgmMode()
{
    return ui->mgmBox->currentText();
}

void RigSetupForm::setMgmMode(QString p)
{
    ui->mgmBox->setCurrentText(p);
}

/***************************** Poll Interval *************************/

void RigSetupForm::pollIntervalSelected()
{
    if (radioData->pollInterval != ui->pollInterval->currentText())
    {
        radioData->pollInterval = ui->pollInterval->currentText();
        radioValueChanged = true;

    }
}

QString RigSetupForm:: getPollInterval()
{
    return ui->pollInterval->currentText();
}

void RigSetupForm::setPollInterval(QString i)
{
    ui->pollInterval->setCurrentIndex(ui->pollInterval->findText(i));
}

void RigSetupForm::pollIntervalVisible(bool s)
{
    ui->pollInterval->setVisible(s);
    ui->pollIntervalLbl->setVisible(s);
}




/************************** TransVert Enable *************************/


void RigSetupForm::enableTransVertSelected(bool /*flag*/)
{
    bool checked = ui->enableTransVert->isChecked();
    if(radioData->transVertEnable != checked)
    {
        radioData->transVertEnable = checked;
        transVertTabEnable(checked);

        setTransVertSwVisible(checked);
        if (!checked)
        {
            radioData->enableTransSwitch = checked;
            radioData->enableLocTVSwMsg = checked;
            setEnableTransVertSw(checked);
            setEnableLocalTransVertSwVisible(checked);
            setEnableLocalTransVertSw(checked);
            setLocTVSWComportVisible(checked);
        }
        radioValueChanged = true;
    }


}

bool RigSetupForm::getTransVertSelected()
{
    return ui->enableTransVert->isChecked();
}

void RigSetupForm::setTransVertSelected(bool flag)
{
    ui->enableTransVert->setChecked(flag);
    transVertTabEnable(flag);

}


/********************* TransVert Switch Enable  *********************************/

void RigSetupForm::enableTransVertSwSel(bool /*flag*/)
{
    bool checked = ui->enableTransVertSw->isChecked();
    if (radioData->enableTransSwitch != checked)
    {
        radioData->enableTransSwitch = checked;
        for (int i = 0; i < radioData->numTransverters; i++)
        {
            transVertTab[i]->setEnableTransVertSwBoxVisible(checked);
        }
        setEnableLocalTransVertSwVisible(checked);
        if (!checked)
        {
            radioData->enableLocTVSwMsg = checked;
            setEnableLocalTransVertSw(checked);
            setLocTVSWComportVisible(checked);

        }
        //setLocTVSWComportVisible(false);
        radioValueChanged = true;
    }

}


bool RigSetupForm::getEnableTransVertSw()
{
    return ui->enableTransVertSw->isChecked();
}

void RigSetupForm::setEnableTransVertSw(bool b)
{
    ui->enableTransVertSw->setChecked(b);
    for (int i = 0; i < radioData->numTransverters; i++)
    {
        transVertTab[i]->setEnableTransVertSwBoxVisible(b);
    }

}


void RigSetupForm::setTransVertSwVisible(bool b)
{
    ui->enableTransVertSw->setVisible(b);
}


/**************** Local Transvert Switch Control *****************************/


void RigSetupForm::localTransVertSwSel(bool /*flag*/)
{

    bool checked = ui->locTvConChk->isChecked();
    if (radioData->enableLocTVSwMsg != checked)
    {
        radioData->enableLocTVSwMsg = checked;
        setLocTVSWComportVisible(checked);
        radioValueChanged = true;
    }

}

bool RigSetupForm::getEnableLocalTransVertSw()
{
    return ui->locTvConChk->isChecked();
}


void RigSetupForm::setEnableLocalTransVertSw(bool b)
{
    ui->locTvConChk->setChecked(b);

}


void RigSetupForm::setEnableLocalTransVertSwVisible(bool visible)
{
     ui->locTvConChk->setVisible(visible);


}



/**************** Local Transvert Switch Comport **************************/



void RigSetupForm::locTVComPortSel(int /*index*/)
{

    if (ui->locTVComPortSel->currentText() != radioData->locTVSwComport)
    {
        radioData->locTVSwComport = ui->locTVComPortSel->currentText();
        radioValueChanged = true;
    }
}

QString RigSetupForm::getLocTVSwComport()
{
    return ui->locTVComPortSel->currentText();
}

void RigSetupForm::setLocTVSwComport(QString p)
{
    ui->locTVComPortSel->setCurrentIndex(ui->locTVComPortSel->findText(p));
}




void RigSetupForm::setLocTVSWComportVisible(bool visible)
{
     ui->locTVComPortSel->setVisible(visible);
     ui->locComportSwLbl->setVisible(visible);
     //setEnableLocalTransVertSwVisible(visible);

}

/************** RigCtld Setup ****************************************/

void RigSetupForm::rigCtldItemsVisible(bool visible)
{
    rigCtldNetworkAddBoxVisible(visible);
    rigCtldPortBoxVisible(visible);
    ui->startMinosRigCtldChkBox->setVisible(visible);
}

void RigSetupForm::rigCtldNetworkAddBoxVisible(bool visible)
{
    ui->rigCtldNetworkAddLbl->setVisible(visible);
    ui->rigCtldNetworkAddBox->setVisible(visible);
}

void RigSetupForm::rigCtldPortBoxVisible(bool visible)
{
    ui->rigCtldNetPortLbl->setVisible(visible);
    ui->rigCtldNetPortBox->setVisible(visible);
}

void RigSetupForm::useRigCtldSelected(bool /*selected*/)
{
    bool checked = ui->useRigCtldChkBox->isChecked();
    if(radioData->rigCtldEnable != checked)
    {
        radioData->rigCtldEnable = checked;
        setStartMinosRigctldCheckbox(true);
        radioData->startMinosRigCtld = true;

        rigCtldItemsVisible(checked);
    }

    radioValueChanged = true;
}




void RigSetupForm::setUseRigctldCheckbox(bool checked)
{
    if (checked)
    {

        ui->useRigCtldChkBox->setCheckState(Qt::CheckState::Checked);
    }
    else
    {
       ui->useRigCtldChkBox->setCheckState(Qt::CheckState::Unchecked);
    }
}

void RigSetupForm::onStartMinosRigCtldChkBox(bool /*selected*/)
{
    bool checked = ui->startMinosRigCtldChkBox->isChecked();
    if(radioData->startMinosRigCtld != checked)
    {
        radioData->startMinosRigCtld = checked;
        setStartMinosRigctldCheckbox(checked);
        radioData->startMinosRigCtld = checked;
        //rigCtldItemsVisible(checked);
    }

    radioValueChanged = true;
}


void RigSetupForm::setStartMinosRigctldCheckbox(bool checked)
{
    if (checked)
    {

        ui->startMinosRigCtldChkBox->setCheckState(Qt::CheckState::Checked);
    }
    else
    {
       ui->startMinosRigCtldChkBox->setCheckState(Qt::CheckState::Unchecked);
    }
}

void RigSetupForm::rigCtldNetworkAddressSelected()
{

    QString savedAddress = radioData->rigCtldNetworkAdd;
    bool addressChanged = false;

    bool addressOk = processNetAddress(ui->rigCtldNetworkAddBox, savedAddress, addressChanged);

    if (addressChanged)
    {
        if (addressOk)
        {
            radioData->rigCtldNetworkAdd = ui->networkAddBox->text().trimmed();
            radioValueChanged = true;
        }
        else
        {
            QMessageBox messageBox;
            QString msg = "Invalid Network Address " + ui->rigCtldNetworkAddBox->text();
            messageBox.critical(this, "Network Address Entry Error", msg);
            ui->rigCtldNetworkAddBox->setFocus();
        }
    }
}

QString RigSetupForm::getRigctldNetworkAddress()
{
    return ui->rigCtldNetworkAddBox->text();
}

void RigSetupForm::setRigctldNetworkAddress(const QString& address)
{
    ui->rigCtldNetworkAddBox->setText(address);
}

void RigSetupForm::rigCtldNetworkPortSelected()
{
    processPortNumber(ui->rigCtldNetworkAddBox, ui->rigCtldNetPortBox, radioData->rigCtldNetworkPort);
}


QString RigSetupForm::getRigctldPortNumber()
{
    return ui->rigCtldNetPortBox->text();
}

void RigSetupForm::setRigctldPortNumber(const QString& port)
{
    ui->rigCtldNetPortBox->setText(port);
}


void RigSetupForm::setRigctldCheckBoxVisible(bool visible)
{
    ui->useRigCtldChkBox->setVisible(visible);
}

/************** RigSetup Enable **************************************/


void RigSetupForm::setEnableRigDataEntry(bool enable)
{
    ui->CIVlineEdit->setEnabled(enable);
    ui->comPortBox->setEnabled(enable);
    ui->comDataBitsBox->setEnabled(enable);
    ui->comSpeedBox->setEnabled(enable);
    ui->comDataBitsBox->setEnabled(enable);
    ui->comHandShakeBox->setEnabled(enable);
    ui->comParityBox->setEnabled(enable);
    ui->netPortBox->setEnabled(enable);
    ui->networkAddBox->setEnabled(enable);
    ui->forceDtrBox->setEnabled(enable);
    ui->forceRtsBox->setEnabled(enable);
    ui->pollInterval->setEnabled(enable);
    ui->mgmBox->setEnabled(enable);
    ui->enableTransVert->setEnabled(enable);

}



/*************************** Serial Data Entry Visible ***************/

void RigSetupForm::serialDataEntryVisible(bool v)
{
    ui->comPortBox->setVisible(v);
    ui->comportLbl->setVisible(v);
    ui->comSpeedBox->setVisible(v);
    ui->speedLbl->setVisible(v);

    ui->advancedCommsChkBox->setVisible(v);
}




/********************* Advanced Comms CheckBox *******************/


void RigSetupForm::onAdvancedCommsSelected(bool selected)
{
    Q_UNUSED(selected)
    bool checked = ui->advancedCommsChkBox->isChecked();
    if (radioData->advancedCommsFlag != checked)
    {
        radioData->advancedCommsFlag = checked;
        advancedSerialDataEntryVisible(checked);
        radioValueChanged = true;

    }
}

void RigSetupForm::setAdvancedCommsFlag(bool state)
{
    radioData->advancedCommsFlag = state;
}

void RigSetupForm::setAdvancedCommsChkBoxVisible(bool visible)
{
    ui->advancedCommsChkBox->setVisible(visible);
}

void RigSetupForm::advancedSerialDataEntryVisible(bool v)
{
    ui->comDataBitsBox->setVisible(v);
    ui->dataLbll->setVisible(v);
    ui->comStopBitsBox->setVisible(v);
    ui->stopLbl->setVisible(v);
    ui->comParityBox->setVisible(v);
    ui->parityLbl->setVisible(v);
    ui->comHandShakeBox->setVisible(v);
    ui->handshakeLbl->setVisible(v);
    ui->forceDtrBox->setVisible(v);
    ui->forceDtrLbl->setVisible(v);
    ui->forceRtsBox->setVisible(v);
    ui->forceRtsLbl->setVisible(v);
}

void RigSetupForm::checkAdvancedCommsCheckBox(bool checked)
{
    ui->advancedCommsChkBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

/*************************** Network Data Entry Visible ***************/

void RigSetupForm::networkDataEntryVisible(bool v)
{
    ui->networkAddBox->setVisible(v);
    ui->networkAddLbl1->setVisible(v);
    ui->netPortBox->setVisible(v);
    ui->netPortLbl->setVisible(v);
}



void RigSetupForm::fillRadioModelInfo()
{

    rigFactory->populateComboRigList(ui->radioModelBox);
}

void RigSetupForm::fillPollInterValInfo()
{
    QStringList pollTimeStr;
    pollTimeStr << "0.5" << "1" << "2" << "3";

    ui->pollInterval->clear();
    ui->pollInterval->addItems(pollTimeStr);
}




void RigSetupForm::fillSpeedInfo()
{
    ui->comSpeedBox->clear();
    ui->comSpeedBox->addItems(serialCommonData::baudrateStr);
}

void RigSetupForm::fillDataBitsInfo()
{

    ui->comDataBitsBox->clear();
    ui->comDataBitsBox->addItems(serialCommonData::databitsStr);
}

void RigSetupForm::fillStopBitsInfo()
{
    ui->comStopBitsBox->clear();
    ui->comStopBitsBox->addItems(serialCommonData::stopbitsStr);
}



void RigSetupForm::fillParityInfo()
{

    ui->comParityBox->clear();
    ui->comParityBox->addItems(serialCommonData::parityStr);
}



void RigSetupForm::fillHandShakeInfo()
{

    ui->comHandShakeBox->clear();
    ui->comHandShakeBox->addItems(serialCommonData::handshakeStr);
}

void RigSetupForm::fillForceLinesInfo()
{
    ui->forceDtrBox->clear();
    ui->forceDtrBox->addItems(serialCommonData::forceLinesStr);
    ui->forceRtsBox->clear();
    ui->forceRtsBox->addItems(serialCommonData::forceLinesStr);
}

void RigSetupForm::fillMgmModes()
{
    QString fileName;

    fileName = RIG_CONFIGURATION_FILEPATH_LOGGER + MINOS_RADIO_CONFIG_FILE;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup("MGM_Modes");


    QStringList mgmModes = config.value("MgmModes", "").toStringList();

    config.endGroup();

     ui->mgmBox->clear();
     ui->mgmBox->addItems(mgmModes);
}



/****************** Support Bands Checkbox ***************************/


void RigSetupForm::initSupBandsChkBoxs()
{
    allSupBandsChkBoxList << ui->sup1_8MhzChkbox << ui->sup3_5MhzChkbox << ui->sup7MhzChkbox
                          << ui->sup14MhzChkbox << ui->sup21MhzChkbox << ui->sup28MhzChkbox
                          << ui->sup50MhzChkbox << ui->sup70MhzChkbox << ui->sup144MhzChkbox
                          << ui->sup432MhzChkbox << ui->sup1296MhzChkbox;

    for (int i = 0; i < allSupBandsChkBoxList.count(); i++)
    {
        connect(allSupBandsChkBoxList[i], &QCheckBox::stateChanged, [=](int state) {onSupbandCheckBoxStateChanged(i, state);});

    }

    hfSupBandsChkBoxList << ui->sup1_8MhzChkbox << ui->sup3_5MhzChkbox << ui->sup7MhzChkbox
                         << ui->sup14MhzChkbox << ui->sup21MhzChkbox << ui->sup28MhzChkbox;

    vhfSupBandsChkBoxList << ui->sup50MhzChkbox << ui->sup70MhzChkbox << ui->sup144MhzChkbox
                          << ui->sup432MhzChkbox << ui->sup1296MhzChkbox;

}


void RigSetupForm::onSupbandCheckBoxStateChanged(int i, int state)
{
    if (i < allSupBandsChkBoxList.count())
    {
        if (state == Qt::Checked)
        {
            radioData->supportBands.setSupportBandFlag(i, true);
        }
        else if (state == Qt::Unchecked)
        {
            radioData->supportBands.setSupportBandFlag(i, false);
        }

        radioValueChanged = true;

    }
}



void RigSetupForm::setSupportBandChkBox(int i, bool checked)
{
    if (checked)
    {
        allSupBandsChkBoxList[i]->setCheckState(Qt::Checked);
    }
    else
    {
        allSupBandsChkBoxList[i]->setCheckState(Qt::Unchecked);
    }


}




void RigSetupForm::setSupportBandCheckBoxVisible(bool visible)
{
    if (hfFlag)
    {
       for (auto &schk: allSupBandsChkBoxList)
       {
           schk->setVisible(visible);
       }
    }
    else
    {
        for (auto &schk: vhfSupBandsChkBoxList)
        {
            schk->setVisible(visible);
        }
    }

    ui->supportedBandGroupBox->setVisible(visible);
    ui->nativeBandLabel->setVisible(visible);
}

bool RigSetupForm::isAnySupportBandChecked()
{
    if (hfFlag)
    {
        for (auto &schk: allSupBandsChkBoxList)
        {
            if (schk->isChecked())
            {
                return true;
            }
        }
    }
    else
    {
        for (auto &schk:vhfSupBandsChkBoxList)
        {
            if (schk->isChecked())
            {
                return true;
            }
        }
    }

    return false;


}


/******************* Transverter ***********************************/


/********************* TabText *****************************************/


void RigSetupForm::setTransVertTabText(int tabNum, QString tabName)
{
    ui->transVertTab->setTabText(tabNum, tabName);
}







void RigSetupForm::addTransVerter()
{

    AddTransVerterDialog addTransDialog(bands, radioData->transVertNames, this);
    if (addTransDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QString transVerterName = addTransDialog.getTransVerterName();

    if (transVerterName == "")
    {
        return;
    }

    if (checkTransVerterNameMatch(transVerterName))
    {
        // error empty name or name already exists
        QMessageBox::information(this, tr("Transverter Name Exists"),
                                 tr("Transverter Name: %1, already exists \nPlease enter another name").arg(transVerterName.trimmed()),
                                  QMessageBox::Ok|QMessageBox::Default,
                                  QMessageBox::NoButton, QMessageBox::NoButton);
        return;
    }


    // add the new transverter
    int tabNum = radioData->numTransverters;
    radioData->transVertNames.append(transVerterName);
    addTransVertTab(tabNum, transVerterName, true);
    radioData->numTransverters = tabNum + 1;
    loadTransVertTab(tabNum);

}


void RigSetupForm::addTransVertTab(int tabNum, QString tabName, bool tabChanged)
{
    radioData->transVertSettings.append(new TransVertParams());
    radioData->transVertSettings[tabNum]->transVertName = tabName;
    radioData->transVertSettings[tabNum]->band = tabName;
    for (int i = 0; i < bands.count(); i++)
    {
         if (bands[i]->name() == tabName)
         {
             radioData->transVertSettings[tabNum]->fLow = bands[i]->fLow;
             radioData->transVertSettings[tabNum]->fHigh = bands[i]->fHigh;
             break;
         }
    }
    transVertTab.append(new TransVertSetupForm(radioData->transVertSettings[tabNum]));
    //addedTransVertTabs.append(tabName);

    ui->transVertTab->insertTab(tabNum, transVertTab[tabNum], tabName);
    ui->transVertTab->setTabColor(tabNum, Qt::darkBlue);      // radioTab promoted to QLogTabWidget
    ui->transVertTab->setCurrentIndex(tabNum);
    transVertTab[tabNum]->setEnableTransVertSwBoxVisible(false);

    // does this radio support antenna sw?

    if (rigFactory->supported_rigs()->value(radioData->rigModel).supportAntSw)
    {
       //transVertTab[tabNum]->antSwNumVisible(true);
       radioData->antSwitchAvail = true;
    }
    else
    {
       //transVertTab[tabNum]->antSwNumVisible(false);
       radioData->antSwitchAvail = false;
    }
    //buildSupBandList();
    transVertTab[tabNum]->transVertValueChanged = tabChanged;

}



void RigSetupForm::loadTransVertTab(int tabNum)
{
    transVertTab[tabNum]->setRadioFreqBox(radioData->transVertSettings[tabNum]->radioFreq);
    transVertTab[tabNum]->setTargetFreqBox(radioData->transVertSettings[tabNum]->targetFreq);
    transVertTab[tabNum]->setOffsetFreqLabel(radioData->transVertSettings[tabNum]->transVertOffset);
    transVertTab[tabNum]->setTransVerSwNum(radioData->transVertSettings[tabNum]->transSwitchNum);
    transVertTab[tabNum]->setEnableTransVertSwBoxVisible(radioData->enableTransSwitch);
}

bool RigSetupForm::checkTransVerterNameMatch(QString transVertName)
{
    for (int i = 0; i < radioData->numTransverters; i++)
    {
        if (ui->transVertTab->tabText(i) == transVertName)
            return true;
    }

    return false;
}


void RigSetupForm::removeTransVerter()
{

    if (transVertTab.count() < 1)
    {
        return;
    }

    int currentIndex = ui->transVertTab->currentIndex();
    QString currentTransVertName = ui->transVertTab->tabText(currentIndex);


    if (currentRadioName == ui_RadioTab->tabText(ui_RadioTab->currentIndex()))
    {
        // can't remove transverter on current RadioName
        QMessageBox msgBox;
        msgBox.setText(tr("You can not remove this transverter - %1, while it is the current radio - %2!").arg(currentTransVertName).arg(currentRadioName));
        msgBox.exec();
        return;
    }




    int status = QMessageBox::question( this,
                            tr("Remove Transverter"),
                            tr("Do you really want to remove transverter - %1?")
                            .arg(currentTransVertName),
                            QMessageBox::Yes|QMessageBox::Default,
                            QMessageBox::No|QMessageBox::Escape,
                            QMessageBox::NoButton);

    if (status != QMessageBox::Yes)
    {
        return;
    }


    // remove this transverter
    ui->transVertTab->removeTab(currentIndex);
    transVertTab.removeAt(currentIndex);

    radioData->transVertNames.removeAt(currentIndex);
    radioData->transVertSettings.removeAt(currentIndex);

    radioData->numTransverters--;
    transverterRemoved = true;


}


bool RigSetupForm::getTransVertRemovedFlag()
{
    return transverterRemoved;
}

void RigSetupForm::setTransVertRemovedFlag(bool value)
{
    transverterRemoved = value;
}

void RigSetupForm::changeBand()
{

    if (transVertTab.count() < 1)
    {
        return;
    }

    int tabNum = ui->transVertTab->currentIndex();
    QString currentTransVertName = ui->transVertTab->tabText(tabNum);


    if (currentRadioName == ui_RadioTab->tabText(ui_RadioTab->currentIndex()))
    {
        // can't change transverter band on current RadioName
        QMessageBox msgBox;
        msgBox.setText(tr("You can not change band on this transverter - %1, while it is the current radio - %2!").arg(currentTransVertName).arg(currentRadioName));
        msgBox.exec();
        return;
    }




    AddTransVerterDialog addTransDialog(bands, radioData->transVertNames, this);
    if (addTransDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QString transVertName = addTransDialog.getTransVerterName();

    if (transVertName == "")
    {
        return;
    }

    if (checkTransVerterNameMatch(transVertName))
    {
        // error empty name or name already exists
        QMessageBox::information(this, tr("Transverter Name Exists"),
                                 tr("Transverter Name: %1, already exists \nPlease enter another name").arg(transVertName.trimmed()),
                                  QMessageBox::Ok|QMessageBox::Default,
                                  QMessageBox::NoButton, QMessageBox::NoButton);
        return;
    }

    QString oldName = radioData->transVertNames[tabNum];
    ui->transVertTab->setTabText(tabNum, transVertName);
    radioData->transVertNames[tabNum] = transVertName;
    radioData->transVertSettings[tabNum]->band = transVertName;
    radioData->transVertSettings[tabNum]->transVertName = transVertName;


    for (int i = 0; i < bands.count(); i++)
    {
         if (bands[i]->name() == transVertName)
         {
             radioData->transVertSettings[tabNum]->fLow = bands[i]->fLow;
             radioData->transVertSettings[tabNum]->fHigh = bands[i]->fHigh;
         }
    }
    //renamedTransVertTabs.append(oldName);
    // remove old entry
    //QString fileName = TRANSVERT_PATH_LOGGER + radioData->radioName + FILENAME_TRANSVERT_RADIOS;

    //QSettings config(fileName, QSettings::IniFormat);
    //config.beginGroup(oldName);
    //config.remove("");      // remove all keys for this group
    //config.endGroup();




    transVertTab[tabNum]->transVertValueChanged = true;

}


void RigSetupForm::transVertTabEnable(bool enable)
{

    ui->addTransvert->setDisabled(!enable);
    ui->removeTransvert->setDisabled(!enable);
    ui->changeBand->setDisabled(!enable);
    //ui->transvertFrame->setDisabled(!enable);
    ui->transVertTab->setDisabled(!enable);
}


void RigSetupForm::transVertTabRemove(int tabNum)
{
    ui->transVertTab->removeTab(tabNum);
    transVertTab.removeAt(tabNum);
}


/*
void RigSetupForm::processNetAddress(QLineEdit* networkAddBox, QString& netAddress)
{
    bool addressOk = false;

    if (networkAddBox->text().trimmed() != netAddress)
    {
        if (isHostLocal(networkAddBox->text().trimmed()))
        {
                addressOk = true;
        }
        else
        {
            QHostAddress address(networkAddBox->text().trimmed());
            if (QAbstractSocket::IPv4Protocol == address.protocol())
            {
                addressOk = true;

            }
        }

        if (addressOk)
        {
            netAddress = networkAddBox->text();
            radioValueChanged = true;
        }
        else
        {
               QMessageBox messageBox;
               QString msg = "Invalid Network Address " + networkAddBox->text();
               messageBox.critical(this, "Network Address Entry Error", msg);
               networkAddBox->setFocus();
        }

    }


}

*/
void RigSetupForm::processPortNumber(QLineEdit* netAddBox, QLineEdit* netPortBox, QString& portNumber)
{

    if (netPortBox->text() != portNumber)
    {
         if (netAddBox->text().isEmpty())
         {
             portNumber = netPortBox->text();
             radioValueChanged = true;
         }

         else if (netPortBox->text().toInt() >= 1 && netPortBox->text().toInt() <= 65535)
         {
             portNumber = netPortBox->text();
             radioValueChanged = true;
         }
         else
         {
             QMessageBox messageBox;
             QString msg = tr("Invalid Network Port Number %1").arg(netPortBox->text());
             messageBox.critical(this, tr("Network Port Number out of range"), msg);
             netPortBox->setFocus();
         }

     }




}
