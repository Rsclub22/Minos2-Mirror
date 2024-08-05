/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////
#include <QMessageBox>
#include "winkeyersetupdialog.h"
#include "ui_winkeyersetupdialog.h"
#include "rigcommon.h"
#include "winKeyerCommon.h"

WinKeyerSetupDialog::WinKeyerSetupDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WinKeyerSetupDialog)
{
    ui->setupUi(this);

    setupWinkeyerStatePtr = QSharedPointer<WinkeyerState>::create();
    setupWinkeyStateStoragePtr = QSharedPointer<WinkeyerStateStorage>::create();

    setupWinkeyStateStoragePtr->setWkstate(setupWinkeyerStatePtr);

    QString fileName = "winkeyerSettings.ini";

    QSettings  winkeyerConfig(fileName, QSettings::IniFormat);
    setupWinkeyStateStoragePtr->loadWinkeyerStateStorageFromFile(winkeyerConfig);

    ui->weightLineEdit->setValidatorRange(0, 90);
    //ui->weightLineEdit->setMaxLength(2);

    ui->TailTimeLineEdit->setValidatorRange(0, 250);
    //ui->TailTimeLineEdit->setMaxLength(3);

    ui->leadTimeLineEdit->setValidatorRange(0, 250);
    //ui->leadTimeLineEdit->setMaxLength(3);

    ui->firstCharExtenLineEdit->setValidatorRange(0, 99);
   // ui->firstCharExtenLineEdit->setMaximumHeight(2);

    ui->keyCompLineEdit->setValidatorRange(0, 250);
    //ui->keyCompLineEdit->setMaxLength(3);



    loadAvailableComports();
    fillKeyerModes();
    fillSidetoneFreq();
    fillHangtime();

    loadSettingsToDialog();

    // connect after loading the stored values from file
    connectSignals();

}

WinKeyerSetupDialog::~WinKeyerSetupDialog()
{
    delete ui;
}


void WinKeyerSetupDialog::connectSignals()
{
    connect(ui->comportSel,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WinKeyerSetupDialog::onComportSelCurrentIndexChanged);
    connect(ui->keyerModeSelect,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WinKeyerSetupDialog::onKeyerModelSelectCurrentIndexChanged);

    connect(ui->sidetoneComboSel,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WinKeyerSetupDialog::onSidetoneComobselIndexedChanged);
    connect(ui->hangTimeComboSel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WinKeyerSetupDialog::onHangTimeComboSelIndexedChanged);

    connect(ui->paddleSwapChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onPaddleswapStateChanged);
    connect(ui->autoSpaceChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onAutoSpaceStateChanged);
    connect(ui->ctSpacingChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onCtSpacingStateChanged);
    connect(ui->enablePttChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onEnablePttStateChanged);

    connect(ui->serialEchoChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onSerialEchoStateChanged);
    connect(ui->paddleEchoChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onPadddleEchoStateChanged);
    connect(ui->paddleMuteChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onPaddleMuteStateChanged);
    connect(ui->paddleWdChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onPaddleWdChkBoxStateChanged);
    connect(ui->speedPotLockRadBut, &QRadioButton::clicked, this, &WinKeyerSetupDialog::SpeedPotLockRadButClicked);
    connect(ui->weightLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onWeightLineEditEditingFinished);
    connect(ui->TailTimeLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onTailTimeLineEditEditingFinished);
    connect(ui->leadTimeLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onLeadTimeLineEditEditingFinished);
    connect(ui->firstCharExtenLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onFirstCharExtenLineEditEditingFinished);
    connect(ui->keyCompLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onkeyCompLineEditEditingFinished);
    connect(ui->minWpmLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onMinWpmLineEditEditingFinished);
    connect(ui->maxWpmLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onMaxWpmLineEditEditingFinished);
    connect(ui->setDefaultsPushButton, &QPushButton::clicked, this, &WinKeyerSetupDialog::onDefaultsPushbuttonClicked);

}


