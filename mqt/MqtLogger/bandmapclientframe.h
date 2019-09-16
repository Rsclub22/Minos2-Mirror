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



namespace Ui {
    class BandmapClientFrame;
}


class BMP_MouseInObject;


class LoggerSpots
{
public:

    LoggerSpots(Callsign _cs, QString _loc,  QString _brg,
                QString _modeStr, QString _modeMask,
                QString _freq, QString _bandStr, QString _bandMask,
                bool _worked, QDateTime _time, bandmapSpotType::SPOT_TYPE _spotType )
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
        spotType = _spotType;

    }

    Callsign getCallsign(){ return cs;}
    void setCallsign(Callsign _cs){cs = _cs;}

    QString getLocator(){return loc;}
    void setLocator(QString _loc){loc = _loc;}

    QString getBearing(){return brg;}
    void setBearing(QString _brg){brg = _brg;}

    QString getFreq(){return freq;}
    void setFreq(QString _freq){freq = _freq;}

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

    bandmapSpotType::SPOT_TYPE getSpotType(){return spotType;}
    void setSpotType(bandmapSpotType::SPOT_TYPE _spotType){spotType = _spotType;}

private:

    Callsign cs;
    QString loc;
    QString brg;
    QString modeStr;
    QString modeMask;
    QString freq;
    QString bandStr;
    QString bandMask;
    bool worked;
    QDateTime time;
    bandmapSpotType::SPOT_TYPE spotType;
};



class BandmapClientFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BandmapClientFrame(QWidget* parent);
    ~BandmapClientFrame() override;

    QTimer* mouseInFrameTimer;

    void setFreq(QString);
    void setContest(BaseContestLog *c);
    void setHoldUpdateFlag(bool state);
    bool isSpotQueueEmpty();
    void buttonHandleDxSpots();
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseTimerCheckNewSpots();
    void setBandmapMarkFreq(QString cs, QString freq, QString loc, QString brg);
    void setBandmapSaveFreq(QString cs, QString freq, QString loc, QString brg);

    void setRotatorBearing(QString s);
    void setRotatorConnected(bool connected);
    void updateZoom(bool dir);
private:

    Ui::BandmapClientFrame *ui;
    bool isProtected;
    BaseContestLog *ct = nullptr;
    QString contestUuid;
    QString contestBandStr;
    int contestBand;
    QString contestModeStr;
    int contestMode;

    QString lastfreq;


    QTimer* purgeTimer;
    QTimer* checkNewSpotsTimer;
    QTimer* checkNewFilters;

    QVector<BandDetail*> bands;
    checkModeAgainstFreq* modeBandPlan;

    BMP_MouseInObject* actionInObject;


    // cluster spots
    QVector<QString> spotQueue;

    // spots from logger
    QVector<LoggerSpots*> logSpotQueue;

    BandmapView *bandmapView;
    QItemSelectionModel *selectionModel;
    QGraphicsView* bandmapGraphicsView;

    QSortFilterProxyModel* bandmapSpotProxyModel;
    BandmapClientFilterDialog* filterSetup;

    QString sfreq;
    double curFreq = 0;
    int mapViewHeight = 0;

    BandmapDataModel *bandmapDataModel;
    BandmapData *bandmapData;

    QMenu* spotsMenu;
    QAction* markSpotAction;
    QAction* unMarkSpotAction;
    QAction* freqAction;
    QAction* bearingAction;
    QAction* logAction;
    QAction* memoryAction;
    QAction* clearSpotAction;
    BandmapData selectedSpotData;
    int selectedSpotRowNum;

    bool purgeSpotFlag;
    bool holdUpdateFlag;
    qlonglong timeToLive;

    QString curRotBearing;
    bool rotatorConnected;

    int getBandOffSet(QString contestBandStr);
    int getModeOffSet(QString contestModeStr);
    void handleDxSpots(QVector<QString> &spotQueue);
    void handleClusterStatusMessage(QString &msg);
    void statusIndicatorToggle(bool on);
    void addDxSpotToBandmapTable(const QString spot);
    void calcSpotDistanceBearing(const QString &_locator, double *distance, int *bearing);
    void checkSpotWorked(QString &callsign, QString &locator, bool *callWorked, bool *locatorWorked);
    bool checkSpotInTable(QStringList &sl);
    void sendFreqToRig(QString freq);


    bool event(QEvent *event) override;
    int findRowToInsert(QString f);
protected:


private slots:

     void clusterClientServerList(QVector<ClusterServer>);
     void dxSpots(QVector<QString>);


     void on_FontChanged();

     void checkNewBandMapSpots();

     void on_contextMenuSelected(const QPoint &pos);
     void on_freqActionSelected();
     void bearingActionSelected();
     void logActionSelected();
     void memoryActionSelected();
     void clearSpotActionSelected();
     void sendBrgToRot(QString brg);
     void on_AfterLogContact(BaseContestLog *c, Callsign cs, QString loc, QString brg, QString freq);
     void filterButtonSelected();
     void checkSavedFilters();
     void onMenuShow();

     void on_FitersChanged(bool state);
     void purgeSpots();
     void on_markSpotActionSelected();
     void on_unMarkSpotActionSelected();
     void addLogSpotToBandmapTable(LoggerSpots *spot);
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
            if (!bandmapFrame->isSpotQueueEmpty())
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
