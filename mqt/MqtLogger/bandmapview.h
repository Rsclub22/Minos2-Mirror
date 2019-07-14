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
    bool eventFilter(QObject *target, QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);


public slots:



private:

    Bandmap *bandmap;
    BandmapFreqDial *dial;
    double curFreq;

};

#endif // BANDMAPVIEW_H
