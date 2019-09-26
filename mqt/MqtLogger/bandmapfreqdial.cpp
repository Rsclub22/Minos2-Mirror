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
    zoomLevel(dialData::MAX_ZOOM_LEVEL),
    dialHeight(_height),
    dialWidth(_width),
    cursorColour(Qt::red)
{
    changeBoundingRect(dialHeight, dialWidth);
}

void BandmapFreqDial::onFontChanged(QFont cf)
{
    newFreqTextWidth = checkFreqWidth(currentFreqInt32);
    QFontMetrics fm(cf);
    if (fontHeight != fm.height())
    {
        fontHeight = fm.height();
    }

    update();
}

void BandmapFreqDial::setCursorColour(QColor colour)
{
    cursorColour = colour;
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


    drawScale(painter, currentFreqInt32, dialHeight);
    drawCursor(painter, currentFreqInt64);


}


//void BandmapFreqDial::drawDial(QPainter *painter)
//{
//    drawScale(painter, currentFreqInt32, dialHeight);
//    drawCursor(painter, currentFreqInt64);
//}



void BandmapFreqDial::setCurFreq(double f)
{
    currentFreqDbl = f;
    qint64 frequency = static_cast<qint64>(f);
    if (frequency != currentFreqInt64)
    {
        currentFreqInt64 = frequency;
        qint32 freq = static_cast<qint32>(frequency/1000);
        currentFreqInt32 = freq;
        newFreqTextWidth = checkFreqWidth(currentFreqInt32);
    }

 }


double BandmapFreqDial::getCurFreq()
{
   return currentFreqDbl;
}

qint32 BandmapFreqDial::getCurFreqInt32()
{
    return currentFreqInt32;
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
    return fm.width(convertFreqDialDisplay(freq));
}

int BandmapFreqDial::getFontHeight()
{
    QFont cf = QApplication::font();
    QFontMetrics fm(cf);
    return fm.height();

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

    Q_UNUSED(frequency)

    dialHeight = scaleHeight;
    qDebug() << "drawscale height " << dialHeight;

    int _fontHeight = getFontHeight();
    if (_fontHeight != fontHeight)
    {
        fontHeight = _fontHeight;
    }

    if (newFreqTextWidth != freqTextWidth)
    {
        freqTextWidth = newFreqTextWidth;
        dialWidth = newFreqTextWidth + dialData::additionalWidth;
        changeBoundingRect(dialHeight, dialWidth);
    }

/*
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

    if (scaleStartFreq < 144150)  //// ************************this needs to be the lower limit of the band
    {
        scaleStartFreq = 144150;
    }
    scaleEndFreq = scaleStartFreq + freqRange;
*/
    trace(QString("bandmapFreqDial: scale startFreq = %1").arg(scaleStartFreq));
    trace(QString("bandmapFreqDial: scale endFreq = %1").arg(scaleEndFreq));


    QRect scaleRec(0,0, dialWidth, dialHeight);
    QBrush scaleBackGndBrush(Qt::lightGray, Qt::SolidPattern);

    QPen scalePen(Qt::black);
    scalePen.setWidth(2);
    painter->setPen(scalePen);

    painter->fillRect(scaleRec, scaleBackGndBrush);

    QRect freqSelRec(dialWidth - FREQ_SEL_WIDTH,0, FREQ_SEL_WIDTH, dialHeight);
    QBrush freqSelBackGndBrush(Qt::gray, Qt::SolidPattern);
    painter->fillRect(freqSelRec, freqSelBackGndBrush);

    painter->drawLine(QPoint(dialWidth,0 + dialData::DIAL_VERT_OFFSET), QPoint(dialWidth, dialHeight));

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
            painter->drawLine(QPoint(dialWidth - dialData::fMajMrkLength, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialWidth, ycoord + dialData::DIAL_VERT_OFFSET));
            painter->drawText(QRect(dialData::fMajTextXStart, ycoord - (fontHeight/2) + dialData::DIAL_VERT_OFFSET, dialData::fMajTextXStart + freqTextWidth, fontHeight),  Qt::AlignLeft, convertFreqDialDisplay(markFreq));
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
                painter->drawLine(QPoint(dialWidth - dialData::fMajMrkLength, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialWidth, ycoord + dialData::DIAL_VERT_OFFSET));
                painter->drawText(QRect(dialData::fMajTextXStart, ycoord - (fontHeight/2) + dialData::DIAL_VERT_OFFSET, dialData::fMajTextXStart + freqTextWidth, fontHeight), Qt::AlignLeft, convertFreqDialDisplay(markFreq));
                markFreq += 5;
            }
            else
            {
                painter->drawLine(QPoint(dialWidth - dialData::fMinMrkLength, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialWidth, ycoord + dialData::DIAL_VERT_OFFSET));

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
                painter->drawLine(QPoint(dialWidth - dialData::fMajMrkLength, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialWidth, ycoord + dialData::DIAL_VERT_OFFSET));
                painter->drawText(QRect(dialData::fMajTextXStart, ycoord - (fontHeight/2) + dialData::DIAL_VERT_OFFSET, dialData::fMajTextXStart + freqTextWidth, fontHeight), Qt::AlignLeft, convertFreqDialDisplay(markFreq));
                markFreq += 10;
            }
            else
            {
                painter->drawLine(QPoint(dialWidth - dialData::fMinMrkLength, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialWidth, ycoord + dialData::DIAL_VERT_OFFSET));
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
                painter->drawText(QRect(dialData::fMajTextXStart, ycoord - (fontHeight/2) + dialData::DIAL_VERT_OFFSET, dialData::fMajTextXStart + freqTextWidth, fontHeight), Qt::AlignLeft, convertFreqDialDisplay(markFreq));
                markFreq += 50;
            }
            else
            {
                painter->drawLine(QPoint(dialWidth - dialData::fMinMrkLength, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialWidth, ycoord + dialData::DIAL_VERT_OFFSET));
            }

            markCount++;

        }

    }

}


