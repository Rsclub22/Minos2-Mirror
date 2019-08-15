#include "bandmapgraphicspanel.h"

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
    //setCurrentIndex();
    //indexAt(event->pos())
    QPoint p = event->pos();
    qDebug() << "x mouse" << p.x();
    qDebug() << "y mouse" << p.y();
    int a = 0;
}


void BandmapGraphicsPanel::keyPressEvent(QKeyEvent *event)
{

}
