// DMKeyerContainer.cpp
#include "DMKeyerContainer.h"
#include "LoggerContest.h"
#include "ContestApp.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTabBar>
#include "MTrace.h"

//=============================================================================
// KeyerTab Implementation
//=============================================================================

KeyerTab::KeyerTab(const QString &keyerType_,
                   TxKeyerFactory *txKeyerFactory,
                   DMKeyerContainer *keyerContainer,
                   QWidget *parent)
    : QWidget(parent)
    , keyerType(keyerType_)
    , active(false)
    , buttonFrame(nullptr)
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // Create a DMButtonFrame in "tab mode" - without combo box
    buttonFrame = new DMButtonFrame(txKeyerFactory, keyerContainer, this);

    // Set the keyer type for this frame (it won't show combo box)
    buttonFrame->setFixedKeyerName(keyerType);

    layout->addWidget(buttonFrame);
}

KeyerTab::~KeyerTab()
{


}

void KeyerTab::setActive(bool active_)
{
    active = active_;
}



//=============================================================================
// DMKeyerContainer Implementation
//=============================================================================

DMKeyerContainer::DMKeyerContainer(QWidget *parent)
    : QWidget(parent)
    , standaloneFrame(nullptr)
    , tabbedWidget(nullptr)
    , tabWidget(nullptr)
    , activeTab(nullptr)
    , currentContest(nullptr)
    , keyerSettings(new KeyerSettings)
{
    mainLayout = new QVBoxLayout(this);

    // Create factory
    txKeyerFactory = new TxKeyerFactory(this);

    // Create toolbar
    toolbarLayout = new QHBoxLayout();

    modeToggleButton = new QPushButton(this);
    modeToggleButton->setToolTip(tr("Switch between single keyer and tabbed view"));
    toolbarLayout->addWidget(modeToggleButton);

    txKeyerSelect = new QComboBox(this);
    txKeyerSelect->setToolTip(tr("Select Keyer in single keyer view"));
    toolbarLayout->addWidget(txKeyerSelect);

    addKeyerButton = new QPushButton(tr("Add Keyer"), this);
    addKeyerButton->setToolTip(tr("Add a new keyer tab"));
    toolbarLayout->addWidget(addKeyerButton);
    addKeyerButton->setVisible(false);

    toolbarLayout->addStretch();

    mainLayout->addLayout(toolbarLayout);

    // Create stacked widget to hold both modes
    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget);

    // Create standalone mode widget
    standaloneFrame = new DMButtonFrame(txKeyerFactory, this, this);
    setupFrameConnections(standaloneFrame);
    stackedWidget->addWidget(standaloneFrame);

    // Create tabbed mode widget
    tabbedWidget = new QWidget(this);
    tabbedLayout = new QVBoxLayout(tabbedWidget);
    tabbedLayout->setContentsMargins(0, 0, 0, 0);

    tabWidget = new QTabWidget(tabbedWidget);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabbedLayout->addWidget(tabWidget);

    stackedWidget->addWidget(tabbedWidget);

    // Connections
    connect(modeToggleButton, &QPushButton::clicked, this, &DMKeyerContainer::onModeToggleClicked);
    connect(addKeyerButton, &QPushButton::clicked, this, &DMKeyerContainer::onAddKeyerClicked);
    connect(txKeyerSelect, &QComboBox::currentIndexChanged, this, &DMKeyerContainer::onKeyerSelectChanged);
    connect(tabWidget, &QTabWidget::currentChanged, this, &DMKeyerContainer::onTabChanged);
    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &DMKeyerContainer::onTabCloseRequested);


    txKeyerSelect->blockSignals(true);  // Prevent signals during setup

    // get saved keyerType
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

    keyerSettings->setCurrentKeyerName(config.value("KeyerName").toString());

    txKeyerSelect->blockSignals(true);  // Prevent signals during setup
    txKeyerFactory->populateComboKeyerList(txKeyerSelect, keyerSettings->getCurrentKeyerName());

    // we add digi modes to list, though it will not act as keyer
    txKeyerSelect->addItem(txKeyerNames[DigitalModes]);

    txKeyerSelect->blockSignals(false);

    // Initial mode is standalone
    switchToStandaloneMode();
    updateViewModeButton();

    onKeyerSelectChanged();

}

DMKeyerContainer::~DMKeyerContainer()
{

}

