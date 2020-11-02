#include "txvmbuttonsframe.h"
#include "ui_txvmbuttonsframe.h"


const int NO_VM_BUTTON_ON = -1;
const int VM_BUTTON_1_ON = 0;
const int VM_BUTTON_2_ON = 1;


static QKeySequence runButShortCut[] {
    QKeySequence(Qt::CTRL + Qt::Key_BracketLeft),
    QKeySequence(Qt::CTRL + Qt::Key_BracketRight)

};

static QKeySequence runButShiftShortCut[] {
    QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_BracketLeft),
    QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_BracketRight)

};





TxVmButtonsFrame::TxVmButtonsFrame(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::TxVmButtonsFrame),
    runButtonOnNum(NO_VM_BUTTON_ON)
{
    ui->setupUi(this);

    txVoiceKeyer = nullptr;
    voiceKeyerType = keyerTypes[VoiceKeyerId::None];

    voiceKeyerFactory = new VoiceKeyerFactory(this);

    vmCommonParams.clear();

    readVmCommonParams(vmCommonParams);

    voiceKeyerFactory->populateComboKeyerList(ui->voiceKeyerSelect);

    initTxVmButton();


}

TxVmButtonsFrame::~TxVmButtonsFrame()
{
    delete ui;

    foreach(auto b, txVmButtonMap)
    {
        delete b;
    }
}


void TxVmButtonsFrame::initTxVmButton()
{
    voiceMemButtonList << ui->vmToolButton1 << ui->vmToolButton2 << ui->vmToolButton3 << ui->vmToolButton4
                       << ui->vmToolButton5 << ui->vmToolButton6 << ui->vmToolButton7 << ui->vmToolButton8;

    for (int i = 0; i < voiceMemButtonList.count(); i++)
    {
        txVmButtonMap[i] = new TxVoiceMemButton(voiceMemButtonList[i], this, i);
        connect( txVmButtonMap[i], SIGNAL( clearActionSelected(int)) , this, SLOT(runButClearActSel(int)), Qt::QueuedConnection );
        connect( txVmButtonMap[i], SIGNAL( buttonActivated(int)) , this, SLOT(runButActivated(int)), Qt::QueuedConnection );
        connect(ui->voiceKeyerSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(onVoiceKeyerSelect(int)));
        connect(ui->vmSetupPb, SIGNAL(clicked()), this, SLOT(onVmSetupClicked()));
    }

    clearButtonLabels();

}


void TxVmButtonsFrame::setVoiceNumMemButtonsVisible()
{
    for (int i = 0; i < vmCommonParams.getNumButtons(); i++)
    {
        voiceMemButtonList[i]->setVisible(true);
    }

    for (int i = vmCommonParams.getNumButtons(); voiceMemButtonList.count(); i++)
    {
        voiceMemButtonList[i]->setVisible(false);
    }
}

void TxVmButtonsFrame::onVmSetupClicked()
{
    if (voiceKeyerType != keyerTypes[VoiceKeyerId::None])
    {
        VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value(ui->voiceKeyerSelect->currentText());
        TxVmSetupDialog txVmSetupDialog(voiceCap, this);
        txVmSetupDialog.setWindowTitle(tr("Voice Memory Setup"));
        txVmSetupDialog.setVmCommonParamsData(&vmCommonParams);

        if (txVmSetupDialog.exec() == QDialog::Accepted)
        {



        }
    }


}


void TxVmButtonsFrame::onVoiceKeyerSelect(int idx)
{
    Q_UNUSED(idx)

    QString voiceKeyerName = ui->voiceKeyerSelect->currentText();
    if (voiceKeyerName.isEmpty())
    {
       voiceKeyerType = keyerTypes[VoiceKeyerId::None];

       txVoiceKeyer = nullptr;

       clearButtonLabels();

       return;
    }


    VoiceKeyerCapabilities voiceCap = voiceKeyerFactory->supportedVoiceKeyers()->value(voiceKeyerName);
    voiceKeyerType = voiceCap.getKeyerType();

    if (voiceKeyerType == keyerTypes[VoiceKeyerId::RigControl])
    {
        trace(QString("[Voice Keyer] Voice Keyer type selected = %1").arg(voiceCap.getKeyerName()));
        txVoiceKeyer = voiceKeyerFactory->createVoiceKeyer(voiceCap.getVmIdNum());
        if (txVoiceKeyer)
        {

           for (int i = 0; i < voiceMemButtonList.count(); i++)
           {
               VoiceKeyerParams vmData;
               txVoiceKeyer->readVmButtonParams(i, vmData);
               setRunButtonText(i, vmData.getVmName());
           }
        }

    }
}

void TxVmButtonsFrame::clearButtonLabels()
{
    for(int i = 0;  i < voiceMemButtonList.count(); i++)
    {
        setRunButtonText(i, "");
    }
}

void TxVmButtonsFrame::buttonUpdate(int buttonNumber)
{

}

void TxVmButtonsFrame::editActionSelected(int buttonNumber)
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None])
    {
        return;
    }

    VoiceKeyerParams vmData;
    if (txVoiceKeyer)
    {
        txVoiceKeyer->readVmButtonParams(buttonNumber, vmData);
        trace(QString("[voiceMemSetup] write selected button no = %1").arg(buttonNumber));
        TxVmButtonDialog vmButtonDialog(this);

        vmButtonDialog.setWindowTitle(tr("Voice Memory %1 - Edit").arg(buttonNumber + 1));
        vmButtonDialog.setVmData(&vmData);
        if (vmButtonDialog.exec() == QDialog::Accepted)
        {
            if (txVoiceKeyer)
            {
                txVoiceKeyer->saveVmButtonParams(vmData);
                setRunButtonText(buttonNumber, vmData.getVmName());
            }

        }
    }

}