void WinKeyerSetupDialog::disconnectSignals()
{
    disconnect(ui->comportSel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WinKeyerSetupDialog::onComportSelCurrentIndexChanged);
    disconnect(ui->keyerModeSelect, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WinKeyerSetupDialog::onKeyerModelSelectCurrentIndexChanged);
    disconnect(ui->sidetoneComboSel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WinKeyerSetupDialog::onSidetoneComobselIndexedChanged);
    disconnect(ui->hangTimeComboSel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &WinKeyerSetupDialog::onHangTimeComboSelIndexedChanged);

    // Disconnecting QCheckBox signals
    disconnect(ui->paddleSwapChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onPaddleswapStateChanged);
    disconnect(ui->autoSpaceChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onAutoSpaceStateChanged);
    disconnect(ui->ctSpacingChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onCtSpacingStateChanged);
    disconnect(ui->enablePttChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onEnablePttStateChanged);
    disconnect(ui->serialEchoChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onSerialEchoStateChanged);
    disconnect(ui->paddleEchoChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onPadddleEchoStateChanged);
    disconnect(ui->paddleMuteChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onPaddleMuteStateChanged);
    disconnect(ui->paddleWdChkBox, &QCheckBox::stateChanged, this, &WinKeyerSetupDialog::onPaddleWdChkBoxStateChanged);

    // Disconnecting QRadioButton signals
    disconnect(ui->speedPotLockRadBut, &QRadioButton::clicked, this, &WinKeyerSetupDialog::SpeedPotLockRadButClicked);

    // Disconnecting QLineEdit signals
    disconnect(ui->weightLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onWeightLineEditEditingFinished);
    disconnect(ui->TailTimeLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onTailTimeLineEditEditingFinished);
    disconnect(ui->leadTimeLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onLeadTimeLineEditEditingFinished);
    disconnect(ui->firstCharExtenLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onFirstCharExtenLineEditEditingFinished);
    disconnect(ui->keyCompLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onkeyCompLineEditEditingFinished);
    disconnect(ui->minWpmLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onMinWpmLineEditEditingFinished);
    disconnect(ui->maxWpmLineEdit, &QLineEdit::editingFinished, this, &WinKeyerSetupDialog::onMaxWpmLineEditEditingFinished);

    // Disconnecting QPushButton signals
    disconnect(ui->setDefaultsPushButton, &QPushButton::clicked, this, &WinKeyerSetupDialog::onDefaultsPushbuttonClicked);
}


void WinKeyerSetupDialog::loadAvailableComports()
{
    fillPortsInfo(ui->comportSel);
}


void WinKeyerSetupDialog::fillKeyerModes()
{
    ui->keyerModeSelect->clear();
    ui->keyerModeSelect->addItems(keyerModeListStr);
}

void WinKeyerSetupDialog::fillSidetoneFreq()
{
    ui->sidetoneComboSel->clear();
    ui->sidetoneComboSel->addItems(sidetoneListStr);
}

void WinKeyerSetupDialog::fillHangtime()
{
    ui->hangTimeComboSel->clear();
    ui->hangTimeComboSel->addItems(hangtimeListStr);
}

