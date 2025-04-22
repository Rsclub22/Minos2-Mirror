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

BandmapDataModel::BandmapDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

int BandmapDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return bandmapData.count();
}

int BandmapDataModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 0;
}


QVariant BandmapDataModel::data(const QModelIndex &/*index*/, int /*role*/) const
{
    return QVariant();
}

// NOTE! This needs modification of the for loop and rowData to trully support multiple rows!!
void BandmapDataModel::sortBandmapModel()
{
    bool invertBandmap = false;
    TContestApp::getContestApp()->loggerBundle.getBoolProfile(elpBandmapInvert, invertBandmap);

    std::sort(bandmapData.begin(), bandmapData.end(),
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

bool BandmapDataModel::insertRows(int row, int count, const QModelIndex &index)
{
    Q_UNUSED(index)

        if (count > 0)
        {

            beginInsertRows(QModelIndex(), row, row + count - 1);
            for (int i = 0; i < count; i++)
            {
                bandmapData.insert(row, rowData[i]);
            }
            rowData.clear();

            sortBandmapModel();

            endInsertRows();
        }
    return true;
}
// not used......
bool BandmapDataModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
    return true;
}


bool BandmapDataModel::removeRows(int _row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, _row, _row + count - 1);

    for (int row = _row + count - 1; row > (_row - 1); row--)
    {
        QSharedPointer<ClusterSpotData> spotData = bandmapData[row];
        bandmapData.removeAt(row);
    }
    endRemoveRows();
    return true;
}

// not used.....
bool BandmapDataModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return true;
}

QSharedPointer<ClusterSpotData> BandmapDataModel::getBandmapDataRow(int row)
{
    return bandmapData[row];
}

void BandmapDataModel::sortModel()
{
    beginResetModel();

    sortBandmapModel();

    endResetModel();
}

BandmapSortFilterProxyModel::BandmapSortFilterProxyModel(QObject *parent):
    QSortFilterProxyModel(parent)
{}

bool BandmapSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    if (filterString.isEmpty())
        return true;

    BandmapDataModel *cgm = dynamic_cast<BandmapDataModel *>(sourceModel());

    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    QSharedPointer<ClusterSpotData> spotData = cgm->getBandmapDataRow(sourceRow);


    QString call = spotData->getDxCall().getFullCall();
    QString loc = spotData->getDxLocator();

    if (call.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;
    if (loc.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

    return false;
}

void BandmapSortFilterProxyModel::setFilterString(QString f)
{
    filterString = f;
    invalidateFilter();
}

QString BandmapSortFilterProxyModel::getFilterString()
{
    return filterString;
}

bool BandmapSortFilterProxyModel::lessThan(const QModelIndex &left,
                      const QModelIndex &right) const
{
    // we don't really need this - we don't use the proxy for sorting

    //Model Indices are to the SOURCE model

    BandmapDataModel *cgm = dynamic_cast<BandmapDataModel *>(sourceModel());

    int lrow = left.row();
    int rrow = right.row();

    QSharedPointer<ClusterSpotData> spotData1 = cgm->getBandmapDataRow(lrow);
    QSharedPointer<ClusterSpotData> spotData2 = cgm->getBandmapDataRow(rrow);

    Frequency ws1 = spotData1->getFreq();
    Frequency ws2 = spotData2->getFreq();

    if (ws1 == ws2)
    {
        QString ss1 = spotData1->getDxCallStr();
        QString ss2 = spotData2->getDxCallStr();
        return ss1 < ss2;
    }
    else
    {
        bool invertBandmap = false;
        TContestApp::getContestApp()->loggerBundle.getBoolProfile(elpBandmapInvert, invertBandmap);
        if (invertBandmap)
        {
            return ws2 < ws1;
        }
        return ws1 < ws2;
    }

    return false;
}

QSharedPointer<ClusterSpotData> BandmapSortFilterProxyModel::getBandmapDataRow(int row)
{
    BandmapDataModel *bmm = dynamic_cast<BandmapDataModel *>(sourceModel());
    return bmm->getBandmapDataRow(row);
}
