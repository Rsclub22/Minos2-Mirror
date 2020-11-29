#ifndef TSINGLELOGFRAME_H
#define TSINGLELOGFRAME_H

#include "base_pch.h"

#include "StackedInfoFrame.h"
#include "ConfigFile.h"
#include "rotatorcommon.h"
#include "rigmemcommondata.h"
#include "MatchTreeFrame.h"
#include "MatchThisFrame.h"
#include "MatchOtherFrame.h"
#include "MatchArchiveFrame.h"
#include "WsjtxFrame.h"

#include "qsologframe.h"
#include "rigcontrolframe.h"
#include "rotcontrolframe.h"
#include "RotPresets.h"
#include "runbuttonsframe.h"

namespace Ui {
class TSingleLogFrame;
}

class TMatchCollection;
class MatchNodeData;
class MatchTreeItem;
class ProtoContest;
class BaseContestLog;
class BaseContact;
class ContactList;
class ListContact;
class FocusWatcher;
class MatchTreeFrame;
class ChatFrame;
class ClusterClientFrame;
class BandmapClientFrame;
class MinosSplitter;

// We may need to define our own validation controls with valid methods
// for each needed type...
//==========================================================


class BaseMatchContest;
class MatchContact;
class SCRow;

class TSingleLogFrame : public QFrame
{
    friend class TSendDM;
    Q_OBJECT

    Ui::TSingleLogFrame *ui;

    QVBoxLayout *verticalLayout = nullptr;
    MinosSplitter *singleLogFrameSplitter = nullptr;

    QTableView *QSOTable;
    RigControlFrame *FKHRigControlFrame = nullptr;
    RunButtonsFrame *runButtonsFrame = nullptr;
    RotControlFrame *FKHRotControlFrame = nullptr;

    RotPresets *rotPresets = nullptr;

    QFrame *CribSheet= nullptr;
    QLabel *NextContactDetailsLabel;
    QLabel *CurrentBandLabel;

    QSOLogFrame *GJVQSOLogFrame = nullptr;
    MatchThisFrame *thisMatchFrame = nullptr;
    MatchOtherFrame *otherMatchFrame = nullptr;
    MatchArchiveFrame *archiveMatchFrame = nullptr;

    ChatFrame *chatFrame = nullptr;

    WsjtxFrame *wsjtxFrame = nullptr;

    ClusterClientFrame *clusterControlFrame = nullptr;

    BandmapClientFrame *bandmapControlFrame = nullptr;

    QVector <MinosSplitter *> rowSplitters;

public:
    explicit TSingleLogFrame(QWidget *parent, BaseContestLog *contest);
    ~TSingleLogFrame();

    void showQSOs();
    void getSplitters();
    void goSerial( );
    BaseContestLog * getContest();
    void closeContest();

    void setActiveControl( int *Key );
    QString makeEntry( bool saveMinos );
    void exportContest();
    void QSOTreeSelectContact( QSharedPointer<BaseContact> lct );

    ScreenContact &getScreenEntry();
    int getBearingFrmQSOLog();
    int getCurrentBearing();

    void refreshMults();

    bool columnsChanged;
    bool splittersChanged;

    // From rigcontrol
    Frequency sCurFreq;
    Frequency sSavedCurFreq;
    ShortFreq curRitFreq;
    QString sCurMode;
    QString sSavedCurMode;

    bool isBandMapLoaded();
    bool bandMapLoaded;

    bool isClusterServerLoaded();
    void setClusterServerLoaded(bool loaded);
    bool clusterServerLoaded;

    bool isClusterClientLoaded();
    void setClusterClientLoaded(bool loaded);
    bool clusterClientLoaded;

    bool rotatorLoaded;
    bool isRotatorLoaded();

    bool keyerLoaded;
    bool isKeyerLoaded();

    bool radioLoaded;
    bool isRadioLoaded();


    void setRotatorState( QString f );
    void setRotatorBearing( QString f );

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


private:
    BaseContestLog * contest;
    QSharedPointer<HtmlDelegate> delegate;
    QSOGridModel qsoModel;
    int splitterHandleWidth;
    QString curScreenLayout;

    QString clusterServerState;
    int lastStanzaCount;

    MatchTreeFrame *xferTree = nullptr;

    FocusWatcher *OtherMatchTreeFW = nullptr;
    FocusWatcher *ArchiveMatchTreeFW = nullptr;

    bool pauseRigControlUpdates = false;

    void transferDetails( MatchTreeItem *MatchTreeIndex );

    void keyPressEvent( QKeyEvent* event );

