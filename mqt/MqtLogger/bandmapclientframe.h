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
#include "dxspotdatamodel.h"
#include "base_pch.h"
#include "clusterClientServer.h"
#include "clusterclientfilterdialog.h"
#include "clustercommon.h"
#include "contest.h"
#include "ContestApp.h"
#include "MinosLoggerEvents.h"
#include "bandmapcallsignmarker.h"
//#include "bandmap.h"
#include "bandmapview.h"
#include "bandmapfreqdial.h"
#include "bandmapdatamodel.h"
#include "bandmapclientfilterdialog.h"
#include "BandList.h"
#include "checkmodeagainstfreq.h"
#include "checkoperatingfreq.h"



namespace Ui {
    class BandmapClientFrame;
}


class BMP_MouseInObject;

/*
class LoggerSpots
{
public:

    LoggerSpots(Callsign _cs, QString _loc,  QString _brg,
                QString _modeStr, QString _modeMask,
                Frequency _freq, QString _bandStr, QString _bandMask,
                bool _worked, QDateTime _time, bool _runModeOn, bool _offRunFreq, bandmapSpotType::SPOT_TYPE _spotType )
    {
        cs = _cs;
        loc = _loc;
        brg = _brg;
        freq = _freq;
        bandStr = _bandStr;
        bandMask = _bandMask;
        modeStr = _modeStr;
        modeMask = _modeMask;
        worked = _worked;
        time = _time;
        runModeOn = _runModeOn;
        offRunFreq = _offRunFreq;
        spotType = _spotType;

    }

    Callsign getCallsign(){ return cs;}
    void setCallsign(Callsign _cs){cs = _cs;}

    QString getLocator(){return loc;}
    void setLocator(QString _loc){loc = _loc;}

    QString getBearing(){return brg;}
    void setBearing(QString _brg){brg = _brg;}

    Frequency getFreq(){return freq;}
    void setFreq(Frequency _freq){freq = _freq;}

    QString getbandStr(){return bandStr;}
    void setBandStr(QString _bandStr){bandStr = _bandStr;}

    QString getBandMask(){return bandMask;}
    void setBandMask(QString _bandMask){bandMask = _bandMask;}


    QString getModeStr(){return modeStr;}
    void setModeStr(QString _modeStr){modeStr = _modeStr;}

    QString getModeMask(){return modeMask;}
    void setModeMask(QString _modeMask){modeMask = _modeMask;}

    bool getWorked(){return worked;}
    void setWorked(bool _worked){worked = _worked;}

    QDateTime getTime(){return time;}
    void setTime(QDateTime _time){time = _time;}

    bool getRunModeOn(){return runModeOn;}
    void setRunModeOn(bool _runModeOn){runModeOn = _runModeOn;}


    bool getOffRunFreq(){return offRunFreq;}
    void setOffRunFreq(bool _offRunFreq){offRunFreq = _offRunFreq;}


    bandmapSpotType::SPOT_TYPE getSpotType(){return spotType;}
    void setSpotType(bandmapSpotType::SPOT_TYPE _spotType){spotType = _spotType;}

private:

    Callsign cs;
    QString loc;
    QString brg;
    QString modeStr;
    QString modeMask;
    Frequency freq;
    QString bandStr;
    QString bandMask;
    bool worked;
    QDateTime time;
    bool runModeOn;
    bool offRunFreq;
    bandmapSpotType::SPOT_TYPE spotType;
};
*/
const QString MODE_BANDPLAN_FILE = "./Configuration/mode_bandplan.json";
const QString OPERATING_FREQ_FILE = "./Configuration/operating_frequencies.json";
const QString BANDPLAN_FREQ_LIMITS_FILE = "./Configuration/bandmap_limits.ini";


class BandmapClientFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BandmapClientFrame(QWidget* parent);
    ~BandmapClientFrame() override;

    QTimer* mouseInFrameTimer;

    void setFreq(Frequency);
    void setContest(BaseContestLog *c);
    void setHoldUpdateFlag(bool state);
    bool getPurgeSpotFlag(){return purgeSpotFlag;}
    bool isSpotQueueEmpty();
    void buttonHandleDxSpots();
    void mouseMoveEvent(QMouseEvent *event) override;

    void setBandmapMarkFreq(QString cs, Frequency freq, QString loc, QString brg);
    void setBandmapSaveFreq(QString cs, Frequency freq, QString loc, QString brg);

    void setRotatorBearing(QString s);
    void setRotatorConnected(bool connected);
    void updateZoom(bool dir);

    void setClusterServerState(QString stateMsg);



    void setClusterServerLoaded(bool loaded);

    void saveTuneAddBandMapSetting(bool state);
    bool readTuneAddBandMapSetting();


    void setRunOnFlag(Frequency _runFreq, bool _runModeOn);
    void setRunOffFreqFlag(Frequency _runFreq, bool _offRunFreq);
    void setBandmapRadioIsConnect(bool state);
    void setBandmapRadioHasError(QString error);
    void setMode(QString mode);


signals:

    void freqDisplayClicked();

