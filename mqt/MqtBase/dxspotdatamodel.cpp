/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                       DXSpot Data Model
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////


#include "dxspotdatamodel.h"

DxSpotDataModel::DxSpotDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{

    dxSpotData = QList<QStringList>(); // create an empty list
}

QVariant DxSpotDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

         if (orientation == Qt::Horizontal) {
             switch (section) {
                case 0:
                    return tr("Time");
                case 1:
                    return tr("Freq");
                case 2:
                    return tr("Dx");
                case 3:
                    return tr("DxLoc");
                case 4:
                    return tr("Spotter");
                case 5:
                    return tr("SpotLoc");
                case 6:
                    return tr("Comment");

                default:
                  return QVariant();
             }
         }
         return QVariant();
}

int DxSpotDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return dxSpotData.count();
}

int DxSpotDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return dxSpotColCount;
}




QVariant DxSpotDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() >= dxSpotData.size() || index.row() < 0)
    {
             return QVariant();
    }
    if (role == Qt::DisplayRole)
    {
        //int r = index.row();
        //int c = index.column();
        QStringList dxSpot = dxSpotData.at(index.row());

         if (dxSpot.size() > 0)
             return dxSpot[index.column()];
    }

    return QVariant();
}

bool DxSpotDataModel::insertRows(int row, int count, const QModelIndex &index)
{
    Q_UNUSED(index);
    int _row = row;
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; i++)
    {
        dxSpotData.insert(_row, rowData);
    }
    endInsertRows();
    return true;
}
// not used......
bool DxSpotDataModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
    return true;
}



bool DxSpotDataModel::removeRows(int _row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, _row, _row + count - 1);
    for (int row=_row; row < _row + count; ++row)
    {
        dxSpotData.removeAt(row);
    }
    endRemoveRows();
    return true;
}

// not used.....
bool DxSpotDataModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return true;
}