    void restoreColumns();

    MatchTreeItem *getXferItem();

    void buildScreenLayout();
    void createScreenComponents();
    void clearScreenLayout();
    void buildRow(SCRow &scrow, int &auxInstance, MinosSplitter *splitterParent);

    void setClusterLoaded(bool loaded);
    void setBandmapLoaded(bool loaded);
    QString getClusterServerState();

    void traceMsg(QString msg);
    void updateFreq(Frequency f);
private slots:
    void onQSOTable_doubleClicked(const QModelIndex &index);

    void on_ContestPageChanged();
    void onOtherMatchTreeFocused(QObject *, bool in, QFocusEvent *);
    void onArchiveTreeFocused(QObject *, bool in, QFocusEvent *);

    void on_XferPressed(BaseContestLog *c, QString basename);
    void MatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, const QItemSelection &selected);

    void on_MatchStarting(BaseContestLog*);
    void NextContactDetailsTimerTimer();
    void PublishTimerTimer();
    void HideTimerTimer();
    void on_MakeEntry(BaseContestLog*);

    void on_AfterSelectContact(QSharedPointer<BaseContact> lct, BaseContestLog *contest);
    void on_AfterLogContact( BaseContestLog *ct);
    void on_NextUnfilled(BaseContestLog*);
    void on_GoToSerial(BaseContestLog*);
    void on_SetMemory(BaseContestLog *, QString, QString);

    void onColumnsChanged();
    void onSplittersChanged();
    void on_sectionResized(int, int, int);
    void EditContact(QSharedPointer<BaseContact> lct );

    void on_KeyerLoaded();



    void on_RadioLoaded();
    void on_SetRadioList();
    void on_SetMode(QString);
    void on_SetFreq(Frequency);
    void on_SetRitFreq(ShortFreq);
    void on_SetRitRadioStatus(bool);
    void on_SetVolume(int level);
    void on_SetRadioStatus(QString);

    void on_RotatorLoaded();
    void on_RotatorList();
    void on_RotatorPresetList(QString);
    void on_RotatorStatus(QString);
    void on_RotatorBearing(QString);
    void on_RotatorMaxAzimuth(int);
    void on_RotatorMinAzimuth(int);
    void on_cwCcwCmdEnable(bool);
    void presetTurn(QString);

    void sendKeyerPlay( int fno );
    void sendKeyerRecord( int fno );
    //void sendBandMap( QString freq, QString call, QString utc, QString loc, QString qth );
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
    void onSplitterMoved(int, int);

    void sendRadioVolume(int);

    void on_FontChanged();
    void invalidateCacheOnDisconnect();
    void dxSpotToLog(memoryData::memData);

    void on_doColumnChanges(BaseContestLog*);
    void on_doSplitterChanges(BaseContestLog*);
    //void sendIgnoreRunChkBoxState(int num, bool checked);
    void on_BandmapMarkFreq(QString cs, Frequency freq, QString loc, QString brg);
    void on_BandmapSaveFreq(QString cs, Frequency freq, QString loc, QString brg);
    void on_rotatorConnected(bool connected);
    void sendRunOnFlag(Frequency, bool);
    void sendRunOffFreqFlag(Frequency, bool);
    void on_ZoomMap(bool dir);
    void on_clusterServerState(QString state);
    void on_clusterServerLoaded();
    void on_SendSpotToClusterServer(Frequency freq, QString callsign, QString loc);
    void on_setClusterTXSpotEnableState(QString state);
    void on_dxSpotToMemory(BaseContestLog *c, memoryData::memData dxData);

    void sendBandmapRadioIsConnected(bool state);
    void sendBandmapRadioHasError(QString error);
    void on_ResendSpotsFromClusterCommand(resendFrameId frameId, QString cmd, int bandmask, QString uuid);

public:
    void on_SetTransVertOffset(double offset, PubSubName psn);
    void on_SetTransVertSwitch(int switchNum, PubSubName psn);
    void on_SetTransVertStatus(bool status, PubSubName psn);
    void on_SetVolumeStatus(bool status, PubSubName psn);

    void on_SetRitEnableStatus(bool status, PubSubName psn);
    void on_SetRitMaxKHzFreq(int maxRitFreq, PubSubName psn);

    void on_SetBandList(QString s, PubSubName psn);
    void on_SetTransVertEnabled(bool status, PubSubName psn);

    void setTuneAddBandMapSetting(bool state);
    bool getTuneAddBandMapSetting();


    void on_SupportStopCommand(bool state);

    void buildFrame();
};

#endif // TSINGLELOGFRAME_H
