////////////////////////////////////////////////////////////////////////////
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Bandmap Dial
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
/////////////////////////////////////////////////////////////////////////////


#include "bandmapfreqdial.h"
#include "ContestApp.h"
#include <QFontMetrics>

BandmapFreqDial::BandmapFreqDial(int _width, int _height):
    height(_height),
    zoomLevel(dialData::MAX_ZOOM_LEVEL),
    dialWidth(_width),
    scaleStartYCoord(0),
    scaleEndYCoord(0),
    operatingFreq(nullptr),
    operatingPlanOk(false),
    cursorColour(Qt::red)

{
    changeBoundingRect(1000, dialWidth);
    setAcceptHoverEvents(true);
}

void BandmapFreqDial::setHeight(int h)
{
    height = h;
}

double BandmapFreqDial::getHzPixelStepR() const
{
    // This routine is the single point that controls scale and zoom
    // Max zoom level is equivalent to the band covering the entire height
    // (i.e. most zoomed out)

    // What should most zoomed in (zoom level 0) represent?

    // default setting is "maximum"


    // (real) pixels/Hz at this zoom level

    qint64 range = contestBandFhigh - contestBandFlow;

    double h = height; // use the actual height, not the mapped height

    double ps = (h * (dialData::MAX_ZOOM_LEVEL - zoomLevel + 1))/range;

    return ps;
}

void BandmapFreqDial::onFontChanged(QFont /*cf*/)
{
    newFreqTextWidth = calcFreqWidth(currentFreq);

    update();
}

void BandmapFreqDial::setCursorColour(QColor colour)
{
    cursorColour = colour;
}

QRectF BandmapFreqDial::boundingRect() const
{
    int fontHeight = getFontHeight();
    int fbh = getFullBandHeight(contestBandFlow, contestBandFhigh);
    return QRectF(0, 0, dialWidth, fbh + fontHeight );
}

void BandmapFreqDial::changeBoundingRect(int /*height*/, int width)
{
    prepareGeometryChange();
    setCurWidth(width);
}

void BandmapFreqDial::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    drawScale(painter);
    drawCursor(painter, currentFreq);
}

