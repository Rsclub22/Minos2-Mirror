#ifndef BANDMAPMARKERDETAILS_H
#define BANDMAPMARKERDETAILS_H


class BandmapSpotMarker;
class QGraphicsLineItem;

class BandmapMarkerDetails
{
public:

    BandmapMarkerDetails();

    BandmapMarkerDetails(QPoint _spotMarkerCoord);

    void setSpotMarkerCoord(QPoint coord);
    QPoint getSpotMarkerCoord() const;

    void setSpotMarkerPtr(BandmapSpotMarker *_spot);
    BandmapSpotMarker *getSpotMarkerPtr() const;

    void setMarkerLinePtr(QGraphicsLineItem *line);
    QGraphicsLineItem *getMarkerLinePtr() const;

    void setSpotRect(QRectF rect);
    QRectF getSpotRect() const;

    void setSpotModelIndex(QModelIndex index);
    QModelIndex getModelIndex() const;

    void setModelRowNum(int row);
    int getModelRowNum() const;



private:

    QPoint spotMarkerCoord;     // text coords
    BandmapSpotMarker* spot;
    QGraphicsLineItem* markerline;
    QRectF spotRect;
    QModelIndex spotIndex;
    int modelRow;




};

#endif // BANDMAPMARKERDETAILS_H