void WinKeyerSetupDialog::loadSettingsToDialog()
{
    setComportComboComboSel(setupWinkeyStateStoragePtr->getComport());

    // get mode
    int selectNum = 0;
    switch (setupWinkeyStateStoragePtr->getWkState()->getModereg() & (NIAMBIC + KEYERMODE))
    {
       case 00:
        selectNum = 1;
           break;
       case KEYERMODE:
        selectNum = 0;
        break;
       case NIAMBIC:
           if (setupWinkeyStateStoragePtr->getWkState()->getPincfg() & DITPRI)
           {
               selectNum = 3;
           }
           else if (setupWinkeyStateStoragePtr->getWkState()->getPincfg() & DAHPRI)
           {
               selectNum = 4;
           }
           else
           {
               selectNum = 2;
           }
           break;
       case KEYERMODE + NIAMBIC:
           selectNum = 5;
           break;
    }

    ui->keyerModeSelect->setCurrentIndex(selectNum);

    // get hang time

    switch (setupWinkeyStateStoragePtr->getWkState()->getPincfg() & (HANG1 + HANG0))
    {
        case 0:
            selectNum = 0;
            break;
        case HANG0:
            selectNum = 1;
            break;
        case HANG1:
            selectNum = 2;
            break;
        case HANG1+HANG0:
            selectNum = 3;
            break;
    }

    ui->hangTimeComboSel->setCurrentIndex(selectNum);

    ui->minWpmLineEdit->setText(QString::number(setupWinkeyStateStoragePtr->getWkState()->getMinwpm()));
    ui->maxWpmLineEdit->setText(QString::number(setupWinkeyStateStoragePtr->getWkState()->getMinwpm() + setupWinkeyStateStoragePtr->getWkState()->getWpmrange()));

    ui->paddleSwapChkBox->setChecked((setupWinkeyStateStoragePtr->getWkState()->getModereg() & PDL_SWAP) ? true : false);
    ui->autoSpaceChkBox->setChecked((setupWinkeyStateStoragePtr->getWkState()->getModereg() & ASPACE) ? true : false);
    ui->ctSpacingChkBox->setChecked((setupWinkeyStateStoragePtr->getWkState()->getModereg() & CT_SPACE) ? true : false);
    ui->enablePttChkBox->setChecked((setupWinkeyStateStoragePtr->getWkState()->getPincfg() & USEPTT) ? true : false);
    ui->serialEchoChkBox->setChecked((setupWinkeyStateStoragePtr->getWkState()->getModereg() & SER_ECHO) ? true : false);
    ui->paddleEchoChkBox->setChecked((setupWinkeyStateStoragePtr->getWkState()->getModereg() & PDL_ECHO) ? true : false);
    ui->paddleMuteChkBox->setChecked((setupWinkeyStateStoragePtr->getX2mode() & PDLMUTE) ? true : false);
    ui->paddleWdChkBox->setChecked((setupWinkeyStateStoragePtr->getWkState()->getModereg() & NONSTICK) ? true : false);

    setSidetoneComboSel(QString::number(62500/setupWinkeyStateStoragePtr->getWkState()->getStconst()));
    ui->weightLineEdit->setText(QString::number(setupWinkeyStateStoragePtr->getWkState()->getWeight()));
    ui->TailTimeLineEdit->setText(QString::number(setupWinkeyStateStoragePtr->getWkState()->getTail()));
    ui->leadTimeLineEdit->setText(QString::number(setupWinkeyStateStoragePtr->getWkState()->getLeadin()));
    ui->firstCharExtenLineEdit->setText(QString::number(setupWinkeyStateStoragePtr->getWkState()->getXtnd()));
    ui->keyCompLineEdit->setText(QString::number(setupWinkeyStateStoragePtr->getWkState()->getKcomp()));
}


void WinKeyerSetupDialog::setComportComboComboSel(QString comport)
{
    ui->comportSel->setCurrentIndex(ui->comportSel->findText(comport));

}


void WinKeyerSetupDialog::setSidetoneComboSel(QString freq)
{
    if (freq == "3906")
    {
        ui->sidetoneComboSel->setCurrentIndex(0);
    }
    else if (freq == "1893")
    {
        ui->sidetoneComboSel->setCurrentIndex(1);
    }
    else if (freq == "1275")
    {
        ui->sidetoneComboSel->setCurrentIndex(2);
    }
    else if (freq == "946")
    {
        ui->sidetoneComboSel->setCurrentIndex(3);
    }
    else if (freq == "753")
    {
        ui->sidetoneComboSel->setCurrentIndex(4);
    }
    else if (freq == "625")
    {
        ui->sidetoneComboSel->setCurrentIndex(5);
    }
    else if (freq == "538")
    {
        ui->sidetoneComboSel->setCurrentIndex(6);
    }
    else if (freq == "469")
    {
        ui->sidetoneComboSel->setCurrentIndex(7);
    }
    else if (freq == "419")
    {
        ui->sidetoneComboSel->setCurrentIndex(8);
    }
    else if (freq == "376")
    {
        ui->sidetoneComboSel->setCurrentIndex(9);
    }
    else
    {
        ui->sidetoneComboSel->setCurrentText("");
    }

}