void BandmapFreqDial::setCurFreq(const Frequency &f)
{
    if (f != currentFreq)
    {
        currentFreq = f;
        newFreqTextWidth = calcFreqWidth(f);
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

int BandmapFreqDial::getCurHeight()
{
    int fbh = getFullBandHeight(contestBandFlow, contestBandFhigh);
    return fbh;
}

void BandmapFreqDial::setCurWidth(int width)
{
    dialWidth = width;
}

int BandmapFreqDial::getCurWidth()
{
    return dialWidth;
}

int BandmapFreqDial::calcFreqWidth(const Frequency &freq)
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

int BandmapFreqDial::getFontHeight() const
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

void BandmapFreqDial::drawMarkerText(QPainter *painter, int ycoord, Frequency markFreq, int fontHeight)
{
    painter->drawLine(QPoint(dialWidth - dialData::fMajMrkLength, ycoord ),
                      QPoint(dialWidth, ycoord ));

    // make sure that text is within the band edges
    int ypos = ycoord - (fontHeight/2);
    if (ypos < 0)
    {
        ypos = ycoord;
    }
    int h = getCurHeight();
    if (ypos + fontHeight >= h)
    {
        ypos = h - fontHeight;
    }
    QRect textPos = QRect(dialData::fMajTextXStart, ypos ,
                          dialData::fMajTextXStart + freqTextWidth, fontHeight);
    QSharedPointer<DialFreqText> dft = QSharedPointer<DialFreqText>(new DialFreqText(textPos, markFreq));
    dialFreqList.append(dft);
    painter->drawText(textPos,  Qt::AlignLeft, convertFreqDialDisplay(markFreq));
}

void BandmapFreqDial::drawMarkerLine(QPainter *painter, int ycoord)
{
    painter->drawLine(QPoint(dialWidth - dialData::fMinMrkLength, ycoord ),
                      QPoint(dialWidth, ycoord ));
}

void BandmapFreqDial::drawScale(QPainter *painter)
{
    dialFreqList.clear();

    int fullBandHeight = getFullBandHeight(contestBandFlow, contestBandFhigh);
    int dialHeight = fullBandHeight ;

    int fontHeight = getFontHeight();
    if (dialHeight == 0)
    {
        return;
    }

    if (newFreqTextWidth != freqTextWidth)
    {
        freqTextWidth = newFreqTextWidth;
        dialWidth = newFreqTextWidth + dialData::additionalWidth;
        changeBoundingRect(dialHeight, dialWidth);
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

    painter->drawLine(QPoint(dialWidth, 0 ), QPoint(dialWidth, dialHeight));

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
                int y = getYCoordOnDial(freqs[0]);
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

    bool invertBandmap = false;
    TContestApp::getContestApp()->loggerBundle.getBoolProfile(elpBandmapInvert, invertBandmap);

    int markCount = 0;
    Frequency markFreq =( invertBandmap?contestBandFhigh:contestBandFlow );

    int ifInc = 100;    // we only display KHz, so smallest "tick"s are 100Hz
    double markStep = getYCoordOnDial(ifInc) - getYCoordOnDial(0);
    int labelStep = 10;
    int i = 0;
    while (height/markStep > 100 || height/markStep < -100)   // actual height, not scaled
    {
        ifInc = ifInc * ((i % 2 == 0)?5:2);
        markStep = getYCoordOnDial(ifInc) - getYCoordOnDial(0);
        i++;
    }

    markStep = std::abs(markStep);

    Frequency fInc(ifInc);
    double ycoord = 0;
    while (floor(ycoord) <= dialHeight)
    {
        if (markCount == 0 || markCount % labelStep == 0)   // just to match other cases
        {
            drawMarkerText(painter, floor(ycoord), markFreq, fontHeight);
        }
        else
        {
            drawMarkerLine(painter, ycoord);
        }
        if (invertBandmap)
        {
            markFreq = markFreq - fInc;
        }
        else
        {
            markFreq = markFreq + fInc;
        }
        ycoord += markStep;
        markCount++;
    }
}

void BandmapFreqDial::setViewPortStartEndFreq(int startPos, int endPos)
{
    scaleStartYCoord = startPos ;
    scaleEndYCoord = endPos;
    scaleStartFreq = getFreqFromYCoordOnDial(startPos );
    scaleEndFreq = getFreqFromYCoordOnDial(endPos );

}

QString BandmapFreqDial::convertFreqDialDisplay(const Frequency &freq)
{
    QString sfreq = freq.str();
    sfreq = sfreq.left(sfreq.length() - 3); // convert to Mhz.xxx

    int len = sfreq.length();

    switch(len)
    {
    case 9:
        sfreq = sfreq.insert(6, '.');
        sfreq = sfreq.insert(3, '.');
        break;
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
    case 4:
        sfreq = sfreq.insert(1, '.');
        break;
    default:
        sfreq = "000.000";  // error

    }
    return sfreq;
}

Frequency BandmapFreqDial::getFreqFromYCoordOnDial(int y)
{
    double ps = getHzPixelStepR();

    Frequency offset = y / ps;

    Frequency f;
    bool invertBandmap = false;
    TContestApp::getContestApp()->loggerBundle.getBoolProfile(elpBandmapInvert, invertBandmap);

    if (invertBandmap)
    {
        f = contestBandFhigh - offset;
    }
    else
    {
        f = contestBandFlow + offset;
    }

    return f;
}

double BandmapFreqDial::getYCoordOnDial(const Frequency &frequency) const
{
    // we have toreturn double, as the calculations resulting can severly
    // distort things if it is only an int

    // Offset of start of dial, KHz
    Frequency offsetFreq;

    bool invertBandmap = false;
    TContestApp::getContestApp()->loggerBundle.getBoolProfile(elpBandmapInvert, invertBandmap);

    if (invertBandmap)
    {
        offsetFreq = contestBandFhigh - frequency;
    }
    else
    {
        offsetFreq = frequency - contestBandFlow;
    }

    double p = (qint64(offsetFreq) * getHzPixelStepR());

    return p;
}

int BandmapFreqDial::getFullBandHeight(const Frequency &flow, const Frequency &fhigh) const
{
    // full band height in pixels at this zoom level

    int dLow = getYCoordOnDial(flow);
    int dHigh = getYCoordOnDial(fhigh);

    int fbh = std::abs(dHigh - dLow);
    return fbh;
}

void BandmapFreqDial::setContestBandLimits(const Frequency &flow, const Frequency &fhigh)
{
    contestBandFlow = flow;
    contestBandFhigh = fhigh;

    newFreqTextWidth = calcFreqWidth(fhigh);

}

void BandmapFreqDial::drawCursor(QPainter *painter, Frequency frequency)
{

    int cursorY = getYCoordOnDial(frequency);

    QPolygon freqCursor;

    freqCursor << QPoint(dialWidth -  10, cursorY - 10 );
    freqCursor << QPoint(dialWidth - 10, cursorY + 10 );
    freqCursor << QPoint(dialWidth, cursorY );
    freqCursor << QPoint(dialWidth -  10, cursorY - 10 );

    QBrush freqCursorBrush(cursorColour, Qt::SolidPattern);

    QPainterPath freqCursorPath;
    freqCursorPath.addPolygon(freqCursor);
    painter->fillPath(freqCursorPath, freqCursorBrush);
}

void BandmapFreqDial::wheelEvent(QGraphicsSceneWheelEvent *event)
{

    int numDegrees = event->delta() / 8;
    int numTicks = numDegrees / 15;

    if (numTicks >= 1)
    {
       changeZoom(false);

    }
    else if (numTicks <= -1)
    {
        changeZoom(true);

    }

    event->accept();
}

void BandmapFreqDial::changeZoom(bool direction)
{
    emit zoomUpdated(direction);
}

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

void BandmapFreqDial::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    QPoint p = event->pos().toPoint();
    Frequency freq = getFreqFromYCoordOnDial(p.y());

    setToolTip(freq.convertFreqStrDisp());
}


