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

enum ClusterTabIndex {DXSPOT_TAB, SEARCH_TAB, CALLSIGN_TAB, LOCATOR_TAB};

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
    BaseContestLog *ct = nullptr;
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

    QVector<DxSpotSortFilterProxyModel*> filterProxyModelList;

    QTableView* dxSpotView;
    QTableView* searchView;
    QTableView* callSignView;
    QTableView* locatorView;

    QVector<QTableView*> spotViewList;

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
    void sendBrgToRot(QString brg);
    memoryData::memData getSpotDataToMemoryVariable(DxSpotSortFilterProxyModel*, int);
    void calcSpotDistanceBearing(const QString &locator, double *distance, int *bearing);

    void sendSpotToMemory(DxSpotSortFilterProxyModel *spotProxyModel, int row);

    void handleClickedItems(DxSpotSortFilterProxyModel *spotProxyModel, const QModelIndex &index);
    void handleVertHeaderClickedItems(DxSpotSortFilterProxyModel *spotProxyModel, int row);
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
    void onCallsignSpotVertHeaderClicked(int);
    void onSearchSpotVertHeaderClicked(int row);
    void checkSpotWorked(QString &callsign, QString &locator, bool *callWorked, bool *locatorWorked);
};

#endif // CLUSTERCLIENTFRAME_H
