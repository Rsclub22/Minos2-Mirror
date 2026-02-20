// DMKeyerContainer.h
// Unified container that supports both standalone (combo box) and tabbed modes
#ifndef DMKEYERCONTAINER_H
#define DMKEYERCONTAINER_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QSharedPointer>
#include <QMap>
#include "LoggerContest.h"
#include "txKeyerFactory.h"
#include "txkeyerCommonConstants.h"
#include "dmbuttonframe.h"

class BaseContestLog;
class LoggerContestLog;
class PubSubName;
class Frequency;
class RadioSettingsDialogChangeFlag;

// Forward declarations
class KeyerTab;
class KeyerSettings;

enum ContainerViewMode {
    StandaloneMode,  // Single DMButtonFrame with combo box (original behavior)
    TabbedMode       // Multiple tabs, each with one keyer type
};

//=============================================================================
// Main container that can switch between standalone and tabbed modes
//=============================================================================
class DMKeyerContainer : public QWidget
{
    Q_OBJECT

public:

    explicit DMKeyerContainer(QWidget *parent = nullptr);
    ~DMKeyerContainer();

    QSharedPointer<KeyerSettings> keyerSettings;

    // Contest and radio settings (forwarded to active or all frames)
    void setContest(BaseContestLog *contest);
    void setSelectedRadio(PubSubName radio);
    void setRadioIsConnected(bool connected);
    void setFreq(Frequency freq);
    void setMode(const QString &mode);

    // Radio parameter updates
    void setPttEnabled(bool state, PubSubName psn);
    void setPttType(int type, PubSubName psn);

    void setVoiceMemAvail(bool avail, PubSubName psn);
    void setRadioPttState(bool state);
    void setNumVoiceMessages(int numMsgs, PubSubName psn);
    void setRigModel(QString rigModel, PubSubName psn);
    void setCwMemType(int cwMemType, PubSubName psn);
    void setRigVoiceKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn);
    void setRigCwKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn);

    // PC CW Keyer updates
    void setPcCwKeyerComport(QString comportStr);
    void setPcCwKeyerConnectionState(QString stateStr);
    void setPcCwKeyerErrorMsg(QString errorMsg);
    void setPcCwKeyerPttEnabled(QString enabled);
    void setPcCwKeyerTxOnState(QString state);
    void setPcCwKeyerCurrentWpm(QString wpm);

    // Logger radio settings changed
    void logRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag> flags);

    // Tab management (only works in TabbedMode)
    void addKeyerTab(const QString &keyerType);
    void removeCurrentTab();
    int getTabCount() const;

    void setErrorMessageDisplayText(const QString errormsg);
    void clearErrorMessageDisplayText();
    void showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour);
signals:


    void keyerSelectChanged();

    void activeKeyerChanged();
    void pttStatus(bool state);
    void sendFreqControl(Frequency freq);
    void sendWpmToPcCwkeyer(int wpm);
    void sendModeToRadio(const QString &mode);
    void containerModeChanged(ContainerViewMode newMode);

    void selectedRadioChanged();
    void isRadioConnectedChanged(bool connected);

    void radioFreqChanged(Frequency freq);
    void radioModeChanged(QString mode);


    void contestChanged();
    void pttEnabledChanged();
    void pttTypeChanged();
    void voiceMemAvailChanged();
    void pttStateChanged();
    void numVoiceMessagesChanged();
    void rigModelChanged();
    void cwMemTypeChanged();
    void rigVoiceKeyerSupportStopFlagChanged();
    void rigCwKeyerSupportStopCmdChanged();
    void pcCwKeyerComportChanged();
    void pcCwKeyerConnectionStateChanged();
    void pcCwKeyerErrorMessageChanged();
    void pcCwKeyerPttEnabledChanged();
    void pcCwKeyerTxOnStateChanged();
    void pcCwKeyerCurrentWpmChanged();
    void loggerRadioSettingsChanged();
    void onPttStateChanged();


private slots:
    void onAddKeyerClicked();
    void onTabChanged(int index);
    void onTabCloseRequested(int index);
    void onModeToggleClicked();

    void onKeyerSelectChanged(int index);
