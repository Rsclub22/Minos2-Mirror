////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Bandmap Dial
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////

#include "bandmapfreqdial.h"



BandmapFreqDial::BandmapFreqDial():
    zoomLevel(0)
{

}




QRectF BandmapFreqDial::boundingRect() const
{
//    qreal penwidth = 1;
//    return QRectF(-10 - penwidth/2, -10 - penwidth/2, 20 + penwidth, 20 + penwidth );
//    return QRectF(0, 0, 100, dialData::MAXSCALEY );
      return QRectF(0, 0, 100, maxScaleY );
}



void BandmapFreqDial::changeBoundingRect(int height)
{
    //prepareGeometryChange();
    setCurHeight(height);
    boundingRect();

}



void BandmapFreqDial::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{


    drawScale(painter, currentFreq, maxScaleY);
    drawCursor(painter, currentFreq);


}



void BandmapFreqDial::setCurFreq(double frequency)
{
    currentFreq = frequency;
}


double BandmapFreqDial::getCurFreq()
{
   return currentFreq;
}



void BandmapFreqDial::setZoomLevel(int level)
{
    zoomLevel = level;
}


int BandmapFreqDial::getZoomLevel()
{
   return zoomLevel;
}


void BandmapFreqDial::setCurHeight(int height)
{
    maxScaleY = height;
}

int BandmapFreqDial::getCurHeight()
{
    return maxScaleY;
}

void BandmapFreqDial::drawScale(QPainter *painter, double _frequency, int scaleHeight)
{

    maxScaleY = scaleHeight;
    qint32 freq = 0;
    qint32 frequency = static_cast<qint32>(_frequency);
    freq = frequency/1000;
    int freqRange = maxScaleY / dialData::khzPixelStep[zoomLevel];
    int mid_freqRange = freqRange/2;
    scaleStartFreq = freq - mid_freqRange;
    if (dialData::roundFactor[zoomLevel] == 5)      //  round to nearest 5khz
    {
       scaleStartFreq =  ((scaleStartFreq + 2) / 5) * 5;
    }
    else if (dialData::roundFactor[zoomLevel] == 10)    // round to nearest 10khz
    {
       scaleStartFreq =  ((scaleStartFreq + 2) / 10) * 10;
    }

    if (scaleStartFreq < 144000)  //// ************************this needs to be the lower limit of the band
    {
        scaleStartFreq = 144000;
    }
    scaleEndFreq = scaleStartFreq + freqRange;

    qDebug() << "scale startF" << scaleStartFreq;
    qDebug() << "scale endF" << scaleEndFreq;

    //QRect scaleRec(0,0,70,dialData::MAXSCALEY);
    QRect scaleRec(0,0,70, maxScaleY);
    QBrush scaleBackGndBrush(Qt::lightGray, Qt::SolidPattern);

    QPen scalePen(Qt::black);
    scalePen.setWidth(2);
    painter->setPen(scalePen);

    painter->fillRect(scaleRec, scaleBackGndBrush);

    //painter->drawLine(QPoint(70,0),QPoint(70,dialData::MAXSCALEY));
    painter->drawLine(QPoint(70,0 + dialData::DIAL_VERT_OFFSET),QPoint(70,maxScaleY));

    QPen markerPen(Qt::blue);
    markerPen.setWidth(1);
    painter->setPen(markerPen);


    int markStep = dialData::khzStep[zoomLevel] * dialData::khzPixelStep[zoomLevel];
    int markCount = 0;
    int markFreq = scaleStartFreq;

    if (dialData::minorMarker[zoomLevel] == 0)
    {

        for (int ycoord = 0; ycoord < maxScaleY; ycoord += markStep)
        {
            painter->drawLine(QPoint(dialData::fMajMrkXStart, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialData::fMajMrkXEnd, ycoord + dialData::DIAL_VERT_OFFSET));
            painter->drawText(QRect(0, ycoord - 7 + dialData::DIAL_VERT_OFFSET, 45, 12), Qt::AlignRight, QString::number(markFreq));
            if (dialData::khzStep[zoomLevel] == 1)
            {
                markFreq += 1;
            }
            else
            {
                markFreq += 50;
            }
        }

    }
    else if (dialData::minorMarker[zoomLevel] == 1)
    {

        for (int ycoord = 0; ycoord < maxScaleY; ycoord += markStep)
        {

            if (markCount == 0 || markCount % 5 == 0)
            {
                painter->drawLine(QPoint(dialData::fMajMrkXStart, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialData::fMajMrkXEnd, ycoord + dialData::DIAL_VERT_OFFSET));
                painter->drawText(QRect(0, ycoord - 7 + dialData::DIAL_VERT_OFFSET, 45, 12), Qt::AlignRight, QString::number(markFreq));
                markFreq += 5;
            }
            else
            {
                painter->drawLine(QPoint(dialData::fMinMrkXStart, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialData::fMinMrkXEnd, ycoord + dialData::DIAL_VERT_OFFSET));
            }

            markCount++;

        }
    }
    else if (dialData::minorMarker[zoomLevel] == 2)
    {
        for (int ycoord = 0; ycoord < maxScaleY; ycoord += markStep)
        {

            if (markCount == 0 || markCount % 10 == 0)
            {
                painter->drawLine(QPoint(dialData::fMajMrkXStart, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialData::fMajMrkXEnd, ycoord + dialData::DIAL_VERT_OFFSET));
                painter->drawText(QRect(0, ycoord - 7 + dialData::DIAL_VERT_OFFSET, 45, 12), Qt::AlignRight, QString::number(markFreq));
                markFreq += 10;
            }
            else
            {
                painter->drawLine(QPoint(dialData::fMinMrkXStart, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialData::fMinMrkXEnd, ycoord + dialData::DIAL_VERT_OFFSET));
            }


            markCount++;
        }


    }
    else if (dialData::minorMarker[zoomLevel] == 3)
    {
        for (int ycoord = 0; ycoord < maxScaleY; ycoord += markStep)
        {

            if (markCount == 0 || markCount % 2 == 0)
            {
                painter->drawLine(QPoint(dialData::fMajMrkXStart, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialData::fMajMrkXEnd, ycoord + dialData::DIAL_VERT_OFFSET));
                painter->drawText(QRect(0, ycoord - 7 + dialData::DIAL_VERT_OFFSET, 45, 12), Qt::AlignRight, QString::number(markFreq));
                markFreq += 50;
            }
            else
            {
                painter->drawLine(QPoint(dialData::fMinMrkXStart, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialData::fMinMrkXEnd, ycoord + dialData::DIAL_VERT_OFFSET));
            }

            markCount++;

        }

    }

}


