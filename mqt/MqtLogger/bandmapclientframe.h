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
#include <QSortFilterProxyModel>
#include <QFrame>
#include "dxspotdatamodel.h"
#include "base_pch.h"
#include "clusterClientServer.h"
#include "clusterclientfilterdialog.h"
#include "clustercommon.h"
#include "contest.h"
#include "ContestApp.h"
#include "bandModeFrequencyPlan.h"
#include "MinosLoggerEvents.h"
#include "bandmapcallsignmarker.h"
//#include "bandmap.h"
#include "bandmapview.h"
#include "bandmapfreqdial.h"
#include "bandmapdatamodel.h"
#include "bandmapclientfilterdialog.h"

namespace Ui {
    class BandmapClientFrame;
}


class BandmapSpotSortFilterProxyModel : public QSortFilterProxyModel
{
public:

    BandmapSpotSortFilterProxyModel(BandmapClientFilterDialog* _filterSetup)
    {
        filterSetup = _filterSetup;
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool matchBand(int sourceRow) const;



    BandmapClientFilterDialog* filterSetup;

};





class BandmapClientFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BandmapClientFrame(QWidget* parent);
    ~BandmapClientFrame() override;


    void setFreq(QString);
    void setContest(BaseContestLog *c);
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

    // cluster spots
    QVector<QString> spotQueue;

    BandmapView *bandmapView;
    QGraphicsView* bandmapGraphicsView;

    BandmapSpotSortFilterProxyModel* bandmapSpotProxyModel;
    BandmapClientFilterDialog* filterSetup;

    QString sfreq;
    double curFreq = 0;
    int mapViewHeight = 0;

    BandmapDataModel *bandmapDataModel;
    BandmapData *bandmapData;

    bool purgeSpotFlag;
    bool holdUpdateFlag;
    qlonglong timeToLive;

    int getBandOffSet(QString contestBandStr);
    int getModeOffSet(QString contestModeStr);
    void handleDxSpots(QVector<QString> &spotQueue);
    void handleClusterStatusMessage(QString &msg);
    void statusIndicatorToggle(bool on);
    void addDxSpotToBandmapTable(const QString spot);
    void calcSpotDistanceBearing(const QString &_locator, double *distance, int *bearing);
    void checkSpotWorked(QString &callsign, QString &locator, bool *callWorked, bool *locatorWorked);
protected:


private slots:

     void clusterClientServerList(QVector<ClusterServer>);
     void dxSpots(QVector<QString>);


     void on_FontChanged();

     void checkBandMapSpots();
};

#endif // BANDMAPCLIENTFRAME_H
