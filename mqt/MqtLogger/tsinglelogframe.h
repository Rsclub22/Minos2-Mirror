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

#include "qsologframe.h"
#include "rigcontrolframe.h"
#include "rotcontrolframe.h"
#include "RotPresets.h"

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
class MinosSplitter;

// We may need to define our own validation controls with valid methods
// for each needed type...
//==========================================================


class BaseMatchContest;
class MatchContact;

class TSingleLogFrame : public QFrame
{
    friend class TSendDM;
    Q_OBJECT

    Ui::TSingleLogFrame *ui;

    QVBoxLayout *verticalLayout = nullptr;
    MinosSplitter *singleLogFrameSplitter = nullptr;

    QTableView *QSOTable;
    RigControlFrame *FKHRigControlFrame = nullptr;
    RotControlFrame *FKHRotControlFrame = nullptr;

    RotPresets *rotPresets = nullptr;

    QFrame *CribSheet= nullptr;
    QLabel *NextContactDetailsLabel;

    QSOLogFrame *GJVQSOLogFrame = nullptr;
    MatchThisFrame *thisMatchFrame = nullptr;
    MatchOtherFrame *otherMatchFrame = nullptr;
    MatchArchiveFrame *archiveMatchFrame = nullptr;

    ChatFrame *chatFrame = nullptr;

    ClusterClientFrame *clusterControlFrame = nullptr;

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

    bool logColumnsChanged;
    bool splittersChanged;

    // From rigcontrol
    QString sCurFreq;
    QString sCurRitFreq;
    QString sCurMode;

    bool isBandMapLoaded();
    bool bandMapLoaded;

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

    void on_NoRadioSetFreq(QString);
    void on_NoRadioSetMode(QString);

    void transferDetails(memoryData::memData &m);
    void getDetails(memoryData::memData &m);
    void getCurrentDetails(memoryData::memData &m);

    void checkConnections();
    void applyScreenLayout();
    QString getCurScreenLayout() const;

    void setCurScreenLayout(const QString &value);

private:
    //    QVector< StackedInfoFrame *> auxFrames;  // NOT shared pointers - singleLogFrame owns them
    BaseContestLog * contest;
    HtmlDelegate *delegate = nullptr;
    QSOGridModel qsoModel;
    int splitterHandleWidth;
    QString curScreenLayout;

    int lastStanzaCount;

    MatchTreeFrame *xferTree = nullptr;

    FocusWatcher *OtherMatchTreeFW = nullptr;
    FocusWatcher *ArchiveMatchTreeFW = nullptr;

    void transferDetails( MatchTreeItem *MatchTreeIndex );

    void keyPressEvent( QKeyEvent* event );

    void restoreColumns();

    MatchTreeItem *getXferItem();

    void buildScreenLayout();
    void createScreenComponents();
    void clearScreenLayout();
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

    void onLogColumnsChanged();
    void onSplittersChanged();
    void on_sectionResized(int, int, int);
    void EditContact(QSharedPointer<BaseContact> lct );

    void on_KeyerLoaded();

    void on_BandMapLoaded();

    void on_RadioLoaded();
    void on_SetRadioList();
    void on_SetBandList(QString);
    void on_SetMode(QString);
    void on_SetFreq(QString);
    void on_SetRitFreq(QString);
    void on_SetRitRadioStatus(bool);
    void on_SetVolume(int level);
    void on_SetRadioStatus(QString);
    void on_SetRadioTpm(int);
    void on_SetRadioTxVertState(bool s);
    void on_SetRitEnableState(bool s);
    void on_SetRadioVolumeState(bool s);

    void on_RotatorLoaded();
    void on_RotatorList();
    void on_RotatorPresetList(QString);
    void on_RotatorStatus(QString);
    void on_RotatorBearing(QString);
    void on_RotatorMaxAzimuth(QString);
    void on_RotatorMinAzimuth(QString);
    void on_cwCcwCmdEnable(bool);
    void presetTurn(QString);

    void sendKeyerPlay( int fno );
    void sendKeyerRecord( int fno );
    void sendBandMap( QString freq, QString call, QString utc, QString loc, QString qth );
    void sendKeyerTone();
    void sendKeyerTwoTone();
    void sendKeyerStop();
    void sendRotator(rpcConstants::RotateDirection direction, int angle );
    void sendRotatorPreset(QString);
    void sendRadioFreq(QString);
    void sendRadioRitFreq(int freq);
    void sendRadioMode(QString);
    void sendRadioRitStatus(bool status);

    void sendSelectRadio(const QString &, const QString &mode);
    void sendSelectRotator(const QString &);
    void onSplitterMoved(int, int);

    void on_doRepaint();
    void sendRadioVolume(int);
    void on_FontChanged();
    void invalidateCacheOnDisconnect();
    void dxSpotToLog(memoryData::memData);
public:
    void sendTpm(int t, QString f);

signals:
    void do_repaint();
};

#endif // TSINGLELOGFRAME_H
