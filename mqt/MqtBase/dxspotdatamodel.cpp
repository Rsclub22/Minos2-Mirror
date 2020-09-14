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

#include <QDebug>
#include "cutils.h"
#include "rigutils.h"
#include "htmldelegate.h"
#include "dxspotdatamodel.h"


DxSpotDataModel::DxSpotDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{


}

DxSpotDataModel::~DxSpotDataModel()
{
    foreach(auto s, dxSpotData)
    {
        delete s;
    }
    dxSpotData.clear();
}

QVariant DxSpotDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            switch (section) {
                case TIME_COL_NUM:
                    return tr("UTC");
                case DATE_COL_NUM:
                    return tr("Date");
                case FREQ_STR_COL_NUM:
                    return tr("Freq");
                case DXSPOT_CALL_COL_NUM:
                    return tr("Dx");
                case DXSPOT_CALL_WORKED_COL_NUM:
                    return tr("Wkd");
                case DXSPOT_MODE_COL_NUM:
                    return tr("Mode");
                case DXLOC_COL_NUM:
                    return tr("Loc");
                case DXDIST_COL_NUM:
                    return tr("Dist");
                case DXBRG_COL_NUM:
                    return tr("Brg");
                case DXLOC_WORKED_COL_NUM:
                    return tr("Wkd");
                case SPOT_CALL_COL_NUM:
                    return tr("Spotter");
                case SPOTLOC_COL_NUM:
                    return tr("Loc");
                case COMMENT_COL_NUM:
                return tr("Comment");
                case DXBANDMASK_COL_NUM:
                    return tr("Band Mask");
                case DXMODEMASK_COL_NUM:
                    return tr("mode Mask");
                case DXSPOT_TO_MEMORY_FLAG_COL_NUM:
                    return tr("Spot to Mem Flag");
                case DXSPOT_PROP_MODE_COL_NUM:
                    return tr("Prop Mode");
                default:
                return QVariant();
            }
        }
        else if (role == Qt::TextAlignmentRole)
        {
            return Qt::AlignLeft;
        }

    }
    if (orientation == Qt::Vertical)
    {
       if (role == Qt::DisplayRole)
       {
           return tr("Mem");
       }
       else if (role == Qt::ForegroundRole)
       {
           SpotData* dxSpot = dxSpotData.at(section);
           if (dxSpot->sentToMemory)
           {
               QColor c = SPOT_TO_MEMORY;
               return c;
           }
           else
           {
               QColor c = NO_SPOT_TO_MEMORY;
               return c;
           }
       }
       else if (role == Qt::SizeHintRole)
       {
           if (delegate)
           {
               // BUT the headers aren't drawn using the delegate, so this
               // all fails to work

               // Do we lose the vertical header?
               QString s = "Memxx";
               QSize r = delegate->docSize(s);
               return r;
           }
       }
       else if (role == Qt::ToolTipRole)
       {
           return "Click here to transfer spot to memory";
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

    int col = index.column();

//    SpotData* dxSpot = new SpotData();

    if (role == Qt::DisplayRole)
    {


        int row = index.row();
        if (row < 0 && row >= dxSpotData.size())
        {
            return QVariant();
        }

        SpotData* dxSpot = dxSpotData.at(row);

        QString d;
        switch (col)
        {
            case TIME_COL_NUM:
                d = dxSpot->spotTime;
            break;
            case DATE_COL_NUM:
                d = dxSpot->dxBandStr;
            break;
            case FREQ_STR_COL_NUM:
                d = removeHundredHzAndHzDigits(dxSpot->dxFreq);
            break;
            case DXSPOT_CALL_COL_NUM:
                if (dxSpot->dxCallWorked == BOOL_YES)
                {
                    QColor colour = CALLSIGN_WORKED_COLOUR;
                    d = HtmlFontColour(colour);
                }
                d = d + dxSpot->dxCall;
            break;

            case DXLOC_COL_NUM:
                if (dxSpot->dxLocatorWorked == BOOL_YES)
                {
                    QColor colour = LOCATOR_WORKED_COLOUR;
                    d = HtmlFontColour(colour);
                }
                d = d + dxSpot->dxLocator;
            break;
            case DXDIST_COL_NUM:
                d = dxSpot->dxDist;
            break;

            case DXBRG_COL_NUM:
                d = dxSpot->dxBrg;
            break;
            case SPOT_CALL_COL_NUM:
                d = dxSpot->spotterCall;
            break;
            case SPOTLOC_COL_NUM:
                d = dxSpot->spotterLocator;
            break;
            case COMMENT_COL_NUM:
                d = escapeXML(dxSpot->spotComment);
            break;
            case RXTIME_COL_NUM:
                d = QString::number(dxSpot->rxTime);
            break;
            case DXSPOT_MODE_COL_NUM:
                d = dxSpot->dxMode;
            break;
            case DXSPOT_PROP_MODE_COL_NUM:
                d = dxSpot->dxPropMode;
            break;
            case DXBANDSTR_COL_NUM:
                d = dxSpot->dxBandStr;
            break;
            default:
                d = "";
        }
        return d;
    }

    if (role == DataStoredRole)
    {

        SpotData* dxSpot = dxSpotData.at(index.row());

        QVariant d;
        switch (col)
        {
            case TIME_COL_NUM:
                d = dxSpot->spotTime;
            break;
            case FREQ_STR_COL_NUM:
                d = dxSpot->dxFreq;
            break;
            case DXSPOT_CALL_COL_NUM:
                d = dxSpot->dxCall;
            break;
            case DXLOC_COL_NUM:
                d = dxSpot->dxLocator;
            break;
            case DXDIST_COL_NUM:
                d = dxSpot->dxDist;
            break;
            case DXBRG_COL_NUM:
                d = dxSpot->dxBrg;
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
            case DXSPOT_CALL_WORKED_COL_NUM:
                d = dxSpot->dxCallWorked;
            break;
            case DXLOC_WORKED_COL_NUM:
                d = dxSpot->dxLocatorWorked;
            break;
            case DXSPOT_TO_MEMORY_FLAG_COL_NUM:
                d = dxSpot->sentToMemory;
            break;
            case DXBANDMASK_COL_NUM:
                d = dxSpot->dxBandMask;
            break;
            case DXMODEMASK_COL_NUM:
                d = dxSpot->dxModeMaskStr;
            break;
            case RXTIME_COL_NUM:
                d = dxSpot->rxTime;
            break;
            case DXSPOT_MODE_COL_NUM:
                d = dxSpot->dxMode;
            break;
            case DXSPOT_PROP_MODE_COL_NUM:
                d = dxSpot->dxPropMode;
            break;
            case DXBANDSTR_COL_NUM:
                d = dxSpot->dxBandStr;
            break;
            case DATE_COL_NUM:
                d = dxSpot->dxBandStr;
            break;


        }

        return d;
    }

    return QVariant();
}


bool DxSpotDataModel::setData(const QModelIndex & index, const QVariant & value, int role)
{

    int row = index.row();
    int col = index.column();

    if (index.isValid() && role == DataStoredRole)
    {

        SpotData* dxSpot = dxSpotData.value(row);

        switch (col)
        {
            case TIME_COL_NUM :
                dxSpot->spotTime = value.toString();
            break;
            case DATE_COL_NUM:
                dxSpot->dxBandStr = value.toString();
            break;
            case FREQ_STR_COL_NUM:
                dxSpot->dxFreq = value.toString();
            break;
            case DXSPOT_CALL_COL_NUM:
                dxSpot->dxCall = value.toString();
            break;
            case DXSPOT_CALL_WORKED_COL_NUM:
                dxSpot->dxCallWorked = value.toBool();
            break;
            case DXLOC_COL_NUM:
                dxSpot->dxLocator = value.toString();
            break;
            case DXDIST_COL_NUM:
                dxSpot->dxDist = value.toString();
            break;
            case DXBRG_COL_NUM:
                dxSpot->dxBrg = value.toString();
            break;
            case DXLOC_WORKED_COL_NUM:
                dxSpot->dxLocatorWorked = value.toBool();
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
            case DXBANDMASK_COL_NUM:
                dxSpot->dxModeMaskStr = value.toString();
            break;
            case DXMODEMASK_COL_NUM:
                dxSpot->dxModeMaskStr = value.toString();
            break;
            case DXSPOT_TO_MEMORY_FLAG_COL_NUM:
                dxSpot->sentToMemory = value.toBool();
            break;
            case DXSPOT_MODE_COL_NUM:
                dxSpot->dxMode  = value.toString();
            break;
            case DXSPOT_PROP_MODE_COL_NUM:
                dxSpot->dxPropMode  = value.toString();
            break;
            case DXBANDSTR_COL_NUM:
                dxSpot->dxBandStr = value.toString();
            break;
            default:
                return false;

        }


        // dxSpotData.replace(row, dxSpot); // not needed
        emit dataChanged(index, index);


            return true;
    }


        return false;
}



// NOTE! This needs modification of the for loop and rowData to trully support multiple rows!!
bool DxSpotDataModel::insertRows(int row, int count, const QModelIndex &index)
{
    Q_UNUSED(index)

    beginInsertRows(QModelIndex(), row , row + count - 1);
    for (int i = 0; i < count; i++)
    {
        dxSpotData.insert(row , rowData);
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

    for (int row = _row + count - 1; row > (_row - 1); row--)
    {
        SpotData *s = dxSpotData.takeAt(row);
        delete s;
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



