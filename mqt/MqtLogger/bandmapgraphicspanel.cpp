#include "bandmapgraphicspanel.h"
#include <QDebug>

BandmapGraphicsPanel::BandmapGraphicsPanel(QWidget *parent)
{

}


void BandmapGraphicsPanel::resizeEvent(QResizeEvent *)
{

    qDebug() << "resize height " << size();
    emit bandmapResize(size().height());

}


void BandmapGraphicsPanel::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    QPoint p = event->pos();
    emit mousePressed(p);
}


void BandmapGraphicsPanel::mouseDoubleClickEvent(QMouseEvent *event)
{
    QGraphicsView::mouseDoubleClickEvent(event);
    QPoint p = event->pos();
    emit mouseDoubleClicked(p);

}

void BandmapGraphicsPanel::keyPressEvent(QKeyEvent *event)
{

}
