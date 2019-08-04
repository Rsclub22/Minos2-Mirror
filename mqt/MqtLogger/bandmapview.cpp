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

BandmarkerDetials::BandmarkerDetials(QPoint _freqLineStart, QPoint _freqLineEnd, QPoint _spotMarkerCoord, BandmapSpotMarker* _spot)
{
    freqLineStart = _freqLineStart;
    freqLineEnd = _freqLineEnd;
    spotMarkerCoord = _spotMarkerCoord;
    spot = _spot;

}



BandmapView::BandmapView(QGraphicsView* _bandmapGraphicsView, QWidget *parent) :
    QAbstractItemView(parent),
    fontHeight(0),
    maxNumSpots(0)
{

    //setFocusPolicy((Qt::WheelFocus));
    //setMinimumSize(minimumSizeHint());

    bandmapScene = new QGraphicsScene(this);


    horizontalScrollBar()->setRange(0, 0);
    verticalScrollBar()->setRange(0, 0);


    bandmapGraphicsView = _bandmapGraphicsView;
    //bandmapGraphicsView->setScene(bandmapScene);
    setScene(bandmapScene);

    bandmapGraphicsView->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    dial = new BandmapFreqDial(70, getBandmapFrameHeight());
    qDebug() << "bandmap height " << getBandmapFrameHeight();
    dialMinZoomLevel = dial->getMinZoomLevel();
    dialMaxZoomLevel = dial->getMaxZoomLevel();
    bandmapScene->addItem(dial);
    dial->setCurFreq(0.0);
    dial->setCursorColour(Qt::black);




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


void BandmapView::paintEvent(QPaintEvent *event)
{

    QPainter* painter = new QPainter(bandmapGraphicsView);

    dial->update();
    drawBandMapSpots(painter, dial);

/*
    QItemSelectionModel *selections = selectionModel();
    QStyleOptionViewItem option = viewOptions();

    QBrush background = option.palette.base();
    QPen foreground(option.palette.color(QPalette::WindowText));

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(event->rect(), background);
    painter.setPen(foreground);

    // Viewport rectangles
    QRect pieRect = QRect(margin, margin, pieSize, pieSize);

    if (validItems <= 0)
        return;

    painter.save();
    painter.translate(pieRect.x() - horizontalScrollBar()->value(),
                      pieRect.y() - verticalScrollBar()->value());
    painter.drawEllipse(0, 0, pieSize, pieSize);
    double startAngle = 0.0;
    int row;

    for (row = 0; row < model()->rowCount(rootIndex()); ++row) {
        QModelIndex index = model()->index(row, 1, rootIndex());
        double value = model()->data(index).toDouble();

        if (value > 0.0) {
            double angle = 360 * value / totalValue;

            QModelIndex colorIndex = model()->index(row, 0, rootIndex());
            QColor color = QColor(model()->data(colorIndex, Qt::DecorationRole).toString());

            if (currentIndex() == index)
                painter.setBrush(QBrush(color, Qt::Dense4Pattern));
            else if (selections->isSelected(index))
                painter.setBrush(QBrush(color, Qt::Dense3Pattern));
            else
                painter.setBrush(QBrush(color));

            painter.drawPie(0, 0, pieSize, pieSize, int(startAngle*16), int(angle*16));

            startAngle += angle;
        }
    }
    painter.restore();

    int keyNumber = 0;

    for (row = 0; row < model()->rowCount(rootIndex()); ++row) {
        QModelIndex index = model()->index(row, 1, rootIndex());
        double value = model()->data(index).toDouble();

        if (value > 0.0) {
            QModelIndex labelIndex = model()->index(row, 0, rootIndex());

            QStyleOptionViewItem option = viewOptions();
            option.rect = visualRect(labelIndex);
            if (selections->isSelected(labelIndex))
                option.state |= QStyle::State_Selected;
            if (currentIndex() == labelIndex)
                option.state |= QStyle::State_HasFocus;
            itemDelegate()->paint(&painter, option, labelIndex);

            ++keyNumber;
        }
    }

*/

}

//void BandmapView::setModel(QAbstractItemModel *model)
//{
//    QAbstractItemView::setModel(model);
//    //hashIsDirty = true;
//}


void BandmapView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{

    QAbstractItemView::dataChanged(topLeft, bottomRight, roles);
    viewport()->update();
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

void BandmapView::updateGeometries()
{

}



int BandmapView::horizontalOffset() const
{


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

}


int BandmapView::verticalOffset() const
{


}


QRegion BandmapView::visualRegionForSelection(const QItemSelection &selection) const
{


}





/*

QRectF BandmapView::viewportRectForRow(int row) const
{
    calculateRectsIfNecessary();
    QRectF rect = rectForRow.value(row).toRect();
    if (!rect.isValid())
        return rect;
    return QRectF(rect.x() - horizontalScrollBar()->value(),
                  rect.y() - verticalScrollBar()->value(),
                  rect.width(), rect.height());
}
*/

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
    QRect viewRect = viewport()->rect();
    QRect itemRect = visualRect(index);

    if (itemRect.left() < viewRect.left())
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()
                + itemRect.left() - viewRect.left());
    else if (itemRect.right() > viewRect.right())
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()
                + qMin(itemRect.right() - viewRect.right(),
                       itemRect.left() - viewRect.left()));
    if (itemRect.top() < viewRect.top())
        verticalScrollBar()->setValue(verticalScrollBar()->value() +
                itemRect.top() - viewRect.top());
    else if (itemRect.bottom() > viewRect.bottom())
        verticalScrollBar()->setValue(verticalScrollBar()->value() +
                qMin(itemRect.bottom() - viewRect.bottom(),
                     itemRect.top() - viewRect.top()));
    viewport()->update();
}


