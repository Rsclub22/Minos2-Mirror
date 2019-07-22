#ifndef BANDMAPVIEW_H
#define BANDMAPVIEW_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QGraphicsView>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QScrollBar>
#include "bandmap.h"
#include "bandmapfreqdial.h"
#include "textmarker.h"

class BandmapView : public QAbstractItemView
{
    Q_OBJECT
public:
    explicit BandmapView(QWidget *parent = nullptr);
    void setModel(QAbstractItemModel *model) override;
    QRect visualRect(const QModelIndex &index) const override;
    void scrollTo(const QModelIndex &index,
                  QAbstractItemView::ScrollHint) override;
    QModelIndex indexAt(const QPoint &point_) const override;


    //QSize minimumSizeHint() const override;
    //QSize sizeHint() const override;

    void setFreq(double f);
protected slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start,
                              int end) override;
    void updateGeometries() override;



signals:
    void clicked(const QModelIndex&);

protected:
protected:
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    bool isIndexHidden(const QModelIndex&) const override{ return false; }
    int horizontalOffset() const override;
    int verticalOffset() const override;
    void scrollContentsBy(int dx, int dy) override;
    void setSelection(const QRect &rect, QFlags<QItemSelectionModel::SelectionFlag> flags) override;
    QRegion visualRegionForSelection(const QItemSelection &selection) const override;

    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mousePressEvent(QMouseEvent *event) override;




public slots:



private slots:
    void bandmapResize(int);

private:

    Bandmap *bandmap;
    QGraphicsScene *bandmapScene;
    BandmapFreqDial *dial;
    double curFreq;

    TextMarker *bandmapSpotMarker;

    int dialMinZoomLevel;
    int dialMaxZoomLevel;

    //void changeZoom(bool direction);
    void drawBandMapSpots();
    QList<QGraphicsItem*> listOfMarkers;

};

#endif // BANDMAPVIEW_H
