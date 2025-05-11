////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Bandmap Client
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef BANDMAPCLIENTFRAME_H
#define BANDMAPCLIENTFRAME_H

#include <QObject>
#include <QFrame>
#include <QShortcut>
#include "clustercommon.h"
#include "contest.h"
#include "bandmapview.h"
#include "bandmapdatamodel.h"
#include "../MqtBase/bandmapclientfilterdialog.h"
#include "BandList.h"
#include "checkmodeagainstfreq.h"
#include "checkoperatingfreq.h"
#include "cutils.h"
#include "bandmapspotdb.h"



namespace Ui {
    class BandmapClientFrame;
}


class BMP_MouseInObject;

class BandmapClientFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BandmapClientFrame(QWidget* parent);
    ~BandmapClientFrame() override;

    QTimer* mouseInFrameTimer = nullptr;

    void refreshLocalSpots();

    void setFreq(Frequency);
    void setContestBandMode(QString band, QString mode);
    void setContest(BaseContestLog *c);
    void setHoldUpdateFlag(bool state);
    bool getPurgeSpotFlag(){return purgeSpotFlag;}
    void buttonHandleDxSpots();
    void mouseMoveEvent(QMouseEvent *event) override;

    void setBandmapMarkFreq(Frequency freq, QString mode);
    void setBandmapSaveFreq(QString cs, Frequency freq, QString mode, QString loc, QString brg, QString exchange);

    void setRotatorBearing(QString s);
    void setRotatorConnected(bool connected);
    void updateZoom(bool dir);

    void setClusterServerState(QString stateMsg);

    void setRunOnFlag(Frequency _runFreq, QString mode, bool _runModeOn);
    void setRunOffFreqFlag(Frequency _runFreq, bool _offRunFreq);
    void setBandmapRadioIsConnect(bool state);
    void setBandmapRadioHasError(QString error);
    void setMode(QString mode);


    void checkLegalFrequencies(Frequency freq);
    
signals:

    void freqDisplayClicked();

