#ifndef DMBUTTONFRAME_H
#define DMBUTTONFRAME_H

#include "AnalysePubSubNotify.h"
#include <QFrame>
#include <QMap>
#include <QVector>

#include "dmFKeydef.h"
#include "frequency.h"

#include "PubSubName.h"
#include "radiodetails.h"
#include "txKeyerbase.h"
#include "txkeyerfactory.h"
#include "rigcontrolcommonconstants.h"
#include "cwspeedcontrol.h"
#include "txkeyerCommonConstants.h"

class QPushButton;
class QFileSystemWatcher;
class LoggerContestLog;
class BaseContestLog;
class RigControlFrame;

namespace Ui {
class DMButtonFrame;
}

class DMButtonFrame : public QFrame
{
    Q_OBJECT

public:
    explicit DMButtonFrame(QWidget *parent = nullptr);
    ~DMButtonFrame();

    void setContest(BaseContestLog *);

    QString parseFKeyMessage(QString mess);
    void parseFKeyFile(QString sfname);
    void setFreq(Frequency f);

    // tvVmButtonFrame

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
    void setRadioPttState(bool state);
    void setRigModel(QString rigModel, PubSubName psn);

    void setSelectedRadio(PubSubName selectedRadio);
    void setRadioIsConnected(bool connected);
    void setRadioName(const QString radName);
    int getCwMemType(PubSubName psn);
    void setMode(const QString m);

    void logRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag> logRadioSettingsFlags);
    void setPcCwKeyerComport(QString comportStr);
    void setPcCwKeyerConnectionState(QString stateStr);
    void setPcCwKeyerErrorMsg(QString errorMsg);
    void setPcCwKeyerPttEnabled(QString enabled);
    void setPcCwKeyerTxOnState(QString state);
    void setPcCwKeyerCurrentWpm(QString wpm);



signals:
    void sendFreqControl(Frequency f);

    // txVmButtonFrame
    void pttStatus(bool);
    void sendRadioMode(QString m);
    void sendWpmToPcCwkeyer(int wpm);

private slots:
    void fKey(BaseContestLog *c, int key, int carr);
    void sandPChanged(bool);
    void fButtonClicked();
    void on_stopButton_clicked();

    void on_editButton_clicked();

    void on_logitButton_clicked();

    void on_wipeButton_clicked();

    void on_chooseButton_clicked();

    void fkeyFileChanged();
    void DMMess(AnalysePubSubNotify an);
    void onModeChange(QString mode);
    void on_fkeysetCombo_textActivated(const QString &arg1);

    // txVmButtonFrame

    void onTxKeyerSelect(int idx);
    void onTxKeyerSetupClicked();
    void onRepeatPauseTimerTimeout();
    void onMsgDurTimerTimeout();
    //void fKey(BaseContestLog *c, int e, int);


    void onRemoteConfigChanged();
    void onRemoteKeyerStarted(int key);
    void onRemoteKeyerStopped();
    void on_pipCb_stateChanged(int arg1);
    void onExtConnectTimer();
    void onInternalVoiceMemoryPlayState(bool playing);
    //void sandPChanged(bool s);

    void onCwEntryReturnPressed();
    void setRadioParams();

private:
    Ui::DMButtonFrame *ui;
    LoggerContestLog *ct = nullptr;
    Frequency curFreq;
    QString fkeyFileName;
    QFileSystemWatcher *qfsw = nullptr;
    QStringList nameList;
    QString currentName = "Default";

    QVector<QPushButton *> fButtons;

    //Keys fkeys;
    KeyerMap allKeyConfigs;

    QString dataSender;
    QString curMode;

    QSharedPointer<TxKeyerBase> txKeyer;
    TxKeyerFactory* txKeyerFactory;

    QList<TxKeyerParams> txKeyParamList;

    QTimer *extKeyerConnectTimer = nullptr;
    //QList<QShortcut *> shortCutKeyList;
    int buttonNumSent = TxKeyerCommon::NO_TXKEYER_BUTTON_SELECTED;

    QString txKeyerType;

    QTimer* msgDurTimer = nullptr;
    QTimer* repeatPauseTimer = nullptr;

    bool messagePlaying = false;

    int selectedEomType = TxKeyerCommon::KeyerEomTypes::Eom_None;

    QMap<PubSubName, RadioDetails> allRadioDetails;

    PubSubName selectedRadio;

    QString savedMode;
    bool radioConnected;
    bool pttState;
    bool sAndPState = true;

    CwSpeedControl *cwSpeedSlider = nullptr;

    bool notifyComboChange = true;


    void showFButtons(bool s);
    QString getFKeysString() const;
    bool parseFKeyString(QString &s);
    //bool parseFKeyArray(QJsonArray s, QString keyset);
    bool parseFKeyArray(const QJsonArray &array, KeySet &dest);
    void rewriteFKeyFile();

    bool isDataMode();

    // txVmbuttonFrame

    void setRunButtonText(const int buttonNumber, const QString name);
    //void setVoiceNumMemButtonsVisible(int);
    void clearButtonLabels();
    void startKeyerMsg(int buttonNumber);
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


    bool eventFilter(QObject *obj, QEvent *event);

    void initCwTextEntryBox(QString radioManufacturer, QString fileName);
    void createButtonsForKeyer(int numButtons);
    void clearButtons();
    void createButtonsForKeyer(int numButtons, int columns);
    void setCwEntryBoxVisible(bool visible);
    void setMessagePlayingFlag(bool playing);
    bool isMessagePlaying();
    void setCwMessagePlayingVisible(bool visible);
    void clearCwMessageDisplay();
    void displayCwMessagePlaying(const QString msg);



    void actionDigitalModeKeyPress(int key, int carr);
    void set_DigiMode_FrameState(QString txKeyerName);
    void set_None_FrameState(QString txKeyerName);
    void set_rigControl_FrameState(QString txKeyerName);
    void set_cwRigControl_FrameState(QString txKeyerName);
    void set_pcCwKeyer_FrameState(QString txKeyerName);
    void set_Internal_FrameState(QString txKeyerName);
    void set_External_FrameState(QString txKeyerName);

    void setWipeButtonVisible(bool visible);
    void setLogItButtonVisible(bool visible);
};

#endif // DMBUTTONFRAME_H