void BandmapFreqDial::calcStartEndFreq(qint32 frequency)
{
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

    //if (scaleStartFreq < 144150)  //// ************************this needs to be the lower limit of the band
    //{
    //    scaleStartFreq = 144150;
    //}
    scaleEndFreq = scaleStartFreq + freqRange;
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


int BandmapFreqDial::getYCoordOnDial(qint64 frequency)
{
    qint32 fmaj = static_cast<qint32>(frequency/1000);
    qint32 fmin = static_cast<qint32>(frequency - (fmaj*1000));
    qint32 offsetFreq = fmaj - scaleStartFreq;
    return (offsetFreq * dialData::khzPixelStep[zoomLevel]) + (fmin/dialData::hzPixelStep[zoomLevel]);

}

QString BandmapFreqDial::getFreqFromYCoordOnDial(int y)
{
    int dialPos = y - dialData::DIAL_VERT_OFFSET;
    qint32 fmaj = dialPos / dialData::khzPixelStep[zoomLevel]  * 1000;
    qint32 fmin = dialPos % dialData::khzPixelStep[zoomLevel] * dialData::hzPixelStep[zoomLevel];
    qint32 ftot = fmaj + fmin;
    qint64 freq = (scaleStartFreq * 1000) + ftot;
    return QString::number(freq);
}

void BandmapFreqDial::drawCursor(QPainter *painter, qint64 frequency)
{


    int cursorY = getYCoordOnDial(frequency);
    trace(QString("bandmapFreqDial: cursor Freq = %1").arg(frequency));
    trace(QString("bandmapFreqDial: cursor Freq y coord = %1").arg(cursorY));

    QPolygon freqCursor;
/*
    freqCursor << QPoint(60,cursorY-5 + dialData::DIAL_VERT_OFFSET);
    freqCursor << QPoint(60,cursorY+10 + dialData::DIAL_VERT_OFFSET);
    freqCursor << QPoint(70,cursorY +dialData::DIAL_VERT_OFFSET);
    freqCursor << QPoint(60,cursorY-5 + dialData::DIAL_VERT_OFFSET);
*/
    freqCursor << QPoint(dialWidth -  10, cursorY - 10 + dialData::DIAL_VERT_OFFSET);
    freqCursor << QPoint(dialWidth - 10, cursorY + 10 + dialData::DIAL_VERT_OFFSET);
    freqCursor << QPoint(dialWidth, cursorY + dialData::DIAL_VERT_OFFSET);
    freqCursor << QPoint(dialWidth -  10, cursorY - 10 + dialData::DIAL_VERT_OFFSET);

    QBrush freqCursorBrush(cursorColour, Qt::SolidPattern);

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
        //if (zoomLevel < dialData::MAX_ZOOM_LEVEL && zoomLevel >= dialData::MIN_ZOOM_LEVEL)
        //{
        //    ++zoomLevel;
        //    update();
            emit zoomUpdated(true);

        //}
    }
    else
    {
        //if (zoomLevel != dialData::MIN_ZOOM_LEVEL && zoomLevel <= dialData::MAX_ZOOM_LEVEL)
        //{
        //    --zoomLevel;
        //    update();
        emit zoomUpdated(false);

        //}
    }
}


//void BandmapFreqDial::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//    QGraphicsItem::mousePressEvent(event);
//}