private:

    Ui::BandmapClientFrame *ui;
    BaseContestLog *ct = nullptr;
    QString contestBandStr;
    Frequency contestBandFlow;
    Frequency contestBandFHigh;
    QString contestModeStr;
    int contestMode = -1;

    QString radioMode;

    QString radioError;
    Frequency lastfreq;
    QPalette *freqDisplayPalette = nullptr;
    bool legalFreq = true;


    QTimer* purgeTimer = nullptr;
    QTimer* checkNewSpotsTimer = nullptr;
    QTimer* waitClusterServerLoadedTimer = nullptr;


    QVector<QSharedPointer<BandInfo> > bands;

    checkModeAgainstFreq* modeBandPlan = nullptr;
    bool modeBandPlanOk = false;

    CheckOperatingFreq* operatingFreqExclusions = nullptr;
    bool operatingFreqExclusionsPlanOk = false;

    BandmapClientFilterSettings filterSettings;

    BMP_MouseInObject* actionInObject = nullptr;


    // cluster spots
    QVector<QSharedPointer<ClusterSpotData> > spotQueue;
    bool clusterServerConnected = false;

    // CQ Frequency
    Frequency runFreq;
    QString runMode;
    bool runModeOn = false;
    bool offRunFreq = false;

    // spots from logger
    QSharedPointer<BandMapSpotDB> bmsdb;
    QVector<QSharedPointer<ClusterSpotData> > logSpotQueue;

    BandmapView *bandmapView = nullptr;
    QItemSelectionModel *selectionModel = nullptr;

    BandmapData *bandmapDataModel = nullptr;
    //BandmapSortFilterProxyModel* bandmapSpotProxyModel = nullptr;
    BandmapClientFilterDialog* filterSetup = nullptr;

    UpperCaseValidator ucValidator;

    Frequency curFreq;

    QMenu* spotsMenu = nullptr;
    QAction* markSpotAction = nullptr;
    QAction* unMarkSpotAction = nullptr;
    QAction* freqAction = nullptr;
    QAction* bearingAction = nullptr;
    QAction* logAction = nullptr;
    QAction* memoryAction = nullptr;
    QAction* saveZoomLevel = nullptr;
    QAction* readSavedZoomLevel = nullptr;
    QAction* resendSpotsAction = nullptr;
    QAction* clearSpotAction = nullptr;
    QAction* clearAllSpotsAction = nullptr;
    QAction* clearClusterSpotsAction = nullptr;

    QMenu* contextSpotsMenu = nullptr;
    QAction* contextSpotsMenu_markSpotAction = nullptr;
    QAction* contextSpotsMenu_unMarkSpotAction = nullptr;
    QAction* contextMoveFreqAction = nullptr;
    QAction* contextSpotsMenu_freqAction = nullptr;
    QAction* contextSpotsMenu_bearingAction = nullptr;
    QAction* contextSpotsMenu_logAction = nullptr;
    QAction* contextSpotsMenu_memoryAction = nullptr;
    QAction* contextSpotsMenu_clearSpotAction = nullptr;

    QShortcut* zoomIn = nullptr;
    QShortcut* zoomOut = nullptr;


    ClusterSpotData contextMenuSelectedSpotData = bandmapSpotType::NONE;
    int contextMenuSelectedSpotDataRowNum = -1;

    bool purgeSpotFlag = false;
    bool holdUpdateFlag = false;
    qlonglong timeToLive = 0;

    QString curRotBearing;
    bool rotatorConnected = false;

    int getModeOffSet(QString contestModeStr);
    void handleDxSpots(QVector<QString> &spotQueue);
    void clusterStatusIndicatorToggle(bool on);
    void addDxSpotToBandmapTable(QSharedPointer<ClusterSpotData> spot);
    bool checkSpotInTable(QSharedPointer<ClusterSpotData> newSpot);
    void sendFreqToRig(Frequency freq);

    bool event(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    bool isFreqLegal(const Frequency &freq, const QString band, const QString mode);

    void getBandLimitsFromBandListXML();
    void traceMsg(QString msg);

    void setCQFreq();
    void addRemoveCQSpot(QSharedPointer<ClusterSpotData> spot);

    void radioStatusIndicatorToggle(bool on);
    bool checkContestBandMatch(Frequency curFreq);
    void saveBandmapZoomLevel(int &level);
    int readBandmapZoomLevel();
    void setZoomLevelLabelText(int level);

    void setTextToFrameTitle(QString text1, QString col, QString text2);
    void readDefaultDistanceFilterSettings(BandmapClientFilterSettings *filterSettings);

    void ShowFilter();

    void restoreSplitters();
    void doMarkSpot(int selRow);
    void doUnMarkSpot(int selRow);
    void doBearingSelected(ClusterSpotData * );
    void doLogSelected(ClusterSpotData *sd);
    void doMemorySelected(ClusterSpotData *d);
    void doClearSpotSelected(ClusterSpotData *sd, int selRow);
    void doClearClusterSpotsSelected();
protected:

    void keyPressEvent(QKeyEvent *event) override;

private slots:

     void dxSpots(QVector<ClusterMessage>);


     void on_FontChanged();

     void checkNewBandMapSpots();
     void timerCheckNewBandMapSpots();

     void on_contextMenuSelected(const QPoint &pos, const QPoint &mapP);
     void on_freqActionSelected();
     void on_bearingActionSelected();
     void on_logActionSelected();
     void on_memoryActionSelected();
     void on_clearSpotActionSelected();
     void sendBrgToRot(QString brg);
     void filterButtonSelected();
     void onMenuShow();

     void on_FiltersChanged(bool state);
     void purgeSpots();
     void on_markSpotActionSelected();
     void on_unMarkSpotActionSelected();
     void addLogSpotToBandmapTable(QSharedPointer<ClusterSpotData> spot);
     void mouseTimerCheckNewSpots();
     void on_FreqDisplayClicked();

     void context_markSpotActionSelected();
     void context_unMarkSpotActionSelected();
     void context_moveFreqActionSelected();
     void context_freqActionSelected();
     void context_logActionSelected();
     void context_memoryActionSelected();
     void context_clearSpotActionSelected();
     void on_clearAllSpotsActionSelected();
     void on_clearClusterSpotsActionSelected();
     void context_bearingActionSelected();

     void on_zoomIn();
     void on_zoomOut();

     void on_saveZoomLevelActionSelected();
     void on_readZoomLevelActionSelected();
     void on_newZoomlevel(int level);
     void on_resendClusterSpotSelected();
     void on_textFilterEdit_textChanged(const QString &arg1);

     void on_waitClusterServerLoadedTimeout();
     void on_clusterStatusIndicatorClicked();
     void onContestBandChanged(BaseContestLog *ct);
     void on_zoomSpinner_valueChanged(int arg1);

     void on_bandmapLimitsChanged();
     void on_doSplitterChanges(BaseContestLog *b);
     void on_bmSplitter_splitterMoved(int, int);
 public slots:
     void on_AfterLogContact(BaseContestLog *c, QSharedPointer<BaseContact>);
};



class BMP_MouseInObject : public QObject
{
    Q_OBJECT
public:
    BMP_MouseInObject(QWidget */*parent*/, BandmapClientFrame* frame)
    {
        bandmapFrame = frame;
    }




    bool eventFilter(QObject *obj, QEvent *event)
    {


        if (event->type() == QEvent::Enter)
        {
            bandmapFrame->setHoldUpdateFlag(true);
        }
        else if (event->type() == QEvent::Leave)
        {
            bandmapFrame->mouseInFrameTimer->stop();
            if (!bandmapFrame->getPurgeSpotFlag())
            {
               bandmapFrame->buttonHandleDxSpots();
            }

            // come what may, release on exit
            // (Have seen it not happen on Mac)
            bandmapFrame->setHoldUpdateFlag(false);

        }

        return QObject::eventFilter(obj, event);
    }



private:

BandmapClientFrame* bandmapFrame;


};



#endif // BANDMAPCLIENTFRAME_H
