#ifndef TSINGLELOGFRAME_H
#define TSINGLELOGFRAME_H

#include "logger_pch.h"
#include "StackedInfoFrame.h"

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
class TSendDM;

// We may need to define our own validation controls with valid methods
// for each needed type...
//==========================================================


class BaseMatchContest;
class MatchContact;

class TSingleLogFrame : public QFrame
{
    Q_OBJECT

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

    void refreshMults();
    TSendDM *sendDM;

    bool logColumnsChanged;
    bool splittersChanged;

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
    void doNextContactDetailsOnLeftClick(bool keepSizes);

    //void on_SetRadioName(QString);
    void on_RotatorAntennaName(QString);

    void on_NoRadioSetFreq(QString);
    void on_NoRadioSetMode(QString);

    void transferDetails(memoryData::memData &m);
    void getDetails(memoryData::memData &m);
    void getCurrentDetails(memoryData::memData &m);

private:
    Ui::TSingleLogFrame *ui;

    QVector< StackedInfoFrame *> auxFrames;  // NOT shared pointers - singleLogFrame owns them
    BaseContestLog * contest;
    QSOGridModel qsoModel;
    int splitterHandleWidth;

    int lastStanzaCount;

    long long curFreq;
    QString sCurFreq;
    QString sCurMode;

    int freqUpDateCnt;
    int modeUpDateCnt;

    void transferDetails( MatchTreeItem *MatchTreeIndex );

    void keyPressEvent( QKeyEvent* event );

    void restoreColumns();

    void doSetAuxWindows(bool saveSplitter);
private slots:
    void on_ContestPageChanged();
    void on_XferPressed();
    void NextContactDetailsTimerTimer();
    void PublishTimerTimer();
    void HideTimerTimer();
    void on_MakeEntry(BaseContestLog*);
    void on_QSOTable_doubleClicked(const QModelIndex &index);
    void on_AfterSelectContact(QSharedPointer<BaseContact> lct, BaseContestLog *contest);
    void on_AfterLogContact( BaseContestLog *ct);
    void on_NextContactDetailsOnLeft();
    void on_NextUnfilled(BaseContestLog*);
    void on_GoToSerial(BaseContestLog*);
    void on_SetMemory(BaseContestLog *, QString, QString);

    void onLogColumnsChanged();
    void onSplittersChanged();
    void on_logFrameSplitter_splitterMoved(int pos, int index);
    void on_CribSplitter_splitterMoved(int pos, int index);
    void on_MultSplitter_splitterMoved(int pos, int index);
    void on_sectionResized(int, int, int);
    void EditContact(QSharedPointer<BaseContact> lct );

    void on_KeyerLoaded();

    void on_BandMapLoaded();

    void on_SetRadioList(QString);
    void on_SetRadioName(QString);
    void on_SetMode(QString);
    void on_SetFreq(QString);
    void on_RadioLoaded();
    void on_SetRadioState(QString);
    void on_SetRadioTxVertState(QString s);

    void on_RotatorLoaded();
    void on_RotatorList(QString);
    void on_RotatorState(QString);
    void on_RotatorBearing(QString);
    void on_RotatorMaxAzimuth(QString);
    void on_RotatorMinAzimuth(QString);

    void sendKeyerPlay( int fno );
    void sendKeyerRecord( int fno );
    void sendBandMap( QString freq, QString call, QString utc, QString loc, QString qth );
    void sendKeyerTone();
    void sendKeyerTwoTone();
    void sendKeyerStop();
    void sendRotator(rpcConstants::RotateDirection direction, int angle );
    void sendRadioFreq(QString);
    void sendRadioMode(QString);

    void sendSelectRadio(QString);
    void sendSelectRotator(QString);


    void on_ControlSplitter_splitterMoved(int pos, int index);

    void setAuxWindows();

};

#endif // TSINGLELOGFRAME_H