void TxVmButtonsFrame::readActionSelected(int buttonNumber)
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None])
    {
        return;
    }
}

void TxVmButtonsFrame::writeActionSelected(int buttonNumber)
{
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None])
    {
        return;
    }
    VoiceKeyerParams vmData;
    vmData.clear();


    trace(QString("[voiceMemSetup] write selected button no = %1").arg(buttonNumber));

    TxVmButtonDialog vmButtonDialog(this);

    vmButtonDialog.setWindowTitle(tr("Voice Memory %1 - Write").arg(buttonNumber + 1));
    vmData.setvmButtonNum(buttonNumber);
    vmData.setType(voiceKeyerType);
    if (voiceKeyerType == keyerTypes[VoiceKeyerId::None])
    {
        return;
    }

    vmButtonDialog.setVmData(&vmData);

    if (vmButtonDialog.exec() == QDialog::Accepted)
    {
        if (txVoiceKeyer)
        {
            txVoiceKeyer->saveVmButtonParams(vmData);
            setRunButtonText(buttonNumber, vmData.getVmName());
        }

    }

}



void TxVmButtonsFrame::setRunButtonText(const int buttonNumber, const QString name)
{
    QString buttonText = QString("%1: %2").arg(buttonNumber + 1).arg(name);
    voiceMemButtonList[buttonNumber]->setText(buttonText);
}



void TxVmButtonsFrame::initRunMemoryButton()
{

}
void TxVmButtonsFrame::loadRunButtonLabels()
{

}

void TxVmButtonsFrame::radioIsConnected(bool on)
{

}

void TxVmButtonsFrame::saveVmCommonParams(VoiceKeyerCommonParams &vmCommonParams)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

    config.setValue("numButtons", vmCommonParams.getNumButtons());

    config.endGroup();

}


void TxVmButtonsFrame::readVmCommonParams(VoiceKeyerCommonParams &vmCommonParams)
{
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH + VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

    vmCommonParams.setNumButtons(config.value("numButtons", VOICEKEYER_MAX_NUMBUTTONS).toInt());
    config.endGroup();
}


//*******************TX Voice Memory Button *************************//


TxVoiceMemButton::TxVoiceMemButton(QToolButton *b, TxVmButtonsFrame *tvmbf, int no)
{
    memNo = no;
    txVmButtonsFrame = tvmbf;

    vmButton = b;

    vmMenu = new QMenu(vmButton);

    vmButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    vmButton->setPopupMode(QToolButton::MenuButtonPopup);
    vmButton->setFocusPolicy(Qt::NoFocus);
    //vmButton->setText(runButData::runButTitle[memNo]);

    //shortKey = new QShortcut(QKeySequence(runButShortCut[memNo]), vmButton);
    //shiftShortKey = new QShortcut(QKeySequence(runButShiftShortCut[memNo]), vmButton);
    readAction = new QAction(tr("&Read"), vmButton);
    newAction = new QAction(tr("&New"),vmButton);
    editAction = new QAction(tr("&Edit"), vmButton);
    vmMenu->addAction(readAction);
    vmMenu->addAction(newAction);
    vmMenu->addAction(editAction);


    vmButton->setMenu(vmMenu);

    //connect(shortKey, SIGNAL(activated()), this, SLOT(readActionSelected()));
    connect( readAction, SIGNAL( triggered() ), this, SLOT(readActionSelected()));
    connect(vmButton, SIGNAL(clicked(bool)), this, SLOT(readActionSelected()));
    connect(vmButton, SIGNAL(clicked(bool)), this, SLOT(buttonSelected()));
    //connect(shortKey, SIGNAL(activated()), this, SLOT(buttonSelected()));
    //connect(shiftShortKey, SIGNAL(activated()), this, SLOT(memoryShortCutSelected()));
    connect( newAction, SIGNAL( triggered() ), this, SLOT(writeActionSelected()));
    connect( editAction, SIGNAL( triggered() ), this, SLOT(editActionSelected()));


}
TxVoiceMemButton::~TxVoiceMemButton()
{

}



void TxVoiceMemButton::memoryShortCutSelected()
{
//    rigControlFrame->memoryShortCutSelected(memNo);
    vmButton->showMenu();
    //emit lostFocus();
}
void TxVoiceMemButton::readActionSelected()
{
    txVmButtonsFrame->readActionSelected(memNo);
}
void TxVoiceMemButton::editActionSelected()
{

    txVmButtonsFrame->editActionSelected(memNo);

}
void TxVoiceMemButton::writeActionSelected()
{
    txVmButtonsFrame->writeActionSelected(memNo);
}




void TxVoiceMemButton::buttonSelected()
{
    emit buttonActivated(memNo);
}

void TxVoiceMemButton::showButtonOnOff(bool state)
{
    if (state)
    {
        //showRunToolButtonOnFreq();
    }
    else
    {
        //memButton->setStyleSheet(RUN_BUTTON_OFF_STYLE);
    }
}



