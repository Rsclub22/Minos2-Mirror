/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                       Bandmap Data Model
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BANDMAPDATAMODEL_H
#define BANDMAPDATAMODEL_H

#include "spotbasedata.h"

class BandmapData
{
    QString filterString;
public:
    explicit BandmapData();

    // Basic functionality:
    int rowCount() const ;

    // Add data:
    bool insertRows(int row, int count) ;

    // Remove data:
    bool removeRows(int row, int count) ;

    QSharedPointer<ClusterSpotData> getBandmapDataRow(int row);

    QVector< QSharedPointer<ClusterSpotData> > rowData;

    QSharedPointer<ClusterSpotData> getSpotData(int row){return bandmapSpots[row];};

    void setFilterString(QString f);
    QString getFilterString();

    void sortBandmapData();

private:

    QVector< QSharedPointer<ClusterSpotData>>  bandmapSpots;
};
#endif // BANDMAPDATAMODEL_H
