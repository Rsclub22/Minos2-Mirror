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

#include "bandmapspotmarker.h"



BandmapView::BandmapView(QWidget *parent) : QWidget(parent)
{
    bandmap = qobject_cast<Bandmap*>(parent);
    Q_ASSERT(bandmap);
    setFocusPolicy((Qt::WheelFocus));
    setMinimumSize(minimumSizeHint());

    bandmapScene = new QGraphicsScene(this);

    bandmap->getBandMapGraphicsView()->setScene(bandmapScene);

    bandmap->getBandMapGraphicsView()->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    dial = new BandmapFreqDial(70, bandmap->getBandmapFrameHeight());
    qDebug() << "bandmap height " << bandmap->getBandmapFrameHeight();
    dialMinZoomLevel = dial->getMinZoomLevel();
    dialMaxZoomLevel = dial->getMaxZoomLevel();
    bandmapScene->addItem(dial);
    dial->setCurFreq(0.0);

    dial->update();
    drawBandMapSpots();

    connect (bandmap->getBandMapGraphicsView(), SIGNAL(bandmapResize(int)), this, SLOT(bandmapResize(int)));


    //QString redHtml = "<font color=\"Red\">";
    //QString endHtml = "</font><br>";
    //QString msg = redHtml + "G8FKH" + endHtml;
    //QPoint pos = QPoint(80,100);
    //BandmapSpotMarker* spot = new BandmapSpotMarker(pos, redHtml + "G8FKH" + endHtml, "testing", Qt::red, bandmapScene);
    //spot->setSpotText(msg);
    //pos = QPoint(80,125);
    //spot = new BandmapSpotMarker(pos, redHtml + "M0ICR" + endHtml, "testing", Qt::red, bandmapScene);


}




QSize BandmapView::minimumSizeHint() const
{
    return QSize(70, bandmap->getBandmapFrameHeight());
     //            QFontMetrics(font()).height() + ExtraHeight);

}


void BandmapView::bandmapResize(int _height)
{
    int height = _height;
    //int width = _width;
    qDebug() << "bandmap resize height " << height;
    dial->setCurHeight(height);
    dial->update();
    drawBandMapSpots();


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

    return QSize(70, bandmap->getBandmapFrameHeight());
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
    //dial->drawScale(&painter, curFreq, bandmap->getBandmapFrameHeight());
    //dial->drawCursor(&painter, curFreq);

    //drawBandMapSpots(&painter, event);

}

void BandmapView::resizeEvent(QResizeEvent *)
{


}
void BandmapView::setFreq(double f)
{

        curFreq = f;
        dial->setCurFreq(f);
        dial->update();


}

void BandmapView::drawBandMapSpots()
{
    if (!listOfMarkers.isEmpty())
    {
        for (int i = 0; i < listOfMarkers.count(); i++)
        {
            bandmapScene->removeItem(listOfMarkers[i]);
        }
    }
    QString redHtml = "<font color=\"Red\">";
    QString endHtml = "</font><br>";
    QString msg = redHtml + "G8FKH" + endHtml;
    QPoint pos = QPoint(80,100);
    BandmapSpotMarker* spot = new BandmapSpotMarker(pos, redHtml + "G8FKH" + endHtml, "testing", Qt::red);
    listOfMarkers.append(spot);
    spot->setSpotText(msg);
    bandmapScene->addItem(spot);
    pos = QPoint(80,125);
    spot = new BandmapSpotMarker(pos, redHtml + "M0ICR" + endHtml, "testing", Qt::red);
    listOfMarkers.append(spot);
    bandmapScene->addItem(spot);

}



