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


#include <QDebug>

const int SPOTMARKER_XOFFSET = 20;




BandmapView::BandmapView(QWidget *parent) :
    QAbstractItemView(parent),
    zoomLevel(0),
    idealWidth(0),
    idealHeight(0),
    fontHeight(0),
    maxNumSpots(0)
{

    setFocusPolicy((Qt::WheelFocus));
    //setMinimumSize(minimumSizeHint());

    bandmapScene = new QGraphicsScene(parent);



    horizontalScrollBar()->setRange(0, 0);
    verticalScrollBar()->setRange(0, 0);


}


void BandmapView::initBandmapView(QGraphicsView* view )
{
    bandmapGraphicsView = view;
    bandmapGraphicsView->setScene(bandmapScene);
    bandmapGraphicsView->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    bandmapGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
    bandmapGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff );
    bandmapScene->setSceneRect(0,0, bandmapGraphicsView->width(), bandmapGraphicsView->height());

    dial = new BandmapFreqDial(70, bandmapGraphicsView->viewport()->height());
    qDebug() << "bandmap height " << getBandmapFrameHeight();
    dialMinZoomLevel = dial->getMinZoomLevel();
    dialMaxZoomLevel = dial->getMaxZoomLevel();
    bandmapScene->addItem(dial);
    dial->setCurFreq(0.0);
    dial->setCursorColour(Qt::black);
    viewport()->update();



    //connect (dial, SIGNAL(dialupdated()), this, SLOT(drawBandMapSpots()));
    connect (dial, SIGNAL(zoomUpdated(bool)), this, SLOT(zoomUpdated(bool)));
    connect (bandmapGraphicsView, SIGNAL(bandmapResize(int)), this, SLOT(bandmapResize(int)));


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


void BandmapView::zoomUpdated(bool dir)
{

    if (dir)
    {
        if (zoomLevel < dialMaxZoomLevel && zoomLevel >= dialMinZoomLevel)
        {
            ++zoomLevel;
            dial->setZoomLevel(zoomLevel);
            viewport()->update();
        }
    }
    else
    {
        if (zoomLevel != dialMinZoomLevel && zoomLevel <= dialMaxZoomLevel)
        {
            --zoomLevel;
            dial->setZoomLevel(zoomLevel);
            viewport()->update();
        }
    }

}

void BandmapView::paintEvent(QPaintEvent *event)
{

    dial->calcStartEndFreq(dial->getCurFreqInt32());
    dial->update();
    drawBandMapSpots();


}

void BandmapView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{

    QAbstractItemView::dataChanged(topLeft, bottomRight, roles);

}

void BandmapView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    int a = 0;
}

void BandmapView::rowsInserted(const QModelIndex &parent, int start, int end)
{

    QAbstractItemView::rowsInserted(parent, start, end);
    viewport()->update();

}


void BandmapView::rowsAboutToBeRemoved(const QModelIndex &parent,
                                         int start, int end)
{

    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}

/*
void BandmapView::bandmapUpdate()
{

    dial->calcStartEndFreq(dial->getCurFreqInt32());
    dial->update();
    drawBandMapSpots();

}
*/


void BandmapView::updateGeometries()
{
    horizontalScrollBar()->setPageStep(viewport()->width());
    horizontalScrollBar()->setRange(0, qMax(0, 2 * totalSize - viewport()->width()));
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0, qMax(0, totalSize - viewport()->height()));
}



int BandmapView::horizontalOffset() const
{

    return horizontalScrollBar()->value();
}



QModelIndex BandmapView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers)
{
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
    int a = 0;

    // do nothing
}


int BandmapView::verticalOffset() const
{

    return verticalScrollBar()->value();
}


QRegion BandmapView::visualRegionForSelection(const QItemSelection &selection) const
{
    int a = 0;

}










QModelIndex BandmapView::indexAt(const QPoint &point_) const
{


    QPoint point(point_);
    point.rx() += horizontalScrollBar()->value();
    point.ry() += verticalScrollBar()->value();
    int a = 0;
    //calculateRectsIfNecessary();
    //QHashIterator<int, QRectF> i(rectForRow);
    //while (i.hasNext()) {
    //    i.next();
   //     if (i.value().contains(point))
    //        return model()->index(i.key(), 0, rootIndex());
   //}
    //return QModelIndex();
}






