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

RigSetupForm::RigSetupForm(RigFactory* rigFactory_, QSharedPointer<scatParams> _radioData,
                           const QVector<QSharedPointer<BandInfo> > _bands, QLogTabWidget* _ui_RadioTab,
                           bool hfFlag_, QWidget *parent):
    QWidget(parent),
    ui(new Ui::rigSetupForm),
    hfFlag(hfFlag_)
{

    ui->setupUi(this);

    rigFactory = rigFactory_;

    radioData = _radioData;

    bands = _bands;

    ui_RadioTab = _ui_RadioTab;


    fillRadioModelInfo();  // add radio models to drop down

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

    connect(ui->radioModelBox, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::radioModelSelected);
    connect(ui->comPortBox, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::comportSelected);
    connect(ui->comSpeedBox, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::comSpeedSelected);
    connect(ui->comDataBitsBox, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::comDataBitsSelected);
    connect(ui->comStopBitsBox, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::comStopBitsSelected);
    connect(ui->comParityBox, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::comParitySelected);
    connect(ui->comHandShakeBox, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::comHandShakeSelected);
    connect(ui->forceDtrBox, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::on_forceDTRSelected);
    connect(ui->forceRtsBox, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::on_forceRTSSelected);
    connect(ui->advancedCommsChkBox, &QCheckBox::clicked,this, &RigSetupForm::onAdvancedCommsSelected);
    connect(ui->networkAddBox, &QLineEdit::editingFinished, this, &RigSetupForm::networkAddressSelected);
    connect(ui->netPortBox, &QLineEdit::editingFinished, this, &RigSetupForm::networkPortSelected);
    connect(ui->pollInterval, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::pollIntervalSelected);
    connect(ui->enableTransVert, &QCheckBox::clicked, this, &RigSetupForm::enableTransVertSelected);
    connect(ui->mgmBox, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::mgmModeSelected);
    connect(ui->CIVlineEdit, &QLineEdit::editingFinished, this, &RigSetupForm::civAddressFinished);


    connect(ui->pttEnable, &QCheckBox::clicked, this, &RigSetupForm::onPttEnableSelected);
    connect(ui->pttCatEnable, &QCheckBox::clicked, this, &RigSetupForm::onPttCatEnableClicked);
    connect(ui->pttDTREnable, &QCheckBox::clicked, this, &RigSetupForm::onPttDtrEnableClicked);
    connect(ui->pttRTSEnable, &QCheckBox::clicked, this, &RigSetupForm::onPttRtsEnableClicked);
    connect(ui->pttComportSel, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::onPttComportSelActivated);

    connect(ui->enableRitChkBox, &QCheckBox::clicked, this, &RigSetupForm::onEnableRitClicked);
    connect(ui->enableSMeterChkBox, &QCheckBox::clicked, this, &RigSetupForm::onEnableSMeterClicked);
    connect(ui->enableVolChkBox, &QCheckBox::clicked, this, &RigSetupForm::onEnableVolClicked);
    connect(ui->enableCatPttChkBox, &QCheckBox::clicked, this, &RigSetupForm::onEnableCatPttClicked);
    connect(ui->enableVoiceTxMemChkBox, &QCheckBox::clicked, this, &RigSetupForm::onEnableVoiceTxMemClicked);
    connect(ui->enableCwTxMemChkBox, &QCheckBox::clicked, this, &RigSetupForm::onEnableCwTxMemClicked);
    connect(ui->enableCatFeaturesChkBox, &QCheckBox::clicked, this, &RigSetupForm::onEnableCatFeaturesClicked);



    connect(ui->useRigCtldChkBox, &QCheckBox::clicked, this, &RigSetupForm::useRigCtldSelected);
    connect(ui->startMinosRigCtldChkBox, &QCheckBox::clicked, this, &RigSetupForm::onStartMinosRigCtldChkBox);
    connect(ui->rigCtldNetworkAddBox, &QLineEdit::editingFinished, this, &RigSetupForm::rigCtldNetworkAddressSelected);
    connect(ui->rigCtldNetPortBox, &QLineEdit::editingFinished, this, &RigSetupForm::rigCtldNetworkPortSelected);

    initSupBandsChkBoxs();


    // transvert
    connect(ui->enableTransVertSw, &QCheckBox::clicked, this, &RigSetupForm::enableTransVertSwSel);
    connect(ui->locTvConChk, &QCheckBox::clicked, this, &RigSetupForm::localTransVertSwSel);
    connect(ui->locTVComPortSel, QOverload<int>::of(&QComboBox::activated), this, &RigSetupForm::locTVComPortSel);


    connect(ui->addTransvert, &QPushButton::clicked, this, &RigSetupForm::addTransVerter);
    connect(ui->removeTransvert, &QPushButton::clicked, this, &RigSetupForm::removeTransVerter);
    connect(ui->changeBand, &QPushButton::clicked, this, &RigSetupForm::changeBand);

}


