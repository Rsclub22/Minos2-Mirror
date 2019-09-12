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
#include "rigutils.h"

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

int BandmapDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return bandmapColCount;
}


QVariant BandmapDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() >= bandmapData.size() || index.row() < 0)
    {
             return QVariant();
    }

    int col = index.column();

    BandmapData* bandmapSpot = new BandmapData();

    if (role == Qt::DisplayRole)
    {


        int row = index.row();
        if (row < 0 && row >= bandmapData.size())
        {
            return QVariant();
        }

        bandmapSpot = bandmapData.at(row);

        QString d;
        switch (col)
        {
            case TIME_COL_NUM:
                d = bandmapSpot->spotTime;
                break;
            case FREQ_COL_NUM:
                d = bandmapSpot->dxFreqStr;
                break;
            case DXBANDSTR_COL_NUM:
                d = bandmapSpot->dxBand;
            break;
            case DXSPOT_CALL_COL_NUM:
                if (bandmapSpot->dxCallWorked == BOOL_YES)
                {
                    d = HtmlFontColour(CALLSIGN_WORKED_COLOUR);
                }
                d = d + bandmapSpot->dxCall;
            break;

            case DXSPOT_MODE_COL_NUM:
                d = bandmapSpot->dxMode;
            break;

            case DXLOC_COL_NUM:
                if (bandmapSpot->dxLocatorWorked == BOOL_NO && bandmapSpot->dxCallWorked == BOOL_YES)
                {
                    d = HtmlFontColour(NOT_WORKED_COLOUR);
                }
                else if (bandmapSpot->dxLocatorWorked == BOOL_YES)
                {
                    d = HtmlFontColour(LOCATOR_WORKED_COLOUR);
                }

                d = d + bandmapSpot->dxLocator;
            break;
            case DXDIST_COL_NUM:
                d = HtmlFontColour(NOT_WORKED_COLOUR);
                d = d + bandmapSpot->dxDist;
            break;
            case DXBRG_COL_NUM:
                d = bandmapSpot->dxBrg;
            break;
            case SPOT_CALL_COL_NUM:
                d = bandmapSpot->spotterCall;
            break;
            case SPOTLOC_COL_NUM:
                d = bandmapSpot->spotterLocator;
            break;
            case COMMENT_COL_NUM:
                d = escapeXML(bandmapSpot->spotComment);
            break;
            case RXTIME_COL_NUM:
                d = QString::number(bandmapSpot->rxTime);
            break;
            case SPOT_TYPE_COL_NUM:
                d = bandmapSpot->spotType;
            break;
            case ROT_BEARING_COL_NUM:
                d = bandmapSpot->rotBrg + QChar('R');
            break;
            case ROT_CONNECTED_COL_NUM:
                d = bandmapSpot->rotConnected;
            break;
            default:
                d = "";
        }
        return d;
    }

    if (role == BMP_DataStoredRole)
    {

        bandmapSpot = bandmapData.at(index.row());

        QVariant d;
        switch (col)
        {
            case TIME_COL_NUM:
                d = bandmapSpot->spotTime;
            break;
            case FREQ_COL_NUM:
                d = bandmapSpot->dxFreqStr;
                break;
            case DXBANDSTR_COL_NUM:
                d = bandmapSpot->dxBand;
            break;
            case DXSPOT_CALL_COL_NUM:
                d = bandmapSpot->dxCall;
            break;
            case DXLOC_COL_NUM:
                d = bandmapSpot->dxLocator;
            break;
            case DXDIST_COL_NUM:
                d = bandmapSpot->dxDist;
            break;
            case DXBRG_COL_NUM:
                d = bandmapSpot->dxBrg;
            break;
            case DXSPOT_MODE_COL_NUM:
                d = bandmapSpot->dxMode;
            break;
            case SPOT_CALL_COL_NUM:
                d = bandmapSpot->spotterCall;
            break;
            case SPOTLOC_COL_NUM:
                d = bandmapSpot->spotterLocator;
            break;
            case COMMENT_COL_NUM:
                d = bandmapSpot->spotComment;
            break;
            case DXSPOT_CALL_WORKED_COL_NUM:
                d = bandmapSpot->dxCallWorked;
            break;
            case DXLOC_WORKED_COL_NUM:
                d = bandmapSpot->dxLocatorWorked;
            break;
            case DXSPOT_TO_MEMORY_FLAG_COL_NUM:
                d = bandmapSpot->sentToMemory;
            break;
            case DXBANDMASK_COL_NUM:
                d = bandmapSpot->dxBandMaskStr;
            break;
            case DXMODEMASK_COL_NUM:
                d = bandmapSpot->dxModeMaskStr;
            break;
            case RXTIME_COL_NUM:
                d = bandmapSpot->rxTime;
            break;
            case SPOT_TYPE_COL_NUM:
                d = bandmapSpot->spotType;
            break;
            case SPOT_IS_SELECTED_COL_NUM:
                d = bandmapSpot->isSelected;
            break;
            case ROT_BEARING_COL_NUM:
                d = bandmapSpot->rotBrg;
            break;
            case ROT_CONNECTED_COL_NUM:
                d = bandmapSpot->rotConnected;
            break;
            default:
            d = "";


        }

        return d;
    }

    return QVariant();
}



