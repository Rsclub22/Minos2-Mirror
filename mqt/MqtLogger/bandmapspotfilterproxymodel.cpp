#include "bandmapspotfilterproxymodel.h"

BandmapSpotFilterProxyModel::BandmapSpotFilterProxyModel(BandmapClientFilterDialog* _filterSetup)
{


    filterSetup = _filterSetup;


}


bool BandmapSpotFilterProxyModel::BandmapSpotFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    return matchBand(sourceRow);

}

bool BandmapSpotFilterProxyModel::BandmapSpotFilterProxyModel::matchBand(int sourceRow) const
{
    bool ok = false;
//    int bandMask = sourceModel()->data(sourceModel()->index(sourceRow, DXBANDMASK_COL_NUM), DataStoredRole).toString().toInt(&ok);

//    if (ok && (bandMask >=0 && bandMask < NUMBANDS) )
//    {
//       return filterSetup->filterSettings.getBandFilter(bandMask);
//    }
//    else if (!ok && filterSetup->getEnableHFSpotsFlag())
//    {
        return true;        // if DXBandMask is empty it must be a HF Spot
//    }
//    else
//    {
//        return false;
//    }

}
