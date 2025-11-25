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

inline const QString rigControlKeyerConfigFilename = "/rigControl/rigControl.json";
inline const QString cwRigControlKeyerConfigFilename = "/cwRigControl/cwRigControl.json";
inline const QString pcCwKeyerKeyerConfigFilename = "pcCwKeyer/pcCwKeyer.json";
inline const QString digitalModesConfigFilename = "DigitalModes/DigitalModes.json";
inline const QString InternalKeyerConfigFilename = "Internal/internal.json";
inline const QString ExternalKeyerConfigFilename = "Enternal/external.json";



const int CHECK_RAD_CONT_CONTEST_OK = 0;
const int CHECK_RAD_CONT_CONTEST_MISSING = 1;
const int CHECK_RAD_CONT_RADIO_MISSING = 2;
const int CHECK_RAD_CONT_KEYER_MISING = 3;
const int CHECK_RAD_CONT_RADIO_NAME_EMPTY = 4;
const int CHECK_RAD_CONT_CONTEST_NAME_EMPTY = 5;

inline const QList<int> checkContestRadioErrorCodes = {CHECK_RAD_CONT_CONTEST_OK,
                                          CHECK_RAD_CONT_CONTEST_MISSING,
                                          CHECK_RAD_CONT_RADIO_MISSING,
                                          CHECK_RAD_CONT_KEYER_MISING,
                                          CHECK_RAD_CONT_RADIO_NAME_EMPTY,
                                          CHECK_RAD_CONT_CONTEST_NAME_EMPTY};

inline const QStringList checkContestRadioErrorCodeStr = {"OK",
                                             "Contest %1 has not been defined, please add the contest, selecting Default contest",
                                            "Radio name %1 has not been defined, for this contest %2, please add the radio",
                                            "KeyerType %1 does not exist in json file",
                                            "Radio name is empty",
                                            "ContestName is empty"};

struct ValidationResult {
    bool isValid = true;
    QStringList errors;
    QStringList warnings;

    void addError(const QString &msg) {
        errors << msg;
        isValid = false;
    }

    void addWarning(const QString &msg) {
        warnings << msg;
    }
};

class DMButtonFrame : public QFrame
{
    Q_OBJECT

public:
    explicit DMButtonFrame(QWidget *parent = nullptr);
    ~DMButtonFrame();

    void setContest(BaseContestLog *);

    //QString parseFKeyMessage(QString mess);
    //void parseFKeyFile(QString sfname);
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




//    void setRadioListFromTslf();
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

    void on_configEditButton_clicked();

    void on_logitButton_clicked();

    void on_wipeButton_clicked();

    void on_chooseButton_clicked();

    void fkeyFileChanged();
    void DMMess(AnalysePubSubNotify an);
    void onModeChange(QString mode);
    void on_fkeysetCombo_textActivated(const QString &arg1);
    //void onFkeysetComboSelected();

    // txVmButtonFrame

    void onTxKeyerSelect(int idx);
    //void onTxKeyerSetupClicked();
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

    void onCwMacroTextProcessed(const QString &cwTextSent);
private:
    Ui::DMButtonFrame *ui;
    LoggerContestLog *ct = nullptr;
    Frequency curFreq;
    QString fkeyFileName;
    QFileSystemWatcher *qfsw = nullptr;
    QStringList nameList;
    QString currentName = "Default";

    QVector<QPushButton *> fButtons;

    TxKeyerCapabilities selectedKeyerCap;

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

    //QString txKeyerType;

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

    QMap<QString, QString> radioMap; // map radio name to path
    QStringList listOfRadios;       // full path and radio name
    QStringList listOfRadioSupportKeyer;

    CwSpeedControl *cwSpeedSlider = nullptr;

    bool notifyComboChange = true;
    bool ignoreFkComboSignal = false;


    void showFButtons(bool s);
    QString getFKeysString() const;
    bool parseFKeyString(QString &s);
    //bool parseFKeyArray(QJsonArray s, QString keyset);
    //bool parseFKeyArray(const QJsonArray &array, KeySet &dest);
    //void rewriteFKeyFile();

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
    void clearCwMessagePlayingDisplay();
    void displayCwMessagePlaying(const QString msg);



    void actionDigitalModeKeyPress(int key, int carr);
    void set_DigiMode_FrameState();
    void set_None_FrameState();
    void set_rigControl_FrameState();
    void set_cwRigControl_FrameState();
    void set_pcCwKeyer_FrameState();
    void set_Internal_FrameState();
    void set_External_FrameState();

    void setWipeButtonVisible(bool visible);
    void setLogItButtonVisible(bool visible);
    QStringList getContestNamesForKeyerType(const QString &keyerType);
    void getVoiceCwMemSupportedRadios(const QStringList &listOfRadios, QStringList& listOfRadioSupportKeyer);
    void clearAllDirtyFlags();
    QStringList getRadioNamesForSelectedContestName(const QString &keyerType);
//    void populateRadioNameCombo(const QString &contestName);

    ValidationResult validateKeyConfigs(const KeyerMap &configs);
//    void connectFkeySetComboToPopulateRadioNameCombo();
//    void disConnectFkeySetComboToPopulateRadioNameCombo();
    void displayErrorMessage(QString msg);
    void clearErrorMessage();
    void checkRadioExists(QString radioName, int &errorCode);
    void clearAll_Ui_Elements();
    void setupRigControl_Ui_Elements();
    void displayButtons();
    void setupCw_RigControl_Ui_Elements();
    bool writeSingleKeyerFile(const QString &filePath, const QString &keyerType, TxKeyerId keyerId);
    bool readSingleKeyerFile(const QString &filePath, const QString &keyerType);
    QString parseFKeyMessage(QString mess);
    TxKeyerId txKeyerNameToId(const QString &name);
    void populateFksetCombo(QString txKeyerType, QString currentName, bool &contestNameOk);
    void checkSavedContestExists(int &errorCode);
    bool checkContestAndRadioAvailable(int &errorCode);
    void setCwFreeTextIndicatorOnOff(bool on);
    void setCwFreeTextIndicatorVisible(bool visible);
    void showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour = QColorConstants::Svg::red);
    bool checkRadioAndKeyerState();

};

#endif // DMBUTTONFRAME_H
