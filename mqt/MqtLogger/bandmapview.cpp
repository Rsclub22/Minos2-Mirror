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
#include <QGraphicsScene>



BandmapView::BandmapView(QWidget *parent) : QWidget(parent)
{
    bandmap = qobject_cast<Bandmap*>(parent);
    Q_ASSERT(bandmap);
    setFocusPolicy((Qt::WheelFocus));
    setMinimumSize(minimumSizeHint());

    bandmapScene = new QGraphicsScene(this);

    bandmap->getBandMapGraphicsView()->setScene(bandmapScene);
    bandmap->getBandMapGraphicsView()->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    dial = new BandmapFreqDial();
    dialMinZoomLevel = dial->getMinZoomLevel();
    dialMaxZoomLevel = dial->getMaxZoomLevel();
    bandmapScene->addItem(dial);
    dial->setCurFreq(0.0);
    dial->setCurHeight(700);
    dial->update();



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
    //QWidget::paintEvent(event);
    QPainter painter(this);
    //qDebug() << "height" << bandmap->getBandmapFrameHeight();
    //qDebug() << "width" << bandmap->getBandmapFrameWidth();

    //dial->drawScale(&painter, curFreq, bandmap->getBandmapFrameHeight());
    //dial->drawCursor(&painter, curFreq);

    //bandmapSpotMarker = new TextMarker(80, 100, "G8FKH", Qt::red);
    //bandmapSpotMarker->drawTextMarker(&painter);
    drawBandMapSpots(&painter, event);

}




void BandmapView::setFreq(double f)
{

        curFreq = f;
        dial->setCurFreq(f);
        dial->update();


}

void BandmapView::drawBandMapSpots(QPainter* painter, QPaintEvent *event)
{
    // check model exists!
/*
    int ExtraHeight = 10;
    QFontMetrics fm(font());
    const int RowHeight = fm.height() + ExtraHeight;
    const int MinY = qMax(0, event->rect().y() - RowHeight);
    const int MaxY = MinY + event->rect().height() + RowHeight;

    int row = MinY/RowHeight;
    int y = row * RowHeight;
    for (; row < bandmap->getBandDataModel()->rowCount(); ++row)
    {
        bandmapScene->addItem(new TextMarker(80, 100, "G8FKH", Qt::red) );
        //paintRow(painter, row, y, RowHeight);
        y += RowHeight;
        if (y > MaxY)
            break;
    }
*/


    bandmapScene->addItem(new TextMarker(80, 100, "G8FKH", Qt::red) );

}