RigSetupForm::~RigSetupForm()
{
    delete ui;
}





QSharedPointer<scatParams> RigSetupForm::getRadioData()
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
        }
        setRigctldCheckBoxVisible(rigCap.supportRigCtld);
        //getRadioData()->rigCtldEnable = rigCap.supportRigCtld;

        if (rigCap.supportGetSupBands)
        {
            setSupportBandCheckBoxVisible(false);
        }
        else
        {
            setSupportBandCheckBoxVisible(true);
        }


        // does this radio support antenna sw?

        for (int i = 0; i < radioData->transVertSettings.count(); i++)
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
             QStringList tvList = radioData->transVertSettings.keys();
             //for (int i = 0; i < radioData->numTransverters; i++)
             foreach(const auto &tv, tvList)
             {
                 transVertTab.value(tv)->setEnableTransVertSwBoxVisible(true);
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

    }

    if (ui->pttEnable->isChecked())
    {
        if (isPttComportEqualCatComport())
        {
            if (!ui->pttCatEnable->isChecked())
            {
                if (ui->pttRTSEnable->isChecked())
                {
                    setForceRTSDisabled(true);
                    setForceDTRDisabled(false);
                }
                else if (ui->pttDTREnable->isChecked())
                {
                    setForceRTSDisabled(false);
                    setForceDTRDisabled(true);
                }
            }
        }
        else
        {
            setForceRTSDisabled(false);
            setForceDTRDisabled(false);
        }
    }
}

QString RigSetupForm::getComport()
{
    return ui->comPortBox->currentText();
}

void RigSetupForm::setComport(QString p)
{

    if (ui->comPortBox->findText(p) ==  -1)
    {
        ui->comportErrorTxt->setText(/*HtmlFontColour(Qt::red) + */tr("%1 Not Available").arg(p));
        ui->comportErrorTxt->setVisible(true);

    }
    else
    {
        ui->comportErrorTxt->setText("");
        ui->comportErrorTxt->setVisible(false);

    }

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


void RigSetupForm::comParitySelected(int)
{
    if (serialCommonData::parityCodesList[ui->comParityBox->currentIndex()] != radioData->parity)
    {
        radioData->parity = serialCommonData::parityCodesList[ui->comParityBox->currentIndex()];

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

        if (isPttComportEqualCatComport())
        {
            if (radioData->forceDtr == serialCommonData::forceLinesCodes::FORCE_LINE_NONE)
            {
                setPttDTRDisabled(false);
            }
            else
            {
                setPttDTRDisabled(true);
            }
        }

    }


}



void RigSetupForm::setForceDTRDisabled(bool state)
{
    ui->forceDtrBox->setDisabled(state);
    ui->forceDtrLbl->setDisabled(state);
}


void RigSetupForm::setForceDTRComboBox(int n)
{
    ui->forceDtrBox->setCurrentIndex(n);
}
/**************** Force RTS Line *************************************/

void RigSetupForm::on_forceRTSSelected()
{
    if (serialCommonData::forceLinesCodesList[ui->forceRtsBox->currentIndex()] != radioData->forceRts)
    {
        radioData->forceRts = serialCommonData::forceLinesCodesList[ui->forceRtsBox->currentIndex()];
        if (isPttComportEqualCatComport())
        {
            if (radioData->forceRts == serialCommonData::forceLinesCodes::FORCE_LINE_NONE)
            {
                setPttRTSDisabled(false);
            }
            else
            {
                setPttRTSDisabled(true);
            }
        }

    }
}

void RigSetupForm::setForceRTSDisabled(bool state)
{
    ui->forceRtsBox->setDisabled(state);
    ui->forceRtsLbl->setDisabled(state);
}

void RigSetupForm::setForceRTSComboBox(int n)
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
        QStringList tvList = radioData->transVertSettings.keys();
        //for (int i = 0; i < radioData->numTransverters; i++)
        foreach(const auto &tv, tvList)
        {
            transVertTab.value(tv)->setEnableTransVertSwBoxVisible(checked);
        }
        setEnableLocalTransVertSwVisible(checked);
        if (!checked)
        {
            radioData->enableLocTVSwMsg = checked;
            setEnableLocalTransVertSw(checked);
            setLocTVSWComportVisible(checked);

        }


    }

}


