#include "bandmapgraphicspanel.h"
#include <QDebug>

BandmapGraphicsPanel::BandmapGraphicsPanel(QWidget *parent)
{
    Q_UNUSED(parent)
}




void BandmapGraphicsPanel::resizeEvent(QResizeEvent *)
{

    qDebug() << "resize height " << size();
    emit bandmapResize(size().height());

}


void BandmapGraphicsPanel::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    Qt::MouseButtons mouseButtons = event->buttons();
    if( mouseButtons == Qt::LeftButton)
    {
        QPoint p = event->pos();
        emit leftMouseButtonPressed(p);
    }


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
