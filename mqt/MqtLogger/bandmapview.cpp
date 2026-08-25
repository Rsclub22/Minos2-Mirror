////////////////////////////////////////////////////////////////////////////
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Bandmap View
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
//
/////////////////////////////////////////////////////////////////////////////
#include <QApplication>

#include "SendRPCDM.h"
#include "bandmapdatamodel.h"
#include "bandmapgraphicspanel.h"
#include "MinosLoggerEvents.h"
#include "bandmapspotmarker.h"
#include "tsinglelogframe.h"
#include "tlogcontainer.h"
#include "ContestApp.h"
#include "delayedaction.h"
#include "MTrace.h"

#include "bandmapview.h"

const int DIAL_CURSOR_BELOW_VIEWSTART_FREQ = 0;
const int DIAL_CURSOR_ABOVE_VIEWSTART_FREQ = 1;
const int DIAL_CURSOR_WITHIN_VIEWPORT = 2;

BandmapView::BandmapView(QWidget *parent) :
    QScrollArea(parent),
    zoomLevel(0),
    contestBandFlow(0),
    contestBandFhigh(0),
    fullBandHeight(4000),
    fontHeight(0)
{
    setFocusPolicy((Qt::ClickFocus));

    bandmapScene = new QGraphicsScene(parent);

    lessGreaterThanDistanceFlag = readLessGreaterThanDistanceFlag();
}

BandmapView::~BandmapView()
{
    setSuppressUpdate(true);
    clearListOfMarkers();
    delete bandmapScene;
}


void BandmapView::initBandmapView(BandmapGraphicsPanel* view )
{
    bandmapGraphicsView = view;

    bandmapGraphicsView->setScene(bandmapScene);
    bandmapGraphicsView->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    bandmapGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded );
    bandmapGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    bandmapScene->setSceneRect(0,0, bandmapGraphicsView->width(), fullBandHeight + horizontalScrollBar()->height() );

    dial = new BandmapFreqDial(70, bandmapGraphicsView->viewport()->height());
    dialMinZoomLevel = dial->getMinZoomLevel();
    dialMaxZoomLevel = dial->getMaxZoomLevel();

    bandmapScene->addItem(dial);
    dial->setCurFreq(Frequency());
    dial->setCursorColour(Qt::black);
    trace("BandmapView::bandmapUpdate() initBandMapView");
    bandmapUpdate(true);

    connect(dial, &BandmapFreqDial::zoomUpdated, this, &BandmapView::zoomUpdated);

    connect(bandmapGraphicsView, &BandmapGraphicsPanel::bandmapResize, this, &BandmapView::bandmapResize);
    connect(bandmapGraphicsView, &BandmapGraphicsPanel::leftMouseButtonPressed, this, &BandmapView::leftMouseButtonPressed);
    connect(bandmapGraphicsView, &BandmapGraphicsPanel::mouseDoubleClicked, this, &BandmapView::mouseDoubleClicked);

    connect(&updateTimer, &QTimer::timeout, this, &BandmapView::updateTimerTimeout);

    bandmapGraphicsView->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( bandmapGraphicsView, &BandmapGraphicsPanel::customContextMenuRequested, this, &BandmapView::on_bandmap_customContextMenuRequested);

    bandmapGraphicsView->setBackgroundBrush(QBrush());
}

void BandmapView::setContest(BaseContestLog *c)
{
    contest = c;
}
void BandmapView::onFontChanged(QFont cf)
{
    panelFont = cf;
    dial->onFontChanged(cf);
    QFontMetrics fm(cf);
    if (fontHeight != fm.height())
    {
        fontHeight = fm.height();
    }
}

void BandmapView::updateZoom(bool dir)
{
    zoomUpdated(dir);
}

int BandmapView::getDialZoomLevel()
{
    return dial->getZoomLevel();
}

void BandmapView::setBandmapZoom(int level)
{
    // we delay this as otherwise starting up with a non-default zoom
    // doesn't show the whole frequency range
    delayedAction(this, [=]{
        if (!LogContainer || LogContainer->isLoggerClosing())
        {
            return;
        }
        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
        if (!tslf)
        {
            return;
        }
        if (level <= dialMaxZoomLevel && level >= dialMinZoomLevel)
        {
            zoomLevel = level;
            dial->setZoomLevel(level);

            setBandmapHeight(contestBandFlow, contestBandFhigh);
            trace("BandmapView::bandmapUpdate() setBandMapZoom ");
            bandmapUpdate(true);
            scrollBandmapCenterToFreq(dial->getCurFreq());

            emit newZoomlevel(level);
        }
    }, 10);
}


void BandmapView::zoomUpdated(bool dir)
{
    zoomLevel = dial->getZoomLevel();

    if (dir)
    {
        setBandmapZoom(++zoomLevel);
    }
    else
    {
        setBandmapZoom(--zoomLevel);
    }

}

bool BandmapView::getSuppressUpdate() const
{
    return suppressUpdate;
}

void BandmapView::setSuppressUpdate(bool value)
{
    suppressUpdate = value;
}

void BandmapView::makeCursorVisibleInBandmap()
{
    int freqYCoord = dial->getYCoordOnDial(curFreq);
    int scrollStart = freqYCoord - (bandmapGraphicsView->viewport()->height() / 2);
    bandmapGraphicsView->verticalScrollBar()->setValue(scrollStart);
}

void BandmapView::scrollBandmapCenterToFreq(Frequency freq)
{
    int freqYCoord = dial->getYCoordOnDial(freq);
    int scrollStart = freqYCoord - (bandmapGraphicsView->viewport()->height() / 2);
    bandmapGraphicsView->verticalScrollBar()->setValue(scrollStart);
    bandmapGraphicsView->horizontalScrollBar()->setValue(0);
}

int BandmapView::getViewPortStartYCoordOnScene()
{
    return bandmapGraphicsView->mapToScene(QPoint(0,0)).toPoint().y();
}

