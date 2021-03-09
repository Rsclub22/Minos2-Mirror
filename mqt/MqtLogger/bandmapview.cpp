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
#include "rigutils.h"
#include "MinosLoggerEvents.h"
#include "rigutils.h"
#include "ContestApp.h"
#include "delayedaction.h"
#include <QDebug>

const int DIAL_CURSOR_BELOW_VIEWSTART_FREQ = 0;
const int DIAL_CURSOR_ABOVE_VIEWSTART_FREQ = 1;
const int DIAL_CURSOR_WITHIN_VIEWPORT = 2;

BandmapView::BandmapView(QWidget *parent) :
    QAbstractItemView(parent),
    zoomLevel(0),
    contestBandFlow(0),
    contestBandFhigh(0),
    idealWidth(0),
    idealHeight(0),
    fullBandHeight(4000),
    fontHeight(0),
    maxNumSpots(0),
    selectedSpot(bandmapSpotType::NONE),
    selectedSpotDataRowNum(NO_SELECTED_ROWNUM),
    selectedSpotViewRowNum(NO_SELECTED_ROWNUM)
{

    setFocusPolicy((Qt::ClickFocus));
    //setMinimumSize(minimumSizeHint());

    bandmapScene = new QGraphicsScene(parent);

    lessGreaterThanDistanceFlag = readLessGreaterThanDistanceFlag();



    //horizontalScrollBar()->setRange(0, 0);
    //verticalScrollBar()->setRange(0, 0);


}

BandmapView::~BandmapView()
{
    setSuppressUpdate(true);
    clearListOfMarkers();
    delete bandmapScene;
    //delete dial;
}


void BandmapView::initBandmapView(QGraphicsView* view )
{
    bandmapGraphicsView = view;
    bandmapGraphicsView->setScene(bandmapScene);
    bandmapGraphicsView->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    bandmapGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded );
    bandmapGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    bandmapScene->setSceneRect(0,0, bandmapGraphicsView->width(), fullBandHeight);



    dial = new BandmapFreqDial(70, bandmapGraphicsView->viewport()->height());
    dialMinZoomLevel = dial->getMinZoomLevel();
    dialMaxZoomLevel = dial->getMaxZoomLevel();

    bandmapScene->addItem(dial);
    dial->setCurFreq(Frequency());
    dial->setCursorColour(Qt::black);
    trace("BandmapView::bandmapUpdate() initBandMapView");
    bandmapUpdate();

    connect(dial, SIGNAL(zoomUpdated(bool)), this, SLOT(zoomUpdated(bool)));

    connect(bandmapGraphicsView, SIGNAL(bandmapResize(int, int)), this, SLOT(bandmapResize(int, int)));
    connect(bandmapGraphicsView, SIGNAL(leftMouseButtonPressed(QPoint)), this, SLOT(leftMouseButtonPressed(QPoint)));
    connect(bandmapGraphicsView, SIGNAL(mouseDoubleClicked(QPoint)), this, SLOT(mouseDoubleClicked(QPoint)));
    connect(bandmapGraphicsView, SIGNAL(nextSpot(bool, bool)), this, SLOT(on_nextSpot(bool, bool)));
    connect(bandmapGraphicsView, SIGNAL(scrollMap(bool)),this, SLOT(on_scrollMap(bool)));

    connect(model(), SIGNAL(rowsRemoved(const QModelIndex, int, int)), SLOT(onRowsRemoved(const QModelIndex, int, int)));
    connect(model(), SIGNAL(rowsInserted(const QModelIndex, int, int)), SLOT(onRowsInserted(const QModelIndex, int, int)));


    bandmapGraphicsView->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( bandmapGraphicsView, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( on_bandmap_customContextMenuRequested( const QPoint& ) ) );


}



void BandmapView::onFontChanged(QFont cf)
{
    dial->onFontChanged(cf);
    QFontMetrics fm(cf);
    if (fontHeight != fm.height())
    {
        fontHeight = fm.height();
    }


}

// this is called from other frames

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
    Frequency scaleStartFreq = dial->getViewPortFreq(getViewPortStartYCoordOnScene(), contestBandFlow);
    Frequency scaleEndFreq = dial->getViewPortFreq(getViewPortEndYCoordOnScene(), contestBandFlow);
    Frequency midScaleFreq = scaleStartFreq + Frequency(qint64(scaleEndFreq - scaleStartFreq) / 2);

    if (level < dialMaxZoomLevel && level >= dialMinZoomLevel)
    {
        zoomLevel = level;
        dial->setZoomLevel(level);
        setBandmapHeight(contestBandFlow, contestBandFhigh);
        trace("BandmapView::bandmapUpdate()setBandMapZoom ");
        bandmapUpdate();
        scrollBandmapCenterToFreq(midScaleFreq);
        emit newZoomlevel(level);
    }
}