void DMKeyerContainer::onKeyerSelectChanged()
{

    QString keyerName = txKeyerSelect->currentText();
    if (keyerSettings->getCurrentKeyerName() != keyerName)
    {

        keyerSettings->setCurrentKeyerName(keyerName);

        // save new keyertype
        QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() + VOICEKEYER_COMMON_PARAMS_FILENAME;
        QSettings config(fileName, QSettings::IniFormat);
        config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);

        config.setValue("KeyerName", keyerName);

        config.endGroup();


        emit keyerSelectChanged();
    }

}


void DMKeyerContainer::setContainerViewMode(ContainerViewMode mode)
{
    if (mode == keyerSettings->getCurrentContainerViewMode())
    {
        return;
    }

    keyerSettings->setCurrentContainerMode(mode);

    if (mode == StandaloneMode)
    {
        switchToStandaloneMode();
    }
    else
    {
        switchToTabbedMode();
    }

    updateViewModeButton();
    emit containerModeChanged(mode);
}

void DMKeyerContainer::switchToStandaloneMode()
{
    stackedWidget->setCurrentWidget(standaloneFrame);
    addKeyerButton->setVisible(false);
    txKeyerSelect->setVisible(true);

    QString keyerType = standaloneFrame->getCurrentKeyerName();
    if (keyerType != keyerSettings->getCurrentKeyerName())
    {
        keyerSettings->setCurrentKeyerName(keyerType);
        emit activeKeyerChanged();
    }
}

void DMKeyerContainer::switchToTabbedMode()
{
    stackedWidget->setCurrentWidget(tabbedWidget);
    addKeyerButton->setVisible(true);
    txKeyerSelect->setVisible(false);

    // If no tabs exist, create one with the current keyer from standalone
    if (tabWidget->count() == 0)
    {
        QString currentKeyer = standaloneFrame->getCurrentKeyerName();
        if (!currentKeyer.isEmpty())
        {
            addKeyerTab(currentKeyer);
        }
    }
}

void DMKeyerContainer::updateViewModeButton()
{
    if (keyerSettings->getCurrentContainerViewMode() == StandaloneMode)
    {
        modeToggleButton->setText(tr("Switch to Tabs"));
        modeToggleButton->setIcon(QIcon(":/icons/tabs.png")); // Optional
    }
    else
    {
        modeToggleButton->setText(tr("Switch to Single"));
        modeToggleButton->setIcon(QIcon(":/icons/single.png")); // Optional
    }
}

void DMKeyerContainer::onModeToggleClicked()
{
    if (keyerSettings->getCurrentContainerViewMode() == StandaloneMode)
    {
        setContainerViewMode(TabbedMode);
    }
    else
    {
        setContainerViewMode(StandaloneMode);
    }

    // Save preference
    QString fileName = VOICEKEYER_COMMON_PARAMS_PATH() +
                       VOICEKEYER_COMMON_PARAMS_FILENAME;
    QSettings config(fileName, QSettings::IniFormat);
    config.beginGroup(VOICEKEYER_COMMON_PARAMS_GROUPNAME);
    config.setValue("ContainerMode", keyerSettings->getCurrentContainerViewMode() == TabbedMode ? "Tabbed" : "Standalone");
    config.endGroup();
}



void DMKeyerContainer::addKeyerTab(const QString &keyerType)
{
    if (keyerSettings->getCurrentContainerViewMode() != TabbedMode)
    {
        return;
    }

    if (keyerType.isEmpty())
    {
        return;
    }

    // Check if already in use
    if (isKeyerTypeInUse(keyerType))
    {
        QMessageBox::information(this, tr("Keyer Already Open"),
                                 tr("This keyer type is already open in another tab."));
        return;
    }

    // Create new keyer tab
    KeyerTab *newTab = createKeyerTab(keyerType);
    if (!newTab)
    {
        return;
    }

    // Generate tab name
    QString tabName = getUniqueTabName(keyerType);

    // Add tab
    int index = tabWidget->addTab(newTab, tabName);
    tabWidget->setCurrentIndex(index);

    // Track this keyer type
    keyerTypesInUse[keyerType] = newTab;

    // If this is the first tab, make it active
    if (tabWidget->count() == 1)
    {
        updateActiveTab(newTab);
    }


}