void WinKeyerSetupDialog::onComportSelCurrentIndexChanged()
{
    if (setupWinkeyStateStoragePtr->getComport() != ui->comportSel->currentText())
    {
        setupWinkeyStateStoragePtr->setComport(ui->comportSel->currentText());
        setupIsDirty = true;
    }
}
void WinKeyerSetupDialog::onKeyerModelSelectCurrentIndexChanged()
{
    int idx = ui->keyerModeSelect->currentIndex();
    quint8 oldModereg = setupWinkeyStateStoragePtr->getWkState()->getModereg();
    quint8 modereg = setupWinkeyStateStoragePtr->getWkState()->getModereg();
    quint8 oldPincfg = setupWinkeyStateStoragePtr->getWkState()->getPincfg();
    quint8 pincfg = setupWinkeyStateStoragePtr->getWkState()->getPincfg();
    quint8 temp = 0;
    switch(idx)
    {
        case 0:
            // Iambic A
            modereg &= ~NIAMBIC;
            modereg |= KEYERMODE;
            break;
        case 1:
            // Iambic B
            modereg &= ~NIAMBIC;
            modereg &= ~KEYERMODE;
            break;
        case 2:
            // Ultimatic
            modereg |= NIAMBIC;
            modereg &= ~KEYERMODE;
            temp = static_cast<quint8>(~(DITPRI+DAHPRI));
            pincfg &= temp;
            break;
        case 3:
            // UltimDit
            modereg |= NIAMBIC;
            modereg &= ~KEYERMODE;
            pincfg |= DITPRI;
            pincfg &= ~DAHPRI;
            break;
        case 4:
            // UltimDah
            modereg |= NIAMBIC;
            modereg &= ~KEYERMODE;
            pincfg |= DAHPRI;
            pincfg &= ~DITPRI;
            break;
        case 5:
            // Vibrobug
            modereg |= NIAMBIC;
            modereg |= KEYERMODE;
            break;
    }

    if (oldModereg != modereg)
    {
        setupWinkeyStateStoragePtr->getWkState()->setModereg(modereg);

        setupIsDirty = true;
    }

    if (oldPincfg != pincfg)
    {
        setupWinkeyStateStoragePtr->getWkState()->setPincfg(pincfg);
        setupIsDirty = true;
    }


}

void WinKeyerSetupDialog::onHangTimeComboSelIndexedChanged()
{
    int idx = ui->hangTimeComboSel->currentIndex();
    quint8 pincfg = setupWinkeyStateStoragePtr->getWkState()->getPincfg();
    switch (idx)
    {
    case 0:
        pincfg &= ~(HANG1+HANG0);
        break;
    case 1:
        pincfg &= ~(HANG1+HANG0);
        pincfg += HANG0;
        break;
    case 2:
        pincfg &= ~(HANG1+HANG0);
        pincfg += HANG1;
        break;
    case 3:
        pincfg &= ~(HANG1+HANG0);
        pincfg += (HANG1+HANG0);
        break;

    }

    if (pincfg != setupWinkeyStateStoragePtr->getWkState()->getPincfg())
    {
        setupWinkeyStateStoragePtr->getWkState()->setPincfg(pincfg);
        setupIsDirty = true;
    }
}

void WinKeyerSetupDialog::onSidetoneComobselIndexedChanged()
{
    int f = ui->sidetoneComboSel->currentText().toInt();
    quint8 stf = static_cast<quint8>(62500/f);
    if (setupWinkeyStateStoragePtr->getWkState()->getStconst() != stf)
    {
        setupWinkeyStateStoragePtr->getWkState()->setStconst(stf);
        setupIsDirty = true;
    }
}


void WinKeyerSetupDialog::onWeightLineEditEditingFinished()
{

    bool ok;
    quint8 w = static_cast<quint8>(ui->weightLineEdit->text().toUInt(&ok));
    if (ok)
    {
        if (w != setupWinkeyStateStoragePtr->getWkState()->getWeight())
        {
            setupWinkeyStateStoragePtr->getWkState()->setWeight(w);
            setupIsDirty = true;
        }
    }


}

