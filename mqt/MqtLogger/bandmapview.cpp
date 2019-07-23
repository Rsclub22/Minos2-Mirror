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




BandmapView::BandmapView(QGraphicsView* _bandmapGraphicsView, QWidget *parent) : QAbstractItemView(parent)
{

    //setFocusPolicy((Qt::WheelFocus));
    //setMinimumSize(minimumSizeHint());

    bandmapScene = new QGraphicsScene(this);

    bandmapGraphicsView = _bandmapGraphicsView;
    bandmapGraphicsView->setScene(bandmapScene);

    bandmapGraphicsView->setAlignment(Qt::AlignTop|Qt::AlignLeft);

    dial = new BandmapFreqDial(70, getBandmapFrameHeight());
    qDebug() << "bandmap height " << getBandmapFrameHeight();
    dialMinZoomLevel = dial->getMinZoomLevel();
    dialMaxZoomLevel = dial->getMaxZoomLevel();
    bandmapScene->addItem(dial);
    dial->setCurFreq(0.0);

    dial->update();
    drawBandMapSpots();

    connect (bandmapGraphicsView, SIGNAL(bandmapResize(int)), this, SLOT(bandmapResize(int)));


    //QString redHtml = "<font color=\"Red\">";
    //QString endHtml = "</font><br>";
    //QString msg = redHtml + "G8FKH" + endHtml;
    //QPoint pos = QPoint(80,100);
    //BandmapSpotMarker* spot = new BandmapSpotMarker(pos, redHtml + "G8FKH" + endHtml, "testing", Qt::red, bandmapScene);
    //spot->setSpotText(msg);
    //pos = QPoint(80,125);
    //spot = new BandmapSpotMarker(pos, redHtml + "M0ICR" + endHtml, "testing", Qt::red, bandmapScene);


}

void BandmapView::setModel(QAbstractItemModel *model)
{
    QAbstractItemView::setModel(model);
    //hashIsDirty = true;
}


void BandmapView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{

    QAbstractItemView::dataChanged(topLeft, bottomRight);
}


void BandmapView::rowsInserted(const QModelIndex &parent, int start, int end)
{

    QAbstractItemView::rowsInserted(parent, start, end);
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






void BandmapView::scrollTo(const QModelIndex &index,
                             QAbstractItemView::ScrollHint)
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
    QRect rect;
    if (index.isValid())
        //rect = viewportRectForRow(index.row()).toRect();
    return rect;
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
    dial->update();
    drawBandMapSpots();


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




void BandmapView::paintEvent(QPaintEvent *event)
{
    //QWidget::paintEvent(event);
    QPainter painter(this);
    //qDebug() << "height" << bandmap->getBandmapFrameHeight();
    //qDebug() << "width" << bandmap->getBandmapFrameWidth();

    //dial->drawScale(&painter, curFreq, bandmap->getBandmapFrameHeight());
    //dial->drawCursor(&painter, curFreq);

    //bandmapSpotMarker = new TextMarker(80, 100, "G8FKH", Qt::red);
    //bandmapSpotMarker->drawTextMarker(&painter);
    //dial->drawScale(&painter, curFreq, bandmap->getBandmapFrameHeight());
    //dial->drawCursor(&painter, curFreq);

    //drawBandMapSpots(&painter, event);

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
        dial->update();


}

void BandmapView::drawBandMapSpots()
{
    if (!listOfMarkers.isEmpty())
    {
        for (int i = 0; i < listOfMarkers.count(); i++)
        {
            bandmapScene->removeItem(listOfMarkers[i]);
        }
    }
    QString redHtml = "<font color=\"Red\">";
    QString endHtml = "</font><br>";
    QString msg = redHtml + "G8FKH" + endHtml;
    QPoint pos = QPoint(80,100);
    BandmapSpotMarker* spot = new BandmapSpotMarker(pos, redHtml + "G8FKH" + endHtml, "testing", Qt::red);
    listOfMarkers.append(spot);
    spot->setSpotText(msg);
    bandmapScene->addItem(spot);
    pos = QPoint(80,125);
    spot = new BandmapSpotMarker(pos, redHtml + "M0ICR" + endHtml, "testing", Qt::red);
    listOfMarkers.append(spot);
    bandmapScene->addItem(spot);

}