void DMKeyerContainer::onAddKeyerClicked()
{
    if (keyerSettings->getCurrentContainerViewMode() != TabbedMode)
    {
        return;
    }

    // Get available keyer types
    QStringList availableNames = getAvailableKeyerNames();

    if (availableNames.isEmpty())
    {
        QMessageBox::information(this, tr("No Keyers Available"),
                                 tr("All available keyer types are already in use."));
        return;
    }

    // Show selection dialog
    bool ok;
    QString selectedName = QInputDialog::getItem(
        this,
        tr("Add Keyer"),
        tr("Select keyer type:"),
        availableNames,
        0,
        false,
        &ok
        );

    if (!ok || selectedName.isEmpty())
    {
        return;
    }

    addKeyerTab(selectedName);
}

void DMKeyerContainer::onTabChanged(int index)
{
    if (keyerSettings->getCurrentContainerViewMode() != TabbedMode)
    {
        return;
    }

    if (index < 0 || index >= tabWidget->count())
    {
        updateActiveTab(nullptr);
        return;
    }

    KeyerTab *tab = qobject_cast<KeyerTab*>(tabWidget->widget(index));
    updateActiveTab(tab);
}

void DMKeyerContainer::onTabCloseRequested(int index)
{
    if (keyerSettings->getCurrentContainerViewMode() != TabbedMode)
    {
        return;
    }

    if (index < 0 || index >= tabWidget->count())
    {
        return;
    }

    KeyerTab *tab = qobject_cast<KeyerTab*>(tabWidget->widget(index));
    if (!tab)
    {
        return;
    }

    // Don't allow closing the last tab
    if (tabWidget->count() == 1)
    {
        QMessageBox::information(this, tr("Cannot Close"),
                                 tr("Cannot close the last keyer tab. Switch to single mode instead."));
        return;
    }

    // Confirm closure
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("Close Keyer Tab"),
        tr("Close keyer tab '%1'?").arg(tab->getKeyerType()),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    // Remove from tracking
    keyerTypesInUse.remove(tab->getKeyerType());

    // If this was active, clear it
    if (tab == activeTab)
    {
        activeTab = nullptr;
    }

    // Remove tab
    tabWidget->removeTab(index);
    tab->deleteLater();

    // Update active to current tab
    int currentIndex = tabWidget->currentIndex();
    if (currentIndex >= 0)
    {
        KeyerTab *newActiveTab = qobject_cast<KeyerTab*>(tabWidget->widget(currentIndex));
        updateActiveTab(newActiveTab);
    }
}

void DMKeyerContainer::removeCurrentTab()
{
    if (keyerSettings->getCurrentContainerViewMode() != TabbedMode)
    {
        return;
    }

    int currentIndex = tabWidget->currentIndex();
    if (currentIndex >= 0)
    {
        onTabCloseRequested(currentIndex);
    }
}

int DMKeyerContainer::getTabCount() const
{
    if (keyerSettings->getCurrentContainerViewMode() == TabbedMode)
    {
        return tabWidget->count();
    }
    return 0;
}

KeyerTab* DMKeyerContainer::createKeyerTab(const QString &keyerType)
{
    KeyerTab *tab = new KeyerTab(keyerType, txKeyerFactory, this);

    if (tab->getFrame())
    {
        setupFrameConnections(tab->getFrame());
    }

    return tab;
}

void DMKeyerContainer::updateActiveTab(KeyerTab *newActiveTab)
{
    if (activeTab)
    {
        activeTab->setActive(false);
    }

    activeTab = newActiveTab;

    int newIndex = tabWidget->indexOf(activeTab);
    if (newIndex >= 0)
    {
        tabWidget->tabBar()->setTabTextColor(newIndex, Qt::red);
    }

    if (activeTab)
    {
        activeTab->setActive(true);
        QString keyerType = activeTab->getKeyerType();
        if (keyerType != keyerSettings->getCurrentKeyerName())
        {
            keyerSettings->setCurrentKeyerName(keyerType);
            emit activeKeyerChanged();
        }
    }
}

bool DMKeyerContainer::isKeyerTypeInUse(const QString &keyerType) const
{
    return keyerTypesInUse.contains(keyerType);
}

QString DMKeyerContainer::getUniqueTabName(const QString &keyerType) const
{
    return keyerType;
}

