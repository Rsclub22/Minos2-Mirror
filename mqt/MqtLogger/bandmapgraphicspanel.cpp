#include "bandmapgraphicspanel.h"
#include <QDebug>
#include <QMouseEvent>

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
    int key = event->key();
    Qt::KeyboardModifiers mods = event->modifiers();
    //bool shift = mods & Qt::ShiftModifier;
    bool ctrl = mods & Qt::ControlModifier;
    bool alt = mods & Qt::AltModifier;

    if (key == Qt::Key_Plus)
    {
        emit zoomMap(false);
    }
    else if (key == Qt::Key_Minus)
    {
        emit zoomMap(true);
    }
    else if (key == Qt::Key_Up && ctrl && alt)
    {
        emit nextSpot(true, true);
    }
    else if (key == Qt::Key_Down && ctrl && alt)
    {
        emit nextSpot(false, true);
    }
    else if (key == Qt::Key_Down && ctrl)
    {
        emit nextSpot(true, false);
    }
    else if (key == Qt::Key_Up && ctrl)
    {
        emit nextSpot(false, false);
    }

}
