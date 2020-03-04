#include "bandmapgraphicspanel.h"
#include <QDebug>
#include <QMouseEvent>

BandmapGraphicsPanel::BandmapGraphicsPanel(QWidget *parent)
{
    Q_UNUSED(parent)

    setFocusPolicy(Qt::NoFocus);
}




void BandmapGraphicsPanel::resizeEvent(QResizeEvent *)
{

    // do we need to keep this.....
    emit bandmapResize(size().height(), size().width());

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
    int Key = event->key();
    Qt::KeyboardModifiers mods = event->modifiers();
    //bool shift = mods & Qt::ShiftModifier;
    bool ctrl = mods & Qt::ControlModifier;
    bool alt = mods & Qt::AltModifier;
/*
    if (Key == Qt::Key_Less)
    {
        emit zoomMap(true);
    }
    else if (Key == Qt::Key_Greater)
    {
        emit zoomMap(false);
    }
    else
*/
    if (Key == Qt::Key_Up && ctrl && alt)
    {
        emit nextSpot(true, true);
    }
    else if (Key == Qt::Key_Down && ctrl && alt)
    {
        emit nextSpot(false, true);
    }
    else if (Key == Qt::Key_Down && ctrl)
    {
         emit nextSpot(true, false);
    }
    else if (Key == Qt::Key_Up && ctrl)
    {
        emit nextSpot(false, false);
    }
    else if (Key == Qt::Key_Up)
    {
        emit scrollMap(true);
    }
    else if (Key == Qt::Key_Down)
    {
        emit scrollMap(false);
    }

    QWidget::keyPressEvent(event);

}