QStringList DMKeyerContainer::getAvailableKeyerNames() const
{
    QStringList available;

    const TxKeyerFactory::TxKeyers *keyers = txKeyerFactory->supportedTxKeyers();

    for (auto it = keyers->constBegin(); it != keyers->constEnd(); ++it) {
        const QString &keyerName = it.key();

        if (keyerName.isEmpty())
        {
            continue;
        }

        // Skip if already in use
        if (isKeyerTypeInUse(keyerName))
        {
            continue;
        }

        available.append(keyerName);
    }

    return available;
}

void DMKeyerContainer::setupFrameConnections(DMButtonFrame *frame)
{
    //***************** I dont think we need this as these signals connect to tslf

    if (!frame)
    {
        return;
    }

    //connect(frame, &DMButtonFrame::pttStatus, this, &DMKeyerContainer::pttStatus);
    //connect(frame, &DMButtonFrame::sendFreqControl, this, &DMKeyerContainer::sendFreqControl);
    //connect(frame, &DMButtonFrame::sendWpmToPcCwkeyer, this, &DMKeyerContainer::sendWpmToPcCwkeyer);
    //connect(frame, &DMButtonFrame::sendModeToRadio, this, &DMKeyerContainer::sendModeToRadio);
}

//=============================================================================
// Forwarding Methods
//=============================================================================

void DMKeyerContainer::setContest(BaseContestLog *contest)
{
    LoggerContestLog* ct = nullptr;

    if (contest)
    {
        // Only wrap if contest is a LoggerContestLog
        ct = dynamic_cast<LoggerContestLog*>(contest);
        if (!ct)
        {
            qWarning() << "setContest: contest is not a LoggerContestLog";
        }
    }

    qDebug() << "Container set contest = %1" << ct;

    keyerSettings->setContest(ct);

    emit contestChanged();
}



void DMKeyerContainer::setSelectedRadio(PubSubName radio)
{
    if (radio != keyerSettings->getSelectedRadio())
    {
        keyerSettings->setSelectedRadio(radio);
        emit selectedRadioChanged();
    }


}

void DMKeyerContainer::setRadioIsConnected(bool connected)
{

    if (connected != keyerSettings->getIsRadioConnected())
    {
        keyerSettings->setIsRadioConnected(connected);
        emit isRadioConnectedChanged(connected);
    }


    /*
    if (currentMode == StandaloneMode)
    {
        standaloneFrame->setRadioIsConnected(connected);
    }
    else
    {
        for (int i = 0; i < tabWidget->count(); ++i)
        {
            KeyerTab *tab = qobject_cast<KeyerTab*>(tabWidget->widget(i));
            if (tab)
            {
                tab->setRadioIsConnected(connected);
            }
        }
    }
*/
}

void DMKeyerContainer::setFreq(Frequency freq)
{
    if (freq != keyerSettings->getFreq())
    {
        keyerSettings->setFreq(freq);
        emit radioFreqChanged(freq);
    }

    /*
    if (currentMode == StandaloneMode)
    {
        standaloneFrame->setFreq(freq);
    }
    else
    {
        for (int i = 0; i < tabWidget->count(); ++i)
        {
            KeyerTab *tab = qobject_cast<KeyerTab*>(tabWidget->widget(i));
            if (tab && tab->getFrame())
            {
                tab->getFrame()->setFreq(freq);
            }
        }
    }
*/
}

void DMKeyerContainer::setMode(const QString &mode)
{
    if (mode != keyerSettings->getRadioMode())
    {
        keyerSettings->setRadioMode(mode);
        emit radioModeChanged(mode);
    }

    /*
    if (currentMode == StandaloneMode)
    {
        standaloneFrame->onModeChange(mode);
    }
    else
    {
        for (int i = 0; i < tabWidget->count(); ++i)
        {
            KeyerTab *tab = qobject_cast<KeyerTab*>(tabWidget->widget(i));
            if (tab && tab->getFrame())
            {
                tab->getFrame()->onModeChange(mode);
            }
        }
    }
*/
}


void DMKeyerContainer::setPttEnabled(bool state, PubSubName psn)
{
    if (state != keyerSettings->getPttEnabled(psn))
    {
        keyerSettings->setPttEnabled(state, psn);
        emit pttEnabledChanged();
    }

}

void DMKeyerContainer::setPttType(int type, PubSubName psn)
{

    if (type != static_cast<int>(keyerSettings->getPttType(psn)))
    {
        keyerSettings->setPttType(type, psn);
        emit pttTypeChanged();
    }


}

