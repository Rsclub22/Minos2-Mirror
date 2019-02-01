#ifndef BANDMAP_H
#define BANDMAP_H

#include <QWidget>
//#include <QPainter>
//#include <QGraphicsItem>
//#include <QList>
#include <QMultiMap>
#include "bandmapcallsignmarker.h"


namespace mapData {



const int MAX_CALLSIGNS = 3;


}



class Bandmap
{
//    Q_OBJECT
public:
    explicit Bandmap();
    void addCallsignMarker(double frequency, QString callsign, QString time, int spotType, int callBearing);
    void addCallsignToDial(QPainter *painter);

signals:



private:

    QMap<double, QVector<BandmapCallsignMarker>> markers;

//    QMultiMap<double, QString> mapMarkers;


};

#endif // BANDMAP_H
