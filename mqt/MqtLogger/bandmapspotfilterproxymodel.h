#ifndef BANDMAPSPOTFILTERPROXYMODEL_H
#define BANDMAPSPOTFILTERPROXYMODEL_H

#include "bandmapclientfilterdialog.h"
#include <QSortFilterProxyModel>

class BandmapSpotFilterProxyModel : public QSortFilterProxyModel
{
public:
    BandmapSpotFilterProxyModel(BandmapClientFilterDialog* _filterSetup);


    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool matchBand(int sourceRow) const;


    BandmapClientFilterDialog* filterSetup;
};

#endif // BANDMAPSPOTFILTERPROXYMODEL_H
