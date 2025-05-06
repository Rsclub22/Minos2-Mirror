#ifndef BANDMAPVIEW_H
#define BANDMAPVIEW_H

#include <QWidget>
#include <QTimer>

#include <QGraphicsView>
#include <QScrollArea>
#include <QScrollBar>
#include <QGraphicsScene>

#include "bandmapfreqdial.h"
#include "bandmapmarkerdetails.h"
#include "clustercommon.h"
#include "spotbasedata.h"
#include "bandmapgraphicspanel.h"

const QChar DEG_SYMBOL(0260); // octal value
const int NO_SELECTED_ROWNUM = -1;

const int KEY_SCROLL_STEP_SIZE = 50;

class BaseContestLog;
class BandmapData;
class BandmapSortFilterProxyModel;
class BandmapView : public QScrollArea
{
    Q_OBJECT

public:
    explicit BandmapView(QWidget *parent = nullptr);
    ~BandmapView() override;

    BandmapData *bandmapDataModel = nullptr;


    QRect visualRect(const QModelIndex &index) const;

    void setContest(BaseContestLog *c);

    void setFreq(Frequency f, bool legalFreq);

    int getBandmapFrameHeight();
    int getBandmapFrameWidth();
    void onFontChanged(QFont cf);
    void initBandmapView(BandmapGraphicsPanel *view);

    void bandmapUpdate(bool now);

    int isClickInRegionOfSpot(QPoint p);


    void getSpotData(int &selectedSpotDataRowNum, int displayedSpotNum, ClusterSpotData &selectedSpot);

    void clearSelectedSpotData();

    int getSelectedSpotViewRowNum(){return selectedSpotViewRowNum;}

    int getSelectedSpotDataRowNum(){return selectedSpotDataRowNum;}

    ClusterSpotData* getSelectedSpotDataPtr(){return &selectedSpot;}

    void clearSelectedSpot();

    void setFilterSettings(BandmapClientFilterSettings *filterSettings_);

    void updateZoom(bool dir);

    void setBandFreqLimits(Frequency flow, Frequency fhigh);
    void setBandmapHeight(Frequency flow, Frequency fhigh);
    void makeCursorVisibleInBandmap();

    void scrollBandmapCenterToFreq(Frequency freq);
    void setFreqOperatingInfo(const QString contestBandStr, const QString contestModeStr, CheckOperatingFreq *operatingFreq, const bool operatingPlanOk);

    int getDialZoomLevel();
    void setBandmapZoom(int level);

    void setDialRadioMode(QString mode);
    bool getSuppressUpdate() const;
    void setSuppressUpdate(bool value);

signals:

    void contextMenuSelected(const QPoint&, const QPoint&);
    void newZoomlevel(int);

protected:
    int horizontalOffset() const;
    int verticalOffset() const ;

private slots:
    void bandmapResize(QSize s);

    void leftMouseButtonPressed(QPoint p);
    void mouseDoubleClicked(QPoint p);

    void on_bandmap_customContextMenuRequested(const QPoint &p);

    void on_nextSpot(bool nextFreqUpDown, bool nextMult);
    int findNextNonWorkedLocatorUpList(int curSpotViewNum);

    void on_scrollMap(bool dir);
    void zoomUpdated(bool dir);
    void updateTimerTimeout();
private:

    QGraphicsScene *bandmapScene;
    BandmapGraphicsPanel* bandmapGraphicsView;

    BandmapFreqDial *dial;
    Frequency curFreq;
    QString curMode;

    QTimer updateTimer;
    bool updateRequired = false;
    bool suppressUpdate = false;
    int totalSize = 300; //for test

    int dialMinZoomLevel;
    int dialMaxZoomLevel;
    int zoomLevel;

    BaseContestLog *contest = nullptr;
    Frequency contestBandFlow;
    Frequency contestBandFhigh;
    Frequency curViewPortStartFreq;

    int fullBandHeight;
    int fontHeight;

    void drawBandMapSpots();
    QVector<BandmapMarkerDetails*> listOfMarkers;

    ClusterSpotData selectedSpot;
    int selectedSpotDataRowNum;
    int selectedSpotViewRowNum;

    BandmapClientFilterSettings* filterSettings;

    bool traceDebugFlag = false;

    bool lessGreaterThanDistanceFlag = false;
    QStringList nearMatches;

    void assembleSpotMsg(int row, QString& markerMsg);
    void assembleToolTip(int row, Frequency freq, QString& toolTipMsg);
    QRectF viewportRectForRow(int row) const;
    QRectF calculateSpotRect(const QString text, const QPoint spotCoord);
    void bandmapSelectFreq(int y);
    void bandmapSelectSpot(QPoint p);
    void setSelectedSpot(int displayedSpotNum);

    void clearListOfMarkers();
    bool matchMode(int sourceRow);
    int findNextOccupiedMarkerUpList(int curSpotViewNum);
    int findNextOccupiedMarkerDownList(int curSpotViewNum);
    int findNextNonWorkedLocatorDownList(int curSpotViewNum);
    int getViewPortStartYCoordOnScene();
    int getViewPortEndYCoordOnScene();
    void traceMsg(QString msg);
    void clearSpotData(ClusterSpotData &selectedSpot);
    void deleteItemsFromMarkerList();
    void assembleCqToolTip(int row, Frequency freq, QString& toolTipMsg);
    void assembleCqMsg(int row, QString& markerMsg);
    int dialCursorWithinViewport(Frequency freq);
    bool matchDistance(int sourceRow);

    void drawBandmapSpot(int row, int &fontOffset, int markersAbove, int &lastOffset, bool &firstDrawn);
    bool readLessGreaterThanDistanceFlag();
    void doBandmapUpdate();
    bool filterAcceptsRow(int sourceRow) const;
};

#endif // BANDMAPVIEW_H
