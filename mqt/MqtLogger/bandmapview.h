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



class BandmarkerDetials
{

public:
    BandmarkerDetials();
    BandmarkerDetials(QPoint _freqLineStart, QPoint _freqLineEnd, BandmapSpotMarker* _spot);



QPoint freqLineStart;
QPoint freqLineEnd;
BandmapSpotMarker* spot;

};



class BandmapView : public QAbstractItemView
{
    Q_OBJECT
public:
    explicit BandmapView(QGraphicsView* bandmapGraphicsView, QWidget *parent = nullptr);

    QModelIndex indexAt(const QPoint &point_) const override;
    void scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint) override;
    QRect visualRect(const QModelIndex &index) const override;
    void setModel(QAbstractItemModel *model) override;





    //QSize minimumSizeHint() const override;
    //QSize sizeHint() const override;

    void setFreq(double f);

    int getBandmapFrameHeight();
    int getBandmapFrameWidth();
protected slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start,
                              int end) override;
    void updateGeometries() override;



signals:
    void clicked(const QModelIndex&);


protected:
    int horizontalOffset() const override;
    bool isIndexHidden(const QModelIndex&) const override{ return false; }
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    void setSelection(const QRect &rect, QFlags<QItemSelectionModel::SelectionFlag> flags) override;
    int verticalOffset() const override;
    QRegion visualRegionForSelection(const QItemSelection &selection) const override;


    //void scrollContentsBy(int dx, int dy) override;

    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;



public slots:



private slots:
    void bandmapResize(int);

private:

    //Bandmap *bandmap;
    QGraphicsScene *bandmapScene;
    QGraphicsView* bandmapGraphicsView;

    BandmapFreqDial *dial;
    double curFreq;



    int dialMinZoomLevel;
    int dialMaxZoomLevel;

    //void changeZoom(bool direction);
    void drawBandMapSpots();
    QVector<BandmarkerDetials*> listOfMarkers;

    void bandmapUpdate();
};

#endif // BANDMAPVIEW_H