void WinKeyerSetupDialog::onTailTimeLineEditEditingFinished()
{
    bool ok;
    quint8 t = static_cast<quint8>(ui->TailTimeLineEdit->text().toUInt(&ok));
    if (ok)
    {
        if (t != setupWinkeyStateStoragePtr->getWkState()->getTail())
        {
            setupWinkeyStateStoragePtr->getWkState()->setTail(t);
            setupIsDirty = true;
        }
    }
}
void WinKeyerSetupDialog::onLeadTimeLineEditEditingFinished()
{
    bool ok;
    quint8 l = static_cast<quint8>(ui->leadTimeLineEdit->text().toUInt(&ok));
    if (ok)
    {
        if (l != setupWinkeyStateStoragePtr->getWkState()->getLeadin())
        {
            setupWinkeyStateStoragePtr->getWkState()->setLeadin(l);
            setupIsDirty = true;
        }
    }
}
void WinKeyerSetupDialog::onFirstCharExtenLineEditEditingFinished()
{
    bool ok;
    quint8 f = static_cast<quint8>(ui->firstCharExtenLineEdit->text().toUInt(&ok));
    if (ok)
    {
        if (f != setupWinkeyStateStoragePtr->getWkState()->getXtnd())
        {
            setupWinkeyStateStoragePtr->getWkState()->setXtnd(f);
            setupIsDirty = true;
        }
    }
}
void WinKeyerSetupDialog::onkeyCompLineEditEditingFinished()
{
    bool ok;
    quint8 k = static_cast<quint8>(ui->keyCompLineEdit->text().toUInt(&ok));
    if (ok)
    {
        if (k != setupWinkeyStateStoragePtr->getWkState()->getKcomp())
        {
            setupWinkeyStateStoragePtr->getWkState()->setKcomp(k);
            setupIsDirty = true;
        }
    }
}
void WinKeyerSetupDialog::onMinWpmLineEditEditingFinished()
{
    bool ok;
    quint8 minWpm = static_cast<quint8>(ui->minWpmLineEdit->text().toUInt(&ok));
    if (ok)
    {
        if (minWpm != setupWinkeyStateStoragePtr->getWkState()->getMinwpm())
        {
            setupWinkeyStateStoragePtr->getWkState()->setMinwpm(minWpm);
            setupIsDirty = true;
        }
    }
}
void WinKeyerSetupDialog::onMaxWpmLineEditEditingFinished()
{
    bool ok;
    quint8 maxWpm = static_cast<quint8>(ui->maxWpmLineEdit->text().toUInt(&ok));
    if (ok)
    {
        if (maxWpm != setupWinkeyStateStoragePtr->getWkState()->getMinwpm())
        {
            setupWinkeyStateStoragePtr->getWkState()->setMinwpm(maxWpm);
            setupIsDirty = true;
        }
    }
}
void WinKeyerSetupDialog::onPaddleswapStateChanged()
{
    bool p = ui->paddleSwapChkBox->isChecked();
    if (p != (setupWinkeyStateStoragePtr->getWkState()->getModereg() & PDL_SWAP) ? true : false)
    {
        quint8 modereg = setupWinkeyStateStoragePtr->getWkState()->getModereg();
        if (p)
        {
            modereg |= PDL_SWAP;
            setupWinkeyStateStoragePtr->getWkState()->setModereg(modereg);

        }
        else
        {
            modereg &= ~PDL_SWAP;
            setupWinkeyStateStoragePtr->getWkState()->setModereg(modereg);
        }

        setupIsDirty = true;
    }
}
void WinKeyerSetupDialog::onAutoSpaceStateChanged()
{
    bool a = ui->autoSpaceChkBox->isChecked();
    if (a != (setupWinkeyStateStoragePtr->getWkState()->getModereg() & ASPACE) ? true : false)
    {
        quint8 modereg = setupWinkeyStateStoragePtr->getWkState()->getModereg();
        if (a)
        {
            modereg |= ASPACE;
        }
        else
        {
            modereg &= ~ASPACE;
        }

        setupWinkeyStateStoragePtr->getWkState()->setModereg(modereg);
        setupIsDirty = true;
    }
}
void WinKeyerSetupDialog::onCtSpacingStateChanged()
{
    bool c = ui->ctSpacingChkBox->isChecked();

    if (c != (setupWinkeyStateStoragePtr->getWkState()->getModereg() & CT_SPACE) ? true : false)
    {
        quint8 modereg = setupWinkeyStateStoragePtr->getWkState()->getModereg();
        if (c)
        {
            modereg |= CT_SPACE;
        }
        else
        {
            modereg &= ~CT_SPACE;
        }

        setupWinkeyStateStoragePtr->getWkState()->setModereg(modereg);
        setupIsDirty = true;
    }
}
void WinKeyerSetupDialog::onEnablePttStateChanged()
{
    bool p = ui->enablePttChkBox->isChecked();
    if (p != (setupWinkeyStateStoragePtr->getWkState()->getPincfg() & USEPTT) ? true : false)
    {
        quint8 pincfg = setupWinkeyStateStoragePtr->getWkState()->getPincfg();
        if (p)
        {
            pincfg |=  USEPTT;
        }
        else
        {
            pincfg &= ~ USEPTT;
        }

        setupWinkeyStateStoragePtr->getWkState()->setPincfg(pincfg);
        setupIsDirty = true;
    }

}
void WinKeyerSetupDialog::onSerialEchoStateChanged()
{
    bool s = ui->serialEchoChkBox->isChecked();
    if (s != (setupWinkeyStateStoragePtr->getWkState()->getModereg() & SER_ECHO) ? true : false)
    {
        quint8 modereg = setupWinkeyStateStoragePtr->getWkState()->getModereg();
        if (s)
        {
            modereg |= SER_ECHO;
        }
        else
        {
            modereg &= ~SER_ECHO;
        }

        setupWinkeyStateStoragePtr->getWkState()->setModereg(modereg);
        setupIsDirty = true;
    }
}
void WinKeyerSetupDialog::onPadddleEchoStateChanged()
{
    bool p = ui->paddleEchoChkBox->isChecked();
    if (p != (setupWinkeyStateStoragePtr->getWkState()->getModereg() & PDL_ECHO) ? true : false)
    {
        quint8 modereg = setupWinkeyStateStoragePtr->getWkState()->getModereg();
        if (p)
        {
            modereg |= PDL_ECHO;
        }
        else
        {
            modereg &= ~PDL_ECHO;
        }

        setupWinkeyStateStoragePtr->getWkState()->setModereg(modereg);
        setupIsDirty = true;
    }
}
void WinKeyerSetupDialog::onPaddleMuteStateChanged()
{
    bool m = ui->paddleMuteChkBox->isChecked();
    if (m != (setupWinkeyStateStoragePtr->getX2mode() & PDLMUTE) ? true : false)
    {
        quint8 x2mode = setupWinkeyStateStoragePtr->getX2mode();
        if (m)
        {
            x2mode |= PDLMUTE;
        }
        else
        {
            x2mode &= ~PDLMUTE;
        }

        setupWinkeyStateStoragePtr->setX2mode(x2mode);
        setupIsDirty = true;
    }
}
void WinKeyerSetupDialog::onPaddleWdChkBoxStateChanged()
{
    bool p = ui->paddleWdChkBox->isChecked();
    if (p != (setupWinkeyStateStoragePtr->getWkState()->getModereg() & NONSTICK) ? true : false)
    {
        quint8 modereg = setupWinkeyStateStoragePtr->getWkState()->getModereg();
        if (p)
        {
            modereg |= NONSTICK;
        }
        else
        {
            modereg &= ~NONSTICK;
        }

        setupWinkeyStateStoragePtr->getWkState()->setModereg(modereg);
        setupIsDirty = true;
    }
}
void WinKeyerSetupDialog::onDefaultsPushbuttonClicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Set to Default Settings", "Do you want to set to default settings?",
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        setupWinkeyStateStoragePtr->getWkState()->setWeight(DEFAULT_WEIGHT);
        setupWinkeyStateStoragePtr->getWkState()->setLeadin(DEFAULT_LEADIN);
        setupWinkeyStateStoragePtr->getWkState()->setTail(DEFAULT_TAIL);
        setupWinkeyStateStoragePtr->getWkState()->setXtnd(DEFAULT_XTND);
        setupWinkeyStateStoragePtr->getWkState()->setKcomp(DEFAULT_KCOMP);
        setupWinkeyStateStoragePtr->getWkState()->setFarns(DEFAULT_FARNS);
        setupWinkeyStateStoragePtr->getWkState()->setSampadj(DEFAULT_SAMPADJ);
        setupWinkeyStateStoragePtr->getWkState()->setDitdahratio(DEFAULT_DITDAHRATIO);
        setupWinkeyStateStoragePtr->getWkState()->setPincfg(DEFAULT_PINCFG);
        setupWinkeyStateStoragePtr->getWkState()->setMinwpm(DEFAULT_MINWPM);
        setupWinkeyStateStoragePtr->getWkState()->setWpmrange(DEFAULT_WPMRANGE);
        setupWinkeyStateStoragePtr->getWkState()->setStconst(DEFAULT_STCONST);
        setupWinkeyStateStoragePtr->getWkState()->setModereg(DEFAULT_MODEREG);
        setupWinkeyStateStoragePtr->getWkState()->setX1mode(DEFAULT_X1MODE);
        setupWinkeyStateStoragePtr->setX2mode(DEFAULT_X2MODE);
        setupWinkeyStateStoragePtr->setWKrtty(DEFAULT_WKRTTY);
        setupWinkeyStateStoragePtr->setRYmode(DEFAULT_RYMODE);
        // other stuff to clear to default
        // DEFAULT_STFREQ
        // DEFAULT_SPEED
        // DEFAULT_VOLUME
        // DEFAULT_MESSAGE

        disconnectSignals();

        loadSettingsToDialog();

        connectSignals();

        setupIsDirty = true;

    }
}
void WinKeyerSetupDialog::SpeedPotLockRadButClicked()
{

}
