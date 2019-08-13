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
#include "clustercommon.h"
#include "bandModeFrequencyPlan.h"
#include "MinosLoggerEvents.h"

namespace Ui {
    class ClusterClientFrame;
}

enum ClusterTabIndex {DXSPOT_TAB, SEARCH_TAB, CALLSIGN_TAB, LOCATOR_TAB};


const int MOUSE_IN_FRAME_TIMEOUT = 10000;
const int CHECKSPOTS_DURATION = 1000;
const int CHECK_NEWFILTERS_DURATION = 1000;

class ClusterClientFrame;






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

class MouseInObject;


class ClusterClientFrame : public QFrame
{
    Q_OBJECT

    void traceMsg(QString msg);
public:
    explicit ClusterClientFrame(QWidget *parent);
    ~ClusterClientFrame() override;

    void setContest(BaseContestLog *contest);

    void mouseMoveEvent(QMouseEvent *event) override;


    void setHoldUpdateFlag(bool state);

    bool isSpotQueueEmpty();
    QTimer* mouseInFrameTimer;
    void buttonHandleDxSpots();
    void restoreColumns();

private:
    Ui::ClusterClientFrame *ui;
    BaseContestLog *ct = nullptr;



    ClusterClientFilterDialog *filterSetup;

    BandModeFrequencyPlan *modeBandPlan;

    HtmlDelegate *delegate = nullptr;

    QTimer* purgeTimer;
    qlonglong timeToLive;
    bool purgeSpotFlag;
    bool holdUpdateFlag;

    QTimer* checkNewSpotsTimer;
    QTimer* checkNewFilters;

    MouseInObject* actionInObject;

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
    QAction* memoryActionOveride;

    QString contestUuid;

    QString contestBandStr;
    int contestBand;

    QString contestModeStr;
    int contestMode;

    bool isProtected;

    void restoreCallsignViewColumns();
    void restoreLocatorViewColumns();
    void restoreSearchViewColumns();

    void restoreDxSpotViewColumns();

    void addDxSpotToTable(const QString spot);


    void handleDxSpots(QVector<QString> &spotQueue);

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
    //void queueIndToggle(bool on);
    void newCallsignSpotIndToggle(bool on);
    void newLocatorSpotIndToggle(bool on);
    void statusIndicatorToggle(bool on);


    //void on_AfterLogContact(BaseContestLog *c, Callsign cs, Locator loc);

    bool event(QEvent *event) override;

    void newSpotIndToggle(bool on);
    int getBandOffSet(QString contestBandStr);

    int getModeOffSet(QString contestModeStr);
    void handleClusterStatusMessage(QString &msg);

    int getNumberSpotsIndicator(const QDateTime& lastTime, DxSpotSortFilterProxyModel *spotProxyModel);
    void restoreSplitters();
private slots:

    void on_AfterLogContact(BaseContestLog *c, Callsign cs, QString loc);

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
    void delayed_afterLogContact(BaseContestLog *c, Callsign cs, QString loc);
    void onDxSpotViewClicked(const QModelIndex &);
    void onSearchSpotViewClicked(const QModelIndex &);
    void onCallsignSpotViewClicked(const QModelIndex &index);
    void onLocatorSpotViewClicked(const QModelIndex &index);
    void onSpotTabChanged(int index);
    void filtersChanged(bool bandfilterChanged, bool modefilterChanged,  bool callsignfilterChanged, bool locatorfilterChanged);
    void onSearchEditingFinished();
    void onDXSpotVertHeaderClicked(int row);
    void onLocatorSpotVertHeaderClicked(int row);
    void onCallsignSpotVertHeaderClicked(int);
    void onSearchSpotVertHeaderClicked(int row);
    void checkSpotWorked(QString &callsign, QString &locator, bool *callWorked, bool *locatorWorked);
    void checkNewSpots();
    void checkSavedFilters();
    void mouseTimerCheckNewSpots();

    void on_dxSpotViewSectionResized(int, int , int);
    void on_callsignViewSectionResized(int, int , int);
    void on_locatorViewSectionResized(int, int , int);
    void on_searchViewSectionResized(int, int , int);

    void on_clusterSplitter_splitterMoved(int pos, int index);

    void memoryActionOverideSelected();

    void on_doColumnChanges(BaseContestLog *);
    void on_doSplitterChanges(BaseContestLog *);
};

class MouseInObject : public QObject
{
    Q_OBJECT
public:
    MouseInObject(QWidget *parent, ClusterClientFrame* frame)
    {
        Q_UNUSED(parent)
        clusterFrame = frame;
    }




bool eventFilter(QObject *obj, QEvent *event)
{


    if (event->type() == QEvent::Enter)
    {
        clusterFrame->setHoldUpdateFlag(true);
    }
    else if (event->type() == QEvent::Leave)
    {
        clusterFrame->mouseInFrameTimer->stop();
        if (!clusterFrame->isSpotQueueEmpty())
        {
            clusterFrame->buttonHandleDxSpots();
        }
        clusterFrame->setHoldUpdateFlag(false);

    }

    return QObject::eventFilter(obj, event);
}



private:

ClusterClientFrame* clusterFrame;


};



#endif // CLUSTERCLIENTFRAME_H
