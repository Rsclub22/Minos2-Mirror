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

class BandmapView : public QWidget
{
    Q_OBJECT
public:
    explicit BandmapView(QWidget *parent = nullptr);
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setFreq(double f);
signals:
    void clicked(const QModelIndex&);

protected:
    bool eventFilter(QObject *target, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    //void wheelEvent(QWheelEvent *event) override;


    void resizeEvent(QResizeEvent *) override;

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
    void drawBandMapSpots(QPainter *painter, QPaintEvent *event);
};

#endif // BANDMAPVIEW_H
