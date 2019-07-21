////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Bandmap Spot Marker
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////

#include "bandmapspotmarker.h"

BandmapSpotMarker::BandmapSpotMarker(const QPoint &_position, QString text, QString _auxText, QColor _colour)
    : QGraphicsTextItem(),
      position(_position),
      displayText(text),
      auxText(_auxText),
      textColour(_colour)
{
    // setFont(QFont("Helvetica", 11));
    setPos(position);
    //scene->clearSelection();
    //scene->addItem(this);
    //setSelected(true);
    setHtml(displayText);

}


void BandmapSpotMarker::setSpotText(QString text)
{
    displayText = text;
    setHtml(displayText);
    update();
}


void BandmapSpotMarker::hoverMoveEvent(QGraphicsSceneHoverEvent* /*event*/)
{

   setToolTip("10:30, 360 deg");

}


void BandmapSpotMarker::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << x() << " " << y();
    qDebug() << displayText;
    QGraphicsItem::mousePressEvent(event);
}
