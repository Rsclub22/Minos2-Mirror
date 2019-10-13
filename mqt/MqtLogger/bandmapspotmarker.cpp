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
#include <QDebug>

BandmapSpotMarker::BandmapSpotMarker(const QPoint &_position)
    : QGraphicsTextItem(),
      position(_position)

{
    setPos(position);


}



void BandmapSpotMarker::setSpotText(QString text)
{
    displayText = text;
    setHtml(displayText);
    update();
}


QString BandmapSpotMarker::getSpotText()
{
    return displayText;
}


void BandmapSpotMarker::setToolTipText(QString text)
{
    auxText = text;
    setToolTip(text);
}

QString BandmapSpotMarker::getToolTipText()
{
    return auxText;
}


void BandmapSpotMarker::hoverMoveEvent(QGraphicsSceneHoverEvent* /*event*/)
{



}


void BandmapSpotMarker::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //qDebug() << x() << " " << y();
    //qDebug() << displayText;
    QGraphicsItem::mousePressEvent(event);
}
