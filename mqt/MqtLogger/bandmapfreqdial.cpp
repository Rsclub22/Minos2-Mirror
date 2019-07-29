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
#include <QFontMetrics>




BandmapFreqDial::BandmapFreqDial(int _width, int _height):
    zoomLevel(0),
    dialHeight(_height),
    dialWidth(_width)
{
    int a = 0;
}




QRectF BandmapFreqDial::boundingRect() const
{

      return QRectF(0, 0, dialWidth, dialHeight );
}



void BandmapFreqDial::changeBoundingRect(int height, int width)
{
    prepareGeometryChange();
    setCurHeight(height);
    setCurWidth(width);
    boundingRect();

}



void BandmapFreqDial::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{


    drawScale(painter, currentFreqInt, dialHeight);
    drawCursor(painter, currentFreqInt);


}



void BandmapFreqDial::setCurFreq(double f)
{
    currentFreqDbl = f;
    qint64 frequency = static_cast<qint64>(f);
    qint32 freq = static_cast<qint32>(frequency/1000);
    if (freq != currentFreqInt)
    {
        currentFreqInt = freq;
        newWidth = checkFreqWidth(currentFreqInt);

    }
}


double BandmapFreqDial::getCurFreq()
{
   return currentFreqDbl;
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
   dialHeight = height;
}

int BandmapFreqDial::getCurHeight()
{
    return dialHeight;
}

void BandmapFreqDial::setCurWidth(int width)
{
    dialWidth = width;
}

int BandmapFreqDial::getCurWidth()
{
    return dialWidth;
}

int BandmapFreqDial::checkFreqWidth(double freq)
{
    qint64 frequency = static_cast<qint64>(freq);
    qint32 f = static_cast<qint32>(frequency/1000);
    return checkFreqWidth(f);

}

int BandmapFreqDial::checkFreqWidth(qint32 freq)
{
    //calc dial width
    QFont cf = QApplication::font();
    QFontMetrics fm(cf);
    return fm.width(convertFreqDialDisplay(freq)) + 30;
}

qint32 BandmapFreqDial::getScaleStartFreq()
{
    return scaleStartFreq;
}

qint32 BandmapFreqDial::getScaleEndFreq()
{
    return scaleEndFreq;
}




void BandmapFreqDial::drawScale(QPainter *painter, qint32 frequency, int scaleHeight)
{

    dialHeight = scaleHeight;
    qDebug() << "drawscale height " << dialHeight;

    if (newWidth != dialWidth)
    {
        dialWidth = newWidth;
        changeBoundingRect(dialHeight, dialWidth);
    }

    qint32 freq = frequency;

    int freqRange = dialHeight / dialData::khzPixelStep[zoomLevel];
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

    //if (scaleStartFreq < 144000)  //// ************************this needs to be the lower limit of the band
    //{
    //    scaleStartFreq = 144000;
   // }
    scaleEndFreq = scaleStartFreq + freqRange;

    qDebug() << "scale startF" << scaleStartFreq;
    qDebug() << "scale endF" << scaleEndFreq;

    //QRect scaleRec(0,0,70,dialData::MAXSCALEY);
    QRect scaleRec(0,0, dialWidth, dialHeight);
    QBrush scaleBackGndBrush(Qt::lightGray, Qt::SolidPattern);

    QPen scalePen(Qt::black);
    scalePen.setWidth(2);
    painter->setPen(scalePen);

    painter->fillRect(scaleRec, scaleBackGndBrush);

    //painter->drawLine(QPoint(70,0),QPoint(70,dialData::MAXSCALEY));
    painter->drawLine(QPoint(dialWidth,0 + dialData::DIAL_VERT_OFFSET),QPoint(dialWidth, dialHeight));

    QPen markerPen(Qt::blue);
    markerPen.setWidth(1);
    painter->setPen(markerPen);


    int markStep = dialData::khzStep[zoomLevel] * dialData::khzPixelStep[zoomLevel];
    int markCount = 0;
    qint32 markFreq = scaleStartFreq;

    if (dialData::minorMarker[zoomLevel] == 0)
    {

        for (int ycoord = 0; ycoord < dialHeight; ycoord += markStep)
        {
            painter->drawLine(QPoint(dialData::fMajMrkXStart, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialData::fMajMrkXEnd, ycoord + dialData::DIAL_VERT_OFFSET));
            painter->drawText(QRect(0, ycoord - 7 + dialData::DIAL_VERT_OFFSET, 45, 12), Qt::AlignRight, convertFreqDialDisplay(markFreq));
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

        for (int ycoord = 0; ycoord < dialHeight; ycoord += markStep)
        {

            if (markCount == 0 || markCount % 5 == 0)
            {
                painter->drawLine(QPoint(dialData::fMajMrkXStart, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialData::fMajMrkXEnd, ycoord + dialData::DIAL_VERT_OFFSET));
                painter->drawText(QRect(0, ycoord - 7 + dialData::DIAL_VERT_OFFSET, 45, 12), Qt::AlignRight, convertFreqDialDisplay(markFreq));
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
        for (int ycoord = 0; ycoord < dialHeight; ycoord += markStep)
        {

            if (markCount == 0 || markCount % 10 == 0)
            {
                painter->drawLine(QPoint(dialData::fMajMrkXStart, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialData::fMajMrkXEnd, ycoord + dialData::DIAL_VERT_OFFSET));
                painter->drawText(QRect(0, ycoord - 7 + dialData::DIAL_VERT_OFFSET, 45, 12), Qt::AlignRight, convertFreqDialDisplay(markFreq));
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
        for (int ycoord = 0; ycoord < dialHeight; ycoord += markStep)
        {

            if (markCount == 0 || markCount % 2 == 0)
            {
                painter->drawLine(QPoint(dialData::fMajMrkXStart, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialData::fMajMrkXEnd, ycoord + dialData::DIAL_VERT_OFFSET));
                painter->drawText(QRect(0, ycoord - 7 + dialData::DIAL_VERT_OFFSET, 45, 12), Qt::AlignRight, convertFreqDialDisplay(markFreq));
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


QString BandmapFreqDial::convertFreqDialDisplay(qint32 freq)
{

    QString sfreq = QString::number(freq);

    int len = sfreq.length();

    switch(len)
    {
    case 8:
        sfreq = sfreq.insert(5, '.');
        sfreq = sfreq.insert(2, '.');
        break;
    case 7:
        sfreq = sfreq.insert(4, '.');
        sfreq = sfreq.insert(1, '.');
        break;
    case 6:
        sfreq = sfreq.insert(3, '.');
        break;
    case 5:
        sfreq = sfreq.insert(2, '.');
        break;
    default:
        sfreq = "000.000";  // error

    }

    return sfreq;


}


void BandmapFreqDial::drawCursor(QPainter *painter, qint32 _frequency)
{

    qint32 frequency = _frequency;
    qint32 fmaj = frequency/1000;
    qint32 fmin = frequency - (fmaj*1000);
    qint32 offsetFreq = fmaj - scaleStartFreq;
    int cursorY = (offsetFreq * dialData::khzPixelStep[zoomLevel]) + (fmin/dialData::hzPixelStep[zoomLevel]);


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



