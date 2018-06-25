#include "dxspotdatamodel.h"

dxSpotDataModel::dxSpotDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant dxSpotDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

         if (orientation == Qt::Horizontal) {
             switch (section) {
                 case 0:
                     return tr("Name");

                 case 1:
                     return tr("Address");

                 default:
                     return QVariant();
             }
         }
         return QVariant();
}

int dxSpotDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return dxSpotData->size();
}

int dxSpotDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return dxSpotColCount;
}

QVariant dxSpotDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() >= dxSpotData->size() || index.row() < 0)
    {
             return QVariant();
    }
    if (role == Qt::DisplayRole)
    {
         QStringList dxSpot = dxSpotData->at(index.row());

         if (dxSpot.size() > 0)
             return dxSpot[index.column()];
    }

    return QVariant();
}

bool dxSpotDataModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endInsertRows();
}

bool dxSpotDataModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
}

bool dxSpotDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    // FIXME: Implement me!
    endRemoveRows();
}

bool dxSpotDataModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
}
