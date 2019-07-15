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
    QPainter painter(this);
    qDebug() << "height" << bandmap->getScrollViewHeight();
    dial->drawScale(&painter, curFreq, 1000);
    dial->drawCursor(&painter, curFreq);
}


void BandmapView::setFreq(double f)
{

        curFreq = f;
        update();


}
