////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Bandmap View
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////


#include "bandmapview.h"
#include <QDebug>



BandmapView::BandmapView(QWidget *parent) : QWidget(parent)
{
    bandmap = qobject_cast<Bandmap*>(parent);
    Q_ASSERT(bandmap);
    setFocusPolicy((Qt::WheelFocus));
    setMinimumSize(minimumSizeHint());
    dial = new BandmapFreqDial();
    dialMinZoomLevel = dial->getMinZoomLevel();
    dialMaxZoomLevel = dial->getMaxZoomLevel();



}


QSize BandmapView::minimumSizeHint() const
{
/*    return QSize(visualizer->widthOfYearColumn() +
                 visualizer->maleFemaleHeaderTextWidth() +
                 visualizer->widthOfTotalColumn(),
                 QFontMetrics(font()).height() + ExtraHeight);
*/

       return QSize(200, 800);

}


QSize BandmapView::sizeHint() const
{
    /*
    int rows = visualizer->model()
               ? visualizer->model()->rowCount() : 1;
    return QSize(visualizer->widthOfYearColumn() +
            qMax(100, visualizer->maleFemaleHeaderTextWidth()) +
            visualizer->widthOfTotalColumn(),
            visualizer->yOffsetForRow(rows));
    */

    return QSize(200,800);
}


bool BandmapView::eventFilter(QObject *target, QEvent *event)
{

}


void BandmapView::mousePressEvent(QMouseEvent *event)
{

}


void BandmapView::keyPressEvent(QKeyEvent *event)
{

}


void BandmapView::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    qDebug() << "height" << bandmap->getBandmapFrameHeight();
    qDebug() << "width" << bandmap->getBandmapFrameWidth();

    dial->drawScale(&painter, curFreq, bandmap->getBandmapFrameHeight());
    dial->drawCursor(&painter, curFreq);

    bandmapSpotMarker = new TextMarker(80, 100, "G8FKH", Qt::red);
    bandmapSpotMarker->drawTextMarker(&painter);
}

void BandmapView::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numTicks = numDegrees / 15;

    if (numTicks == 1)
    {
       changeZoom(true);
    }
    else
    {
        changeZoom(false);
    }

    event->accept();
}


void BandmapView::changeZoom(bool direction)
{
    if (direction)
    {
        if (dial->getZoomLevel() < dialMaxZoomLevel && dial->getZoomLevel() >= dialMinZoomLevel)
        {
            int newLevel = dial->getZoomLevel();
            dial->setZoomLevel(++newLevel);
            update();
        }
    }
    else
    {
        if (dial->getZoomLevel() != dialMinZoomLevel && dial->getZoomLevel() <= dialMaxZoomLevel)
        {
            int newLevel = dial->getZoomLevel();
            dial->setZoomLevel(--newLevel);
            update();
        }
    }
}


void BandmapView::setFreq(double f)
{

        curFreq = f;
        update();


}

void BandmapView::drawBandMapSpots(QPainter* painter, QPaintEvent *event)
{
    // check model exists!

    int ExtraHeight = 10;
    QFontMetrics fm(font());
    const int RowHeight = fm.height() + ExtraHeight;
    const int MinY = qMax(0, event->rect().y() - RowHeight);
    const int MaxY = MinY + event->rect().height() + RowHeight;

    int row = MinY/RowHeight;
    int y = row * RowHeight;
    for (; row < bandmap->getBandDataModel()->rowCount(); ++row)
    {
        bandmapSpotMarker = new TextMarker(80, 100, "G8FKH", Qt::red);
        //paintRow(painter, row, y, RowHeight);
        y += RowHeight;
        if (y > MaxY)
            break;
    }

}
