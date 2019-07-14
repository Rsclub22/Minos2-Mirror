#include "textmarker.h"
#include <QDebug>

TextMarker::TextMarker(int x, int y, QString callsign, QColor txt_colour)
{
    xCoord = x;
    yCoord = y;
    callSign = callsign;
    colour = txt_colour;
    setAcceptHoverEvents(true);

}

void TextMarker::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->setPen(colour);
    painter->drawText(xCoord, yCoord, callSign);
    //painter->drawRect(0,0,40,10);
}

QRectF TextMarker::boundingRect() const
{
    qreal penWidth = 3;
//    return QRectF(-10 - penWidth/2, -10 - penWidth/2, 20 + penWidth/2, 20 + penWidth/2);
    return QRectF(80 - penWidth/2, yCoord - penWidth/2 - 10,40,10);

}


void TextMarker::hoverMoveEvent(QGraphicsSceneHoverEvent* /*event*/)
{

   setToolTip("10:30, 360 deg");

}


void TextMarker::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << x() << " " << y();
    qDebug() << callSign;
    QGraphicsItem::mousePressEvent(event);
}
