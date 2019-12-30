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
    scaleStartFreq(0),
    scaleEndFreq(0),
    scaleStartYCoord(0),
    scaleEndYCoord(0),
    fullBandHeight(2000),
    contestBandFlow(0),
    contestBandFhigh(0),
    operatingFreq(nullptr),
    operatingPlanOk(false),
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

qint64 BandmapFreqDial::getScaleStartFreq()
{
    return scaleStartFreq;
}

qint64 BandmapFreqDial::getScaleEndFreq()
{
    return scaleEndFreq;
}




void BandmapFreqDial::drawScale(QPainter *painter, qint32 frequency, int scaleHeight)
{

    Q_UNUSED(frequency)
    Q_UNUSED(scaleHeight)

    dialFreqList.clear();

    //dialHeight = scaleHeight;
    dialHeight = fullBandHeight;


    int _fontHeight = getFontHeight();
    if (_fontHeight != fontHeight)
    {
        fontHeight = _fontHeight;
    }

    if (newFreqTextWidth != freqTextWidth)
    {
        freqTextWidth = newFreqTextWidth;
        dialWidth = newFreqTextWidth + dialData::additionalWidth;
        changeBoundingRect(fullBandHeight, dialWidth);
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


    QRect scaleRec(0, 0, dialWidth, dialHeight);
    QBrush scaleBackGndBrush(Qt::lightGray, Qt::SolidPattern);

    QPen scalePen(Qt::black);
    scalePen.setWidth(2);
    painter->setPen(scalePen);

    painter->fillRect(scaleRec, scaleBackGndBrush);

    QRect freqSelRec(dialWidth - FREQ_SEL_WIDTH, 0, FREQ_SEL_WIDTH, dialHeight);
    QBrush freqSelBackGndBrush(Qt::gray, Qt::SolidPattern);
    painter->fillRect(freqSelRec, freqSelBackGndBrush);

    painter->drawLine(QPoint(dialWidth, 0 + dialData::DIAL_VERT_OFFSET), QPoint(dialWidth, dialHeight));

    // draw non operating freqs

    ModeFreqDetail<double> listOfFreqs;

    if (operatingPlanOk)
    {
        int freqOk = operatingFreq->getFreqLimitsForDial(listOfFreqs, contestBandStr, contestModeStr);
        if (freqOk == FREQ_OK)
        {

            QBrush noOperateFreqBackGndBrush(Qt::darkRed, Qt::SolidPattern);


            for (int i = 0; i < listOfFreqs.count(); i++)
            {
                QList<double> freqs = listOfFreqs.freq[i];
                if (freqs.count() == 0)
                    continue;
                if (freqs[1] > contestBandFhigh)
                {
                    freqs[1] = contestBandFhigh;
                }

                int x = dialWidth - NO_OP_FREQ_WIDTH;
                int y = getYCoordOnDial(static_cast<qint64>(freqs[0] * 1000)) + dialData::DIAL_VERT_OFFSET;
                int width = NO_OP_FREQ_WIDTH;
                int height = getYCoordOnDial(static_cast<qint64>(freqs[1] * 1000)) - getYCoordOnDial(static_cast<qint64>(freqs[0] * 1000));
                painter->fillRect(x, y, width, height, noOperateFreqBackGndBrush);
            }

        }
        else if (freqOk == MODE_MISSING)
        {
            trace(QString("bandmapDial: operating freq Mode missing from file"));
        }
        else if (freqOk == BAND_MISSING)
        {
            trace(QString("bandmapDial: operating freq Band missing from file"));
        }



    }



    QPen markerPen(Qt::blue);
    markerPen.setWidth(1);
    painter->setPen(markerPen);


    int markStep = dialData::khzStep[zoomLevel] * dialData::khzPixelStep[zoomLevel];
    int markCount = 0;
    qint32 markFreq = contestBandFlow;


    if (dialData::minorMarker[zoomLevel] == 0)
    {

        for (int ycoord = 0; ycoord < dialHeight; ycoord += markStep)
        {

            painter->drawLine(QPoint(dialWidth - dialData::fMajMrkLength, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialWidth, ycoord + dialData::DIAL_VERT_OFFSET));
            QRect textPos = QRect(dialData::fMajTextXStart, ycoord - (fontHeight/2) + dialData::DIAL_VERT_OFFSET, dialData::fMajTextXStart + freqTextWidth, fontHeight);
            QSharedPointer<DialFreqText> dft = QSharedPointer<DialFreqText>(new DialFreqText(textPos, markFreq));
            dialFreqList.append(dft);
            painter->drawText(textPos,  Qt::AlignLeft, convertFreqDialDisplay(markFreq));
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
                QRect textPos = QRect(dialData::fMajTextXStart, ycoord - (fontHeight/2) + dialData::DIAL_VERT_OFFSET, dialData::fMajTextXStart + freqTextWidth, fontHeight);
                QSharedPointer<DialFreqText> dft = QSharedPointer<DialFreqText>(new DialFreqText(textPos, markFreq));
                dialFreqList.append(dft);
                painter->drawText(textPos, Qt::AlignLeft, convertFreqDialDisplay(markFreq));
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
                QRect textPos = QRect(dialData::fMajTextXStart, ycoord - (fontHeight/2) + dialData::DIAL_VERT_OFFSET, dialData::fMajTextXStart + freqTextWidth, fontHeight);
                QSharedPointer<DialFreqText> dft = QSharedPointer<DialFreqText>(new DialFreqText(textPos, markFreq));
                dialFreqList.append(dft);
                painter->drawText(textPos, Qt::AlignLeft, convertFreqDialDisplay(markFreq));
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
                QRect textPos = QRect(dialData::fMajTextXStart, ycoord - (fontHeight/2) + dialData::DIAL_VERT_OFFSET, dialData::fMajTextXStart + freqTextWidth, fontHeight);
                QSharedPointer<DialFreqText> dft = QSharedPointer<DialFreqText>(new DialFreqText(textPos, markFreq));
                dialFreqList.append(dft);
                painter->drawText(textPos, Qt::AlignLeft, convertFreqDialDisplay(markFreq));
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




void BandmapFreqDial::setViewPortStartEndFreq(int startPos, int endPos, double contestBandFlow)
{
    scaleStartYCoord = startPos ;
    scaleEndYCoord = endPos;
    scaleStartFreq = getViewPortFreq(startPos, contestBandFlow);
    scaleEndFreq = getViewPortFreq(endPos, contestBandFlow);

}








/*

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
*/

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
    //qint32 offsetFreq = fmaj - scaleStartFreq;
    qint32 offsetFreq = fmaj - contestBandFlow;
    return (offsetFreq * dialData::khzPixelStep[zoomLevel]) + (fmin/dialData::hzPixelStep[zoomLevel]);

}



QString BandmapFreqDial::getFreqFromYCoordOnDial(int y)
{
    int dialPos = y - dialData::DIAL_VERT_OFFSET;
    qint32 fmaj = dialPos / dialData::khzPixelStep[zoomLevel]  * 1000;
    qint32 fmin = dialPos % dialData::khzPixelStep[zoomLevel] * dialData::hzPixelStep[zoomLevel];
    qint32 ftot = fmaj + fmin;
    qint64 freq = (contestBandFlow * 1000) + ftot;
    return QString::number(freq);
}


int BandmapFreqDial::getFullBandHeight(double flow, double fhigh)
{
    qint32 bandRange = static_cast<qint32>(fhigh - flow);
    qint32 bandRangeKhz = bandRange / 1000;
    fullBandHeight = static_cast<int>(bandRangeKhz * dialData::khzPixelStep[zoomLevel]);
    return fullBandHeight;

}

void BandmapFreqDial::setContestBandLimits(double flow, double fhigh)
{
    contestBandFlow = static_cast<qint32>(flow / 1000);
    contestBandFhigh = static_cast<qint32>(fhigh / 1000);
}

qint64 BandmapFreqDial::getViewPortFreq(int startPos, double contestBandFlow)
{
    qint64 calcFreq = 0;
    if (zoomLevel >= 0 && zoomLevel <= dialData::MAX_ZOOM_LEVEL)
    {
        qint64 offSetF = startPos / dialData::khzPixelStep[zoomLevel] * 1000;
        calcFreq = static_cast<qint64>(contestBandFlow) + offSetF;
    }

    return calcFreq;

}

void BandmapFreqDial::drawCursor(QPainter *painter, qint64 frequency)
{

    int cursorY = getYCoordOnDial(frequency);

    QPolygon freqCursor;

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


qint32 BandmapFreqDial::checkSelectedFreqTextOnDial(QPoint p)
{
    for (int i = 0; i < dialFreqList.count(); i++)
    {
        if (dialFreqList[i]->getTextRect().contains(p))
        {
            return dialFreqList[i]->getFreqText();
        }
    }

    return 0;
}

void BandmapFreqDial::setFreqOperatingInfo(const QString _contestBandStr, const QString _contestModeStr, CheckOperatingFreq *_operatingFreq, const bool _operatingPlanOk)
{

    if (_operatingPlanOk)
    {
        contestBandStr = _contestBandStr;
        contestModeStr = _contestModeStr;
        operatingFreq = _operatingFreq;
        operatingPlanOk = _operatingPlanOk;
    }

}
