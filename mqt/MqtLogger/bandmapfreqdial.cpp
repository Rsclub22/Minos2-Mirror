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
#include "ContestApp.h"
#include <QFontMetrics>




BandmapFreqDial::BandmapFreqDial(int _width, int _height):
    zoomLevel(dialData::MAX_ZOOM_LEVEL),
    dialHeight(_height),
    dialWidth(_width),
    scaleStartYCoord(0),
    scaleEndYCoord(0),
    fullBandHeight(2000),
    operatingFreq(nullptr),
    operatingPlanOk(false),
    cursorColour(Qt::red)

{
    changeBoundingRect(dialHeight, dialWidth);

}

void BandmapFreqDial::onFontChanged(QFont cf)
{
    newFreqTextWidth = checkFreqWidth(currentFreq);
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


    drawScale(painter, currentFreq, dialHeight);
    drawCursor(painter, currentFreq);


}





void BandmapFreqDial::setCurFreq(const Frequency &f)
{
    if (f != currentFreq)
    {
        currentFreq = f;
        newFreqTextWidth = checkFreqWidth(f);
    }

}


void BandmapFreqDial::setRadioMode(QString mode)
{

    radioMode = mode;
}


Frequency BandmapFreqDial::getCurFreq()
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

int BandmapFreqDial::checkFreqWidth(const Frequency &freq)
{
    //calc dial width
    QFont cf = QApplication::font();
    QFontMetrics fm(cf);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return fm.horizontalAdvance(convertFreqDialDisplay(freq));
#else
    return fm.width(convertFreqDialDisplay(freq));
#endif

}

int BandmapFreqDial::getFontHeight()
{
    QFont cf = QApplication::font();
    QFontMetrics fm(cf);
    return fm.height();

}

Frequency BandmapFreqDial::getScaleStartFreq()
{
    return scaleStartFreq;
}

Frequency BandmapFreqDial::getScaleEndFreq()
{
    return scaleEndFreq;
}




void BandmapFreqDial::drawScale(QPainter *painter, Frequency frequency, int scaleHeight)
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

    ModeFreqDetail<Frequency> listOfFreqs;

    bool operatingFreqFlag;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpBandMapTurnOffOperatingFreqStrip, operatingFreqFlag );


    if (operatingPlanOk && !operatingFreqFlag)
    {
        QString mode;
        bool savedFollowRadioModeFlag;
        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpBandMapFollowRadioModeOperatingFreqStrip, savedFollowRadioModeFlag);
        if (savedFollowRadioModeFlag)
        {
            mode = radioMode;
        }
        else
        {
            mode = contestModeStr;
        }


        int freqOk = operatingFreq->getFreqLimitsForDial(listOfFreqs, contestBandStr, mode);


        if (freqOk == FREQ_OK)
        {

            QBrush noOperateFreqBackGndBrush(Qt::darkRed, Qt::SolidPattern);


            for (int i = 0; i < listOfFreqs.count(); i++)
            {
                QList<Frequency> freqs = listOfFreqs.freq[i];
                if (freqs.count() == 0)
                    continue;
                if (freqs[1] > contestBandFhigh)
                {
                    freqs[1] = contestBandFhigh;
                }

                int x = dialWidth - NO_OP_FREQ_WIDTH;
                int y = getYCoordOnDial(freqs[0]) + dialData::DIAL_VERT_OFFSET;
                int width = NO_OP_FREQ_WIDTH;
                int height = getYCoordOnDial(freqs[1]) - getYCoordOnDial(freqs[0]);
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
    Frequency markFreq = contestBandFlow;


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
                markFreq = markFreq + Frequency(1000);
            }
            else
            {
                markFreq = markFreq + Frequency(50000);
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
                markFreq = markFreq + Frequency(5000);
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
                markFreq = markFreq + Frequency(10000);
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
                markFreq = markFreq + Frequency(50000);
            }
            else
            {
                painter->drawLine(QPoint(dialWidth - dialData::fMinMrkLength, ycoord + dialData::DIAL_VERT_OFFSET), QPoint(dialWidth, ycoord + dialData::DIAL_VERT_OFFSET));
            }

            markCount++;

        }

    }

}




void BandmapFreqDial::setViewPortStartEndFreq(int startPos, int endPos, Frequency contestBandFlow)
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

QString BandmapFreqDial::convertFreqDialDisplay(const Frequency &freq)
{

    QString sfreq = freq.str();
    sfreq = sfreq.left(sfreq.length() - 3); // convert to KHz

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



int BandmapFreqDial::getYCoordOnDial(const Frequency &frequency)
{
    Frequency fmaj = qint64(frequency)/1000;
    Frequency fmin = frequency - Frequency(qint64(fmaj)*1000);
    //qint32 offsetFreq = fmaj - scaleStartFreq;
    Frequency cbfl = Frequency(qint64(contestBandFlow)/1000);
    Frequency offsetFreq = fmaj - cbfl;
    return (qint64(offsetFreq) * dialData::khzPixelStep[zoomLevel]) + (qint64(fmin)/dialData::hzPixelStep[zoomLevel]);

}



Frequency BandmapFreqDial::getFreqFromYCoordOnDial(int y)
{
    int dialPos = y - dialData::DIAL_VERT_OFFSET;
    Frequency fmaj = (dialPos / dialData::khzPixelStep[zoomLevel]) * 1000 ;
    Frequency fmin = dialPos % dialData::khzPixelStep[zoomLevel] * dialData::hzPixelStep[zoomLevel];
    Frequency ftot = fmaj + fmin;
    Frequency freq = contestBandFlow + ftot;
    return freq;
}


int BandmapFreqDial::getFullBandHeight(const Frequency &flow, const Frequency &fhigh)
{
    Frequency bandRange = fhigh - flow;
    fullBandHeight = static_cast<int>(qint64(bandRange)/1000 * dialData::khzPixelStep[zoomLevel]);
    return fullBandHeight;

}

void BandmapFreqDial::setContestBandLimits(const Frequency &flow, const Frequency &fhigh)
{
    contestBandFlow = flow;
    contestBandFhigh = fhigh;
}

Frequency BandmapFreqDial::getViewPortFreq(int startPos, Frequency contestBandFlow)
{
    Frequency calcFreq;
    if (zoomLevel >= 0 && zoomLevel <= dialData::MAX_ZOOM_LEVEL)
    {
        qint64 offSetF = startPos / dialData::khzPixelStep[zoomLevel] * 1000;
        calcFreq = contestBandFlow + Frequency(offSetF);
    }

    return calcFreq;

}

void BandmapFreqDial::drawCursor(QPainter *painter, Frequency frequency)
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


Frequency BandmapFreqDial::checkSelectedFreqTextOnDial(QPoint p)
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




