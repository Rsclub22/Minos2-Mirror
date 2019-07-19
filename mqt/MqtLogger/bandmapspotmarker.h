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


#ifndef BANDMAPSPOTMARKER_H
#define BANDMAPSPOTMARKER_H

#include <QObject>
#include <QGraphicsTextItem>
#include <QGraphicsScene>

class BandmapSpotMarker : public QGraphicsTextItem
{

    Q_OBJECT
public:
    BandmapSpotMarker(const QPoint &_position, const QString text, const QString _auxText, const QColor _colour, QGraphicsScene* scene);


    void setSpotText(QString text);



protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

private:

    QPoint position;
    QString displayText;
    QString auxText;
    QColor textColour;

};

#endif // BANDMAPSPOTMARKER_H