void DMKeyerContainer::setVoiceMemAvail(bool avail, PubSubName psn)
{
    if (avail != keyerSettings->isVoiceMemAvail(psn))
    {
        keyerSettings->setVoiceMemAvail(avail, psn);
        emit voiceMemAvailChanged();
    }



}

void DMKeyerContainer::setRadioPttState(bool state)
{
    if (state != keyerSettings->getPttState())
    {
        keyerSettings->setPttState(state);
        emit pttStateChanged();
    }


}

void DMKeyerContainer::setNumVoiceMessages(int numMsgs, PubSubName psn)
{
    if (numMsgs != keyerSettings->getNumVoiceMessages(psn))
    {
        keyerSettings->setNumVoiceMessages(numMsgs, psn);
        emit numVoiceMessagesChanged();
    }


}

void DMKeyerContainer::setRigModel(QString rigModel, PubSubName psn)
{
    if (rigModel != keyerSettings->getRigModel(psn))
    {
        keyerSettings->setRigModel(rigModel, psn);
        emit rigModelChanged();
    }


}

void DMKeyerContainer::setCwMemType(int cwMemType, PubSubName psn)
{
    if (cwMemType != keyerSettings->getCwMemType(psn))
    {
        keyerSettings->setCwMemType(cwMemType, psn);
        emit cwMemTypeChanged();
    }

}

void DMKeyerContainer::setRigVoiceKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn)
{
    if ( supportStopCmd != keyerSettings->getRigVoiceKeyerSupportStopFlag(psn))
    {
        keyerSettings->setRigVoiceKeyerSupportStopFlag(supportStopCmd, psn);
        emit rigVoiceKeyerSupportStopFlagChanged();

    }


}

void DMKeyerContainer::setRigCwKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn)
{
    if ( supportStopCmd != keyerSettings->getRigCwKeyerSupportStopFlag(psn))
    {
        keyerSettings->setRigCwKeyerSupportStopFlag(supportStopCmd, psn);
        emit rigCwKeyerSupportStopCmdChanged();
    }
}

void DMKeyerContainer::setPcCwKeyerComport(QString comportStr)
{
    if ( comportStr != keyerSettings->getPcCwKeyerComport())
    {
        keyerSettings->setPcCwKeyerComport(comportStr);
        emit  pcCwKeyerComportChanged();
    }

}

void DMKeyerContainer::setPcCwKeyerConnectionState(QString stateStr)
{
    if ( stateStr != keyerSettings->getPcCwKeyerConnectionState())
    {
        keyerSettings->setPcCwKeyerConnectionState(stateStr);
        emit pcCwKeyerConnectionStateChanged();
    }

}

void DMKeyerContainer::setPcCwKeyerErrorMsg(QString errorMsg)
{
    if ( errorMsg != keyerSettings->getPcCwKeyerErrorMessage())
    {
        keyerSettings->setPcCwKeyerErrorMessage(errorMsg);
        emit pcCwKeyerErrorMessageChanged();
    }

}

void DMKeyerContainer::setPcCwKeyerPttEnabled(QString enabled)
{
    if ( enabled != keyerSettings->getPcCwKeyerPttEnabled())
    {
        keyerSettings->setPcCwKeyerPttEnabled(enabled);
        emit pcCwKeyerPttEnabledChanged();
    }

}

void DMKeyerContainer::setPcCwKeyerTxOnState(QString state)
{
    if ( state != keyerSettings->getPcCwKeyerTxOnState())
    {
        keyerSettings->setPcCwKeyerTxOnState(state);
        emit pcCwKeyerTxOnStateChanged();
    }

}

void DMKeyerContainer::setPcCwKeyerCurrentWpm(QString wpm)
{
    if ( wpm != keyerSettings->getPcCwKeyerCurrentWpm())
    {
        keyerSettings->setPcCwKeyerCurrentWpm(wpm);
        emit pcCwKeyerCurrentWpmChanged();
    }


}

void DMKeyerContainer::logRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag> flags)
{
    logMessage(QString("Log Radio Settings Changed "));

    if ( flags != keyerSettings->getLogRadioSettings())
    {
        keyerSettings->setLogRadioSettings(flags);
        emit loggerRadioSettingsChanged();
    }


}


void DMKeyerContainer::logMessage(QString msg)
{
    trace(QString("[DMKeyerContainer] -  %1").arg(msg));
}