bool RigSetupForm::getEnableTransVertSw()
{
    return ui->enableTransVertSw->isChecked();
}

void RigSetupForm::setEnableTransVertSw(bool b)
{
    ui->enableTransVertSw->setChecked(b);

    QStringList tvList = radioData->transVertSettings.keys();
    //for (int i = 0; i < radioData->numTransverters; i++)
    foreach(const auto &tv, tvList)
    {
        transVertTab.value(tv)->setEnableTransVertSwBoxVisible(b);
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

    }


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


    // this config is only ever preset at install time
    QStringList mgmModes = config.value("MgmModes", "").toStringList();

    config.endGroup();

     ui->mgmBox->clear();
     ui->mgmBox->addItems(mgmModes);
}


/********************** Enable\Disable Features Checkboxes **************/


void RigSetupForm::onEnableRitClicked()
{
    bool checked = ui->enableRitChkBox->isChecked();
    if (radioData->enableDisableCatFeature.ritEnable != checked)
    {
        radioData->enableDisableCatFeature.ritEnable = checked;

    }

}


void RigSetupForm::onEnableSMeterClicked()
{
    bool checked = ui->enableSMeterChkBox->isChecked();
    if (radioData->enableDisableCatFeature.sMeterEnable != checked)
    {
        radioData->enableDisableCatFeature.sMeterEnable = checked;

    }
}

void RigSetupForm::onEnableVolClicked()
{
    bool checked = ui->enableVolChkBox->isChecked();
    if (radioData->enableDisableCatFeature.volumeEnable != checked)
    {
        radioData->enableDisableCatFeature.volumeEnable = checked;

    }
}
void RigSetupForm::onEnableCatPttClicked()
{
    bool checked = ui->enableCatPttChkBox->isChecked();
    if (radioData->enableDisableCatFeature.catEnable != checked)
    {
        radioData->enableDisableCatFeature.catEnable = checked;

    }
}
void RigSetupForm::onEnableVoiceTxMemClicked()
{
    bool checked = ui->enableVoiceTxMemChkBox->isChecked();
    if (radioData->enableDisableCatFeature.voiceMemEnable != checked)
    {
        radioData->enableDisableCatFeature.voiceMemEnable = checked;

    }

}
void RigSetupForm::onEnableCwTxMemClicked()
{
    bool checked = ui->enableCwTxMemChkBox->isChecked();
    if (radioData->enableDisableCatFeature.cWMemEnable != checked)
    {
        radioData->enableDisableCatFeature.cWMemEnable = checked;

    }
}
void RigSetupForm::onEnableCatFeaturesClicked()
{
    if(ui->enableCatFeaturesChkBox->isChecked())
    {
        radioData->enableDisableCatFeature.enableDisplay = true;
        setEnableDisableCatFeaturesGroupVisible(true);

    }
    else
    {
        radioData->enableDisableCatFeature.enableDisplay = false;
        setEnableDisableCatFeaturesGroupVisible(false);

    }
}



