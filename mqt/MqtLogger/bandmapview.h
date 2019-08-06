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
#include "bandmapfreqdial.h"
#include "bandmapspotmarker.h"
#include "bandmapdatamodel.h"


class BandmarkerDetials
{

public:
    BandmarkerDetials();
    BandmarkerDetials(QPoint _spotMarkerCoord, BandmapSpotMarker* _spot, QGraphicsLineItem* _markerline);


QPoint spotMarkerCoord;     // text coords
BandmapSpotMarker* spot;
QGraphicsLineItem* markerline;

};



class BandmapView : public QAbstractItemView
{
    Q_OBJECT

public:
    explicit BandmapView(QWidget *parent = nullptr);

    QModelIndex indexAt(const QPoint &point_) const override;
    void scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint) override;
    QRect visualRect(const QModelIndex &index) const override;






    //QSize minimumSizeHint() const override;
    //QSize sizeHint() const override;

    void setFreq(double f);

    int getBandmapFrameHeight();
    int getBandmapFrameWidth();
    void onFontChanged(QFont cf);
    void initBandmapView(QGraphicsView *view);


    void bandmapUpdate();

protected slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) override;

    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start,
                              int end) override;
    void updateGeometries() override;



signals:
    void clicked(const QModelIndex&);


protected:
    int horizontalOffset() const override;
    int verticalOffset() const override;
    bool isIndexHidden(const QModelIndex&) const override{ return false; }
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    void setSelection(const QRect &rect, QFlags<QItemSelectionModel::SelectionFlag> flags) override;

    QRegion visualRegionForSelection(const QItemSelection &selection) const override;


    //void scrollContentsBy(int dx, int dy) override;

//    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;







private slots:
    void bandmapResize(int);
    void drawBandMapSpots();

private:

    //Bandmap *bandmap;
    QGraphicsScene *bandmapScene;
    QGraphicsView* bandmapGraphicsView;
    BandmapFreqDial *dial;
    double curFreq;



    int dialMinZoomLevel;
    int dialMaxZoomLevel;

    int fontHeight;
    int maxNumSpots;

    //void changeZoom(bool direction);

    QVector<BandmarkerDetials*> listOfMarkers;






};

#endif // BANDMAPVIEW_H
