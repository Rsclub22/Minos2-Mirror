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
#include "clusterclientfilterdialog.h"
#include "MinosLoggerEvents.h"

namespace Ui {
    class ClusterClientFrame;
}


class DxSpotSortFilterProxyModel : public QSortFilterProxyModel
{
public:


    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool matchBand(int sourceRow) const;

    DxSpotSortFilterProxyModel(ClusterClientFilterDialog* _filterSetup)
    {
        filterSetup = _filterSetup;
    }

    ClusterClientFilterDialog* filterSetup;

};


class SearchSortFilterProxyModel : public DxSpotSortFilterProxyModel
{
public:


    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;


    SearchSortFilterProxyModel(ClusterClientFilterDialog* _filterSetup) : DxSpotSortFilterProxyModel(_filterSetup)
    {

    }

    QString searchParameter;

};



class CallsignSortFilterProxyModel : public DxSpotSortFilterProxyModel
{
public:


    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;


    CallsignSortFilterProxyModel(ClusterClientFilterDialog* _filterSetup) : DxSpotSortFilterProxyModel(_filterSetup)
    {

    }

};

class LocatorSortFilterProxyModel : public DxSpotSortFilterProxyModel
{
public:


    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;


    LocatorSortFilterProxyModel(ClusterClientFilterDialog* _filterSetup) : DxSpotSortFilterProxyModel(_filterSetup)
    {
        filterSetup = _filterSetup;
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
    ClusterClientFilterDialog *filterSetup;

    QTimer* purgeTimer;
    int timeToLive;
    bool purgeSpotFlag;

    QVector<QString> spotQueue;

    DxSpotDataModel* dxSpotDataModel;
    DxSpotSortFilterProxyModel* dxSpotProxyModel;
    SearchSortFilterProxyModel* searchSortProxyModel;
    CallsignSortFilterProxyModel* callSignProxyModel;
    LocatorSortFilterProxyModel* locatorProxyModel;

    QTableView* dxSpotView;
    QTableView* searchView;
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
    void setupSearchSpotView();
    void setupDXSpotView();
    void setupCallsignSpotView();
    void setupLocatorSpotView();
    void sendFreqToRig(QString freq);
    memoryData::memData getSpotDataToMemoryVariable(int);
private slots:


    void clusterClientServerList(QVector<ClusterServer>);
    void dxSpots(QVector<QString>);
    void on_FontChanged();
    void filterButtonSelected();
    void purgeSpots();
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
    void onDxSpotViewClicked(const QModelIndex &);
    void onSearchSpotViewClicked(const QModelIndex &);
    void onCallsignSpotViewClicked(const QModelIndex &index);
    void onLocatorSpotViewClicked(const QModelIndex &index);
    void onSpotTabChanged(int index);
    void filtersChanged(int);
    void onSearchEditingFinished();
    void onDXSpotVertHeaderClicked(int row);
    void onLocatorSpotVertHeaderClicked(int row);
    void onCallsignSpotVertHeaderClicked(int row);
    void onSearchSpotVertHeaderClicked(int row);
};

#endif // CLUSTERCLIENTFRAME_H
