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

#include "bandmapdatamodel.h"
#include "ContestApp.h"

// Note this is being used by a custom view with no horizontal headers.

BandmapData::BandmapData()
{

}

int BandmapData::rowCount() const
{
    return bandmapSpots.count();
}

// NOTE! This needs modification of the for loop and rowData to trully support multiple rows!!
void BandmapData::sortBandmapData()
{
    bool invertBandmap = false;
    TContestApp::getContestApp()->loggerBundle.getBoolProfile(elpBandmapInvert, invertBandmap);

    std::sort(bandmapSpots.begin(), bandmapSpots.end(),
              [=](const QSharedPointer<ClusterSpotData> a, const QSharedPointer<ClusterSpotData> b)->bool
                {
                    if (a->getFreq() == b->getFreq())
                    {
                        return a->getDxCall() < b->getDxCall();
                    }
                    if (invertBandmap)
                    {
                        return b->getFreq() < a->getFreq();
                    }
                    return a->getFreq() < b->getFreq();
                }
    );

}

bool BandmapData::insertRows(int row, int count)
{
        if (count > 0)
        {
            for (int i = 0; i < count; i++)
            {
                bandmapSpots.insert(row, rowData[i]);
            }
            rowData.clear();

            sortBandmapData();
        }
    return true;
}

bool BandmapData::removeRows(int _row, int count)
{
    for (int row = _row + count - 1; row > (_row - 1); row--)
    {
        QSharedPointer<ClusterSpotData> spotData = bandmapSpots[row];
        bandmapSpots.removeAt(row);
    }
    return true;
}

QSharedPointer<ClusterSpotData> BandmapData::getBandmapDataRow(int row)
{
    return bandmapSpots[row];
}

void BandmapData::setFilterString(QString f)
{
    filterString = f;
}

QString BandmapData::getFilterString()
{
    return filterString;
}