private:
    // Mode switching
    void switchToStandaloneMode();
    void switchToTabbedMode();
    void updateViewModeButton();

    // Tab management helpers
    KeyerTab* createKeyerTab(const QString &keyerType);
    void updateActiveTab(KeyerTab *newActiveTab);
    bool isKeyerTypeInUse(const QString &keyerType) const;
    QString getUniqueTabName(const QString &keyerType) const;
    QStringList getAvailableKeyerNames() const;
    void setupFrameConnections(DMButtonFrame *frame);

    // UI components
    QVBoxLayout *mainLayout;
    QHBoxLayout *toolbarLayout;
    QPushButton *addKeyerButton;
    QPushButton *modeToggleButton;
    QComboBox *txKeyerSelect;
    QStackedWidget *stackedWidget;

    QHBoxLayout *keyerErrorMessageLayout;
    KeyerErrorMessageWidget *keyerErrorMessageDisplay;


    // Standalone mode widget
    DMButtonFrame *standaloneFrame;

    // Tabbed mode widgets
    QWidget *tabbedWidget;
    QVBoxLayout *tabbedLayout;
    QTabWidget *tabWidget;

    // Factory for creating keyers
    TxKeyerFactory *txKeyerFactory;

    // Track which keyer types are in use (tabbed mode only)
    QMap<QString, KeyerTab*> keyerTypesInUse;

    // Active tab (tabbed mode only)
    KeyerTab *activeTab;

    // Contest reference
    LoggerContestLog *currentContest;
    void logMessage(QString msg);
    void setContainerViewMode(ContainerViewMode mode);
    void initialKeyerSelection();
};



//=============================================================================
// KeyerTab - Wrapper for DMButtonFrame in tabbed mode
//=============================================================================
class KeyerTab : public QWidget
{
    Q_OBJECT

public:
    explicit KeyerTab(const QString &keyerType, TxKeyerFactory *txKeyerFactory, DMKeyerContainer *keyerContainer,
                      QWidget *parent = nullptr);
    ~KeyerTab();

    // Accessors
    QString getKeyerType() const { return keyerType; }
    DMButtonFrame* getFrame() const { return buttonFrame; }
    bool isActive() const { return active; }

    // Set this tab as active/inactive
    void setActive(bool active);



private:
    QString keyerType;
    bool active;
    DMButtonFrame *buttonFrame;
    QVBoxLayout *layout;
};




class KeyerSettings :  public QObject
{
    Q_OBJECT

public:

    explicit KeyerSettings(){}



    void setContest(LoggerContestLog* contest)
    {
        currentContest = contest;
    }
    LoggerContestLog* getContest() const
    {
        return currentContest;
    }



    ContainerViewMode getCurrentContainerViewMode() const { return currentContainerViewMode; }
    void setCurrentContainerMode(ContainerViewMode mode){ currentContainerViewMode = mode;};

    TxKeyerId getCurrentKeyerId() const { return currentKeyerId; }
    void setCurrentKeyerId(TxKeyerId id) {
        qDebug() << "set activeKeyerId " << static_cast<int>(id);
        currentKeyerId = id;
    }

    QString getCurrentKeyerName()const {return currentKeyerName;}
    void setCurrentKeyerName(const QString keyerName){ currentKeyerName = keyerName; }

    void setSelectedRadio(PubSubName selRadio){ selectedRadio = selRadio; }
    PubSubName getSelectedRadio(){ return selectedRadio; }

    void setIsRadioConnected(bool connected){ isRadioConnected = connected; }
    bool getIsRadioConnected(){ return isRadioConnected; }

    void setFreq(Frequency f){ freq = f; }
    Frequency getFreq(){ return freq; }

    void setRadioMode(QString m) { mode = m; }
    QString getRadioMode(){ return mode;}

    void setPttState(bool state){ pttState = state; }
    bool getPttState(){ return pttState; }