int BandmapView::getViewPortEndYCoordOnScene()
{
    return bandmapGraphicsView->mapToScene( QPoint(
                                                bandmapGraphicsView->viewport()->width(),
                                                bandmapGraphicsView->viewport()->height() )).toPoint().y();
}
void BandmapView::doBandmapUpdate()
{
    if (!TSingleLogFrame::inApplyScreenLayout)
    {
        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
        if (!tslf)
        {
            return;
        }

        if (!getSuppressUpdate())
        {
            if (updateRequired)
            {
                dial->update();

                // delay the spots until any dial update has happened
                trace("Request delayed drawBandMapSpots()");
                delayedAction(this, [=](){
                    trace("Execute delayed drawBandMapSpots()");

                    drawBandMapSpots();
                }
                );
            }
        }
    }
}

void BandmapView::bandmapUpdate(bool now)
{
    if (!contest || contest->isReadOnly())
    {
        return;
    }
    if (!getSuppressUpdate())
    {
        updateRequired = true;
        updateTimer.start(now?0:1000);    // reset the interval if already started
    }
}

void BandmapView::updateTimerTimeout()
{
    updateTimer.stop();
    doBandmapUpdate();
    updateRequired = false;
}
void BandmapView::leftMouseButtonPressed(QPoint p)
{
    QPoint mappedP = bandmapGraphicsView->mapToScene(p).toPoint();

    if (mappedP.x() < dial->getCurWidth() && mappedP.x() >= 0)
    {
        // select the freq
        bandmapSelectFreq(mappedP.y());
    }
    else
    {
        bandmapSelectSpot(mappedP);
    }
}

void BandmapView::mouseDoubleClicked(QPoint p)
{
    QPoint mapP = bandmapGraphicsView->mapToScene(p).toPoint();

    int spotNum = isClickInRegionOfSpot(mapP);

    if (spotNum >= 0 && selectedSpot)
    {
        memoryData::memData spotData;
        spotData.callsign = selectedSpot->getDxCallStr();
        spotData.time = selectedSpot->getSpotTime();

        bool showDerivedLocFlag;
        TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpShowDerivedLoc, showDerivedLocFlag );

        if (showDerivedLocFlag || !selectedSpot->getDxLocatorIsFromNode())
        {
            spotData.locator = selectedSpot->getDxLocator();
        }
        spotData.bearing = selectedSpot->getDxBrg().toInt();
        spotData.fromBandmapOrMemory = true;
        spotData.exchange = selectedSpot->getDistrict();
        spotData.mode = selectedSpot->getMode();
        spotData.freq = selectedSpot->getFreq();

        MinosLoggerEvents::SendSpotToLog(spotData);
    }
}

void BandmapView::on_bandmap_customContextMenuRequested( const QPoint& p)
{
    QPoint mapP = bandmapGraphicsView->mapToScene(p).toPoint();

    if (mapP.x() >= dial->getCurWidth() && mapP.x() <= bandmapGraphicsView->width())
    {
        emit contextMenuSelected(p, mapP);
    }
}

int BandmapView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int BandmapView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

QRect BandmapView::visualRect(const QModelIndex &index) const
{
    QRect rect;
    if (index.isValid())
    {
        rect = viewportRectForRow(index.row()).toRect();
    }
    return rect;
}

QRectF BandmapView::viewportRectForRow(int row) const
{
    QRectF rect = QRectF();
    for (int i = 0; i < listOfMarkers.count(); i++)
    {
        if (listOfMarkers[i] != nullptr)
        {
            listOfMarkers[i]->getSpotRect();
            if (listOfMarkers[i]->getModelRowNum() == row)
            {
                rect = listOfMarkers[i]->getSpotRect();
                break;

            }
        }
    }
    return rect;
}

void BandmapView::setFilterSettings(BandmapClientFilterSettings* filterSettings_)
{
    filterSettings = filterSettings_;
}

void BandmapView::bandmapResize(QSize s)
{
    dial->setHeight(s.height() - bandmapGraphicsView->horizontalScrollBar()->height());
    trace("hscrollbar set value to 0");
    bandmapGraphicsView->horizontalScrollBar()->setValue(0);

    trace("BandmapView::bandmapUpdate() bandmapView::bandmapResize");
    bandmapUpdate(false);
}

int BandmapView::getBandmapFrameHeight()
{
    return bandmapGraphicsView->viewport()->height();
}

int BandmapView::getBandmapFrameWidth()
{
    return bandmapGraphicsView->viewport()->width();
}

void BandmapView::setFreq(Frequency cf, bool legalFreq)
{
    curFreq = cf;

    int dfwv = dialCursorWithinViewport(curFreq);
    Frequency freqWidth = dial->getScaleEndFreq() - dial->getScaleStartFreq();

    Frequency edgeAmount = freqWidth/5;

    if (dfwv == DIAL_CURSOR_BELOW_VIEWSTART_FREQ)
    {
        // tuning up, move viewport
        bandmapGraphicsView->verticalScrollBar()->setValue(dial->getYCoordOnDial(curFreq - edgeAmount));
    }
    else if (dfwv == DIAL_CURSOR_ABOVE_VIEWSTART_FREQ)
    {
        // tuning down, move viewport
        bandmapGraphicsView->verticalScrollBar()->setValue(dial->getYCoordOnDial(curFreq - freqWidth + edgeAmount));
    }

    dial->setCurFreq(curFreq);
    if (legalFreq)
    {
        dial->setCursorColour(Qt::black);
    }
    else
    {
        dial->setCursorColour(Qt::red);
    }

    if (!curFreq.isClear())
    {
        trace(QString("BandmapView::bandmapUpdate() bandmapView::setFreq %1").arg(curFreq.traceStr()));
        bandmapUpdate(true);
    }
    if (selectedSpot && selectedSpot->getFreq() != curFreq)
    {
        clearSelectedSpot();
    }
}

void BandmapView::setDialRadioMode(QString mode)
{
    int colPos = mode.indexOf(":");
    if (colPos > 0)
    {
        curMode = curMode.left(colPos);
    }
    else
    {
        curMode = mode;
    }
    dial->setRadioMode(mode);
    trace("BandmapView::bandmapUpdate() bandmapView::setDialRadioMode");
    bandmapUpdate(true);
}

