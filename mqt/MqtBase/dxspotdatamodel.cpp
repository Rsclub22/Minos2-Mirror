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

    //dxSpotData = QVector<SpotData>(); // create an empty list
}

QVariant DxSpotDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
             return QVariant();

         if (orientation == Qt::Horizontal) {
             switch (section) {
                case TIME_COL_NUM:
                    return tr("Time");
                case FREQ_COL_NUM:
                    return tr("Freq");
                case DXSPOT_CALL_COL_NUM:
                    return tr("Dx");
                case DXSPOT_CALL_WORKED_COL_NUM:
                    return tr("Wkd");
                case DXLOC_COL_NUM:
                    return tr("Loc");
                case DXLOC_WORKED_COL_NUM:
                    return tr("Wkd");
                case SPOT_CALL_COL_NUM:
                    return tr("Spotter");
                case SPOTLOC_COL_NUM:
                    return tr("Loc");
                case COMMENT_COL_NUM:
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

    QVariant d;

    if (role == Qt::DisplayRole)
    {
        SpotData* dxSpot = dxSpotData.at(index.row());

        int col = index.column();

        switch (col)
        {
            case TIME_COL_NUM:
                d = dxSpot->spotTime;
            break;
            case FREQ_COL_NUM:
                d = dxSpot->dxFreq;
            break;
            case DXSPOT_CALL_COL_NUM:
                d = dxSpot->dxCall;
            break;
            case DXSPOT_CALL_WORKED_COL_NUM:
                d = dxSpot->dxCallWorked;
            break;
            case DXLOC_COL_NUM:
                d = dxSpot->dxLocator;
            break;
            case DXLOC_WORKED_COL_NUM:
                d = dxSpot->dxLocatorWorked;
            break;
            case SPOT_CALL_COL_NUM:
                d = dxSpot->spotterCall;
            break;
            case SPOTLOC_COL_NUM:
                d = dxSpot->spotterLocator;
            break;
            case COMMENT_COL_NUM:
                d = dxSpot->spotComment;
            break;
            default:
                d = "";

        }

    }

    return d;
}


bool DxSpotDataModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
            int row = index.row();

            SpotData* dxSpot = dxSpotData.value(row);

            int col = index.column();

            switch (col)
            {
                case TIME_COL_NUM :
                    dxSpot->spotTime = value.toString();
                break;
                case FREQ_COL_NUM:
                    dxSpot->dxFreq = value.toString();
                break;
                case DXSPOT_CALL_COL_NUM:
                    dxSpot->dxCall = value.toString();
                break;
                case DXSPOT_CALL_WORKED_COL_NUM:
                    dxSpot->dxCallWorked = value.toString();
                case DXLOC_COL_NUM:
                    dxSpot->dxLocator = value.toString();
                break;
                case DXLOC_WORKED_COL_NUM:
                    dxSpot->dxLocatorWorked = value.toString();
                break;
                case SPOT_CALL_COL_NUM:
                    dxSpot->spotterCall = value.toString();
                break;
                case SPOTLOC_COL_NUM:
                    dxSpot->spotterLocator = value.toString();
                break;
                case COMMENT_COL_NUM:
                    dxSpot->spotComment = value.toString();
                break;
                default:
                    return false;

            }

            dxSpotData.replace(row, dxSpot);
            emit(dataChanged(index, index));

            return true;
        }

        return false;
}



// NOTE! This needs modification of the for loop and rowData to trully support multiple rows!!
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



