#ifndef BANDMAPVIEW_H
#define BANDMAPVIEW_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QScrollBar>
#include "bandmap.h"
#include "bandmapfreqdial.h"

class BandmapView : public QWidget
{
    Q_OBJECT
public:
    explicit BandmapView(QWidget *parent = nullptr);
    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setFreq(double f);
signals:
    void clicked(const QModelIndex&);

protected:
    bool eventFilter(QObject *target, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;


public slots:



private:

    Bandmap *bandmap;
    BandmapFreqDial *dial;
    double curFreq;

    int dialMinZoomLevel;
    int dialMaxZoomLevel;

    void changeZoom(bool direction);
};

#endif // BANDMAPVIEW_H
