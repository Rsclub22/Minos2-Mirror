#include "bandmapgraphicspanel.h"
#include <QMouseEvent>

BandmapGraphicsPanel::BandmapGraphicsPanel(QWidget */*parent*/)
{
    setFocusPolicy(Qt::NoFocus);
}
void BandmapGraphicsPanel::resizeEvent(QResizeEvent *)
{
    emit bandmapResize(size());
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
