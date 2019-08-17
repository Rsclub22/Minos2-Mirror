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


const QChar DEG_SYMBOL = 0260; // octal value
const int NO_SELECTED_ROWNUM = -1;




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

    int rows(const QModelIndex &index) const;

protected slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) override;

    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void updateGeometries() override;



signals:
    //void clicked(const QModelIndex&);


protected:
    int horizontalOffset() const override;
    int verticalOffset() const override;
    bool isIndexHidden(const QModelIndex&) const override{ return false; }
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    void setSelection(const QRect &rect, QFlags<QItemSelectionModel::SelectionFlag> flags) override;

    QRegion visualRegionForSelection(const QItemSelection &selection) const override;


    //void scrollContentsBy(int dx, int dy) override;

    //void paintEvent(QPaintEvent*) override;
    //void resizeEvent(QResizeEvent*) override;
    //void mousePressEvent(QMouseEvent *event) override;
    //void keyPressEvent(QKeyEvent *event) override;







private slots:
    void bandmapResize(int);
    void zoomUpdated(bool dir);
    void mousePressed(QPoint p);




private:

    //Bandmap *bandmap;
    QGraphicsScene *bandmapScene;
    QGraphicsView* bandmapGraphicsView;

    BandmapFreqDial *dial;
    double curFreq;

    int totalSize = 300; //for test

    int dialMinZoomLevel;
    int dialMaxZoomLevel;
    int zoomLevel;

    int idealWidth;
    int idealHeight;
    int fontHeight;
    int maxNumSpots;

    //void changeZoom(bool direction);
    void drawBandMapSpots();
    QVector<BandmapMarkerDetails*> listOfMarkers;

    BandmapData selectedSpot;
    int selectedSpotRowNum;






    QString assembleSpotMsg(int row);
    QString assembleToolTip(int row, QString freq);
    QRectF viewportRectForRow(int row) const;
    QRectF calculateSpotRect(const QString text, const QPoint spotCoord);
    void bandmapSelectFreq(int y);
    bool bandmapSelectSpot(QPoint p);
    void sendFreqToRig(QString freq);
    void clearSelectedSpot();
    void setSelectedSpot(int displayedSpotNum);
};

#endif // BANDMAPVIEW_H