void RigSetupForm::loadEnableShowCatFeaturesBox(const RigCapabilities rigCap)
{
    ui->enableCatFeaturesChkBox->setChecked(radioData->enableDisableCatFeature.enableDisplay);
    if (ui->enableCatFeaturesChkBox->isChecked())
    {
        setEnableDisableCatFeaturesGroupVisible(true);
    }
    else
    {
       setEnableDisableCatFeaturesGroupVisible(false);
    }

    ui->enableRitChkBox->setChecked(radioData->enableDisableCatFeature.ritEnable);
    if (rigCap.supportGetRit || rigCap.supportSetRit)
    {
        ui->enableRitChkBox->setVisible(true);
    }
    else
    {
        ui->enableRitChkBox->setVisible(false);
    }


    ui->enableSMeterChkBox->setChecked(radioData->enableDisableCatFeature.sMeterEnable);
    if(rigCap.supportSMeter)
    {
        ui->enableSMeterChkBox->setVisible(true);
    }
    else
    {
        ui->enableSMeterChkBox->setVisible(false);
    }

    ui->enableVolChkBox->setChecked(radioData->enableDisableCatFeature.volumeEnable);
    if (rigCap.supportVolume)
    {
        ui->enableVolChkBox->setVisible(true);
    }
    else
    {
        ui->enableVolChkBox->setVisible(false);
    }

    ui->enableCatPttChkBox->setChecked(radioData->enableDisableCatFeature.catEnable);
    if (rigCap.supportGetPtt && rigCap.supportSetPtt)
    {
        ui->enableCatPttChkBox->setVisible(true);
    }
    else
    {
        ui->enableCatPttChkBox->setVisible(false);
    }

    ui->enableVoiceTxMemChkBox->setChecked(radioData->enableDisableCatFeature.voiceMemEnable);
    if (rigCap.supportVoiceMemory)
    {
        ui->enableVoiceTxMemChkBox->setVisible(true);
    }
    else
    {
        ui->enableVoiceTxMemChkBox->setVisible(false);
    }

    ui->enableCwTxMemChkBox->setChecked(radioData->enableDisableCatFeature.cWMemEnable);
    if (rigCap.supportCwMemory)
    {
        ui->enableCwTxMemChkBox->setVisible(true);
    }
    else
    {
        ui->enableCwTxMemChkBox->setVisible(false);
    }



}



void RigSetupForm::setEnableDisableCatFeaturesGroupVisible(bool visible)
{
     ui->enable_disableFeaturesGroup->setVisible(visible);
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
        SupCheckBoxData scbd;
        scbd.supBandChkBox = allSupBandsChkBoxList[i];
        scbd.bandType = bands[i].data()->getType();
        allSupBandsChkBoxesMap.insert(bands[i].data()->name(), scbd);

        connect(allSupBandsChkBoxList[i], &QCheckBox::stateChanged, this, [=](int state) {onSupbandCheckBoxStateChanged(i, state);});

    }


}


void RigSetupForm::onSupbandCheckBoxStateChanged(int i, int state)
{
    QString selBand = BandList::findBandNameFromIndex(i, bands);

    if (i < allSupBandsChkBoxList.count())
    {
        if (state == Qt::Checked)
        {
            radioData->supportBands.setSupportBandFlag(selBand, true);
        }
        else if (state == Qt::Unchecked)
        {
            radioData->supportBands.setSupportBandFlag(selBand, false);
        }



    }
}



void RigSetupForm::setSupportBandChkBox(int i, bool checked)
{
        allSupBandsChkBoxList[i]->setChecked(checked);
}

void RigSetupForm::setSupportBandChkBox(QString band, bool checked)
{
    if (allSupBandsChkBoxesMap.contains(band))
    {
        allSupBandsChkBoxesMap.value(band).supBandChkBox->setChecked(checked);
    }

}


void RigSetupForm::setSupportBandCheckBoxVisible(bool visible)
{
    if (hfFlag)
    {
       foreach (auto &b, bands)
       {
           if (allSupBandsChkBoxesMap.contains(b.data()->name()))
           {
                allSupBandsChkBoxesMap.value(b.data()->name()).supBandChkBox->setVisible(visible);

           }
       }
    }
    else
    {
        foreach (auto &b, bands)
        {
            if (b.data()->getType() != HF_BANDTYPE)
            {
                if (allSupBandsChkBoxesMap.contains(b.data()->name()))
                {
                    allSupBandsChkBoxesMap.value(b.data()->name()).supBandChkBox->setVisible(visible);

                }
            }

        }
    }

    ui->supportedBandGroupBox->setVisible(visible);
    ui->nativeBandLabel->setVisible(visible);
}