bool BandmapDataModel::setData(const QModelIndex & index, const QVariant & value, int role)
{

    int row = index.row();
    int col = index.column();

    if (index.isValid() && role == BMP_DataStoredRole)
    {

        BandmapData* bandmapSpot = bandmapData.value(row);

        switch (col)
        {
            case TIME_COL_NUM :
                bandmapSpot->spotTime = value.toString();
            break;
            case FREQ_COL_NUM:
                bandmapSpot->dxFreqStr = value.toString();
            break;
            case DXBANDSTR_COL_NUM:
                bandmapSpot->dxBand = value.toString();
            break;
            case DXSPOT_CALL_COL_NUM:
                bandmapSpot->dxCall = value.toString();
            break;
            case DXSPOT_CALL_WORKED_COL_NUM:
                bandmapSpot->dxCallWorked = value.toBool();
            break;
            case DXSPOT_MODE_COL_NUM:
                bandmapSpot->dxMode =value.toString();
            break;
            case DXLOC_COL_NUM:
                bandmapSpot->dxLocator = value.toString();
            break;
            case DXDIST_COL_NUM:
                bandmapSpot->dxDist = value.toString();
            break;
            case DXBRG_COL_NUM:
                bandmapSpot->dxBrg = value.toString();
            break;
            case DXLOC_WORKED_COL_NUM:
                bandmapSpot->dxLocatorWorked = value.toBool();
            break;
            case SPOT_CALL_COL_NUM:
                bandmapSpot->spotterCall = value.toString();
                break;
            case SPOTLOC_COL_NUM:
                bandmapSpot->spotterLocator = value.toString();
                break;
            case COMMENT_COL_NUM:
                bandmapSpot->spotComment = value.toString();
                break;
            case DXBANDMASK_COL_NUM:
                bandmapSpot->dxModeMaskStr = value.toString();
                break;
            case DXMODEMASK_COL_NUM:
                bandmapSpot->dxModeMaskStr = value.toString();
                break;
            case DXSPOT_TO_MEMORY_FLAG_COL_NUM:
                bandmapSpot->sentToMemory = value.toBool();
                break;
            case SPOT_TYPE_COL_NUM:
                bandmapSpot->spotType = static_cast<bandmapSpotType::SPOT_TYPE>(value.toInt());
            break;
            case SPOT_IS_SELECTED_COL_NUM:
                bandmapSpot->isSelected = value.toBool();
            break;
            case ROT_BEARING_COL_NUM:
                bandmapSpot->rotBrg = value.toString();
            break;
            case ROT_CONNECTED_COL_NUM:
                bandmapSpot->rotConnected = value.toBool();
            break;

            default:
                return false;

        }


        bandmapData.replace(row, bandmapSpot);
        emit dataChanged(index, index);


            return true;
    }


        return false;
}



// NOTE! This needs modification of the for loop and rowData to trully support multiple rows!!
bool BandmapDataModel::insertRows(int row, int count, const QModelIndex &index)
{
    Q_UNUSED(index);

    beginInsertRows(QModelIndex(), row , row + count - 1);
    for (int i = 0; i < count; i++)
    {
        bandmapData.insert(row , rowData);
    }
    endInsertRows();
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
/*
    for (int row =_row; row < _row + count; ++row)
    {
        dxSpotData.removeAt(row);
    }
*/

    for (int row = _row + count - 1; row > (_row - 1); row--)
    {
        BandmapData* spotData = bandmapData[row];
        bandmapData.removeAt(row);
        if (spotData != nullptr)
        {
            delete(spotData);
        }
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
