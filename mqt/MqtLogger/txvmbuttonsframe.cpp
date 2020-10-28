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

}

void TxVmButtonsFrame::runButtonUpdate(int)
{

}

void TxVmButtonsFrame::runButReadActSel(int buttonNumber)
{

}
void TxVmButtonsFrame::runButWriteActSel(int buttonNumber)
{

}
void TxVmButtonsFrame::runButEditActSel(int buttonNumber)
{

}
void TxVmButtonsFrame::runButOffActionSelected(int buttonNumber)
{

}
void TxVmButtonsFrame::setRunButtonActive(int buttonNumber)
{

}
void TxVmButtonsFrame::runModeOff(int buttonNumber)
{

}
void TxVmButtonsFrame::switchRunButton(int buttonNumber)
{

}
void TxVmButtonsFrame::setRunFreq(int buttonNumber)
{

}
void TxVmButtonsFrame::setRunButtonText(int buttonNumber)
{

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

//*******************TX Voice Memory Button *************************//


TxVoiceMemButton::TxVoiceMemButton(QToolButton *b, TxVmButtonsFrame *rcf, int no)
{
    memNo = no;
    //rigControlFrame = rcf;

    vmButton = b;

    vmMenu = new QMenu(vmButton);

    vmButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    vmButton->setPopupMode(QToolButton::MenuButtonPopup);
    vmButton->setFocusPolicy(Qt::NoFocus);
    //vmButton->setText(runButData::runButTitle[memNo]);

    shortKey = new QShortcut(QKeySequence(runButShortCut[memNo]), vmButton);
    shiftShortKey = new QShortcut(QKeySequence(runButShiftShortCut[memNo]), vmButton);
    runOffAction = new QAction(tr("&Off"), vmButton);
    readAction = new QAction(tr("&Read"), vmButton);
    writeAction = new QAction(tr("&New"),vmButton);
    editAction = new QAction(tr("&Edit"), vmButton);
    clearAction = new QAction(tr("&Clear"),vmButton);
    vmMenu->addAction(runOffAction);
    vmMenu->addAction(readAction);
    vmMenu->addAction(writeAction);
    vmMenu->addAction(editAction);
    vmMenu->addAction(clearAction);
    vmButton->setMenu(vmMenu);

    //connect(shortKey, SIGNAL(activated()), this, SLOT(readActionSelected()));
    //connect( readAction, SIGNAL( triggered() ), this, SLOT(readActionSelected()) );
    //connect(memButton, SIGNAL(clicked(bool)), this, SLOT(readActionSelected()));
    connect( readAction, SIGNAL( triggered() ), this, SLOT(buttonSelected()) );
    connect(vmButton, SIGNAL(clicked(bool)), this, SLOT(buttonSelected()));
    connect(shortKey, SIGNAL(activated()), this, SLOT(buttonSelected()));
    connect(shiftShortKey, SIGNAL(activated()), this, SLOT(memoryShortCutSelected()));
    connect( writeAction, SIGNAL( triggered() ), this, SLOT(writeActionSelected()) );
    connect( editAction, SIGNAL( triggered() ), this, SLOT(editActionSelected()) );
    connect( clearAction, SIGNAL( triggered() ), this, SLOT(clearActionSelected()) );
    connect( runOffAction, SIGNAL( triggered() ), this, SLOT(runOffActionSelected()) );

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
    //rigControlFrame->runButReadActSel(memNo);
}
void TxVoiceMemButton::editActionSelected()
{
    //rigControlFrame->runButEditActSel(memNo);
}
void TxVoiceMemButton::writeActionSelected()
{
    //rigControlFrame->runButWriteActSel(memNo);
}
void TxVoiceMemButton::clearActionSelected()
{
    emit clearActionSelected(memNo);
}

void TxVoiceMemButton::runOffActionSelected()
{
    //rigControlFrame->runButOffActionSelected(memNo);
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



