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

    void setFreq(double f, bool legalFreq);

    int getBandmapFrameHeight();
    int getBandmapFrameWidth();
    void onFontChanged(QFont cf);
    void initBandmapView(QGraphicsView *view);


    void bandmapUpdate();

    int rows(const QModelIndex &index) const;

    int isClickInRegionOfSpot(QPoint p);


    void getSpotData(int &selectedSpotDataRowNum, int displayedSpotNum, BandmapData &selectedSpot);

    void clearSelectedSpotData();

    int getSelectedSpotViewRowNum(){return selectedSpotViewRowNum;}
    int getSelectedSpotDataRowNum(){return selectedSpotDataRowNum;}
    BandmapData* getSelectedSpotDataPtr(){return &selectedSpot;}

    void clearSelectedSpot();

    void setFilter(BandmapClientFilterDialog *filter);

    void updateZoom(bool dir);

    void setBandFreqLimits(double flow, double fhigh);
    void setBandmapHeight(double flow, double fhigh);
    void makeCursorVisibleInBandmap();

    void scrollBandmapCenterToFreq(qint64 freq);
    void setFreqOperatingInfo(const QString contestBandStr, const QString contestModeStr, CheckOperatingFreq *operatingFreq, const bool operatingPlanOk);



signals:

    void contextMenuSelected(const QPoint&, const QPoint&);

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
    void zoomUpdated(bool dir);
    void leftMouseButtonPressed(QPoint p);
    void mouseDoubleClicked(QPoint p);



    void on_bandmap_customContextMenuRequested(const QPoint &p);

    void on_nextSpot(bool nextFreqUpDown, bool nextMult);
    int findNextNonWorkedLocatorUpList(int curSpotViewNum);
    void on_vertScrollBandChanged(int);

    void on_scrollMap(bool dir);
private:

    QGraphicsScene *bandmapScene;
    QGraphicsView* bandmapGraphicsView;

    BandmapFreqDial *dial;
    double curFreq;


    int totalSize = 300; //for test

    int dialMinZoomLevel;
    int dialMaxZoomLevel;
    int zoomLevel;

    double contestBandFlow;
    double contestBandFhigh;
    qint64 curViewPortStartFreq;

    int idealWidth;
    int idealHeight;
    int fullBandHeight;
    int fontHeight;
    int maxNumSpots;

    //void changeZoom(bool direction);
    void drawBandMapSpots();
    QVector<BandmapMarkerDetails*> listOfMarkers;

    BandmapData selectedSpot;
    int selectedSpotDataRowNum;
    int selectedSpotViewRowNum;

    BandmapClientFilterDialog* filterSetup;




    QString assembleSpotMsg(int row);
    QString assembleToolTip(int row, QString freq);
    QRectF viewportRectForRow(int row) const;
    QRectF calculateSpotRect(const QString text, const QPoint spotCoord);
    void bandmapSelectFreq(int y);
    void bandmapSelectSpot(QPoint p);
    void sendFreqToRig(QString freq);
    void setSelectedSpot(int displayedSpotNum);

    void clearListOfMarkers();
    bool matchMode(int sourceRow);
    int findNextOccupiedMarkerUpList(int curSpotViewNum);
    int findNextOccupiedMarkerDownList(int curSpotViewNum);
    int findNextNonWorkedLocatorDownList(int curSpotViewNum);
    int getViewPortStartYCoordOnScene();
    int getViewPortEndYCoordOnScene();
    void traceMsg(QString msg);
    void clearSpotData(BandmapData &selectedSpot);
    void deleteItemsFromMarkerList();
    QString assembleCqToolTip(int row, QString freq);
    QString assembleCqMsg(int row);
};

#endif // BANDMAPVIEW_H