QRect BandmapView::visualRect(const QModelIndex &index) const
{
//    QRect rect = itemRect(index);
//    if (!rect.isValid())
//        return rect;

//    return QRect(rect.left() - horizontalScrollBar()->value(),
//                 rect.top() - verticalScrollBar()->value(),
//                 rect.width(), rect.height());

    return viewport()->rect();

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
            viewport()->update();

}

void BandmapView::drawBandMapSpots(QPainter* painter, BandmapFreqDial *dial)
{
    if (!listOfMarkers.isEmpty())
    {
        for (int i = 0; i < listOfMarkers.count(); i++)
        {
            if (listOfMarkers[i]->spot != nullptr)
            {
                bandmapScene->removeItem(listOfMarkers[i]->spot);
            }

        }
    }

    listOfMarkers.clear();

    qint32 startFreq = dial->getScaleStartFreq();
    qint32 endFreq = dial->getScaleEndFreq();
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

    int textYCoord = 0;

    for (int i = 0; i < maxNumSpots; i++)
    {
        BandmarkerDetials* markerDetials = new BandmarkerDetials(QPoint(0,0), QPoint(0, 0), QPoint(dialWidth + 10, textYCoord), nullptr);
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
            qint64 f_int64 = freq.toLongLong();
            qint32 f_int32 = freq.toLong();

            if (f_int32 >= startFreq * 1000 && f_int32 <= endFreq * 1000)
            {
                yCoord = dial->getYCoordOnDial(f_int64);

                for (int markNum = 0; markNum < listOfMarkers.count(); markNum++)
                {
                    qDebug() << listOfMarkers[markNum]->spotMarkerCoord.y();
                    if (listOfMarkers[markNum]->spotMarkerCoord.y() >= yCoord)
                    {
                        if (listOfMarkers[markNum]->spot == nullptr)
                        {
                            index = model()->index(row, DXSPOT_CALL_COL_NUM);
                            QString callsign = model()->data(index, Qt::DisplayRole).toString();
                            BandmapSpotMarker* spot = new BandmapSpotMarker(QPoint(listOfMarkers[markNum]->spotMarkerCoord.x(), listOfMarkers[markNum]->spotMarkerCoord.y()), callsign, "testing", Qt::red);
                            bandmapScene->addItem(spot);
                            spot->setSpotText(callsign + " " + freq);
                            listOfMarkers[markNum]->spot = spot;
                            QPoint startMarkerLine = QPoint(dialWidth + 10, listOfMarkers[markNum]->spotMarkerCoord.y() + fontHeight/2);
                            QPoint endMarkerLine = QPoint(dialWidth, yCoord);
                            painter->setPen(Qt::black);
                            painter->drawLine(startMarkerLine, endMarkerLine);
                            listOfMarkers[markNum]->freqLineStart = startMarkerLine;
                            listOfMarkers[markNum]->freqLineEnd = endMarkerLine;
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