int BandmapView::dialCursorWithinViewport(Frequency freq)
{
    int sceneStartYCoord = bandmapGraphicsView->mapToScene(0,0).toPoint().y();
    int sceneEndYCoord = bandmapGraphicsView->mapToScene(0, bandmapGraphicsView->viewport()->height()).toPoint().y();

    dial->setViewPortStartEndFreq(sceneStartYCoord, sceneEndYCoord );

    Frequency freqWidth = dial->getScaleEndFreq() - dial->getScaleStartFreq();
    Frequency edgeAmount = freqWidth/5;

    if (freq < dial->getScaleStartFreq() + edgeAmount)
    {
        return DIAL_CURSOR_BELOW_VIEWSTART_FREQ;
    }
    else if (freq > dial->getScaleEndFreq() - edgeAmount)
    {
        return DIAL_CURSOR_ABOVE_VIEWSTART_FREQ;
    }

    return DIAL_CURSOR_WITHIN_VIEWPORT;
}

void BandmapView::bandmapSelectFreq(int y)
{
    Frequency f = dial->getFreqFromYCoordOnDial(y);
    MinosLoggerEvents::SendFreqToRig(f);
}

void BandmapView::setBandFreqLimits(Frequency flow, Frequency fhigh)
{
    contestBandFlow = flow;
    contestBandFhigh = fhigh;
    dial->setContestBandLimits(contestBandFlow, contestBandFhigh);
}

void BandmapView::setBandmapHeight(Frequency flow, Frequency fhigh)
{
    fullBandHeight = dial->getFullBandHeight(flow, fhigh);
    dial->changeBoundingRect(fullBandHeight + horizontalScrollBar()->height() , dial->getCurWidth());
    bandmapScene->setSceneRect(0,0, bandmapGraphicsView->width(), fullBandHeight + horizontalScrollBar()->height() );
}

int BandmapView::isClickInRegionOfSpot(QPoint p)
{
    if (!listOfMarkers.isEmpty())
    {
        for (int i = 0; i < listOfMarkers.count(); i++)
        {
            if (listOfMarkers[i]->getSpotRect().contains(p))
            {
                return i;
            }
        }
    }
    return -1;
}


void BandmapView::bandmapSelectSpot(QPoint p)
{
    clearSelectedSpot();        // clear any selected spots
    int spotViewNum = isClickInRegionOfSpot(p);

    if (spotViewNum != -1)
    {
        if (spotViewNum >= 0)
        {
            setSelectedSpot(spotViewNum);        // mark new selected spot

            MinosLoggerEvents::SendFreqToRig(selectedSpot->getFreq());
        }
    }
}

void BandmapView::clearSelectedSpot()
{
    if (selectedSpot)
    {
        selectedSpot->setIsSelected(false);
        selectedSpot.clear();
        trace("BandmapView::bandmapUpdate() bandmapView::clearSelectedSpot");
        bandmapUpdate(true);
    }
}


void BandmapView::clearSelectedSpotData()
{
    if (selectedSpot)
    {
        selectedSpotViewRowNum = NO_SELECTED_ROWNUM;
        selectedSpot.clear();
    }
}

void BandmapView::setSelectedSpot(int spotViewNum)
{
    if (spotViewNum > listOfMarkers.count()
            || spotViewNum == NO_SELECTED_ROWNUM
            || listOfMarkers[spotViewNum]->getModelRowNum() > bandmapDataModel->rowCount()
            || listOfMarkers[spotViewNum]->getModelRowNum() < 0)
    {
        return;
    }

    selectedSpotViewRowNum = spotViewNum;

    selectedSpot = getSpotData(spotViewNum);

    selectedSpot->setIsSelected(true);
    trace("BandmapView::bandmapUpdate() bandmapView::setSelectedSpot");
    bandmapUpdate(true);
}

void BandmapView::clearListOfMarkers()
{
    for (int i = 0; i < listOfMarkers.count(); i++)
    {
        delete listOfMarkers[i];
    }
    listOfMarkers.clear();
}

QSharedPointer<ClusterSpotData> BandmapView::getSpotData(int selectedSpotViewRowNum)
{
    QSharedPointer<ClusterSpotData> sd;
    int selectedSpotDataRowNum = listOfMarkers[selectedSpotViewRowNum]->getModelRowNum();

    if (selectedSpotDataRowNum >= 0 && selectedSpotDataRowNum < bandmapDataModel->rowCount())
    {
        sd = bandmapDataModel->getBandmapDataRow(selectedSpotDataRowNum);
    }
    return sd;
}

