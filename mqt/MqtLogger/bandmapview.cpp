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


#include <QDebug>



BandmapView::BandmapView(QWidget *parent) :
    QAbstractItemView(parent),
    curFreq(0.0),
    zoomLevel(0),
    contestBandFlow(0),
    contestBandFhigh(0),
    idealWidth(0),
    idealHeight(0),
    fullBandHeight(4000),
    fontHeight(0),
    maxNumSpots(0),
    selectedSpotDataRowNum(NO_SELECTED_ROWNUM),
    selectedSpotViewRowNum(NO_SELECTED_ROWNUM)
{

    setFocusPolicy((Qt::WheelFocus));
    //setMinimumSize(minimumSizeHint());

    bandmapScene = new QGraphicsScene(parent);



    //horizontalScrollBar()->setRange(0, 0);
    //verticalScrollBar()->setRange(0, 0);


}

BandmapView::~BandmapView()
{
    clearListOfMarkers();
    delete bandmapScene;
    //delete dial;
}


void BandmapView::initBandmapView(QGraphicsView* view )
{
    bandmapGraphicsView = view;
    bandmapGraphicsView->setScene(bandmapScene);
    bandmapGraphicsView->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    bandmapGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
    bandmapGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    bandmapScene->setSceneRect(0,0, bandmapGraphicsView->width(), fullBandHeight);

    dial = new BandmapFreqDial(70, bandmapGraphicsView->viewport()->height());
    //qDebug() << "bandmap height " << getBandmapFrameHeight();
    dialMinZoomLevel = dial->getMinZoomLevel();
    dialMaxZoomLevel = dial->getMaxZoomLevel();

    bandmapScene->addItem(dial);
    dial->setCurFreq(0.0);
    dial->setCursorColour(Qt::black);
    bandmapUpdate();



    //connect (dial, SIGNAL(dialupdated()), this, SLOT(drawBandMapSpots()));
    connect(dial, SIGNAL(zoomUpdated(bool)), this, SLOT(zoomUpdated(bool)));
    connect(bandmapGraphicsView, SIGNAL(bandmapResize(int, int)), this, SLOT(bandmapResize(int, int)));
    connect(bandmapGraphicsView, SIGNAL(leftMouseButtonPressed(QPoint)), this, SLOT(leftMouseButtonPressed(QPoint)));
    connect(bandmapGraphicsView, SIGNAL(mouseDoubleClicked(QPoint)), this, SLOT(mouseDoubleClicked(QPoint)));
    connect(bandmapGraphicsView, SIGNAL(zoomMap(bool)), this, SLOT(zoomUpdated(bool)));
    connect(bandmapGraphicsView, SIGNAL(nextSpot(bool, bool)), this, SLOT(on_nextSpot(bool, bool)));
    connect(bandmapGraphicsView, SIGNAL(scrollMap(bool)),this, SLOT(on_scrollMap(bool)));

    connect(view->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(on_vertScrollBandChanged(int)));

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


void BandmapView::zoomUpdated(bool dir)
{
    zoomLevel = dial->getZoomLevel();

    qint32 scaleStartFreq = static_cast<qint32>(dial->getViewPortFreq(getViewPortStartYCoordOnScene(), contestBandFlow)) / 1000;
    qint32 scaleEndFreq = static_cast<qint32>(dial->getViewPortFreq(getViewPortEndYCoordOnScene(), contestBandFlow)) / 1000;
    qint64 midScaleFreq = static_cast<qint64>((scaleStartFreq + ((scaleEndFreq - scaleStartFreq) / 2)) * 1000);

    if (dir)
    {

        if (zoomLevel < dialMaxZoomLevel && zoomLevel >= dialMinZoomLevel)
        {
            ++zoomLevel;
            dial->setZoomLevel(zoomLevel);
            setBandmapHeight(contestBandFlow, contestBandFhigh);
            dial->update();
            bandmapUpdate();
            scrollBandmapCenterToFreq(midScaleFreq);
        }
    }
    else
    {
        if (zoomLevel != dialMinZoomLevel && zoomLevel <= dialMaxZoomLevel)
        {
            --zoomLevel;
            dial->setZoomLevel(zoomLevel);
            setBandmapHeight(contestBandFlow, contestBandFhigh);
            dial->update();
            bandmapUpdate();
            scrollBandmapCenterToFreq(midScaleFreq);
        }
    }

}

void BandmapView::on_vertScrollBandChanged(int value)
{
    //Q_UNUSED(value)

        dial->setViewPortStartEndFreq(value, getViewPortEndYCoordOnScene(), contestBandFlow);
        trace(QString("Scroll changed: zoomlevel = %1").arg(zoomLevel));
        trace(QString("scroll changed: value = %1").arg(value));
        trace(QString("scroll changed: end Y coord = %1").arg(getViewPortEndYCoordOnScene()));
        bandmapUpdate();
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

    int freqYCoord = dial->getYCoordOnDial(static_cast<qint64>(curFreq));
    int scrollStart = freqYCoord - (bandmapGraphicsView->viewport()->height() / 2);
    trace(QString("bandmapView: makeCursorVisible coord = %1").arg(scrollStart));
    bandmapGraphicsView->verticalScrollBar()->setValue(scrollStart);

}


void BandmapView::scrollBandmapCenterToFreq(qint64 freq)
{

    trace(QString("centerToFreq: freq = %1").arg(freq));

    int freqYCoord = dial->getYCoordOnDial(freq);
    int scrollStart = freqYCoord - (bandmapGraphicsView->viewport()->height() / 2);
    trace(QString("centerToFreq: fYCoord = %1").arg(freqYCoord));
    trace(QString("bandmapView: scrollBandmapCenterToFreq coord = %1").arg(scrollStart));

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
            return i;
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
            return i;
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
            if (!locWorked)
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
            if (!locWorked)
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
    bandmapUpdate();

}


void BandmapView::rowsAboutToBeRemoved(const QModelIndex &parent,
                                         int start, int end)
{

    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}


void BandmapView::bandmapUpdate()
{

    //dial->calcStartEndFreq(dial->getCurFreqInt32());
    //dial->setViewPortStartEndFreq(getViewPortStartYCoordOnScene(), getViewPortEndYCoordOnScene() ,contestBandFlow);
    dial->update();
    drawBandMapSpots();

}


void BandmapView::leftMouseButtonPressed(QPoint p)
{
    QPoint mappedP = bandmapGraphicsView->mapToScene(p).toPoint();

    qint32 freq = dial->checkSelectedFreqTextOnDial(mappedP);

    if (freq > 0)
    {
        sendFreqToRig(QString::number(freq));
    }
    else if (mappedP.x() <= dial->getCurWidth() && mappedP.x() >= dial->getCurWidth() - FREQ_SEL_WIDTH)
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
    int spotNum = isClickInRegionOfSpot(p);

    if (spotNum >= 0)
    {
        //clearSelectedSpot();    // clear any spot previously selected
        //setSelectedSpot(spotNum);


        memoryData::memData spotData;
        spotData.callsign = selectedSpot.dxCall;
        spotData.time = selectedSpot.spotTime;
        spotData.freq = selectedSpot.dxFreqStr;
        spotData.locator = selectedSpot.dxLocator;
        spotData.bearing = selectedSpot.dxBrg.toInt();

        MinosLoggerEvents::SendSpotToLog(spotData);


    }


}


void BandmapView::on_bandmap_customContextMenuRequested( const QPoint& p)
{
    qDebug() << QString("1 - context select point x = %1, y = %2").arg(p.x()).arg(p.y());

    QPoint mapP = bandmapGraphicsView->mapToScene(p).toPoint();

    if (mapP.x() >= dial->getCurWidth() && mapP.x() <= bandmapGraphicsView->width())
    {
        qDebug() << QString("2 - context select map to scene point x = %1, y = %2").arg(mapP.x()).arg(mapP.y());
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

    /*
    if (index.isValid()) {
        if ((cursorAction == MoveLeft && index.row() > 0) ||
            (cursorAction == MoveRight &&
             index.row() + 1 < model()->rowCount())) {
            const int offset = (cursorAction == MoveLeft ? -1 : 1);
            index = model()->index(index.row() + offset,
                                   index.column(), index.parent());
        }
        else if ((cursorAction == MoveUp && index.row() > 0) ||
                 (cursorAction == MoveDown &&
                  index.row() + 1 < model()->rowCount())) {
            QFontMetrics fm(font());
            const int RowHeight = (fm.height() + ExtraHeight) *
                                  (cursorAction == MoveUp ? -1 : 1);
            QRect rect = viewportRectForRow(index.row()).toRect();
            QPoint point(rect.center().x(),
                         rect.center().y() + RowHeight);
            while (point.x() >= 0) {
                index = indexAt(point);
                if (index.isValid())
                    break;
                point.rx() -= fm.width("n");
            }
        }
    }
*/
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
    // cannot scroll
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


void BandmapView::setFilter(BandmapClientFilterDialog* filter)
{
    filterSetup = filter;
}


//QSize BandmapView::minimumSizeHint() const
//{
//    return QSize(70, bandmap->getBandmapFrameHeight());
//                 QFontMetrics(font()).height() + ExtraHeight);

//}





void BandmapView::bandmapResize(int height, int width)
{

    //dial->setCurHeight(height);
    //bandmapUpdate();


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


void BandmapView::setFreq(double f, bool legalFreq)
{
    curFreq = f;
    qint32 freqInt32 = static_cast<qint32>(f / 1000);
    dial->setViewPortStartEndFreq(bandmapGraphicsView->verticalScrollBar()->value(), getViewPortEndYCoordOnScene(), contestBandFlow);

    int viewportYStart = bandmapGraphicsView->verticalScrollBar()->value();
    //qDebug() << "freqInt32 " << freqInt32;
    //qDebug() << "dial start f " << dial->getScaleStartFreq();
    //qDebug() << "dial end f" << dial->getScaleEndFreq();
    //qDebug() << "YcoordStart " << viewportYStart;

    if (freqInt32 > dial->getScaleEndFreq())
    {
        // tuning up, move viewport

        bandmapGraphicsView->verticalScrollBar()->setValue(viewportYStart + (dialData::khzPixelStep[zoomLevel] * 2));
    }
    else if (freqInt32 < dial->getScaleStartFreq())
    {
        // tuning down, move viewport
        bandmapGraphicsView->verticalScrollBar()->setValue(viewportYStart - (dialData::khzPixelStep[zoomLevel] * 2));
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
    bandmapUpdate();
}

void BandmapView::bandmapSelectFreq(int y)
{
    QString f = dial->getFreqFromYCoordOnDial(y);
    MinosLoggerEvents::SendFreqStrToRig(f);

}


void BandmapView::setBandFreqLimits(double flow, double fhigh)
{
    contestBandFlow = flow;
    contestBandFhigh = fhigh;
    dial->setContestBandLimits(contestBandFlow, contestBandFhigh);
}

void BandmapView::setBandmapHeight(double flow, double fhigh)
{
    fullBandHeight = dial->getFullBandHeight(flow, fhigh);
    dial->changeBoundingRect(fullBandHeight, dial->getCurWidth());
    bandmapScene->setSceneRect(0,0, bandmapGraphicsView->width(), fullBandHeight);

}

void BandmapView::sendFreqToRig(QString freq)
{
    QString f = freq.remove('.').append(QString("000"));
    MinosLoggerEvents::SendFreqStrToRig(f);
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

    if (spotViewNum >= 0)
    {
        clearSelectedSpot();       // clear any spot previously selected
        setSelectedSpot(spotViewNum);        // mark new selected spot

        MinosLoggerEvents::SendFreqStrToRig(selectedSpot.dxFreqStr);
    }
    else
    {
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

            bandmapUpdate();



        }
    }
}


void BandmapView::clearSelectedSpotData()
{

    clearSpotData(selectedSpot);
}



void BandmapView::clearSpotData(BandmapData &selectedSpot)
{
    selectedSpotDataRowNum = NO_SELECTED_ROWNUM;
    selectedSpotViewRowNum = NO_SELECTED_ROWNUM;
    selectedSpot.spotTime = "";
    selectedSpot.dxFreqStr = "";
    //selectedSpot.dxFreq = model()->data(model()->index(selectedSpotRowNum, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toLongLong();
    selectedSpot.dxCall = "";
    selectedSpot.dxLocator = "";
    selectedSpot.dxDist = "";
    selectedSpot.dxBrg = "";
    selectedSpot.dxCallWorked = false;
    selectedSpot.dxLocatorWorked = false;
    selectedSpot.isSelected = false;
    selectedSpot.spotType = bandmapSpotType::SPOT_TYPE::NONE;
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
    selectedSpot.isSelected = true;
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


void BandmapView::getSpotData(int &selectedSpotDataRowNum, int selectedSpotViewRowNum, BandmapData &selectedSpot)
{
    selectedSpotDataRowNum = listOfMarkers[selectedSpotViewRowNum]->getModelRowNum();

    if (selectedSpotDataRowNum >= 0 && selectedSpotDataRowNum < model()->rowCount())
    {
        selectedSpot.spotTime = model()->data(model()->index(selectedSpotDataRowNum, TIME_COL_NUM), BMP_DataStoredRole).toString();
        selectedSpot.dxFreqStr = model()->data(model()->index(selectedSpotDataRowNum, FREQ_STR_COL_NUM), BMP_DataStoredRole).toString();
        //selectedSpot.dxFreq = model()->data(model()->index(selectedSpotDataRowNum, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toLongLong();
        selectedSpot.dxCall = model()->data(model()->index(selectedSpotDataRowNum, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toString();
        selectedSpot.dxLocator = model()->data(model()->index(selectedSpotDataRowNum, DXLOC_COL_NUM), BMP_DataStoredRole).toString();
        selectedSpot.dxDist = model()->data(model()->index(selectedSpotDataRowNum, DXDIST_COL_NUM), BMP_DataStoredRole).toString();
        selectedSpot.dxBrg = model()->data(model()->index(selectedSpotDataRowNum, DXBRG_COL_NUM), BMP_DataStoredRole).toString();
        selectedSpot.dxCallWorked = model()->data(model()->index(selectedSpotDataRowNum, DXSPOT_CALL_WORKED_COL_NUM), BMP_DataStoredRole).toBool();
        selectedSpot.dxLocatorWorked = model()->data(model()->index(selectedSpotDataRowNum, DXLOC_WORKED_COL_NUM), BMP_DataStoredRole).toBool();
        selectedSpot.spotType = static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(selectedSpotDataRowNum, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());

    }


}





void BandmapView::drawBandMapSpots()
{


    traceMsg(QString("Drawspots: Start Drawing - Clear Map"));

    deleteItemsFromMarkerList();

    clearListOfMarkers();

    // don't draw spots when freq is zero, or no spots
    if (curFreq == 0.0 || model()->rowCount() == 0)
    {
        traceMsg(QString("CurFreq = %1, Number of Spots = %2 - Don't draw markers").arg(curFreq).arg(model()->rowCount()));
        return;
    }

    qint32 startFreq = dial->getScaleStartFreq();
    qint32 endFreq = dial->getScaleEndFreq();
    int dialWidth = dial->getCurWidth();
    int dialHeight = dial->getCurHeight();

    traceMsg(QString("scale startFreq = %1").arg(startFreq));
    traceMsg(QString("scale endFreq = %1").arg(endFreq));

    QFont cf = QApplication::font();
    QFontMetrics fm(cf);
    fontHeight = fm.height();


    if (dialHeight == 0 || fontHeight == 0)
        return;

    maxNumSpots = dialHeight/fontHeight;
    if (maxNumSpots == 0)
        return;

    int textYCoord = 0;

    for (int i = 0; i < maxNumSpots; i++)
    {
        BandmapMarkerDetails* markerDetails = new BandmapMarkerDetails(QPoint(dialWidth + SPOTMARKER_XOFFSET, textYCoord));
        listOfMarkers.append(markerDetails);
        textYCoord += fontHeight;
    }

    int yCoord = 0;
    int numrows = model()->rowCount();

    if (numrows != 0)
    {
        traceMsg(QString("Drawspots: Number of Rows to Check = %1").arg(numrows));
        traceMsg(QString("dump list of spots and freq"));
        for (int row = 0; row < numrows; row++)
        {
            QString freq = model()->data(model()->index(row, FREQ_STR_COL_NUM), Qt::DisplayRole).toString().remove('.');
            QString callsign = model()->data(model()->index(row, DXSPOT_CALL_COL_NUM), Qt::DisplayRole).toString();
            traceMsg(QString("DB# = %1, Callsign = %2, Freq = %3").arg(row).arg(callsign).arg(freq));
        }



        for (int row = 0; row < numrows; ++row)
        {
            traceMsg(QString("Drawspots: Row = %1").arg(row));
            // check mode against the filter settings
            if (matchMode(row))
            {

                QString freq = model()->data(model()->index(row, FREQ_STR_COL_NUM), Qt::DisplayRole).toString().remove('.');
                //traceMsg(QString("Drawspots: marker freq = %1").arg(freq));
            qint64 f_int64 = freq.toLongLong();
            qint32 f_int32 = freq.toLong();

            if (f_int32 >= startFreq * 1000 && f_int32 <= endFreq * 1000)
            {
                yCoord = dial->getYCoordOnDial(f_int64);
                    //traceMsg(QString("Drawspots: spot freq within scale"));
                    //traceMsg(QString("Drawspots: cursor Freq = %1").arg(f_int64));
                    //traceMsg(QString("Drawspots: cursor Freq y coord = %1").arg(yCoord));
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
                            traceMsg(QString("Drawspots: addmarker = #%1 %2:%3").arg(markNum).arg(callsign).arg(freq));

                            QPoint spotCoord = QPoint(listOfMarkers[markNum]->getSpotMarkerCoord().x(), listOfMarkers[markNum]->getSpotMarkerCoord().y());
                            BandmapSpotMarker* spot = new BandmapSpotMarker(spotCoord);
                            //traceMsg(QString("Drawspots: spot coord x = %1 y = %2").arg(listOfMarkers[markNum]->getSpotMarkerCoord().x()).arg(listOfMarkers[markNum]->getSpotMarkerCoord().y()));

                            bandmapScene->addItem(spot);
                            QString spotMsg = assembleSpotMsg(row);
                            QRectF spotRect = calculateSpotRect(spotMsg, spotCoord);
                            spot->setSpotText(spotMsg);
                            spot->setToolTipText(assembleToolTip(row, freq));
                            listOfMarkers[markNum]->setSpotMarkerPtr(spot);

                            QPoint startMarkerLine = QPoint(dialWidth + SPOTMARKER_XOFFSET, listOfMarkers[markNum]->getSpotMarkerCoord().y() + fontHeight);
                            //traceMsg(QString("Drawspots: marker start coord x = %1 y = %2").arg(dialWidth + SPOTMARKER_XOFFSET).arg(listOfMarkers[markNum]->getSpotMarkerCoord().y() + fontHeight));
                            //traceMsg(QString("Drawspots: font addition = %1").arg(fontHeight));
                            QPoint endMarkerLine = QPoint(dialWidth, yCoord + dialData::DIAL_VERT_OFFSET);
                            //traceMsg(QString(Drawspots: marker end coord x = %1 y = %2").arg(dialWidth).arg(yCoord + dialData::DIAL_VERT_OFFSET));

                            QLine markerLineCoord = QLine(startMarkerLine, endMarkerLine);
                            QLineF markerLineCoordsF = QLineF(markerLineCoord);

                            QGraphicsLineItem* markerLine = new QGraphicsLineItem(markerLineCoordsF);
                            bandmapScene->addItem(markerLine);

                            listOfMarkers[markNum]->setMarkerLinePtr(markerLine);
                            listOfMarkers[markNum]->setSpotRect(spotRect);
                            //listOfMarkers[markNum]->setModelIndex() =
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


void BandmapView::deleteItemsFromMarkerList()
{

    if (!listOfMarkers.isEmpty())
    {
        traceMsg(QString("Drawspots: Remove %1 markers").arg(listOfMarkers.count()));
        for (int i = 0; i < listOfMarkers.count(); i++)
        {
            if (listOfMarkers[i]->getSpotMarkerPtr() != nullptr)
            {
                BandmapSpotMarker* s = listOfMarkers[i]->getSpotMarkerPtr();
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
    bool ok = false;
    int modeMask = model()->data(model()->index(sourceRow, DXMODEMASK_COL_NUM), BMP_DataStoredRole).toString().toInt(&ok);
    QString callsign = model()->data(model()->index(sourceRow, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toString();
    QString freq = model()->data(model()->index(sourceRow, FREQ_STR_COL_NUM), BMP_DataStoredRole).toString();
    QString mode = model()->data(model()->index(sourceRow, DXSPOT_MODE_COL_NUM), BMP_DataStoredRole).toString();
    traceMsg(QString("matchMode - Callsign = %1, freq. = %2, mode = %3, modemask = %4").arg(callsign).arg(freq).arg(mode).arg(QString::number(modeMask)));
    if (ok && modeMask >= 0)
    {
        traceMsg(QString("matchMode - mode match ok"));
        return filterSetup->filterSettings.getModeFilter(modeMask);
    }
    else
    {
        traceMsg(QString("matchMode - no mode match"));
        return false;

    }

}




QRectF BandmapView::calculateSpotRect(const QString text, const QPoint spotCoord)
{
    QFontMetrics fm(font());
    const int rowHeight = fm.height();
    //const int maxWidth = viewport()->width();
   
  
    int textWidth = fm.width(text);
    
    return QRectF(spotCoord.x(),spotCoord.y(), textWidth, rowHeight);
   
    
}




QString BandmapView::assembleSpotMsg(int row)
{

    QString callsign = model()->data(model()->index(row, DXSPOT_CALL_COL_NUM), Qt::DisplayRole).toString();
    QString freqStr =  model()->data(model()->index(row, FREQ_STR_COL_NUM), BMP_DataStoredRole).toString();
    qint64 freq = freqStr.toLongLong();
    freq = freq / 1000;
    qint32 curFreq = dial->getCurFreqInt32();
    QString dxLoc = model()->data(model()->index(row, DXLOC_COL_NUM), Qt::DisplayRole).toString();
    QString dxDist = model()->data(model()->index(row, DXDIST_COL_NUM), Qt::DisplayRole).toString();
    QString dxBrg = model()->data(model()->index(row, DXBRG_COL_NUM), Qt::DisplayRole).toString();
    QString rotBrg = model()->data(model()->index(row, ROT_BEARING_COL_NUM), Qt::DisplayRole).toString();
    bool rotConnected = model()->data(model()->index(row, ROT_CONNECTED_COL_NUM), BMP_DataStoredRole).toBool();

    bandmapSpotType::SPOT_TYPE spotType = static_cast<bandmapSpotType::SPOT_TYPE>(model()->data(model()->index(row, SPOT_TYPE_COL_NUM), BMP_DataStoredRole).toInt());


    qlonglong spotTime = model()->data(model()->index(row, RXTIME_COL_NUM), BMP_DataStoredRole).toLongLong();
    bool olderThan3Min = spotTimedOut(spotTime, NEW_SPOT_TIME);

    QString newSpotMsg = "";
    if (!olderThan3Min)
    {
        newSpotMsg = HtmlFontColour(BANDMAP_NEW_COLOUR) + "New" +  HtmlFontColour(NOT_WORKED_COLOUR);
    }


    QChar degSym = QChar(DEG_SYMBOL);
    if (dxLoc.isEmpty())
    {
       if (rotConnected)
       {
           dxBrg = rotBrg;
       }
       else
       {
           dxBrg = "";
           degSym = QChar(' ');
       }
    }
    else
    {
    if (dxBrg.isEmpty())
    {
        degSym = QChar(' ');
            dxBrg = "";
    }
    }






    QString bLineStart = "";
    QString bLineEnd = "";

    if (freq >= curFreq - 1 && freq <= curFreq +1)
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
    QString elapsedTimeStr = QString::number(elapsedTime);

    QString msg = QString("%1%2 @ .%3 %4  %5  %6%7%8 %9 min %10  %11").arg(bLineStart).arg(callsign).arg(extractKhz(freqStr)).arg(dxLoc).arg(dxDist).arg(dxBrg).arg(degSym).arg(bLineEnd).arg(elapsedTimeStr).arg(markSym).arg(newSpotMsg);

    if (model()->data(model()->index(row, SPOT_IS_SELECTED_COL_NUM), BMP_DataStoredRole).toBool())
    {
        msg = QString("<div style='background:rgba(200, 200, 200, 75%);'>" + msg + QString("</div<"));           // show selected

    }

    return msg;

}


QString BandmapView::assembleToolTip(int row, QString freq)
{
    QString callsign = model()->data(model()->index(row, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toString();
    QString spotterCallsign =  model()->data(model()->index(row, SPOT_CALL_COL_NUM), BMP_DataStoredRole).toString();
    QString spotterLocator = model()->data(model()->index(row, SPOTLOC_COL_NUM), BMP_DataStoredRole).toString();
    QString spotterComment = model()->data(model()->index(row, COMMENT_COL_NUM), BMP_DataStoredRole).toString().replace('<', " (").replace('>', ") ");
    QString computedMode = model()->data(model()->index(row, DXSPOT_MODE_COL_NUM), BMP_DataStoredRole).toString();

    qlonglong spotTime = model()->data(model()->index(row, RXTIME_COL_NUM), BMP_DataStoredRole).toLongLong();
    qlonglong elapsedTime = spotElapsedTime(spotTime) / 60;
    QString elapsedTimeStr = QString::number(elapsedTime);

    QString msg = QString("%1 - %2 [%3 %4 @ %5 min] \nThe computed mode is %6\n%7").arg(callsign).arg(convertFreqStrDisp(freq)).arg(spotterCallsign).arg(spotterLocator).arg(elapsedTimeStr).arg(computedMode).arg(spotterComment);

    return msg;

}


void BandmapView::setFreqOperatingInfo(const QString contestBandStr, const QString contestModeStr, CheckOperatingFreq *operatingFreq, const bool operatingPlanOk)
{
    if (operatingPlanOk)
    {
        dial->setFreqOperatingInfo(contestBandStr, contestModeStr, operatingFreq, operatingPlanOk);
    }

}



void BandmapView::traceMsg(QString msg)
{
    trace(QString("bandmapView: %1").arg(msg));
}
