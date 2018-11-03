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

class CallsignSortFilterProxyModel : public QSortFilterProxyModel
{
public:


    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    CallsignSortFilterProxyModel()
    {
    }


};

class LocatorSortFilterProxyModel : public QSortFilterProxyModel
{
public:


    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    LocatorSortFilterProxyModel()
    {
    }


};


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
    BaseContestLog *ct;
    ClusterClientFilterTab *filterSetup;

    QTimer* purgeTimer;
    int timeToLive;
    bool purgeSpotFlag;

    QVector<QString> spotQueue;

    DxSpotDataModel* dxSpotDataModel;
    CallsignSortFilterProxyModel* callSignProxyModel;
    LocatorSortFilterProxyModel* locatorProxyModel;

    QTableView* dxSpotView;
    QTableView* callSignView;
    QTableView* locatorView;

    QMenu* spotsMenu;
    QAction* freqAction;
    QAction* bearingAction;
    QAction* logAction;
    QAction* memoryAction;
    QAction* clearSpotAction;
    QAction* clearAllSpotsAction;


    void restoreDxSpotViewColumns();

    void addDxSpotToTable(QString spot);
    bool spotTimedOut(QString spotTime);

    void handleDxSpots(QVector<QString> spotQueue);

    void setAllTabsColor(QColor c);
private slots:


    void clusterClientServerList(QVector<ClusterServer>);
    void dxSpots(QVector<QString>);
    void on_FontChanged();
    void filterButtonSelected();
    void purgeSpots();
    void onDxSpotView_doubleClicked(const QModelIndex &);
    void onMenuShow();
    void on_freqActionSelected();
    void bearingActionSelected();
    void logActionSelected();
    void memoryActionSelected();
    void clearSpotActionSelected();
    void clearAllSpotsActionSelected();
    void on_AfterLogContact(BaseContestLog *c);
    void restoreCallsignViewColumns();
    void restoreLocatorViewColumns();
    void onCallsignSpotView_doubleClicked(const QModelIndex &index);
    void onLocatorSpotView_doubleClicked(const QModelIndex &index);
    void onSpotTabChanged(int index);
};

#endif // CLUSTERCLIENTFRAME_H
