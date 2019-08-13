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


#include <QDebug>

const int SPOTMARKER_XOFFSET = 20;


BandmarkerDetials::BandmarkerDetials( QPoint _spotMarkerCoord, BandmapSpotMarker* _spot, QGraphicsLineItem* _markerLine)
{
    spotMarkerCoord = _spotMarkerCoord;
    spot = _spot;
    markerline = _markerLine;

}



BandmapView::BandmapView(QWidget *parent) :
    QAbstractItemView(parent),
    zoomLevel(0),
    fontHeight(0),
    maxNumSpots(0)
{

    //setFocusPolicy((Qt::WheelFocus));
    //setMinimumSize(minimumSizeHint());

    bandmapScene = new QGraphicsScene(parent);



    horizontalScrollBar()->setRange(0, 0);
    verticalScrollBar()->setRange(0, 0);


    //bandmapGraphicsView = new QGraphicsView(bandmapScene);
    //bandmapGraphicsView->setScene(bandmapScene);




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
    bandmapUpdate();



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
            bandmapUpdate();
        }
    }
    else
    {
        if (zoomLevel != dialMinZoomLevel && zoomLevel <= dialMaxZoomLevel)
        {
            --zoomLevel;
            dial->setZoomLevel(zoomLevel);
            bandmapUpdate();
        }
    }

}



void BandmapView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{

    QAbstractItemView::dataChanged(topLeft, bottomRight, roles);

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

    dial->calcStartEndFreq(dial->getCurFreqInt32());
    dial->update();
    drawBandMapSpots();

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
}


int BandmapView::verticalOffset() const
{

    return verticalScrollBar()->value();
}


QRegion BandmapView::visualRegionForSelection(const QItemSelection &selection) const
{


}










QModelIndex BandmapView::indexAt(const QPoint &point_) const
{
    //QPoint point(point_);
    //point.rx() += horizontalScrollBar()->value();
    //point.ry() += verticalScrollBar()->value();
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

    //QRect rect;
    //    if (index.isValid())
    //        rect = viewportRectForRow(index.row()).toRect();
        return rect();


}






//QSize BandmapView::minimumSizeHint() const
//{
//    return QSize(70, bandmap->getBandmapFrameHeight());
     //            QFontMetrics(font()).height() + ExtraHeight);

//}





void BandmapView::bandmapResize(int _height)
{
    int height = _height;
    //int width = _width;
    qDebug() << "bandmap resize height " << height;
    dial->setCurHeight(height);
    bandmapUpdate();


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

}


void BandmapView::keyPressEvent(QKeyEvent *event)
{

}






void BandmapView::resizeEvent(QResizeEvent *)
{


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
            bandmapUpdate();

}

void BandmapView::drawBandMapSpots()
{
    if (!listOfMarkers.isEmpty())
    {
        for (int i = 0; i < listOfMarkers.count(); i++)
        {
            if (listOfMarkers[i]->spot != nullptr)
            {
                bandmapScene->removeItem(listOfMarkers[i]->spot);
                delete(listOfMarkers[i]->spot);
            }
            if (listOfMarkers[i]->markerline != nullptr)
            {
                bandmapScene->removeItem(listOfMarkers[i]->markerline);
                delete(listOfMarkers[i]->markerline);
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
        BandmarkerDetials* markerDetials = new BandmarkerDetials(QPoint(dialWidth + SPOTMARKER_XOFFSET, textYCoord), nullptr, nullptr);
        listOfMarkers.append(markerDetials);
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

                    if (listOfMarkers[markNum]->spotMarkerCoord.y() + fontOffset  >= yCoord && markNum != 0)
                    {
                        if (listOfMarkers[markNum]->spot == nullptr)
                        {
                            //index = model()->index(row, DXSPOT_CALL_COL_NUM);
                            //QString callsign = model()->data(index, Qt::DisplayRole).toString();
                            //trace(QString("bandmapView: addmarker = %1").arg(callsign));

                            BandmapSpotMarker* spot = new BandmapSpotMarker(QPoint(listOfMarkers[markNum]->spotMarkerCoord.x(), listOfMarkers[markNum]->spotMarkerCoord.y()));
                            trace(QString("bandmapView: spot coord x = %1 y = %2").arg(listOfMarkers[markNum]->spotMarkerCoord.x()).arg(listOfMarkers[markNum]->spotMarkerCoord.y()));

                            bandmapScene->addItem(spot);
                            spot->setSpotText(assembleSpotMsg(row));
                            spot->setToolTipText(assembleToolTip(row, freq));
                            listOfMarkers[markNum]->spot = spot;
                            QPoint startMarkerLine = QPoint(dialWidth + SPOTMARKER_XOFFSET, listOfMarkers[markNum]->spotMarkerCoord.y() + fontHeight);
                            trace(QString("bandmapView: marker start coord x = %1 y = %2").arg(dialWidth + SPOTMARKER_XOFFSET).arg(listOfMarkers[markNum]->spotMarkerCoord.y() + fontHeight));
                            trace(QString("bandmapView: font addition = %1").arg(fontHeight));
                            QPoint endMarkerLine = QPoint(dialWidth, yCoord + dialData::DIAL_VERT_OFFSET);
                            trace(QString("bandmapView: marker end coord x = %1 y = %2").arg(dialWidth).arg(yCoord + dialData::DIAL_VERT_OFFSET));

                            QLine markerLineCoord = QLine(startMarkerLine, endMarkerLine);
                            QLineF markerLineCoordsF = QLineF(markerLineCoord);

                            QGraphicsLineItem* markerLine = new QGraphicsLineItem(markerLineCoordsF);
                            bandmapScene->addItem(markerLine);

                            listOfMarkers[markNum]->markerline = markerLine;

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
    QString callsign = model()->data(model()->index(row, DXSPOT_CALL_COL_NUM), Qt::DisplayRole).toString();
    QString spotterCallsign =  model()->data(model()->index(row, SPOT_CALL_COL_NUM), Qt::DisplayRole).toString();
    QString spotterLocator = model()->data(model()->index(row, SPOTLOC_COL_NUM), Qt::DisplayRole).toString();
    QString spotterComment = model()->data(model()->index(row, COMMENT_COL_NUM), Qt::DisplayRole).toString();

    QString elapsedTime = QString("0");

    QString msg = QString("%1 - %2 [%3 %4 @ %5 min] - %6").arg(callsign).arg(freq).arg(spotterCallsign).arg(spotterLocator).arg(elapsedTime).arg(spotterComment);
    qDebug() << "tooltip = " << msg;
    return msg;

}


