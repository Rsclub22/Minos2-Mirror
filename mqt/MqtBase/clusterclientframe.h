////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Client
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef CLUSTERCLIENTFRAME_H
#define CLUSTERCLIENTFRAME_H

#include <QObject>
#include <QFrame>
#include "dxspotdatamodel.h"
#include "base_pch.h"
#include "clusterClientServer.h"
#include "clusterclientfiltertab.h"
#include "MinosLoggerEvents.h"

namespace Ui {
    class ClusterClientFrame;
}


class ClusterClientFrame : public QFrame
{
    Q_OBJECT

    void traceMsg(QString msg);
public:
    explicit ClusterClientFrame(QWidget *parent);
    ~ClusterClientFrame() override;
    void setStandAlone();
    void setContest(BaseContestLog *contest);




private:
    Ui::ClusterClientFrame *ui;
    BaseContestLog *contest;
    ClusterClientFilterTab *filterSetup;

    QTimer* purgeTimer;
    int timeToLive;
    bool purgeSpotFlag;

    QVector<QString> spotQueue;

    DxSpotDataModel* dxSpotDataModel;
    QTableView* dxSpotView;
    void restoreDxSpotViewColumns();

    void addDxSpotToTable(QString spot);
    bool spotTimedOut(QString spotTime);

    void handleDxSpots(QVector<QString> spotQueue);

private slots:


    void clusterClientServerList(QVector<ClusterServer>);
    void dxSpots(QVector<QString>);
    void on_FontChanged();
    void filterButtonSelected();
    void purgeSpots();
};

#endif // CLUSTERCLIENTFRAME_H