    void setPttEnabled(bool state, PubSubName psn)
    {
        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            rd.setPttEnabled(state);
            allRadioDetails[psn] = rd;
        }
        else
        {
            rd.setPttEnabled (state);
            allRadioDetails[psn] = rd;
        }
    }


    bool getPttEnabled(PubSubName psn)
    {

        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            return rd.getPttEnabled();
        }


        return false;
    }

    void setPttType(int type, PubSubName psn)
    {

        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            rd.setPttType(type);
            allRadioDetails[psn] = rd;
        }
        else
        {
            rd.setPttType(type);
            allRadioDetails[psn] = rd;
        }


    }

    serialCommonData::MINOS_PTT_TYPES getPttType(PubSubName psn)
    {

        // convert int back to MINOS_PTT_TYPES

        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            return static_cast<serialCommonData::MINOS_PTT_TYPES>(rd.getPttType());
        }


        return serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE;

    }




    void setVoiceMemAvail(bool avail, PubSubName psn)
    {

        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            rd.setVoiceMemAvail(avail);
            allRadioDetails[psn] = rd;
        }
        else
        {
            rd.setVoiceMemAvail(avail);
            allRadioDetails[psn] = rd;
        }
    }

    bool isVoiceMemAvail(PubSubName psn)
    {
        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            return rd.getVoiceMemAvail();
        }

        return false;
    }

    void setNumVoiceMessages(int numMsgs, PubSubName psn)
    {

        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            rd.setNumVoiceMessages(numMsgs);
            allRadioDetails[psn] = rd;
        }
        else
        {
            rd.setNumVoiceMessages(numMsgs);
            allRadioDetails[psn] = rd;
        }


    }

    // This is max number of voice messages available on a radio
    int getNumVoiceMessages(PubSubName psn)
    {

        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            return rd.getNumVoiceMessages();
        }
        else
        {
            return MAXIMUM_BUTTONS;
        }
    }

    void setRigVoiceKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn)
    {


        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            rd.setRigVoiceKeyerSupportStopCmd(supportStopCmd);
            allRadioDetails[psn] = rd;
        }
        else
        {
            rd.setRigVoiceKeyerSupportStopCmd(supportStopCmd);
            allRadioDetails[psn] = rd;
        }



    }



    bool getRigVoiceKeyerSupportStopFlag(PubSubName psn)
    {
        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            return rd.getRigVoiceKeyerSupportStopCmd();
        }

        return true;

    }


    void setRigCwKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn)
    {


        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            rd.setRigCwKeyerSupportStopCmd(supportStopCmd);
            allRadioDetails[psn] = rd;
        }
        else
        {
            rd.setRigCwKeyerSupportStopCmd(supportStopCmd);
            allRadioDetails[psn] = rd;
        }



    }


    bool getRigCwKeyerSupportStopFlag(PubSubName psn)
    {
        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            return rd.getRigCwKeyerSupportStopCmd();
        }

        return true;

    }

    void setRigModel(QString rigModel, PubSubName psn)
    {

        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            rd.setRigModel(rigModel);
            allRadioDetails[psn] = rd;
        }
        else
        {
            rd.setRigModel(rigModel);
            allRadioDetails[psn] = rd;
        }




    }

    QString getRigModel(PubSubName psn)
    {
        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            return rd.getRigModel();
        }

        return "";

    }



    bool isCwMemTypeAvail(PubSubName psn)
    {
        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            if (rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::KENWOOD
                || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::YAESU
                || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::ICOM
                || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::ELECRAFT
                || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO
                || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::FLEX_RADIO_APACHE
                || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::OPENHPSDR
                || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::QRPLABS
                || rd.getCwMemType() == hamlibData::CW_MEMORY_TYPES::THETIS)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        return false;
    }


    void setCwMemType(int cwMemType, PubSubName psn)
    {


        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            rd.setCwMemType(cwMemType);
            allRadioDetails[psn] = rd;
        }
        else
        {
            rd.setCwMemType(cwMemType);
            allRadioDetails[psn] = rd;
        }


    }



    int getCwMemType(PubSubName psn)
    {
        RadioDetails rd;
        if (allRadioDetails.contains(psn))
        {
            rd = allRadioDetails[psn];
            return rd.getCwMemType();
        }

        return hamlibData::CW_MEMORY_TYPES::NONE;

    }

    void setPcCwKeyerComport(QString comport){ pcCwKeyerComport = comport; }
    QString getPcCwKeyerComport(){ return pcCwKeyerComport; }

    void setPcCwKeyerConnectionState(QString state){ pcCwKeyerConnectionState = state; }
    QString getPcCwKeyerConnectionState(){ return pcCwKeyerConnectionState; }

    void setPcCwKeyerErrorMessage(QString msg){ pcCwKeyerErrorMessage = msg; }
    QString getPcCwKeyerErrorMessage(){ return pcCwKeyerErrorMessage; }

    void setPcCwKeyerPttEnabled(QString enabled){ pcCwKeyerPttEnabled = enabled; }
    QString getPcCwKeyerPttEnabled(){ return pcCwKeyerPttEnabled; }

    void setPcCwKeyerCurrentWpm(QString wpm){ pcCwKeyerCurrentWpm = wpm; }
    QString getPcCwKeyerCurrentWpm(){ return pcCwKeyerCurrentWpm; }

    void setPcCwKeyerTxOnState(QString state){ pcCwKeyerTxOnState = state; }
    QString getPcCwKeyerTxOnState(){ return pcCwKeyerTxOnState; }

    void setLogRadioSettings(QSharedPointer<RadioSettingsDialogChangeFlag> flags_){ flags = flags_;}
    QSharedPointer<RadioSettingsDialogChangeFlag> getLogRadioSettings(){ return flags; }

private:

    LoggerContestLog* currentContest = nullptr;
    ContainerViewMode currentContainerViewMode = ContainerViewMode::StandaloneMode;
    TxKeyerId currentKeyerId = TxKeyerId::None;
    QString currentKeyerName = getTxKeyerDisplayName(TxKeyerId::None);

    // radio settings

    PubSubName selectedRadio;
    bool isRadioConnected;
    Frequency freq;
    QString mode;
    bool pttState = false;

    QMap<PubSubName, RadioDetails> allRadioDetails;

    // PC CW Keyer
    QString pcCwKeyerComport;
    QString pcCwKeyerConnectionState;
    QString pcCwKeyerErrorMessage;
    QString pcCwKeyerPttEnabled;
    QString pcCwKeyerCurrentWpm;
    QString pcCwKeyerTxOnState;

    QSharedPointer<RadioSettingsDialogChangeFlag> flags;


};

#endif // DMKEYERCONTAINER_H