int BandmapView::getSpotDataRow(QSharedPointer<ClusterSpotData> sd)
{
    return bandmapDataModel->getSpotDataRow(sd);
}
bool BandmapView::filterAcceptsRow(int sourceRow) const
{
    QString filterString = bandmapDataModel->getFilterString();
    if (filterString.isEmpty())
        return true;

    BandmapData *cgm = bandmapDataModel;

    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    QSharedPointer<ClusterSpotData> spotData = cgm->getBandmapDataRow(sourceRow);

    QString call = spotData->getDxCall().getFullCall();
    QString loc = spotData->getDxLocator();

    if (call.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;
    if (loc.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

    return false;
}
void BandmapView::drawBandmapSpot(int row, int &fontOffset, int markersAbove, int &lastOffset, bool &firstDrawn)
{
    ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();

    bandmapSpotType::SPOT_TYPE savedSpotType = pSpot->getSpotType();
    if (savedSpotType == bandmapSpotType::DELETED)
    {
        return;
    }
    if (matchMode(row) && matchDistance(row) && filterAcceptsRow(row))
    {
        Frequency spotFreq = pSpot->getFreq();

        if (spotFreq < contestBandFlow || spotFreq > contestBandFhigh)
        {
            return;
        }

        int centreYCoord = dial->getYCoordOnDial(curFreq);
        int dialWidth = dial->getCurWidth();

        BandmapMarkerDetails* markerDetails = new BandmapMarkerDetails(QPoint(0, 0));
        listOfMarkers.append(markerDetails);

        int yCoord = dial->getYCoordOnDial(spotFreq);
        int syCoord;

        if (row >= markersAbove)
        {
            // marker at higher frequency
            syCoord = centreYCoord + fontOffset;
            if ( firstDrawn)// don't move first one
            {
                syCoord = yCoord;
                fontOffset = yCoord - centreYCoord;
            }
            else
            {
                if (syCoord < yCoord )
                {
                    syCoord = yCoord;
                    fontOffset = yCoord - centreYCoord;
                }
                if (fontOffset <= lastOffset + fontHeight)
                {
                    fontOffset = lastOffset + fontHeight;
                    if (fontOffset < 0)
                        fontOffset = 0;
                    syCoord = centreYCoord + fontOffset;

                }
            }
        }
        else
        {
            // marker at lower frequency
            syCoord = centreYCoord - fontOffset;
            if (syCoord > yCoord)
            {
                syCoord = yCoord;
                fontOffset = centreYCoord - yCoord;
            }
            if (lastOffset > 0 && fontOffset <= lastOffset + fontHeight)
            {
                fontOffset = lastOffset + fontHeight;
                syCoord = centreYCoord - fontOffset;

            }
        }
        lastOffset = fontOffset;
        //spotCoord is left, top
        QPoint spotCoord = QPoint(dialWidth + SPOTMARKER_XOFFSET, syCoord - fontHeight/2);
        QPoint startMarkerLine = QPoint(dialWidth + SPOTMARKER_XOFFSET, syCoord + fontHeight/3);

        markerDetails->setSpotMarkerCoord(QPoint(dialWidth + SPOTMARKER_XOFFSET, syCoord));
        BandmapSpotMarker* spot = new BandmapSpotMarker(spotCoord); //spotcoord is used for the text position
        bandmapScene->addItem(spot);

        QPoint endMarkerLine = QPoint(dialWidth, yCoord );

        QLine markerLineCoord = QLine(startMarkerLine, endMarkerLine);
        QLineF markerLineCoordsF = QLineF(markerLineCoord);

        QGraphicsLineItem* markerLine = new QGraphicsLineItem(markerLineCoordsF);
        bandmapScene->addItem(markerLine);

        QString spotMsg;
        QRectF spotRect;
        QString spotTooltipText;

        if (savedSpotType == bandmapSpotType::CQ)
        {
            assembleCqMsg(row, spotMsg);
            spotRect = calculateSpotRect(spotMsg, spotCoord);
            assembleCqToolTip(row, spotFreq, spotTooltipText);
        }
        else
        {
            assembleSpotMsg(row, spotMsg);
            spotRect = calculateSpotRect(spotMsg, spotCoord);
            assembleToolTip(row, spotFreq, spotTooltipText);
        }
        spot->setSpotText(spotMsg);
        spot->setToolTipText(spotTooltipText);

        markerDetails->setSpotMarkerPtr(spot);
        markerDetails->setMarkerLinePtr(markerLine);
        markerDetails->setSpotRect(spotRect);
        markerDetails->setModelRowNum(row);

        fontOffset += fontHeight;
        firstDrawn = false;
    }
}

void BandmapView::drawBandMapSpots()
{
    if (!parent())
    {
        return;     // lambda in BandmapView::bandmapUpdate() fired after we have been detached
    }
    TContestApp *ta = TContestApp::getContestApp();
    if (!ta)
    {
        return;
    }
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    if (!tslf)
    {
        return;
    }

    if (TSingleLogFrame::inApplyScreenLayout)
    {
        return;
    }

    nearMatches.clear();
    traceMsg(QString("drawBandMapSpots: Start Drawing - Clear Map"));

    deleteItemsFromMarkerList();

    clearListOfMarkers();

    // don't draw spots when freq is zero, or no spots
    int numrows = bandmapDataModel->rowCount();
    if ( numrows == 0)
    {
        traceMsg(QString("CurFreq = %1, Number of Spots = %2 - Don't draw markers").arg(curFreq.traceStr()).arg(bandmapDataModel->rowCount()));
        return;
    }
    traceMsg(QString("drawBandMapSpots: Number of Rows to Check = %1").arg(numrows));

    bool invertBandmap = false;
    TContestApp::getContestApp()->loggerBundle.getBoolProfile(elpBandmapInvert, invertBandmap);

    int dialWidth = dial->getCurWidth();
    int dialHeight = dial->getCurHeight();

    QFont cf = panelFont;
    QFontMetrics fm(cf);
    fontHeight = fm.height();

    if (dialHeight == 0 || fontHeight == 0)
        return;

    bool centreTextOnFrequency = true;
    bool btemp;
    ta->loggerBundle.getBoolProfile( elpBandmapOldStyle, btemp );
    centreTextOnFrequency = !btemp;

    // this is for test
    ta ->loggerBundle.getBoolProfile( elpBandMapTraceDebug, traceDebugFlag );
    if (traceDebugFlag)
    {
        traceMsg(QString("dump list of spots and freq"));

        for (int row = 0; row < numrows; row++)
        {
            ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();
            Frequency freq = pSpot->getFreq();
            QString callsign = pSpot->getDxCallStr();
            traceMsg(QString("DB# = %1, Callsign = %2, Freq = %3").arg(row).arg( callsign, freq.traceStr()));
        }
    }

    if (centreTextOnFrequency)
    {
        int markersAbove = 0;
        int centreSpot;
        for (centreSpot = 0; centreSpot < bandmapDataModel->rowCount(); ++centreSpot)
        {
            ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(centreSpot).data();

            Frequency freq = pSpot->getFreq();

            // If they are in the table, we have to account for them
            // we can filter when looking at the individuals later

            if (invertBandmap)
            {
                if (freq < curFreq)
                {
                    break;
                }
            }
            else if (freq > curFreq)
            {
                break;
            }
            markersAbove++;
        }
        int fontOffset = -1;
        int lastOffset = -1;
        int firstOffset = -1;
        bool firstDrawn = true;
        for (int row = markersAbove - 1; row >= 0; --row)
        {
            drawBandmapSpot(row, fontOffset, markersAbove, lastOffset, firstDrawn);
            if (firstOffset == -1)
            {
                firstOffset = fontOffset;
            }
        }
        fontOffset = -1;
        lastOffset = -1;
        if (firstOffset >= 0)
        {
            lastOffset = -firstOffset + fontHeight;
        }

        for (int row = markersAbove; row < numrows; ++row)
        {
            drawBandmapSpot(row, fontOffset, markersAbove, lastOffset, firstDrawn);
        }

        if (listOfMarkers.size() == 0)
        {
            return;
        }
        std::sort(listOfMarkers.begin(), listOfMarkers.end(),
                  [invertBandmap](const BandmapMarkerDetails* a, const BandmapMarkerDetails* b) -> bool
                    {
                        if (invertBandmap)
                        {
                            return b->getSpotMarkerPtr()->y() < a->getSpotMarkerPtr()->y();
                        }
                        return a->getSpotMarkerPtr()->y() < b->getSpotMarkerPtr()->y();
                    }
                  );
        int miny = listOfMarkers[0]->getSpotMarkerCoord().y() - fontHeight;
        int maxy = listOfMarkers[listOfMarkers.size() - 1]->getSpotMarkerCoord().y() + fontHeight;

        miny = std::min(0, miny);
        int maxycorr = -std::min(miny, 0);
        maxy = std::max(fullBandHeight + maxycorr, maxy + maxycorr);

        bandmapScene->setSceneRect(0, miny, bandmapGraphicsView->width(), maxy + horizontalScrollBar()->height() );

    }
    else
    {
        int maxNumSpots = dialHeight/fontHeight;

        if (maxNumSpots == 0)
            return;
        int textYCoord = 0;

        for (int i = 0; i < maxNumSpots; i++)
        {
            // listOfMarkers should be naturally sorted so long as bandMapDataModel is
            BandmapMarkerDetails* markerDetails = new BandmapMarkerDetails(QPoint(dialWidth + SPOTMARKER_XOFFSET, textYCoord));
            listOfMarkers.append(markerDetails);
            textYCoord += fontHeight;
        }

        for (int row = 0; row < numrows; ++row)
        {
            ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();
            bandmapSpotType::SPOT_TYPE savedSpotType = pSpot->getSpotType();
            if (savedSpotType == bandmapSpotType::DELETED)
            {
                continue;;
            }
           // check mode and distance against the filter settings
            if (matchMode(row) && matchDistance(row) && filterAcceptsRow(row))
            {

                Frequency f = pSpot->getFreq();

                if (f <= contestBandFhigh && f >= contestBandFlow)
                {
                    int yCoord = dial->getYCoordOnDial(f);
                    for (int markNum = 0; markNum < listOfMarkers.count(); markNum++)
                    {
                        int fontOffset;
                        if (markNum == 0)
                        {
                            fontOffset = 0;
                        }
                        else
                        {
                            fontOffset = fontHeight;
                        }

                        if (listOfMarkers[markNum]->getSpotMarkerCoord().y() + fontOffset  >= yCoord && markNum != 0)
                        {
                            if (listOfMarkers[markNum]->getSpotMarkerPtr() == nullptr)
                            {
                                QPoint spotCoord = QPoint(listOfMarkers[markNum]->getSpotMarkerCoord().x(), listOfMarkers[markNum]->getSpotMarkerCoord().y());
                                BandmapSpotMarker* spot = new BandmapSpotMarker(spotCoord);

                                bandmapScene->addItem(spot);

                                QString spotMsg;
                                QRectF spotRect;
                                QString spotTooltipText;

                                if (savedSpotType == bandmapSpotType::CQ)
                                {
                                    assembleCqMsg(row, spotMsg);
                                    spotRect = calculateSpotRect(spotMsg, spotCoord);
                                    assembleCqToolTip(row, f, spotTooltipText);
                                }
                                else
                                {
                                    assembleSpotMsg(row, spotMsg);
                                    spotRect = calculateSpotRect(spotMsg, spotCoord);
                                    assembleToolTip(row, f, spotTooltipText);
                                }


                                spot->setSpotText(spotMsg);

                                spot->setToolTipText(spotTooltipText);

                                listOfMarkers[markNum]->setSpotMarkerPtr(spot);

                                QPoint startMarkerLine = QPoint(dialWidth + SPOTMARKER_XOFFSET, listOfMarkers[markNum]->getSpotMarkerCoord().y() + fontHeight);

                                QPoint endMarkerLine = QPoint(dialWidth, yCoord );

                                QLine markerLineCoord = QLine(startMarkerLine, endMarkerLine);
                                QLineF markerLineCoordsF = QLineF(markerLineCoord);

                                QGraphicsLineItem* markerLine = new QGraphicsLineItem(markerLineCoordsF);
                                bandmapScene->addItem(markerLine);

                                listOfMarkers[markNum]->setMarkerLinePtr(markerLine);
                                listOfMarkers[markNum]->setSpotRect(spotRect);
                                listOfMarkers[markNum]->setModelRowNum(row);

                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    bool disablePlaceHolders;
    TContestApp::getContestApp()->loggerBundle.getBoolProfile(elpBandMapDisablePlaceHolders, disablePlaceHolders);

    if (!disablePlaceHolders)
    {
        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
        if (tslf && tslf->getContest() == contest)
        {
            nearMatches.sort();
            tslf->setPlaceholders(nearMatches);

            //xxxxxxxx here post nearMatches so that DataModes see them
            // Do we publish, or send to interested parties?

            LogContainer->sendDM->publishPlaceHolders(nearMatches);
        }
    }
    traceMsg(QString("drawBandMapSpots: finished"));
}


void BandmapView::deleteItemsFromMarkerList()
{
    if (!listOfMarkers.isEmpty())
    {
        for (int i = 0; i < listOfMarkers.count(); i++)
        {
            if (listOfMarkers[i]->getSpotMarkerPtr() != nullptr)
            {
                BandmapSpotMarker* s = listOfMarkers[i]->getSpotMarkerPtr();
                s->clearSpotText();
                s->clearToolTipText();

                bandmapScene->removeItem(listOfMarkers[i]->getSpotMarkerPtr());
                delete s;
            }
            if (listOfMarkers[i]->getMarkerLinePtr() != nullptr)
            {
                QGraphicsLineItem* l = listOfMarkers[i]->getMarkerLinePtr();
                bandmapScene->removeItem(listOfMarkers[i]->getMarkerLinePtr());
                delete l;
            }
        }
    }
}

bool BandmapView::matchMode(int sourceRow)
{
    ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(sourceRow).data();

    QString mode = pSpot->getMode();

    if (!mode.isEmpty())
    {
        return filterSettings->getModeFilter(mode);
    }
    else
    {
        return false;
    }
}

bool BandmapView::matchDistance(int sourceRow)
{
    if (!filterSettings->getIgnoreDistanceFlag())
    {
        bool ok = false;

        ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(sourceRow).data();

        bandmapSpotType::SPOT_TYPE savedSpot = pSpot->getSpotType();
        QString distanceStr = pSpot->getDxDist();
        if (distanceStr.isEmpty() && filterSettings->getIgnoreEmptyDistanceFlag()
            && savedSpot == bandmapSpotType::CLUSTER )
        {
            return false;
        }

        int distance = distanceStr.toInt(&ok);
        if (ok)
        {
            return filterSettings->testDistance(distance, lessGreaterThanDistanceFlag);
        }
    }
    return true;
}


QRectF BandmapView::calculateSpotRect(const QString text, const QPoint spotCoord)
{
    QFontMetrics fm(font());
    const int rowHeight = fm.height();

    int textWidth = fm.boundingRect(text).width();
    
    return QRectF(spotCoord.x(),spotCoord.y(), textWidth, rowHeight);
}

void BandmapView::assembleCqMsg(int row, QString& markerMsg)
{
    ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();

    bool offRunFreq = pSpot->getOffRunFreq();
    QString msgColourStart;
    if (offRunFreq)
    {
        msgColourStart = HtmlFontColour("yellow");
    }
    else
    {
        msgColourStart = HtmlFontColour("orange");
    }

    QString msgColourEnd = HtmlFontColour("black");


    Frequency freq = pSpot->getFreq();

    Frequency cFreq = dial->getCurFreq();

    QString bLineStart = "";
    QString bLineEnd = "";

    if (freq == cFreq )
    {
        bLineStart = "<b>";
        bLineEnd = "</b>";
    }


    QString msg = tr("%1%2CQ Frequency @ .%3%4%5").arg(bLineStart, msgColourStart, freq.extractKhz(), msgColourEnd, bLineEnd);
    markerMsg = msg;
}

void BandmapView::assembleSpotMsg(int row, QString& markerMsg)
{
    ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();

    QString dxCallsign = pSpot->getDxCallStr();
    Frequency freq = pSpot->getFreq();
    Frequency cFreq = dial->getCurFreq();
    QString dxLoc = pSpot->getDxLocator();
    QString dxMode = pSpot->getMode();
    bool locWkd = pSpot->getDxLocatorWorked();
    QString dxDist = pSpot->getDxDist();
    QString dxBrg = pSpot->getDxBrg();
    QString rotBrg = pSpot->getRotBrg();
    bool rotConnected = pSpot->getRotConnected();
    bool dxLocFromNodeFlag = pSpot->getDxLocatorIsFromNode();

    QString dxQth = pSpot->getDistrict();
    bool qthWorked = pSpot->getDistrictWorked();

    bool showDerivedLocFlag;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpShowDerivedLoc, showDerivedLocFlag );

    bandmapSpotType::SPOT_TYPE spotType = pSpot->getSpotType();

    qlonglong spotTime = pSpot->getRxTime();
    bool olderThan3Min = spotTimedOut(spotTime, NEW_SPOT_TIME);

    QString newSpotMsg = "";
    if (!olderThan3Min)
    {
        newSpotMsg = HtmlFontColour(BANDMAP_NEW_COLOUR) + tr("New") +  HtmlFontColour(NOT_WORKED_COLOUR);
    }
    Callsign cs;
    cs.setFullCall(dxCallsign);

    CheckableContact test(contest, cs, contest->currentBand.getValue(), contest->currentMode.getValue());
    CheckableContact *cc = contest->haveWorked(&test);
    bool callWkd = cc != nullptr;
//    traceMsg(QString("test callsign %1 mode %2").arg(test->cs.getFullCall(), test->mode.getValue()));
    if (cc!= nullptr)
    {
        pSpot->setDxCallWorked(true);
//        traceMsg(QString("cc callsign %1 mode %2").arg(cc->cs.getFullCall(), cc->mode.getValue()));
    }
    else
    {
        pSpot->setDxCallWorked(false);
//        traceMsg("No dup contact found");
    }

    QString callsign;
    if (callWkd)
    {
        callsign = QString("%1%2%3").arg(HtmlFontColour(CALLSIGN_WORKED_COLOUR), dxCallsign, HtmlFontColour(NOT_WORKED_COLOUR));
    }
    else
    {
        callsign = dxCallsign;
    }
    QString district;
    if (qthWorked)
    {
        district = QString("%1%2%3").arg(HtmlFontColour(QTH_WORKED_COLOUR), dxQth, HtmlFontColour(NOT_WORKED_COLOUR));
    }
    else
    {
        district = dxQth;
    }

    QString locator;
    QString bearing;
    QString distance;

    if (!dxLocFromNodeFlag || showDerivedLocFlag)
    {
        if (!dxLoc.isEmpty() && dxLocFromNodeFlag)
        {
            dxLoc = "<i>" + dxLoc + "</i>";
        }
        if (locWkd && !dxLocFromNodeFlag)
        {
            locator = QString("%1%2%3").arg(HtmlFontColour(LOCATOR_WORKED_COLOUR), dxLoc, HtmlFontColour(NOT_WORKED_COLOUR));
        }
        else
        {
            if (!dxLoc.isEmpty())
            {
                locator = dxLoc;
            }
        }

        QChar degSym = QChar(DEG_SYMBOL);
        if (dxLoc.isEmpty())
        {
           if (rotConnected)
           {
               bearing = QString("%1%2 R").arg(rotBrg).arg(degSym);
           }
        }
        else
        {
            if (!dxBrg.isEmpty())
            {
                bearing = QString("%1%2").arg(dxBrg).arg(degSym);
            }
        }

        if (!dxDist.isEmpty())
        {
            distance = QString("%1 km").arg(dxDist);
        }
    }

    QString bLineStart = "";
    QString bLineEnd = "";

    int tol = 0;
    if (curMode == PH || curMode == hamlibData::USB || curMode == hamlibData::LSB || curMode == hamlibData::FM)
    {
        tol = 1000;
    }
    else if (curMode == PSK || curMode == RY || curMode == hamlibData::RTTY)
    {
        tol = 100;
    }
    else if (curMode == hamlibData::CW)
    {
        tol = 100;
    }
    Frequency f = cFreq;
    int offset = std::abs(freq - f);
    if (tol > 0 && offset < tol )
    {
        // highlight this line as current frequency
        bLineStart = "<b>";
        bLineEnd = "</b>";

        QString nm;
        QTextStream os(&nm);
        os.setFieldWidth(5);
        os << offset;
        os.setFieldWidth(0);
        os << "|" << dxCallsign << "|" << (dxLocFromNodeFlag?QString():dxLoc) << "|" << dxMode;
        os << "|" << dxQth;

        nearMatches.push_back(nm);
    }

    QString markSym = "";
    if (spotType == bandmapSpotType::MARKED || spotType == bandmapSpotType::SAVED)
    {
        markSym = HtmlFontColour(MARKED_SPOT_COLOUR) + "#" + HtmlFontColour(NOT_WORKED_COLOUR);
    }
    else if (spotType == bandmapSpotType::CLUSTER)
    {
        markSym = HtmlFontColour(CLUSTER_SPOT_COLOUR) + "*" + HtmlFontColour(NOT_WORKED_COLOUR);
    }
    else if (spotType == bandmapSpotType::CLUSTER_MARKED)
    {
        markSym = HtmlFontColour(CLUSTER_SPOT_COLOUR) + "*" + HtmlFontColour(MARKED_SPOT_COLOUR) + "#" + HtmlFontColour(NOT_WORKED_COLOUR);
    }

    qlonglong st = pSpot->getRxTime();
    qlonglong elapsedTime = spotElapsedTime(st) / 60;
    QString etc = formatTime(elapsedTime);

    QString msg = QString("%1%2 %3 @ .%4 %5 %6 %7 %8 %9 %10%11")
                      .arg(bLineStart, callsign, district, freq.extractKhz(), locator, distance)
                      .arg(bearing, etc, markSym, newSpotMsg, bLineEnd);

    if (pSpot->getIsSelected())
    {
        msg = QString("<div style='background:rgba(200, 200, 200, 75%);'>" + msg + QString("</div<"));           // show selected

    }

    markerMsg = msg;
}


void BandmapView::assembleCqToolTip(int row, Frequency freq, QString& toolTipMsg)
{
    ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();

    QString mode = pSpot->getMode();
    QString msg = tr("CQ Frequency = %1\nThe mode is %2").arg(freq.convertFreqStrDisp(), mode);
    toolTipMsg = msg;
}

void BandmapView::assembleToolTip(int row, Frequency freq, QString& toolTipMsg)
{
    ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();

    QChar degSym = QChar(DEG_SYMBOL);
    QString callsign = pSpot->getDxCallStr();
    QString locator = pSpot->getDxLocator();
    QString bearing = pSpot->getDxBrg();
    if (!bearing.isEmpty())
    {
        bearing += degSym;
    }
    QString distance = pSpot->getDxDist();
    if (!distance.isEmpty())
    {
        distance += " km";
    }
    QString spotterCallsign = pSpot->getSpotterCallStr();
    QString spotterLocator = pSpot->getSpotterLocator();

    QString spotterComment = pSpot->getSpotComment();
    QString computedMode = pSpot->getMode();
    bandmapSpotType::SPOT_TYPE spotType = pSpot->getSpotType();

    QString spotName = ClusterSpotData::spotName(spotType);
    QString spotModeMsg = tr("The computed mode is");
    if (spotType == bandmapSpotType::LOGGED)
    {
        spotModeMsg = tr("The mode is");
    }

    qlonglong spotTime = pSpot->getRxTime();
    qlonglong elapsedTime = spotElapsedTime(spotTime) / 60;
    QString etc = formatTime(elapsedTime);

    QString msg = tr("%1 - %2, %3, %4, %5 [%6 %7 @ %8] \n%9 %10\n%11\n%12")
                                            .arg(callsign, freq.convertFreqStrDisp(), locator, bearing, distance)
                                            .arg(spotterCallsign, spotterLocator, etc, spotModeMsg, computedMode, spotterComment, spotName);

    toolTipMsg = msg;
}


void BandmapView::setFreqOperatingInfo(const QString contestBandStr, const QString contestModeStr, CheckOperatingFreq *operatingFreq, const bool operatingPlanOk)
{
    if (operatingPlanOk)
    {
        dial->setFreqOperatingInfo(contestBandStr, contestModeStr, operatingFreq, operatingPlanOk);
    }

}

bool BandmapView::readLessGreaterThanDistanceFlag()
{
    QSettings config(CLUSTER_FILTER_FILE(), QSettings::IniFormat);
    return config.value(LESS_GREATER_THAN_DISTANCE_FLAG_INI_NAME, false).toBool();

}

void BandmapView::traceMsg(QString msg)
{
    trace(QString("[bandmapView] %1").arg(msg));
}


void BandmapView::traceTables()
{
    traceMsg(QString("curFreq %1 selectedSpotViewRowNum %2").arg(curFreq.traceStr()).arg(selectedSpotViewRowNum));
    int numrows = bandmapDataModel->rowCount();

    for (int row = 0; row < numrows; row++)
    {
        ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();
        Frequency freq = pSpot->getFreq();
        QString callsign = pSpot->getDxCallStr();
        QString spt = pSpot->spotName();
        traceMsg(QString("bandmapDataModel Callsign = %1, Freq = %2, Type = %3")
                     .arg( callsign, freq.traceStr(), spt));
    }
    numrows = listOfMarkers.count();

    for (int row = 0; row < numrows; row++)
    {
        int spotRow =  listOfMarkers[row]->getModelRowNum();
        ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(spotRow).data();
        Frequency freq = pSpot->getFreq();
        QString callsign = pSpot->getDxCallStr();
        QString spt = pSpot->spotName();
        traceMsg(QString("listOfMarkers Callsign = %1, Freq = %2 type = %3 y = %4")
                     .arg( callsign, freq.traceStr(), spt).arg(listOfMarkers[row]->getSpotRect().y()));
    }
}

void BandmapView::onNextUnworkedSpot(bool nextFreqHF, bool nextMult)
{
    if (updateRequired)
    {
        // make sure the vectors are up to date
        drawBandMapSpots();
        updateRequired = false;
        updateTimerTimeout();
    }
    traceTables();
    bool invertBandmap = false;
    TContestApp::getContestApp()->loggerBundle.getBoolProfile(elpBandmapInvert, invertBandmap);

    int freqRowNum = NO_SELECTED_ROWNUM;
    if (selectedSpot)
    {
        freqRowNum = getSelectedSpotViewRowNum();
    }
    else
    {
        if ((invertBandmap && nextFreqHF) || (!invertBandmap && !nextFreqHF))
        {
            // go from bottom to top
            for (int row = listOfMarkers.count() - 1; row >= 0; --row)
            {
                // bandmap is sorted "visually", so flips when BM is inverted
                int vrow = listOfMarkers[row]->getModelRowNum();
                ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(vrow).data();

                Frequency freq = pSpot->getFreq();

                if (freq <= curFreq)
                {
                    freqRowNum = row + 1;
                    break;
                }
            }
        }
        else
        {
            // go from top to bottom
            for (int row = 0; row < listOfMarkers.count(); ++row)
            {
                // bandmap is sorted "visually", so flips when BM is inverted
                int vrow = listOfMarkers[row]->getModelRowNum();
                ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(vrow).data();

                Frequency freq = pSpot->getFreq();

                if (freq >= curFreq)
                {
                    freqRowNum = row - 1;
                    break;
                }
            }
        }
    }

    // freqRowNum should be the "previous" spot in the correct direction
    // from the current frequency, or -1 if itb is the first in list

    if ((!invertBandmap && nextFreqHF) || (invertBandmap && !nextFreqHF))
    {
        int newMarkNum = NO_SELECTED_ROWNUM;
        if (!nextMult)
        {
            // search from this spot downwards
            newMarkNum = findNextUnworkedMarkerHF(freqRowNum);
        }
        else
        {
            newMarkNum = findNextNonWorkedLocatorHF(freqRowNum);
        }
        if (newMarkNum == NO_SELECTED_ROWNUM)
        {
            return;     // nothing found
        }

        clearSelectedSpot();
        freqRowNum = newMarkNum;
        setSelectedSpot(freqRowNum);
        MinosLoggerEvents::SendFreqToRig(selectedSpot->getFreq());
    }
    else
    {
        int newMarkNum = NO_SELECTED_ROWNUM;
        if (!nextMult)
        {
            newMarkNum = findNextUnworkedMarkerLF(freqRowNum);
        }
        else
        {
            newMarkNum = findNextNonWorkedLocatorLF(freqRowNum);
        }
        if (newMarkNum == NO_SELECTED_ROWNUM)
        {
            return; // nothing found
        }

        clearSelectedSpot();
        freqRowNum = newMarkNum;
        setSelectedSpot(freqRowNum);
        MinosLoggerEvents::SendFreqToRig(selectedSpot->getFreq());
    }
}


int BandmapView::findNextUnworkedMarkerHF(int curSpotViewNum)
{
    traceTables();

    if (curSpotViewNum + 1 >= listOfMarkers.count())
    {
        return NO_SELECTED_ROWNUM;
    }

    for (int i = curSpotViewNum + 1; i < listOfMarkers.count(); i++)
    {
        if (listOfMarkers[i]->getSpotMarkerPtr())
        {
            int row = listOfMarkers[i]->getModelRowNum();

            ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();

            bandmapSpotType::SPOT_TYPE savedSpot = pSpot->getSpotType();
            bool callWorked = pSpot->getDxCallWorked();
            if ( !callWorked && savedSpot != bandmapSpotType::CQ)
            {
                return i;
            }
        }
    }

    return curSpotViewNum;
}

int BandmapView::findNextUnworkedMarkerLF(int curSpotViewNum)
{
    traceTables();

    if (curSpotViewNum - 1 < 0)
    {
        return NO_SELECTED_ROWNUM;
    }

    int numRows = listOfMarkers.count();
    for (int i = curSpotViewNum - 1; i >= 0; i--)
    {
        if (numRows > i && listOfMarkers[i]->getSpotMarkerPtr())
        {
            int row = listOfMarkers[i]->getModelRowNum();
            ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();
            bandmapSpotType::SPOT_TYPE savedSpot = pSpot->getSpotType();
            bool callWorked = pSpot->getDxCallWorked();
            if ( !callWorked && savedSpot != bandmapSpotType::CQ)
            {
                return i;
            }
        }
    }

    return curSpotViewNum;
}
int BandmapView::findNextNonWorkedLocatorHF(int curSpotViewNum)
{
    traceTables();

    if (curSpotViewNum + 1 >= listOfMarkers.count())
    {
        return NO_SELECTED_ROWNUM;
    }
    for (int i = curSpotViewNum + 1; i < listOfMarkers.count(); i++)
    {
        if (listOfMarkers[i]->getSpotMarkerPtr() )
        {
            int row = listOfMarkers[i]->getModelRowNum();
            ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();
            bool locWorked = pSpot->getDxCallWorked();
            bandmapSpotType::SPOT_TYPE savedSpot = pSpot->getSpotType();
            if (!locWorked && savedSpot != bandmapSpotType::CQ)
            {
                return i;
            }
        }
    }

    return curSpotViewNum;
}

int BandmapView::findNextNonWorkedLocatorLF(int curSpotViewNum)
{
    traceTables();

    if (curSpotViewNum - 1 < 0)
    {
        return NO_SELECTED_ROWNUM;
    }
    int numRows = listOfMarkers.count();
    for (int i = curSpotViewNum - 1; i >= 0; i--)
    {
        if (i < numRows && listOfMarkers[i]->getSpotMarkerPtr() )
        {
            int row = listOfMarkers[i]->getModelRowNum();
            ClusterSpotData *pSpot = bandmapDataModel->getBandmapDataRow(row).data();
            bool locWorked = pSpot->getDxLocatorWorked();
            bandmapSpotType::SPOT_TYPE savedSpot = pSpot->getSpotType();
            if (!locWorked && savedSpot != bandmapSpotType::CQ)
            {
                return i;
            }
        }
    }

    return curSpotViewNum;
}