private:

    Ui::BandmapClientFrame *ui;
    bool isProtected;
    BaseContestLog *ct = nullptr;
    QString contestUuid;
    QString contestBandStr;
    int contestBand;
    Frequency contestBandFlow;
    Frequency contestBandFHigh;
    QString contestModeStr;
    int contestMode;

    QString radioMode;

    bool radioIsConnected;
    QString radioError;
    Frequency lastfreq;
    QPalette *freqDisplayPalette;
    bool legalFreq = true;


    QTimer* purgeTimer;
    QTimer* checkNewSpotsTimer;
    QTimer* checkNewFilters;

    QVector<QSharedPointer<BandInfo> > bands;
    checkModeAgainstFreq* modeBandPlan;
    bool modeBandPlanOk;

    BandmapClientFilterSettings filterSettings;

    CheckOperatingFreq* operatingFreq;
    bool operatingFreqPlanOk;

    BMP_MouseInObject* actionInObject;


    // cluster spots
    QVector<QString> spotQueue;
    bool clusterServerLoaded;
    bool clusterServerConnected;

    // CQ Frequency
    Frequency runFreq;
    bool runModeOn;
    bool offRunFreq;

    // spots from logger
    QVector<BandmapSpotData*> logSpotQueue;

    BandmapView *bandmapView;
    QItemSelectionModel *selectionModel;
    QGraphicsView* bandmapGraphicsView;

    BandmapSortFilterProxyModel* bandmapSpotProxyModel;
    BandmapClientFilterDialog* filterSetup;

    UpperCaseValidator ucValidator;

    //QString sfreq;
    Frequency curFreq;
    int mapViewHeight = 0;

    BandmapDataModel *bandmapDataModel;
    BandmapSpotData *bandmapData;

    QMenu* spotsMenu;
    QAction* markSpotAction;
    QAction* unMarkSpotAction;
    QAction* freqAction;
    QAction* bearingAction;
    QAction* logAction;
    QAction* memoryAction;
    QAction* saveZoomLevel;
    QAction* readSavedZoomLevel;

    QAction* clearSpotAction;
    QAction* clearAllSpotsAction;

    QMenu* contextSpotsMenu;
    QAction* contextSpotsMenu_markSpotAction;
    QAction* contextSpotsMenu_unMarkSpotAction;
    QAction* contextSpotsMenu_freqAction;
    QAction* contextSpotsMenu_bearingAction;
    QAction* contextSpotsMenu_logAction;
    QAction* contextSpotsMenu_memoryAction;
    QAction* contextSpotsMenu_clearSpotAction;

    QShortcut* zoomIn;
    QShortcut* zoomOut;


    //BandmapData actionMenuSelectedSpotData;
    //int actionMenuSelectedSpotDataRowNum;
    BandmapSpotData contextMenuSelectedSpotData;
    int contextMenuSelectedSpotDataRowNum;

    bool purgeSpotFlag;
    bool holdUpdateFlag;
    qlonglong timeToLive;

    QString curRotBearing;
    bool rotatorConnected;



    //int getBandOffSet(QString contestBandStr);
    int getModeOffSet(QString contestModeStr);
    void handleDxSpots(QVector<QString> &spotQueue);
    //void handleClusterStatusMessage(QString &msg);
    void clusterStatusIndicatorToggle(bool on);
    void addDxSpotToBandmapTable(const QString spot);
    void calcSpotDistanceBearing(const QString &_locator, double *distance, int *bearing);
    void checkSpotWorked(QString &callsign, QString &locator, bool *callWorked, bool *locatorWorked);
    bool checkSpotInTable(QStringList &sl);
    void sendFreqToRig(Frequency freq);

    bool event(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    int findRowToInsert(QString f);
    bool isFreqLegal(const Frequency &freq, const QString band, const QString mode);

    QString readBandmapFreqLimit(QString band, QString mode);
    void getBandLimitsFromBandListXML();
    void traceMsg(QString msg);

    void setCQFreq();
    void addRemoveCQSpot(const BandmapSpotData *spot);

    void radioStatusIndicatorToggle(bool on);
    bool checkContestBandMatch(Frequency curFreq);
    void saveBandmapZoomLevel(int &level);
    int readBandmapZoomLevel();
    void setZoomLevelLabelText(int level);

protected:


private slots:

     void clusterClientServerList(QVector<ClusterServer>);
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
     void on_AfterLogContact(BaseContestLog *c, Callsign cs, QString loc, QString brg, Frequency freq, QString mode, QString districtMult);
     void filterButtonSelected();
     void checkSavedFilters();
     void onMenuShow();

     void on_FiltersChanged(bool state);
     void purgeSpots();
     void on_markSpotActionSelected();
     void on_unMarkSpotActionSelected();
     void addLogSpotToBandmapTable(const BandmapSpotData *spot);
     void mouseTimerCheckNewSpots();
     void on_FreqDisplayClicked();



     void context_markSpotActionSelected();
     void context_unMarkSpotActionSelected();
     void context_freqActionSelected();
     void context_logActionSelected();
     void context_memoryActionSelected();
     void context_clearSpotActionSelected();
     void on_clearAllSpotsActionSelected();
     void context_bearingActionSelected();

     void on_zoomIn();
     void on_zoomOut();

     void on_saveZoomLevelActionSelected();
     void on_readZoomLevelActionSelected();
     void on_newZoomlevel(int level);
     void requestSpots();
     void on_pushbuttonPressed();
     void on_textFilterEdit_textChanged(const QString &arg1);
};



class BMP_MouseInObject : public QObject
{
    Q_OBJECT
public:
    BMP_MouseInObject(QWidget *parent, BandmapClientFrame* frame)
    {
        Q_UNUSED(parent)
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

            bandmapFrame->setHoldUpdateFlag(false);

        }

        return QObject::eventFilter(obj, event);
    }



private:

BandmapClientFrame* bandmapFrame;


};



#endif // BANDMAPCLIENTFRAME_H
