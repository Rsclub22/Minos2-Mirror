#ifndef BANDMAPVIEW_H
#define BANDMAPVIEW_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QGraphicsView>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QGraphicsScene>
#include <QSortFilterProxyModel>
#include "bandmapfreqdial.h"
#include "bandmapspotmarker.h"
#include "bandmapdatamodel.h"
#include "bandmapmarkerdetails.h"
#include "bandmapclientfilterdialog.h"
#include "bandmapcommon.h"
#include "spotdatabase.h"
#include "bandmapgraphicspanel.h"

const QChar DEG_SYMBOL = 0260; // octal value
const int NO_SELECTED_ROWNUM = -1;

const int KEY_SCROLL_STEP_SIZE = 50;


class BandmapView : public QAbstractItemView
{
    Q_OBJECT

public:
    explicit BandmapView(QWidget *parent = nullptr);
    ~BandmapView() override;

    QModelIndex indexAt(const QPoint &point_) const override;
    void scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint) override;
    QRect visualRect(const QModelIndex &index) const override;






    //QSize minimumSizeHint() const override;
    //QSize sizeHint() const override;

    void setFreq(Frequency f, bool legalFreq);

    int getBandmapFrameHeight();
    int getBandmapFrameWidth();
    void onFontChanged(QFont cf);
    void initBandmapView(BandmapGraphicsPanel *view);


    void bandmapUpdate();

    int rows(const QModelIndex &index) const;

    int isClickInRegionOfSpot(QPoint p);


    void getSpotData(int &selectedSpotDataRowNum, int displayedSpotNum, BandmapSpotData &selectedSpot);

    void clearSelectedSpotData();

    int getSelectedSpotViewRowNum(){return selectedSpotViewRowNum;}

    int getSelectedSpotDataRowNum(){return selectedSpotDataRowNum;}

    BandmapSpotData* getSelectedSpotDataPtr(){return &selectedSpot;}

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

protected slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) override;

    void rowsInserted(const QModelIndex &parent, int start, int end) override;

    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
    void updateGeometries() override;





protected:
    int horizontalOffset() const override;
    int verticalOffset() const override;
    bool isIndexHidden(const QModelIndex&) const override{ return false; }
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    void setSelection(const QRect &rect, QFlags<QItemSelectionModel::SelectionFlag> flags) override;

    QRegion visualRegionForSelection(const QItemSelection &selection) const override;





private slots:
    void bandmapResize(int, int);

    void leftMouseButtonPressed(QPoint p);
    void mouseDoubleClicked(QPoint p);



    void on_bandmap_customContextMenuRequested(const QPoint &p);

    void on_nextSpot(bool nextFreqUpDown, bool nextMult);
    int findNextNonWorkedLocatorUpList(int curSpotViewNum);

    void on_scrollMap(bool dir);
    void onRowsRemoved(const QModelIndex &parent, int first, int last);
    void onRowsInserted(const QModelIndex &parent, int first, int last);
    void zoomUpdated(bool dir);

private:

    QGraphicsScene *bandmapScene;
    BandmapGraphicsPanel* bandmapGraphicsView;

    BandmapFreqDial *dial;
    Frequency curFreq;

    bool suppressUpdate = false;
    int totalSize = 300; //for test

    int dialMinZoomLevel;
    int dialMaxZoomLevel;
    int zoomLevel;

    Frequency contestBandFlow;
    Frequency contestBandFhigh;
    Frequency curViewPortStartFreq;

    int idealWidth;
    int idealHeight;
    int fullBandHeight;
    int fontHeight;
    int maxNumSpots;

    //void changeZoom(bool direction);
    void drawBandMapSpots();
    QVector<BandmapMarkerDetails*> listOfMarkers;

    BandmapSpotData selectedSpot;
    int selectedSpotDataRowNum;
    int selectedSpotViewRowNum;

    BandmapClientFilterSettings* filterSettings;

    bool traceDebugFlag = false;

    bool lessGreaterThanDistanceFlag = false;


    void assembleSpotMsg(int row, QString& markerMsg);
    void assembleToolTip(int row, Frequency freq, QString& toolTipMsg);
    QRectF viewportRectForRow(int row) const;
    QRectF calculateSpotRect(const QString text, const QPoint spotCoord);
    void bandmapSelectFreq(int y);
    void bandmapSelectSpot(QPoint p);
    void sendFreqToRig(Frequency freq);
    void setSelectedSpot(int displayedSpotNum);

    void clearListOfMarkers();
    bool matchMode(int sourceRow);
    int findNextOccupiedMarkerUpList(int curSpotViewNum);
    int findNextOccupiedMarkerDownList(int curSpotViewNum);
    int findNextNonWorkedLocatorDownList(int curSpotViewNum);
    int getViewPortStartYCoordOnScene();
    int getViewPortEndYCoordOnScene();
    void traceMsg(QString msg);
    void clearSpotData(BandmapSpotData &selectedSpot);
    void deleteItemsFromMarkerList();
    void assembleCqToolTip(int row, Frequency freq, QString& toolTipMsg);
    void assembleCqMsg(int row, QString& markerMsg);
    int dialCursorWithinViewport(Frequency freq);
    bool matchDistance(int sourceRow);

    void drawBandmapSpot(int row, int &fontOffset, int markersAbove, int &lastOffset);
    bool readLessGreaterThanDistanceFlag();
};

#endif // BANDMAPVIEW_H
