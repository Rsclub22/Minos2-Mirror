#include "bandmapmarkerdetails.h"



BandmapMarkerDetails::BandmapMarkerDetails(QPoint _spotMarkerCoord)
{
    spotMarkerCoord = _spotMarkerCoord;
    spot = nullptr;
    markerline = nullptr;
    spotRect = QRect();
    spotIndex = QModelIndex();
    modelRow = -1;

}

void BandmapMarkerDetails::setSpotMarkerCoord(QPoint coord)
{
    spotMarkerCoord = coord;
}

QPoint BandmapMarkerDetails::getSpotMarkerCoord() const
{
    return spotMarkerCoord;
}

void BandmapMarkerDetails::setSpotMarkerPtr(BandmapSpotMarker* _spot)
{
    spot = _spot;
}

BandmapSpotMarker* BandmapMarkerDetails::getSpotMarkerPtr() const
{
    return spot;
}

void BandmapMarkerDetails::setMarkerLinePtr(QGraphicsLineItem* line)
{
    markerline = line;
}

QGraphicsLineItem* BandmapMarkerDetails::getMarkerLinePtr() const
{
    return markerline;
}

void BandmapMarkerDetails::setSpotRect(QRectF rect)
{
    spotRect = rect;
}

QRectF BandmapMarkerDetails::getSpotRect() const
{
    return  spotRect;
}

void BandmapMarkerDetails::setSpotModelIndex(QModelIndex index)
{
    spotIndex = index;
}

QModelIndex BandmapMarkerDetails::getModelIndex() const
{
    return spotIndex;
}

void BandmapMarkerDetails::setModelRowNum(int row)
{
    modelRow = row;
}

int BandmapMarkerDetails::getModelRowNum() const
{
    return modelRow;
}


