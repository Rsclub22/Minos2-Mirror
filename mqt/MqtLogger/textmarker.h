#ifndef TEXTMARKER_H
#define TEXTMARKER_H
#include <QPainter>
#include <QGraphicsItem>

namespace textData {



const int XCOORD = 80;


}



class TextMarker: public QGraphicsItem
{
public:
    TextMarker( int x, int y, QString callsign, QColor colour);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:

    int xCoord;
    int yCoord;
    QString callSign;
    QColor colour;


    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
};

#endif // TEXTMARKER_H