void BandmapFreqDial::drawCursor(QPainter *painter, double _frequency)
{


    qint32 fmaj = 0;
    qint32 fmin = 0;
    qint32 frequency = static_cast<qint32>(_frequency);
    fmaj = frequency/1000;
    fmin = frequency - (fmaj*1000);
    qint32 offsetFreq = fmaj - scaleStartFreq;
    qint32 cursorY = (offsetFreq * dialData::khzPixelStep[zoomLevel]) + (fmin/dialData::hzPixelStep[zoomLevel]);


    QPolygon freqCursor;

    freqCursor << QPoint(60,cursorY-5 + dialData::DIAL_VERT_OFFSET);
    freqCursor << QPoint(60,cursorY+10 + dialData::DIAL_VERT_OFFSET);
    freqCursor << QPoint(70,cursorY +dialData::DIAL_VERT_OFFSET);
    freqCursor << QPoint(60,cursorY-5 + dialData::DIAL_VERT_OFFSET);

    QBrush freqCursorBrush(Qt::red, Qt::SolidPattern);

    QPainterPath freqCursorPath;
    freqCursorPath.addPolygon(freqCursor);
    painter->fillPath(freqCursorPath, freqCursorBrush);



}

void BandmapFreqDial::wheelEvent(QGraphicsSceneWheelEvent *event)
{

    int numDegrees = event->delta() / 8;
    int numTicks = numDegrees / 15;

    if (numTicks == 1)
    {
       changeZoom(false);
    }
    else
    {
        changeZoom(true);
    }

    event->accept();
}


void BandmapFreqDial::changeZoom(bool direction)
{
    if (direction)
    {
        if (zoomLevel < dialData::MAX_ZOOM_LEVEL && zoomLevel >= dialData::MIN_ZOOM_LEVEL)
        {
            ++zoomLevel;
            update();
        }
    }
    else
    {
        if (zoomLevel != dialData::MIN_ZOOM_LEVEL && zoomLevel <= dialData::MAX_ZOOM_LEVEL)
        {
            --zoomLevel;
            update();
        }
    }
}


//void BandmapFreqDial::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//    QGraphicsItem::mousePressEvent(event);
//}



