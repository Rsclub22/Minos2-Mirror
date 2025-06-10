#ifndef TXVMBUTTONSFRAME_H
#define TXVMBUTTONSFRAME_H

#include <QShortcut>
#include <QGroupBox>
#include <QKeyEvent>

#include "PubSubName.h"
#include "radiodetails.h"
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"
#include "rigcontrolcommonconstants.h"

class QToolButton;
class QMenu;
class BaseContestLog;

namespace Ui {
class TxVmButtonsFrame;
}

using namespace voiceKeyerCommon;

class TxVmButtonsFrame;



class TxVoiceMemButton: public QObject
{


    Q_OBJECT
public:
    explicit TxVoiceMemButton(QToolButton *b, TxVmButtonsFrame *vmf, int no);
    ~TxVoiceMemButton();


    TxVmButtonsFrame *txVmButtonsFrame;
    QToolButton* vmButton;
    QMenu* vmMenu;
    QShortcut* shortKey;

    QAction* newAction;
    QAction* editAction;

    void showButtonOnOff(bool state);

private slots:
    void memoryShortCutSelected();
    void readActionSelected();
    void editActionSelected();
    void newActionSelected();
    void buttonSelected();
signals:

    void buttonActivated(int);



private:
    int memNo;




};

class TxVmButtonsFrame : public QGroupBox
{
    Q_OBJECT

public:
    explicit TxVmButtonsFrame(QWidget *parent = nullptr);
    ~TxVmButtonsFrame();

    QMap<int, TxVoiceMemButton*> txVmButtonMap;

    void readActionSel(int buttonNumber);
    void readActionSelected(int buttonNumber);
    void editActionSelected(int buttonNumber);
    void newActionSelected(int buttonNumber);

    void setPttEnabled(bool state, PubSubName psn);

    void setPttType(int type, PubSubName psn);
    void setVoiceMemAvail(bool avail, PubSubName psn);
    void setNumVoiceMessages(int numMsgs, PubSubName psn);
    void setCwMemType(int cwMemType, PubSubName psn);
    void setRigVoiceKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn);
    bool getRigVoiceKeyerSupportStopFlag(PubSubName psn);
    void setRigCwKeyerSupportStopFlag(bool supportStopCmd, PubSubName psn);
    bool getRigCwKeyerSupportStopFlag(PubSubName psn);
    void setPttState(bool state);
    void setRigModel(QString rigModel, PubSubName psn);

    void setSelectedRadio(PubSubName selectedRadio);
    void setRadioIsConnected(bool connected);
    void setRadioName(const QString radName);
    int getCwMemType(PubSubName psn);
    void setMode(const QString m);
    void setContest(BaseContestLog *);
    void logRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag> logRadioSettingsFlags);
signals:

    void pttStatus(bool);
    void sendRadioMode(QString m);

private:
    Ui::TxVmButtonsFrame *ui;

    QSharedPointer<VoiceKeyerBase> txVoiceKeyer;
    VoiceKeyerFactory* voiceKeyerFactory;

    BaseContestLog *ct = nullptr;

    QTimer *extKeyerConnectTimer = nullptr;
    //QList<QShortcut *> shortCutKeyList;

    QShortcut *stopButtonShortcut;
    QString voiceKeyerType;


    QTimer* msgDurTimer;
    QTimer* repeatPauseTimer;
    int buttonNumSent ;

    int selectedEomType = voiceKeyerCommon::VoiceCwKeyerEomTypes::Eom_None;

    QList<QToolButton*> voiceMemButtonList;

    QList<VoiceKeyerParams> vmKeyParamList;

    QMap<PubSubName, RadioDetails> allRadioDetails;

    PubSubName selectedRadio;
    QString curMode;
    QString savedMode;
    bool radioConnected;
    bool pttState;
    bool sAndPState = true;



    bool notifyComboChange = true;
    void initTxVmButtonFrame();



    void setRunButtonText(const int buttonNumber, const QString name);
    void setVoiceNumMemButtonsVisible(int);
    void clearButtonLabels();
    void startVMMsg(int buttonNumber);
    void createKeyer(QString voiceKeyerName);
    void setPttStatusIndicatorOnOff(bool on);
    bool isVoiceMemAvail(PubSubName psn);
    bool isCwMemTypeAvail(PubSubName psn);
    void setAvailIndicatorVisible(bool visible);
    void setAvailIndicatorOnOff(bool on);
    void setAvailIndicatorForRadioOnOff(PubSubName radName);
    void setRepeatIndicatorVisible(bool visible);
    void setRepeatIndicatorForMessageOnOff(bool state);
    void setFrameState(QString voiceKeyerName);
    void setRepeatIndicatorOnOff(bool on);
    void pttStopMessage(bool state);
    void turnOffVMButton();


    void updateFrameState();
    void setTXStatusVisible(bool visible);
    void sendModeToRadio(const QString m);
    void setSaveButtonByRadionameText(QString selectedRadioName);
    void checkButtonIniFileVersion(QString voiceKeyerType);
    void loadButtonData();
    void checkCommonIniFileVersion(QString voiceKeyerType);
    int getNumCwMessages(PubSubName psn);
    int getNumVoiceMessages(PubSubName psn);
    QString getRigModel(PubSubName psn);

    serialCommonData::MINOS_PTT_TYPES getPttType(PubSubName psn);
    void setPttTypeLabelsVisible(bool visible);
    void setPttTypeText(serialCommonData::MINOS_PTT_TYPES pttType);
    bool getPttEnabled(PubSubName psn);
    void setPttEnabledIndicatorOnOff(bool on);
    void setEomTypeLabelsVisible(bool visible);
    void setEomLabelText(int selectedEomType);
    void setErrorMessageVisible(bool visible);
    void setKeyerIndicatorGroupBoxVisible(bool visible);
    void setPttIndicatorGroupBoxVisible(bool visible);

    void logMessage(QString msg);

    bool isVoiceMode();
    void setAvailIndicatorOnOffForPcCwKeyer();
    bool isPcCwKeyerLoaded();
    bool isPcCwKeyerConnected();
    void initCwTextEntryBox();
private slots:

    void onVoiceKeyerSelect(int idx);
    void onVmSetupClicked();
    void onRepeatPauseTimerTimeout();
    void onVmStopClicked();
    void onMsgDurTimerTimeout();
    void fKey(BaseContestLog *c, int e, int);


    void onRemoteConfigChanged();
    void onRemoteKeyerStarted(int key);
    void onRemoteKeyerStopped();
    void on_pipCb_stateChanged(int arg1);
    void onExtConnectTimer();
    void onInternalVoiceMemoryPlayState(bool playing);
    void sandPChanged(bool s);

    void onCwEntryReturnPressed();
};

#endif // TXVMBUTTONSFRAME_H
