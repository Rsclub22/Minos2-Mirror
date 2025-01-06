#ifndef TSINGLELOGFRAME_H
#define TSINGLELOGFRAME_H

#include "ContestPage.h"
#include "dmbuttonframe.h"
#include "rigmemcommondata.h"
#include "qsotableframe.h"
#include "MatchTreeFrame.h"
#include "MatchThisFrame.h"
#include "MatchOtherFrame.h"
#include "MatchArchiveFrame.h"
#include "WsjtxFrame.h"

#include "qsologframe.h"
#include "rigcontrolframe.h"
#include "rotcontrolframe.h"
#include "rotatorskyscanframe.h"
#include "RotPresets.h"
#include "runbuttonsframe.h"
#include "BandSwitchFrame.h"
#include "txvmbuttonsframe.h"
#include "qrzdisplayframe.h"
#include "qsomapframe.h"

namespace Ui {
class TSingleLogFrame;
}


// We may need to define our own validation controls with valid methods
// for each needed type...
//==========================================================

class QTableView;
class TSingleLogFrame : public ContestPage
{
    friend class TSendDM;
    Q_OBJECT

    Ui::TSingleLogFrame *ui;

public:
    explicit TSingleLogFrame(QWidget *parent, BaseContestLog *contest);
    virtual ~TSingleLogFrame();

    QSOTableFrame *QSOListFrame = nullptr;
    RigControlFrame *FKHRigControlFrame = nullptr;
    RunButtonsFrame *runButtonsFrame = nullptr;
    BandSwitchFrame *bandSwitchFrame = nullptr;
    RotControlFrame *FKHRotControlFrame = nullptr;
    RotatorSkyScanFrame *skyScanControlFrame = nullptr;
    TxVmButtonsFrame *txVmButtonsFrame = nullptr;
    QrzDisplayFrame *qrzDisplayFrame = nullptr;

    RotPresets *rotPresets = nullptr;

    QFrame *CribSheet= nullptr;
    MinosSplitter *cribSplitter = nullptr;
    QLabel *NextContactDetailsLabel = nullptr;
    QLabel *CurrentBandLabel = nullptr;

    QSOLogFrame *GJVQSOLogFrame = nullptr;
    MatchThisFrame *thisMatchFrame = nullptr;
    MatchOtherFrame *otherMatchFrame = nullptr;
    MatchArchiveFrame *archiveMatchFrame = nullptr;

    ChatFrame *chatFrame = nullptr;

    WsjtxFrame *wsjtxFrame = nullptr;

    ClusterClientFrame *clusterControlFrame = nullptr;

    BandmapClientFrame *bandmapControlFrame = nullptr;

    QSOMapFrame *qsoMapFrame = nullptr;

    DMButtonFrame *dmButtonFrame = nullptr;

    void buildRow(ContestPage *cp, SCRow &scrow, int &auxInstance, MinosSplitter *splitterParent);

    void startNextEntry();
    void goSerial( );
    void closeContest();

    void addAllQSOsToBandmap();

    void setActiveControl( int *Key );
    QString makeEntry(bool saveMinos , bool sendEntry);
    void exportContest();

    void EditContact(CheckableContact *lct , bool nextUnfilled);

    ScreenContact &getScreenEntry();
    int getBearingFrmQSOLog();
    int getCurrentBearing();

    void refreshMults();

    // From rigcontrol
    Frequency sCurFreq;
    Frequency sSavedCurFreq;
    ShortFreq curRitFreq;
    QString sCurMode;
    QString sSavedCurMode;

    bool isBandMapLoaded();
    bool bandMapLoaded = false;

    void updateTrees();
    void updateQSODisplay();

    bool getStanza( unsigned int stanza, QString &stanzaData );

    void goNextUnfilled();

    void on_NoRadioSetFreq(Frequency);
    void on_NoRadioSetMode(QString);

    void transferDetails(memoryData::memData &m);
    void getDetails(memoryData::memData &m);
    void getCurrentDetails(memoryData::memData &m);

    void checkConnections();

    void applyScreenLayout();
    QString getCurScreenLayout() const;
    void setCurScreenLayout(const QString &value);

    void on_SetTransVertOffset(double offset, PubSubName psn);
    void on_SetTransVertSwitch(int switchNum, PubSubName psn);
    void on_SetTransVertEnabled(bool status, PubSubName psn);

    void on_SetTransVertStatus(bool status, PubSubName psn);
    void on_SetVolumeStatus(bool status, PubSubName psn);

    void on_SetRitEnableStatus(bool status, PubSubName psn);
    void on_SetRitMaxKHzFreq(int maxRitFreq, PubSubName psn);

    void on_SetBandList(QString s, PubSubName psn);

    void on_SupportStopCommand(bool state);
    void sendRigTxVoiceMessage(QString msgNum);
    void sendRigStopTxVoiceMessage(QString msg);

    void onSetPttEnabled(bool state, PubSubName psn);
    void onSetPttType(int type, PubSubName psn);
    void on_SetPttState(bool state);

    void onSetRigModel(QString rigModel, PubSubName psn);

    void setPlaceholders(QStringList nearMatches);

    void buildFrame(int slotNo);