bool RigSetupForm::isAnySupportBandChecked()
{
    if (hfFlag)
    {
        foreach (auto &b, bands)
        {
            if (allSupBandsChkBoxesMap.contains(b.data()->name()))
            {
                if (allSupBandsChkBoxesMap.value(b.data()->name()).supBandChkBox->isChecked())
                {
                    return true;
                }
            }
        }
    }
    else
    {
        foreach (auto &b, bands)
        {
            if (b.data()->getType() != HF_BANDTYPE)
            {
                if (allSupBandsChkBoxesMap.contains(b.data()->name()))
                {
                    if (allSupBandsChkBoxesMap.value(b.data()->name()).supBandChkBox->isChecked())
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;


}


/******************* Transverter ***********************************/


/********************* TabText *****************************************/


void RigSetupForm::setTransVertTabText(int tabNum, QString tabName)
{
    Q_UNUSED(tabName)
    Q_UNUSED(tabNum)
    //ui->transVertTab->setTabText(tabNum, tabName);
}







void RigSetupForm::addTransVerter()
{
    QStringList tvList = radioData->transVertSettings.keys();
    AddTransVerterDialog addTransDialog(bands, tvList, this);
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
    int tabNum = radioData->transVertSettings.count();
    //radioData->transVertNames.append(transVerterName);
    addTransVertTab(tabNum, transVerterName, true);
    loadTransVertTab(transVerterName);

}


void RigSetupForm::addTransVertTab(int tabNum, QString tabName, bool tabChanged)
{
    radioData->transVertSettings.insert(tabName, QSharedPointer<TransVertParams>(new TransVertParams()));
    radioData->transVertSettings.value(tabName)->transVertName = tabName;
    radioData->transVertSettings.value(tabName)->band = tabName;
    //for (int i = 0; i < bands.count(); i++)
    //{
    //     if (bands[i]->name() == tabName)
    //     {
    //         radioData->transVertSettings.value(tabName)->fLow = bands[i]->fLow;
     //        radioData->transVertSettings.value(tabName)->fHigh = bands[i]->fHigh;
     //        break;
     //    }
   // }

    transVertTab.insert(tabName, new TransVertSetupForm(radioData, tabName, bands));
    //addedTransVertTabs.append(tabName);

    ui->transVertTab->insertTab(tabNum, transVertTab.value(tabName), tabName);
    ui->transVertTab->setTabColor(tabNum, Qt::darkBlue);      // radioTab promoted to QLogTabWidget
    ui->transVertTab->setCurrentIndex(tabNum);
    transVertTab.value(tabName)->setEnableTransVertSwBoxVisible(false);

    transVertTab.value(tabName)->transVertValueChanged = tabChanged;

}



void RigSetupForm::loadTransVertTab(QString tabName)
{
    transVertTab.value(tabName)->setRadioFreqBox(radioData->transVertSettings.value(tabName)->radioFreq);
    transVertTab.value(tabName)->setTargetFreqBox(radioData->transVertSettings.value(tabName)->targetFreq);
    transVertTab.value(tabName)->setOffsetFreqLabel(radioData->transVertSettings.value(tabName)->transVertOffset);
    transVertTab.value(tabName)->setTransVerSwNum(radioData->transVertSettings.value(tabName)->transSwitchNum);
    transVertTab.value(tabName)->setEnableTransVertSwBoxVisible(radioData->enableTransSwitch);
}

bool RigSetupForm::checkTransVerterNameMatch(QString transVertName)
{
    for (int i = 0; i < radioData->transVertSettings.count(); i++)
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
        msgBox.setText(tr("You can not remove this transverter - %1, while it is the current radio - %2!").arg(currentTransVertName, currentRadioName));
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
    QString transvertName = ui->transVertTab->tabText(currentIndex);

    ui->transVertTab->removeTab(currentIndex);
    transVertTab.remove(transvertName);
    radioData->transVertSettings.remove(transvertName);


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
        msgBox.setText(tr("You can not change band on this transverter - %1, while it is the current radio - %2!").arg(currentTransVertName, currentRadioName));
        msgBox.exec();
        return;
    }



    QStringList tvList = radioData->transVertSettings.keys();
    AddTransVerterDialog addTransDialog(bands, tvList, this);
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

    // as you can't hide tabs until Qt5.15...have to remove
    QSharedPointer<TransVertParams> currentParams = QSharedPointer<TransVertParams>(new TransVertParams());
    currentParams = radioData->transVertSettings.value(currentTransVertName);

    radioData->transVertSettings.remove(currentTransVertName);

    radioData->transVertSettings.insert(transVertName, currentParams);
    radioData->transVertSettings.value(transVertName)->band = transVertName;
    radioData->transVertSettings.value(transVertName)->transVertName = transVertName;


    //foreach (auto &b, bands)
    //{
    //     if (b->name() == transVertName)
    //     {
    //       radioData->transVertSettings.value(transVertName)->fLow = b->fLow;
    //       radioData->transVertSettings.value(transVertName)->fHigh = b->fHigh;
    //     }
   // }




    transVertTab.remove(currentTransVertName);
    transVertTab.insert(transVertName, new TransVertSetupForm(radioData, transVertName, bands));

    ui->transVertTab->setTabText(tabNum, transVertName);

    //radioData->transVertNames[tabNum] = transVertName;



    //transVertTab[tabNum]->transVertValueChanged = true;

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
    //transVertTab.remove(tabNum);
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

         }

         else if (netPortBox->text().toInt() >= 1 && netPortBox->text().toInt() <= 65535)
         {
             portNumber = netPortBox->text();

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

void RigSetupForm::setPttInitialState()
{
    radioData->enablePTT = false;
    radioData->portType = serialCommonData::PTTMethodCodes::PTT_METHOD_CAT;
    setPttControlsVisible(false);
    ui->pttCatEnable->setChecked(true);
    setPTTCheckBoxChecked(false);
    setPttComport("");
    setPTTCheckBoxChecked(false);
}

void RigSetupForm::setPttControlsVisible(bool visible)
{
    ui->pttComportLbl->setVisible(visible);
    ui->pttCatEnable->setVisible(visible);
    ui->pttDTREnable->setVisible(visible);
    ui->pttRTSEnable->setVisible(visible);
    ui->pttGroupBox->setVisible(visible);
}

void RigSetupForm::setPTTCheckBoxChecked(bool checked)
{
    ui->pttEnable->setChecked(checked);
}

void RigSetupForm::setPTTCheckBoxDisabled(bool disabled)
{
    ui->pttEnable->setDisabled(disabled);
}

void RigSetupForm::loadAvailPttComports()
{
    fillPortsInfo(ui->pttComportSel);
}

void RigSetupForm::setPttComport(QString p)
{
    ui->pttComportSel->setCurrentIndex(ui->pttComportSel->findText(p));
}



void RigSetupForm::setPttTypeRadioButtons(int type)
{
    serialCommonData::PTTMethodCodes pttType = static_cast<serialCommonData::PTTMethodCodes>(type);

    if (pttType == serialCommonData::PTTMethodCodes::PTT_METHOD_CAT)
    {
        ui->pttCatEnable->setChecked(true);
        pttComportSelDisabled(true);
        if (isPttComportEqualCatComport())
        {
            if (radioData->handshake != serialCommonData::handshakeCodes::HANDSHAKE_HARDWARE
                    || radioData->forceRts == serialCommonData::forceLinesCodes::FORCE_LINE_NONE)
            {
                setForceRTSDisabled(false);
            }
            else
            {
                setForceRTSDisabled(true);
            }
            if (radioData->forceDtr == serialCommonData::forceLinesCodes::FORCE_LINE_NONE)
            {
                setForceDTRDisabled(false);
            }
            else
            {
                setForceDTRDisabled(true);
            }
        }
        else
        {
            setForceRTSDisabled(false);
            setForceDTRDisabled(false);
        }

    }
    else if (pttType == serialCommonData::PTTMethodCodes::PTT_METHOD_RTS)
    {
        ui->pttRTSEnable->setChecked(true);
        pttComportSelDisabled(false);
        if (isPttComportEqualCatComport())
        {
            setForceRTSDisabled(true);
        }
    }
    else if (pttType == serialCommonData::PTTMethodCodes::PTT_METHOD_DTR)
    {
        ui->pttDTREnable->setChecked(true);
        if (isPttComportEqualCatComport())
        {
            setForceDTRDisabled(true);
        }
    }
}

void RigSetupForm::onPttEnableSelected(bool /*checked*/)
{

    bool checked = ui->pttEnable->isChecked();
    if (radioData->enablePTT != checked)
    {
        radioData->enablePTT = checked;
        setPttControlsVisible(checked);
        if (checked)
        {
            if (isPttComportEqualCatComport())
            {
                if (ui->pttRTSEnable->isChecked())
                {
                    setForceDTRDisabled(true);
                    setForceRTSDisabled(false);
                }
                else if (ui->pttDTREnable->isChecked())
                {
                    setForceRTSDisabled(true);
                    setForceDTRDisabled(false);
                }
            }
        }
        else
        {
           setForceDTRDisabled(false);
           setForceRTSDisabled(false);
        }

    }


}

void RigSetupForm::onPttCatEnableClicked(bool /*checked*/)
{
    if (ui->pttCatEnable->isChecked() && (static_cast<serialCommonData::PTTMethodCodes>(radioData->pttType) != serialCommonData::PTTMethodCodes::PTT_METHOD_CAT))
    {
        radioData->pttType = static_cast<int>(serialCommonData::PTTMethodCodes::PTT_METHOD_CAT);
    }

    if (ui->pttCatEnable->isChecked())
    {
        pttComportSelDisabled(true);
        setForceRTSDisabled(false);
        setForceDTRDisabled(false);

    }
}


void RigSetupForm::pttComportSelDisabled(bool state)
{
    ui->pttComportSel->setDisabled(state);
}

void RigSetupForm::onPttDtrEnableClicked(bool /*checked*/)
{
    if (ui->pttDTREnable->isChecked() && (static_cast<serialCommonData::PTTMethodCodes>(radioData->pttType) != serialCommonData::PTTMethodCodes::PTT_METHOD_DTR))
    {
        radioData->pttType = static_cast<int>(serialCommonData::PTTMethodCodes::PTT_METHOD_DTR);
    }

    if (ui->pttDTREnable->isChecked())
    {
        ui->pttComportSel->setDisabled(false);
        if (isPttComportEqualCatComport())
        {
            setForceDTRDisabled(true);
            setForceRTSDisabled(false);
        }
        else
        {
            setForceDTRDisabled(false);
        }

    }


}


void RigSetupForm::setPttComportToolTip(QString toolTip)
{
    ui->pttComportSel->setToolTip(toolTip);
}


void RigSetupForm::onPttRtsEnableClicked(bool /*checked*/)
{
    if (ui->pttRTSEnable->isChecked() && (static_cast<serialCommonData::PTTMethodCodes>(radioData->pttType) != serialCommonData::PTTMethodCodes::PTT_METHOD_RTS))
    {
            radioData->pttType = static_cast<int>(serialCommonData::PTTMethodCodes::PTT_METHOD_RTS);
    }

    if (ui->pttRTSEnable->isChecked())
    {
        ui->pttComportSel->setDisabled(false);
        if (isPttComportEqualCatComport())
        {
            setForceRTSDisabled(true);
            setForceDTRDisabled(false);

        }
        else
        {
            setForceDTRDisabled(false);
        }
    }


}

void RigSetupForm::onPttComportSelActivated(int /*idx*/)
{
    if (ui->pttComportSel->currentText() != radioData->pttSerialPort)
    {
        radioData->pttSerialPort = ui->pttComportSel->currentText();

        if (isPttComportEqualCatComport())
        {
            if (!radioData->advancedCommsFlag)
            {
                radioData->advancedCommsFlag = true;
                ui->advancedCommsChkBox->setChecked(true);
                advancedSerialDataEntryVisible(true);
            }

            if (ui->pttRTSEnable->isChecked())
            {
                setForceRTSDisabled(true);
                setForceDTRDisabled(false);
                if (radioData->forceRts == serialCommonData::forceLinesCodes::FORCE_LINE_ON
                        || radioData->forceRts == serialCommonData::forceLinesCodes::FORCE_LINE_OFF)
                {
                    radioData->forceRts = serialCommonData::forceLinesCodes::FORCE_LINE_NONE;
                    setForceRTSComboBox(radioData->forceRts);
                }
            }
            else if (ui->pttDTREnable->isChecked())
            {
                setForceDTRDisabled(true);
                setForceRTSDisabled(false);
                if (radioData->forceDtr == serialCommonData::forceLinesCodes::FORCE_LINE_ON
                        || radioData->forceDtr == serialCommonData::forceLinesCodes::FORCE_LINE_OFF)
                {
                    radioData->forceDtr = serialCommonData::forceLinesCodes::FORCE_LINE_NONE;
                    setForceDTRComboBox(radioData->forceDtr);
                }
            }
        }
        else
        {
            setForceDTRDisabled(false);
            setForceRTSDisabled(false);
        }


    }
}

bool RigSetupForm::isPttComportEqualCatComport()
{
    if (radioData->comport == radioData->pttSerialPort)
    {
        return true;
    }

    return false;

}


void RigSetupForm::setPttRTSDisabled(bool state)
{
    ui->pttRTSEnable->setDisabled(state);
}

void RigSetupForm::setPttDTRDisabled(bool state)
{
    ui->pttRTSEnable->setDisabled(state);
}