void BandmapView::zoomUpdated(bool dir)
{
    zoomLevel = dial->getZoomLevel();

    Frequency scaleStartFreq = dial->getViewPortFreq(getViewPortStartYCoordOnScene(), contestBandFlow);
    Frequency scaleEndFreq = dial->getViewPortFreq(getViewPortEndYCoordOnScene(), contestBandFlow);
    Frequency midScaleFreq = scaleStartFreq + Frequency(qint64(scaleEndFreq - scaleStartFreq) / 2);

    if (dir)
    {

        if (zoomLevel < dialMaxZoomLevel && zoomLevel >= dialMinZoomLevel)
        {
            ++zoomLevel;
            dial->setZoomLevel(zoomLevel);
            setBandmapHeight(contestBandFlow, contestBandFhigh);
            trace("BandmapView::bandmapUpdate() zoomUpdated 1");
            bandmapUpdate();
            scrollBandmapCenterToFreq(midScaleFreq);
            emit newZoomlevel(zoomLevel);
        }
    }
    else
    {
        if (zoomLevel != dialMinZoomLevel && zoomLevel <= dialMaxZoomLevel)
        {
            --zoomLevel;
            dial->setZoomLevel(zoomLevel);
            setBandmapHeight(contestBandFlow, contestBandFhigh);
            trace("BandmapView::bandmapUpdate() zoomUpdated 2");
            bandmapUpdate();
            scrollBandmapCenterToFreq(midScaleFreq);
            emit newZoomlevel(zoomLevel);
        }
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




void BandmapView::on_scrollMap(bool dir)
{
    int scrollValue = bandmapGraphicsView->verticalScrollBar()->value();

    if (dir)
    {
        scrollValue -= KEY_SCROLL_STEP_SIZE;
       if (scrollValue < bandmapGraphicsView->verticalScrollBar()->minimum())
       {
           scrollValue = bandmapGraphicsView->verticalScrollBar()->minimum();
       }
    }
    else
    {
        scrollValue += KEY_SCROLL_STEP_SIZE;
        if (scrollValue > bandmapGraphicsView->verticalScrollBar()->maximum())
        {
            scrollValue = bandmapGraphicsView->verticalScrollBar()->maximum();
        }
    }

    bandmapGraphicsView->verticalScrollBar()->setValue(scrollValue);
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




void BandmapView::on_nextSpot(bool nextFreqUpDown, bool nextMult)
{

    if (selectedSpotViewRowNum != NO_SELECTED_ROWNUM)
    {
        if (!nextMult)
        {
            if (nextFreqUpDown)
            {
                if (selectedSpotViewRowNum != (listOfMarkers.count() - 1))
                {
                    int newMarkNum = findNextOccupiedMarkerUpList(selectedSpotViewRowNum);
                    if (newMarkNum == selectedSpotViewRowNum)
                    {
                        return; // nothing found
                    }

                    clearSelectedSpot();
                    selectedSpotViewRowNum = newMarkNum;
                    setSelectedSpot(selectedSpotViewRowNum);
                }

            }
            else
            {
                if (selectedSpotViewRowNum != 0)
                {
                    int newMarkNum = findNextOccupiedMarkerDownList(selectedSpotViewRowNum);
                    if (newMarkNum == selectedSpotViewRowNum)
                    {
                        return;     // nothing found
                    }

                    clearSelectedSpot();
                    selectedSpotViewRowNum = newMarkNum;
                    setSelectedSpot(selectedSpotViewRowNum);
                }
            }
        }
        else if (nextFreqUpDown)
        {
            if (selectedSpotViewRowNum != (listOfMarkers.count() - 1))
            {
                int newMarkNum = findNextNonWorkedLocatorDownList(selectedSpotViewRowNum);
                if (newMarkNum == selectedSpotViewRowNum)
                {
                    return; // nothing found
                }

                clearSelectedSpot();
                selectedSpotViewRowNum = newMarkNum;
                setSelectedSpot(selectedSpotViewRowNum);
            }
        }
        else
        {
            if (selectedSpotViewRowNum != 0)
            {
                int newMarkNum = findNextNonWorkedLocatorUpList(selectedSpotViewRowNum);
                if (newMarkNum == selectedSpotViewRowNum)
                {
                    return;     // nothing found
                }

                clearSelectedSpot();
                selectedSpotViewRowNum = newMarkNum;
                setSelectedSpot(selectedSpotViewRowNum);
            }
        }
    }
}


int BandmapView::findNextOccupiedMarkerUpList(int curSpotViewNum)
{
    if (curSpotViewNum + 1 >= listOfMarkers.count())
    {
        return curSpotViewNum;
    }
    for (int i = curSpotViewNum + 1; i < listOfMarkers.count(); i++)
    {
        if (listOfMarkers[i]->getSpotMarkerPtr() != nullptr)
        {
            int row = listOfMarkers[i]->getModelRowNum();
            bandmapSpotType::SPOT_TYPE savedSpot = static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(row, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
            if ( savedSpot != bandmapSpotType::CQ)
            {
                return i;
            }
        }
    }

    return curSpotViewNum;
}

int BandmapView::findNextOccupiedMarkerDownList(int curSpotViewNum)
{
    if (curSpotViewNum - 1 < 0)
    {
        return curSpotViewNum;
    }
    for (int i = curSpotViewNum - 1; i >= 0; i--)
    {
        if (listOfMarkers[i]->getSpotMarkerPtr() != nullptr)
        {
            int row = listOfMarkers[i]->getModelRowNum();
            bandmapSpotType::SPOT_TYPE savedSpot = static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(row, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
            if ( savedSpot != bandmapSpotType::CQ)
            {
                return i;
            }
        }
    }

    return curSpotViewNum;
}


int BandmapView::findNextNonWorkedLocatorUpList(int curSpotViewNum)
{
    if (curSpotViewNum + 1 >= listOfMarkers.count())
    {
        return curSpotViewNum;
    }
    for (int i = curSpotViewNum + 1; i < listOfMarkers.count(); i++)
    {
        if (listOfMarkers[i]->getSpotMarkerPtr() != nullptr)
        {
            int row = listOfMarkers[i]->getModelRowNum();
            bool locWorked = model()->data(model()->index(row, DXLOC_WORKED_COL_NUM), BMP_DataStoredRole).toBool();
            bandmapSpotType::SPOT_TYPE savedSpot = static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(row, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
            if (!locWorked && savedSpot != bandmapSpotType::CQ)
            {
                return i;
            }
        }
    }

    return curSpotViewNum;
}

int BandmapView::findNextNonWorkedLocatorDownList(int curSpotViewNum)
{
    if (curSpotViewNum - 1 < 0)
    {
        return curSpotViewNum;
    }
    for (int i = curSpotViewNum - 1; i >= 0; i--)
    {
        if (listOfMarkers[i]->getSpotMarkerPtr() != nullptr)
        {
            int row = listOfMarkers[i]->getModelRowNum();
            bool locWorked = model()->data(model()->index(row, DXLOC_WORKED_COL_NUM), BMP_DataStoredRole).toBool();
            bandmapSpotType::SPOT_TYPE savedSpot = static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(row, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
            if (!locWorked && savedSpot != bandmapSpotType::CQ)
            {
                return i;
            }
        }
    }

    return curSpotViewNum;
}

void BandmapView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{

    QAbstractItemView::dataChanged(topLeft, bottomRight, roles);
    trace("BandmapView::bandmapUpdate() BandmapView::dataChanged");
    bandmapUpdate();

}



void BandmapView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // do nothing

    Q_UNUSED(selected)
    Q_UNUSED(deselected)

}

void BandmapView::rowsInserted(const QModelIndex &parent, int start, int end)
{

    QAbstractItemView::rowsInserted(parent, start, end);
    trace("BandmapView::bandmapUpdate() bandmapView::rowsInserted");
    bandmapUpdate();

}


void BandmapView::rowsAboutToBeRemoved(const QModelIndex &parent,
                                         int start, int end)
{

    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);

}

void BandmapView::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)

    trace("BandmapView::bandmapUpdate() bandmapView::rowsRemoved");
    bandmapUpdate();

}

// this is not used...
void BandmapView::onRowsInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
}


void BandmapView::bandmapUpdate()
{
    if (!getSuppressUpdate())
    {
        dial->update();
        // delay the spots until any dial update has happened
        delayedAction(this, [=](){
        drawBandMapSpots();});
    }
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

    if (spotNum >= 0)
    {

        memoryData::memData spotData;
        spotData.callsign = selectedSpot.getDxCallStr();
        spotData.time = selectedSpot.getSpotTime();
        spotData.freq = selectedSpot.getFreq();
        spotData.locator = selectedSpot.getDxLocator();
        spotData.bearing = selectedSpot.getDxBrg().toInt();
        spotData.fromBandmapOrMemory = true;
        spotData.exchange = selectedSpot.getDistrict();

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

void BandmapView::updateGeometries()
{

}



int BandmapView::horizontalOffset() const
{

    return horizontalScrollBar()->value();
}



QModelIndex BandmapView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers)
{
    // unused
    Q_UNUSED(cursorAction)

    QModelIndex index = currentIndex();

    return index;
}


void BandmapView::setSelection(const QRect &rect, QFlags<QItemSelectionModel::SelectionFlag> flags)
{
    // do nothing
    Q_UNUSED(rect)
    Q_UNUSED(flags)
}


int BandmapView::verticalOffset() const
{

    return verticalScrollBar()->value();
}


QRegion BandmapView::visualRegionForSelection(const QItemSelection &selection) const
{
    // unused
    Q_UNUSED(selection)
    return QRegion();
}










QModelIndex BandmapView::indexAt(const QPoint &point_) const
{

    // not used
    QPoint point(point_);
    point.rx() += horizontalScrollBar()->value();
    point.ry() += verticalScrollBar()->value();

    return QModelIndex();
}






void BandmapView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint)
{

    Q_UNUSED(index)
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






void BandmapView::bandmapResize(int height, int width)
{
    Q_UNUSED(height)
    Q_UNUSED(width)
    trace("BandmapView::bandmapUpdate() bandmapView::bandmapResize");
    bandmapUpdate();


}



int BandmapView::rows(const QModelIndex &index) const
{
    return model()->rowCount(model()->parent(index));
}

int BandmapView::getBandmapFrameHeight()
{
    return bandmapGraphicsView->viewport()->height();
}

int BandmapView::getBandmapFrameWidth()
{
    return bandmapGraphicsView->viewport()->width();
}


void BandmapView::setFreq(Frequency f, bool legalFreq)
{
    // The 1000 and 2000 below aren't enough - should they be configurable?
    // or a proportion of the freq range?
    curFreq = f;

    int dfwv = dialCursorWithinViewport(f);
    Frequency freqWidth = dial->getScaleEndFreq() - dial->getScaleStartFreq();

    Frequency edgeAmount = freqWidth/5;

    if (dfwv == DIAL_CURSOR_BELOW_VIEWSTART_FREQ)
    {
        // tuning up, move viewport
        bandmapGraphicsView->verticalScrollBar()->setValue(dial->getYCoordOnDial(f - edgeAmount));
    }
    else if (dfwv == DIAL_CURSOR_ABOVE_VIEWSTART_FREQ)
    {
        // tuning down, move viewport
        bandmapGraphicsView->verticalScrollBar()->setValue(dial->getYCoordOnDial(f - freqWidth + edgeAmount));
    }



    dial->setCurFreq(f);
    if (legalFreq)
    {
        dial->setCursorColour(Qt::black);
    }
    else
    {
        dial->setCursorColour(Qt::red);
    }

    if (!f.isClear())
    {
        trace("BandmapView::bandmapUpdate() bandmapView::setFreq");
        bandmapUpdate();
    }

}


void BandmapView::setDialRadioMode(QString mode)
{
    dial->setRadioMode(mode);
    trace("BandmapView::bandmapUpdate() bandmapView::setDialRadioMode");
    bandmapUpdate();
}


int BandmapView::dialCursorWithinViewport(Frequency freq)
{
    int sceneStartYCoord = bandmapGraphicsView->mapToScene(0,0).toPoint().y();
    int sceneEndYCoord = bandmapGraphicsView->mapToScene(0, bandmapGraphicsView->viewport()->height() - bandmapGraphicsView->horizontalScrollBar()->height()).toPoint().y();

    dial->setViewPortStartEndFreq(sceneStartYCoord, sceneEndYCoord, contestBandFlow);

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
    dial->changeBoundingRect(fullBandHeight, dial->getCurWidth());
    bandmapScene->setSceneRect(0,0, bandmapGraphicsView->width(), fullBandHeight);

}

void BandmapView::sendFreqToRig(Frequency freq)
{
    MinosLoggerEvents::SendFreqToRig(freq);
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
    int spotViewNum = isClickInRegionOfSpot(p);

    if (spotViewNum != -1)
    {
        if (spotViewNum >= 0)
        {
            clearSelectedSpot();       // clear any spot previously selected
            setSelectedSpot(spotViewNum);        // mark new selected spot

            MinosLoggerEvents::SendFreqToRig(selectedSpot.getFreq());
        }
    }
    else
    {
        // not in region
        clearSelectedSpot();        // clear any selected spots
    }

}


void BandmapView::clearSelectedSpot()
{

    for (int i = 0; i < model()->rowCount(); i++)
    {
        if (model()->data(model()->index(i, SPOT_IS_SELECTED_COL_NUM), BMP_DataStoredRole).toBool())
        {
            model()->setData(model()->index(i, SPOT_IS_SELECTED_COL_NUM), false, BMP_DataStoredRole);

            clearSpotData(selectedSpot );

            trace("BandmapView::bandmapUpdate() bandmapView::clearSelectedSpot");
            bandmapUpdate();



        }
    }
}


void BandmapView::clearSelectedSpotData()
{

    clearSpotData(selectedSpot);
}



void BandmapView::clearSpotData(BandmapSpotData &selectedSpot)
{
    selectedSpotDataRowNum = NO_SELECTED_ROWNUM;
    selectedSpotViewRowNum = NO_SELECTED_ROWNUM;
    selectedSpot.clear();

}

void BandmapView::setSelectedSpot(int spotViewNum)
{

    if (spotViewNum > listOfMarkers.count() || spotViewNum == NO_SELECTED_ROWNUM || listOfMarkers[spotViewNum]->getModelRowNum() > model()->rowCount() || listOfMarkers[spotViewNum]->getModelRowNum() < 0)
    {
        return;
    }

    selectedSpotViewRowNum = spotViewNum;

    getSpotData(selectedSpotDataRowNum, selectedSpotViewRowNum, selectedSpot);

    model()->setData(model()->index(selectedSpotDataRowNum , SPOT_IS_SELECTED_COL_NUM), true, BMP_DataStoredRole);
    selectedSpot.setIsSelected(true);
    trace("BandmapView::bandmapUpdate() bandmapView::setSelectedSpot");
    bandmapUpdate();
}

void BandmapView::clearListOfMarkers()
{
    for (int i = 0; i < listOfMarkers.count(); i++)
    {
        delete listOfMarkers[i];
    }
    listOfMarkers.clear();
}


void BandmapView::getSpotData(int &selectedSpotDataRowNum, int selectedSpotViewRowNum, BandmapSpotData &selectedSpot)
{
    selectedSpotDataRowNum = listOfMarkers[selectedSpotViewRowNum]->getModelRowNum();

    if (selectedSpotDataRowNum >= 0 && selectedSpotDataRowNum < model()->rowCount())
    {
        selectedSpot.setSpotTime(model()->data(model()->index(selectedSpotDataRowNum, TIME_COL_NUM), BMP_DataStoredRole).toString());
        selectedSpot.setFreq(qvariant_cast<Frequency>(model()->data(model()->index(selectedSpotDataRowNum, FREQ_COL_NUM), BMP_DataStoredRole)));
        selectedSpot.setDxCall(model()->data(model()->index(selectedSpotDataRowNum, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toString());
        selectedSpot.setDxLocator(model()->data(model()->index(selectedSpotDataRowNum, DXLOC_COL_NUM), BMP_DataStoredRole).toString());
        selectedSpot.setDxDist(model()->data(model()->index(selectedSpotDataRowNum, DXDIST_COL_NUM), BMP_DataStoredRole).toString());
        selectedSpot.setDxBrg(model()->data(model()->index(selectedSpotDataRowNum, DXBRG_COL_NUM), BMP_DataStoredRole).toString());
        selectedSpot.setDxCallWorked(model()->data(model()->index(selectedSpotDataRowNum, DXSPOT_CALL_WORKED_COL_NUM), BMP_DataStoredRole).toBool());
        selectedSpot.setDxLocatorWorked(model()->data(model()->index(selectedSpotDataRowNum, DXLOC_WORKED_COL_NUM), BMP_DataStoredRole).toBool());
        selectedSpot.setDistrict(model()->data(model()->index(selectedSpotDataRowNum, DX_DISTRICT_COL_NUM), BMP_DataStoredRole).toString());
        selectedSpot.setDistrictWorked(model()->data(model()->index(selectedSpotDataRowNum, DX_DISTRICT_WORKED_COL_NUM), BMP_DataStoredRole).toBool());
        selectedSpot.setSpotType(static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(selectedSpotDataRowNum, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt()));

    }


}





void BandmapView::drawBandmapSpot(int row, int &fontOffset, int markersAbove, int &lastOffset)
{
    bandmapSpotType::SPOT_TYPE savedSpotType = static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(row, SPOT_TYPE_COL_NUM ),  BMP_DataStoredRole).toInt());
    if (savedSpotType == bandmapSpotType::DELETED)
    {
        return;
    }
    if (matchMode(row) && matchDistance(row))
    {
        int centreYCoord = dial->getYCoordOnDial(curFreq);
        int dialWidth = dial->getCurWidth();

        BandmapMarkerDetails* markerDetails = new BandmapMarkerDetails(QPoint(0, 0));
        listOfMarkers.append(markerDetails);

        Frequency f = qvariant_cast<Frequency>(model()->data(model()->index(row, FREQ_COL_NUM), BMP_DataStoredRole));
        int yCoord = dial->getYCoordOnDial(f);
        int syCoord;

        if (row >= markersAbove)
        {
            // marker at higher frequency
            syCoord = centreYCoord + fontOffset;
            if (syCoord < yCoord)
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
            //trace(QString("DrawBandmapSpot (above) curfreq %1 freq %2 syCoord %3 lastoffset %4").arg(curFreq.traceStr()).arg(f.traceStr()).arg(syCoord).arg(lastOffset));

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
            //trace(QString("DrawBandmapSpot (below) curfreq %1 freq %2 syCoord %3 lastoffset %4").arg(curFreq.traceStr()).arg(f.traceStr()).arg(syCoord).arg(lastOffset));
        }
        lastOffset = fontOffset;
        //spotCoord is left, top
        QPoint spotCoord = QPoint(dialWidth + SPOTMARKER_XOFFSET, syCoord - fontHeight/2);
        QPoint startMarkerLine = QPoint(dialWidth + SPOTMARKER_XOFFSET, syCoord + fontHeight/3);

        BandmapSpotMarker* spot = new BandmapSpotMarker(spotCoord); //spotcoord is used for the text position
        bandmapScene->addItem(spot);


        QPoint endMarkerLine = QPoint(dialWidth, yCoord + dialData::DIAL_VERT_OFFSET);

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
            assembleCqToolTip(row, f, spotTooltipText);
        }
        else
        {
            assembleSpotMsg(row, spotMsg);
            spotRect = calculateSpotRect(spotMsg, spotCoord);
            assembleToolTip(row, f, spotTooltipText);
        }

        //trace(spotMsg);
        spot->setSpotText(spotMsg);
        spot->setToolTipText(spotTooltipText);

        markerDetails->setSpotMarkerPtr(spot);
        markerDetails->setMarkerLinePtr(markerLine);
        markerDetails->setSpotRect(spotRect);
        markerDetails->setModelRowNum(row);

        fontOffset += fontHeight;
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

    traceMsg(QString("Drawspots: Start Drawing - Clear Map"));

    deleteItemsFromMarkerList();

    clearListOfMarkers();

    // don't draw spots when freq is zero, or no spots
    if (curFreq.isClear() || model()->rowCount() == 0)
    {
        traceMsg(QString("CurFreq = %1, Number of Spots = %2 - Don't draw markers").arg(curFreq.traceStr()).arg(model()->rowCount()));
        return;
    }


    Frequency startFreq = contestBandFlow;
    Frequency endFreq = contestBandFhigh;

    int dialWidth = dial->getCurWidth();
    int dialHeight = dial->getCurHeight();


    QFont cf = QApplication::font();
    QFontMetrics fm(cf);
    fontHeight = fm.height();


    if (dialHeight == 0 || fontHeight == 0)
        return;

    maxNumSpots = dialHeight/fontHeight;

    if (maxNumSpots == 0)
        return;

    bool centreTextOnFrequency = true;
    bool btemp;
    ta->loggerBundle.getBoolProfile( elpBandmapOldStyle, btemp );
    centreTextOnFrequency = !btemp;

    if (!centreTextOnFrequency)
    {
        int textYCoord = 0;

        for (int i = 0; i < maxNumSpots; i++)
        {
            BandmapMarkerDetails* markerDetails = new BandmapMarkerDetails(QPoint(dialWidth + SPOTMARKER_XOFFSET, textYCoord));
            listOfMarkers.append(markerDetails);
            textYCoord += fontHeight;
        }
    }
    int numrows = model()->rowCount();

    if (numrows != 0)
    {
        traceMsg(QString("Drawspots: Number of Rows to Check = %1").arg(numrows));

        // this is for test
        ta ->loggerBundle.getBoolProfile( elpBandMapTraceDebug, traceDebugFlag );
        if (traceDebugFlag)
        {
            traceMsg(QString("dump list of spots and freq"));
            for (int row = 0; row < numrows; row++)
            {
                Frequency freq = qvariant_cast<Frequency>(model()->data(model()->index(row, FREQ_COL_NUM), BMP_DataStoredRole));
                QString callsign = model()->data(model()->index(row, DXSPOT_CALL_COL_NUM), Qt::DisplayRole).toString();
                traceMsg(QString("DB# = %1, Callsign = %2, Freq = %3").arg(row).arg(callsign).arg(freq.traceStr()));
            }
        }

        if (centreTextOnFrequency)
        {
            int markersAbove = 0;
            int centreSpot;
            for (centreSpot = 0; centreSpot < model()->rowCount(); ++centreSpot)
            {
                // BUT unfortunately the BandMapDataModel isn't sorted
                Frequency freq = qvariant_cast<Frequency>(model()->data(model()->index(centreSpot, FREQ_COL_NUM), BMP_DataStoredRole));
                // If they are in the table, we have to account for them
                // we can filter when looking at the individuals later
                if (freq > curFreq)
                {
                    break;
                }
                markersAbove++;
            }
            int fontOffset = -1;
            int lastOffset = -1;
            int firstOffset = -1;
            for (int row = markersAbove - 1; row >= 0; --row)
            {
                drawBandmapSpot(row, fontOffset, markersAbove, lastOffset);
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
//            if (lastOffset < 0)
//            {
//                lastOffset = 0;
//            }
            for (int row = markersAbove; row < numrows; ++row)
            {
                drawBandmapSpot(row, fontOffset, markersAbove, lastOffset);
            }
            std::sort(listOfMarkers.begin(), listOfMarkers.end(),
                      [](const BandmapMarkerDetails* a, const BandmapMarkerDetails* b) -> bool
                        {
                            return a->getSpotMarkerPtr()->y() < b->getSpotMarkerPtr()->y();
                        }
                      );
            trace("sort finished");
        }
        else
        {
            for (int row = 0; row < numrows; ++row)
            {
                bandmapSpotType::SPOT_TYPE savedSpotType = static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(row, SPOT_TYPE_COL_NUM ),  BMP_DataStoredRole).toInt());
                if (savedSpotType == bandmapSpotType::DELETED)
                {
                    continue;;
                }
               // check mode and distance against the filter settings
                if (matchMode(row) && matchDistance(row))
                {

                    Frequency f = qvariant_cast<Frequency>(model()->data(model()->index(row, FREQ_COL_NUM), BMP_DataStoredRole));

                    if (f >= startFreq && f <= endFreq)
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
                                    QString callsign = model()->data(model()->index(row, DXSPOT_CALL_COL_NUM), Qt::DisplayRole).toString();

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

                                    QPoint endMarkerLine = QPoint(dialWidth, yCoord + dialData::DIAL_VERT_OFFSET);

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

    }


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

    //bool ok = false;
    QString mode = model()->data(model()->index(sourceRow, DXSPOT_MODE_COL_NUM), BMP_DataStoredRole).toString();

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

        bandmapSpotType::SPOT_TYPE savedSpot = static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(sourceRow, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());
        QString distanceStr = model()->data(model()->index(sourceRow, DXDIST_COL_NUM), BMP_DataStoredRole).toString();
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

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    int textWidth = fm.horizontalAdvance(text);
#else
    int textWidth = fm.width(text);
#endif
    
    return QRectF(spotCoord.x(),spotCoord.y(), textWidth, rowHeight);
   
    
}

void BandmapView::assembleCqMsg(int row, QString& markerMsg)
{
    bool offRunFreq = model()->data(model()->index(row, OFF_RUN_FREQ_COL_NUM), BMP_DataStoredRole).toBool();
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


    Frequency freq = qvariant_cast<Frequency>(model()->data(model()->index(row, FREQ_COL_NUM), BMP_DataStoredRole));

    Frequency curFreq = dial->getCurFreq();

    QString bLineStart = "";
    QString bLineEnd = "";

    if (freq == curFreq )
    {
        bLineStart = "<b>";
        bLineEnd = "</b>";
    }


    QString msg = tr("%1%2CQ Frequency @ .%3%4%5").arg(bLineStart).arg(msgColourStart).arg(freq.extractKhz()).arg(msgColourEnd).arg(bLineEnd);
    markerMsg = msg;
    //msg.detach();
}


void BandmapView::assembleSpotMsg(int row, QString& markerMsg)
{

    QString dxCallsign = model()->data(model()->index(row, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toString();
    bool callWkd = model()->data(model()->index(row,DXSPOT_CALL_WORKED_COL_NUM), BMP_DataStoredRole).toBool();
    QString freqStr =  model()->data(model()->index(row, FREQ_COL_NUM), BMP_DataStoredRole).toString();
    Frequency freq = qvariant_cast<Frequency>(model()->data(model()->index(row, FREQ_COL_NUM), BMP_DataStoredRole));
    Frequency curFreq = dial->getCurFreq();
    QString dxLoc = model()->data(model()->index(row, DXLOC_COL_NUM), BMP_DataStoredRole).toString();
    bool locWkd = model()->data(model()->index(row, DXLOC_WORKED_COL_NUM), BMP_DataStoredRole).toBool();
    QString dxDist = model()->data(model()->index(row, DXDIST_COL_NUM), BMP_DataStoredRole).toString();
    QString dxBrg = model()->data(model()->index(row, DXBRG_COL_NUM), BMP_DataStoredRole).toString();
    QString rotBrg = model()->data(model()->index(row, ROT_BEARING_COL_NUM), BMP_DataStoredRole).toString();
    bool rotConnected = model()->data(model()->index(row, ROT_CONNECTED_COL_NUM), BMP_DataStoredRole).toBool();

    bandmapSpotType::SPOT_TYPE spotType = static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(row, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());


    qlonglong spotTime = model()->data(model()->index(row, RXTIME_COL_NUM), BMP_DataStoredRole).toLongLong();
    bool olderThan3Min = spotTimedOut(spotTime, NEW_SPOT_TIME);

    QString newSpotMsg = "";
    if (!olderThan3Min)
    {
        newSpotMsg = HtmlFontColour(BANDMAP_NEW_COLOUR) + tr("New") +  HtmlFontColour(NOT_WORKED_COLOUR);
    }

    QString callsign;
    if (callWkd)
    {
        callsign = QString("%1%2%3").arg(HtmlFontColour(CALLSIGN_WORKED_COLOUR)).arg(dxCallsign).arg(HtmlFontColour(NOT_WORKED_COLOUR));
    }
    else
    {
        callsign = dxCallsign;
    }

    QString locator;
    if (locWkd)
    {
        locator = QString("%1%2%3").arg(HtmlFontColour(CALLSIGN_WORKED_COLOUR)).arg(dxLoc).arg(HtmlFontColour(NOT_WORKED_COLOUR));
    }
    else
    {
        if (!dxLoc.isEmpty())
        {
            locator = dxLoc;
        }
    }

    QChar degSym = QChar(DEG_SYMBOL);
    QString bearing;
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

    QString distance;
    if (!dxDist.isEmpty())
    {
        distance = QString("%1 km").arg(dxDist);
    }



    QString bLineStart = "";
    QString bLineEnd = "";

    if (std::abs(freq - curFreq) < 1000 )
    {
        bLineStart = "<b>";
        bLineEnd = "</b>";
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

    qlonglong elapsedTime = spotElapsedTime(spotTime) / 60;
    QString elapsedTimeStr = QString::number(elapsedTime) + " " + tr("min");

    QString msg = QString("%1%2 @ .%3 %4 %5 %6 %7 %8 %9%10").arg(bLineStart).arg(callsign).arg(freq.extractKhz()).arg(locator).arg(distance).arg(bearing).arg(elapsedTimeStr).arg(markSym).arg(newSpotMsg).arg(bLineEnd);

    if (model()->data(model()->index(row, SPOT_IS_SELECTED_COL_NUM), BMP_DataStoredRole).toBool())
    {
        msg = QString("<div style='background:rgba(200, 200, 200, 75%);'>" + msg + QString("</div<"));           // show selected

    }

    markerMsg = msg;
    //msg.detach();

}


void BandmapView::assembleCqToolTip(int row, Frequency freq, QString& toolTipMsg)
{

    QString mode = model()->data(model()->index(row, DXSPOT_MODE_COL_NUM), BMP_DataStoredRole).toString();
    QString msg = tr("CQ Frequency = %1\nThe mode is %2").arg(freq.convertFreqStrDisp()).arg(mode);
    toolTipMsg = msg;
    //msg.detach();

}


void BandmapView::assembleToolTip(int row, Frequency freq, QString& toolTipMsg)
{
    QChar degSym = QChar(DEG_SYMBOL);
    QString callsign = model()->data(model()->index(row, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toString();
    QString locator = model()->data(model()->index(row, DXLOC_COL_NUM), BMP_DataStoredRole).toString();
    QString bearing = model()->data(model()->index(row, DXBRG_COL_NUM), BMP_DataStoredRole).toString();
    if (!bearing.isEmpty())
    {
        bearing += degSym;
    }
    QString distance = model()->data(model()->index(row, DXDIST_COL_NUM), BMP_DataStoredRole).toString();
    if (!distance.isEmpty())
    {
        distance += " km";
    }
    QString spotterCallsign =  model()->data(model()->index(row, SPOTTER_CALL_COL_NUM), BMP_DataStoredRole).toString();
    QString spotterLocator = model()->data(model()->index(row, SPOTTER_LOC_COL_NUM), BMP_DataStoredRole).toString();

    QString spotterComment = model()->data(model()->index(row, COMMENT_COL_NUM), BMP_DataStoredRole).toString().replace('<', " (").replace('>', ") ");
    QString computedMode = model()->data(model()->index(row, DXSPOT_MODE_COL_NUM), BMP_DataStoredRole).toString();
    bandmapSpotType::SPOT_TYPE spotType = static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(row, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());

    QString spotName = BandmapSpotData::spotName(spotType);
    QString spotModeMsg = tr("The computed mode is");
    if (spotType == bandmapSpotType::LOGGED)
    {
        spotModeMsg = tr("The mode is");
    }

    qlonglong spotTime = model()->data(model()->index(row, RXTIME_COL_NUM), BMP_DataStoredRole).toLongLong();
    qlonglong elapsedTime = spotElapsedTime(spotTime) / 60;
    QString elapsedTimeStr = QString::number(elapsedTime);

    QString msg = tr("%1 - %2, %3, %4, %5 [%6 %7 @ %8 min] \n%9 %10\n%11\n%12")
                                            .arg(callsign)
                                            .arg(freq.convertFreqStrDisp())
                                            .arg(locator)
                                            .arg(bearing)
                                            .arg(distance)

                                            .arg(spotterCallsign)
                                            .arg(spotterLocator)
                                            .arg(elapsedTimeStr)
                                            .arg(spotModeMsg)
                                            .arg(computedMode)
                                            .arg(spotterComment)
                                            .arg(spotName);

    toolTipMsg = msg;
    //msg.detach();

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
    QSettings config(CLUSTER_FILTER_FILE, QSettings::IniFormat);
    return config.value(LESS_GREATER_THAN_DISTANCE_FLAG_INI_NAME, false).toBool();

}

void BandmapView::traceMsg(QString msg)
{
    trace(QString("[bandmapView] %1").arg(msg));
}