    bool doKeyPressEvent(QKeyEvent *event);
    void transferFromWSJTX(QString call);
    void onSetVoiceMemAvail(bool avail, PubSubName psn);
    void onSetNumVoiceMessages(int numMsgs, PubSubName psn);
    void onSetCwMemType(int cwMemType, PubSubName psn);
    void onRigVoiceKeyerMessageSupportStop(bool supportStopCmd, PubSubName psn);
    void onRigCwKeyerMessageSupportStop(bool supportStopCmd, PubSubName psn);
    void sendRigTxCwMessage(QString msg);

    void on_SetMode(QString);
    void on_SetFreq(Frequency);
    void on_SetRitFreq(ShortFreq);
    void on_SetRitRadioStatus(bool);
    void on_SetVolume(int level);
    void on_SetRadioStatus(QString);

    bool getRadioReadOnlyFlag();

    void on_RotatorPresetList(QString);
    void on_RotatorStatus(QString);
    void on_RotatorBearing(QString);
    void on_RotatorMaxAzimuth(int);
    void on_RotatorMinAzimuth(int);
    void on_cwCcwCmdEnable(bool);
    void presetTurn(QString);

    void xferFromKST(QString call, QString loc, QString freq);

    void setCallPlaceholder(QString call);
private:
    QString curScreenLayout;

    int lastStanzaCount = 0;

    MatchTreeFrame *xferTree = nullptr;

    FocusWatcher *OtherMatchTreeFW = nullptr;
    FocusWatcher *ArchiveMatchTreeFW = nullptr;

    bool pauseRigControlUpdates = false;

    void transferDetails( MatchTreeItem *MatchTreeIndex );

    MatchTreeItem *getXferItem();

    void createScreenComponents();

    void buildScreenLayout(int slotNo);
    void clearScreenLayout(bool clearAllTabs);

    void setClusterLoaded(bool loaded);
    void setBandmapLoaded(bool loaded);

    void traceMsg(QString msg);
    void updateFreq(Frequency f);
    void buildScreen(SCScreen &s, int t, int &auxInstance);

    void setQrzDisplayFrameLoaded(bool loaded);
    void doSendEntry(QString expName);



private slots:
    void on_ContestPageChanged();
    void onOtherMatchTreeFocused(QObject *, bool in, QFocusEvent *);
    void onArchiveTreeFocused(QObject *, bool in, QFocusEvent *);

    void on_XferPressed(BaseContestLog *c, QString basename);
    void MatchTreeSelected(MatchType m, BaseContestLog *c, QString basename);

    void on_MatchStarting(BaseContestLog*);
    void NextContactDetailsTimerTimer();
    void PublishTimerTimer();
    void HideTimerTimer();
    void on_MakeEntry(BaseContestLog*, bool e);

    void on_AfterLogContact(BaseContestLog *ct);
    void on_NextUnfilled(BaseContestLog*);
    void on_GoToSerial(BaseContestLog*);
    void on_SetMemory(BaseContestLog *, QString, QString);

    void on_SetRadioList();

    void on_RotatorList();

    void sendKeyerPlay( int fno );
    void sendKeyerRecord( int fno );
    void sendKeyerTone();
    void sendKeyerTwoTone();
    void sendKeyerStop();
    void sendRotator(rpcConstants::RotateDirection direction, int angle );
    void sendRotatorPreset(QString);
    void sendRadioFreq(Frequency);
    void sendRadioRitFreq(ShortFreq freq);
    void sendRadioMode(QString);
    void sendRadioRitStatus(bool status);

    void sendSelectRadio(const QString &, const QString &band, const Frequency &freq, const QString &mode);
    void sendSelectRotator(const QString &);

    void sendRadioVolume(int);

    void on_FontChanged();
    void invalidateCacheOnDisconnect();
    void dxSpotToLog(memoryData::memData);

    void on_doColumnChanges(BaseContestLog*);
    void on_BandmapMarkFreq(Frequency freq, QString mode);
    void on_BandmapSaveFreq(QString cs, Frequency freq, QString mode, QString loc, QString brg, QString exchange);
    void on_rotatorConnected(bool connected);
    void sendRunOnFlag(Frequency, QString mode, bool);
    void sendRunOffFreqFlag(Frequency, bool);
    void on_clusterServerState(QString state);
    void on_SendSpotToClusterServer(Frequency freq, QString callsign, QString loc);
    void on_setClusterTXSpotEnableState(QString state);
    void on_dxSpotToMemory(BaseContestLog *c, memoryData::memData dxData);

    void sendBandmapRadioIsConnected(bool state);
    void sendBandmapRadioHasError(QString error);

    void on_ResendSpotsFromClusterCommand(resendFrameId frameId, QString cmd, QString bandmask, QString uuid);
    void onLogRadioSettingsChanged(QSharedPointer<RadioSettingsDialogChangeFlag>);
    void sendBandToRig(QString band);
    void on_sendReconnectFlagToClusterServer(bool state);

    void onQrzCallsignRequest(QString callsign);
    void onQrzInfoToLog(QString callsign, QString qraLocator, QString name);
    void onShowCribBand();
};

#endif // TSINGLELOGFRAME_H