void BandmapView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint)
{
    // cannot scroll
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



//QSize BandmapView::minimumSizeHint() const
//{
//    return QSize(70, bandmap->getBandmapFrameHeight());
//                 QFontMetrics(font()).height() + ExtraHeight);

//}





void BandmapView::bandmapResize(int _height)
{
    int height = _height;
    //int width = _width;
    qDebug() << "bandmap resize height " << height;
    dial->setCurHeight(height);
    viewport()->update();


}



//QSize BandmapView::sizeHint() const
//{
    /*
    int rows = visualizer->model()
               ? visualizer->model()->rowCount() : 1;
    return QSize(visualizer->widthOfYearColumn() +
            qMax(100, visualizer->maleFemaleHeaderTextWidth()) +
            visualizer->widthOfTotalColumn(),
            visualizer->yOffsetForRow(rows));
    */

//    return QSize(70, bandmap->getBandmapFrameHeight());
//}


//bool BandmapView::eventFilter(QObject *target, QEvent *event)
//{

//}


void BandmapView::mousePressEvent(QMouseEvent *event)
{
    QAbstractItemView::mousePressEvent(event);
    setCurrentIndex(indexAt(event->pos()));
}


void BandmapView::keyPressEvent(QKeyEvent *event)
{
    int a = 0;
}






void BandmapView::resizeEvent(QResizeEvent *)
{
    updateGeometries();
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


void BandmapView::setFreq(double f)
{
    curFreq = f;
    dial->setCurFreq(f);
    viewport()->update();
}

void BandmapView::drawBandMapSpots()
{
    if (!listOfMarkers.isEmpty())
    {
        for (int i = 0; i < listOfMarkers.count(); i++)
        {
            if (listOfMarkers[i]->getSpotMarkerPtr() != nullptr)
            {
                delete(listOfMarkers[i]->getSpotMarkerPtr());
                bandmapScene->removeItem(listOfMarkers[i]->getSpotMarkerPtr());
            }
            if (listOfMarkers[i]->getMarkerLinePtr() != nullptr)
            {
                delete(listOfMarkers[i]->getMarkerLinePtr());
                bandmapScene->removeItem(listOfMarkers[i]->getMarkerLinePtr());

            }

        }
    }

    listOfMarkers.clear();

    qint32 startFreq = dial->getScaleStartFreq();
    qint32 endFreq = dial->getScaleEndFreq();
    int dialWidth = dial->getCurWidth();
    int dialHeight = dial->getCurHeight();

    trace(QString("bandmapView: scale startFreq = %1").arg(startFreq));
    trace(QString("bandmapView: scale endFreq = %1").arg(endFreq));

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
        for (int row = 0; row < numrows; ++row)
        {

            QModelIndex index = model()->index(row, FREQ_COL_NUM);
            QString freq = model()->data(index, Qt::DisplayRole).toString() + "000";
            trace(QString("bandmapView: marker freq = %1").arg(freq));
            qint64 f_int64 = freq.toLongLong();
            qint32 f_int32 = freq.toLong();

            if (f_int32 >= startFreq * 1000 && f_int32 <= endFreq * 1000)
            {
                yCoord = dial->getYCoordOnDial(f_int64);
                trace(QString("bandmapView: cursor Freq = %1").arg(f_int64));
                trace(QString("bandmapView: cursor Freq y coord = %1").arg(yCoord));
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
                            //index = model()->index(row, DXSPOT_CALL_COL_NUM);
                            //QString callsign = model()->data(index, Qt::DisplayRole).toString();
                            //trace(QString("bandmapView: addmarker = %1").arg(callsign));

                            QPoint spotCoord = QPoint(listOfMarkers[markNum]->getSpotMarkerCoord().x(), listOfMarkers[markNum]->getSpotMarkerCoord().y());
                            BandmapSpotMarker* spot = new BandmapSpotMarker(spotCoord);
                            trace(QString("bandmapView: spot coord x = %1 y = %2").arg(listOfMarkers[markNum]->getSpotMarkerCoord().x()).arg(listOfMarkers[markNum]->getSpotMarkerCoord().y()));

                            bandmapScene->addItem(spot);
                            QString spotMsg = assembleSpotMsg(row);
                            QRectF spotRect = calculateSpotRect(spotMsg, spotCoord);
                            spot->setSpotText(spotMsg);
                            spot->setToolTipText(assembleToolTip(row, freq));
                            listOfMarkers[markNum]->setSpotMarkerPtr(spot);

                            QPoint startMarkerLine = QPoint(dialWidth + SPOTMARKER_XOFFSET, listOfMarkers[markNum]->getSpotMarkerCoord().y() + fontHeight);
                            trace(QString("bandmapView: marker start coord x = %1 y = %2").arg(dialWidth + SPOTMARKER_XOFFSET).arg(listOfMarkers[markNum]->getSpotMarkerCoord().y() + fontHeight));
                            trace(QString("bandmapView: font addition = %1").arg(fontHeight));
                            QPoint endMarkerLine = QPoint(dialWidth, yCoord + dialData::DIAL_VERT_OFFSET);
                            trace(QString("bandmapView: marker end coord x = %1 y = %2").arg(dialWidth).arg(yCoord + dialData::DIAL_VERT_OFFSET));

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




                /*
                // find a free slot to display
                for (int i = 0; i < listOfMarkers.count(); i++)
                {

                    if (yCoord >= listOfMarkers[i]->spotMarkerCoord.y())
                    {
                        if (listOfMarkers[i]->spot != nullptr)
                        {
                            index = model()->index(row, DXSPOT_CALL_COL_NUM);
                            QString callsign = model()->data(index, Qt::DisplayRole).toString();
                            BandmapSpotMarker* spot = new BandmapSpotMarker(QPoint(listOfMarkers[i]->spotMarkerCoord.x(), listOfMarkers[i]->spotMarkerCoord.y()), callsign, "testing", Qt::red);
                            bandmapScene->addItem(spot);
                            spot->setSpotText(callsign + " " + freq);
                            listOfMarkers[i]->spot = spot;
                        }
                    }
                }
                */

            }


        }

    }


    /*
        QString redHtml = "<font color=\"Red\">";
        QString endHtml = "</font><br>";
        QString msg = redHtml + "G8FKH" + endHtml;
        QPoint pos = QPoint(80,100);
        BandmapSpotMarker* spot = new BandmapSpotMarker(pos, redHtml + "G8FKH" + endHtml, "testing", Qt::red);
        BandmarkerDetials* markerDetials = new BandmarkerDetials(QPoint(0,0), QPoint(0,0), spot);
        listOfMarkers.append(markerDetials);
        spot->setSpotText(msg);
        bandmapScene->addItem(spot);
        pos = QPoint(80,125);
        spot = new BandmapSpotMarker(pos, redHtml + "M0ICR" + endHtml, "testing", Qt::red);
        listOfMarkers.append(markerDetials);
        bandmapScene->addItem(spot);
    */
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
    QString dxLoc = model()->data(model()->index(row, DXLOC_COL_NUM), Qt::DisplayRole).toString();
    QString dxDist = model()->data(model()->index(row, DXDIST_COL_NUM), Qt::DisplayRole).toString();
    QString dxBrg = model()->data(model()->index(row, DXBRG_COL_NUM), Qt::DisplayRole).toString();
    QChar degSym = QChar(DEG_SYMBOL);
    if (dxBrg.isEmpty())
    {
        degSym = QChar(' ');
    }
    QString msg = QString("%1  %2  %3  %4%5").arg(callsign).arg(dxLoc).arg(dxDist).arg(dxBrg).arg(degSym);
    qDebug() << "spot = " << msg;
    return msg;

}


QString BandmapView::assembleToolTip(int row, QString freq)
{
    QString callsign = model()->data(model()->index(row, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toString();
    QString spotterCallsign =  model()->data(model()->index(row, SPOT_CALL_COL_NUM), BMP_DataStoredRole).toString();
    QString spotterLocator = model()->data(model()->index(row, SPOTLOC_COL_NUM), BMP_DataStoredRole).toString();
    QString spotterComment = model()->data(model()->index(row, COMMENT_COL_NUM), BMP_DataStoredRole).toString().replace('<', " (").replace('>', ") ");

    QString elapsedTime = QString("0");

    QString msg = QString("%1 - %2 [%3 %4 @ %5 min] \n%6").arg(callsign).arg(convertFreqStrDisp(freq)).arg(spotterCallsign).arg(spotterLocator).arg(elapsedTime).arg(spotterComment);

    return msg;

}


