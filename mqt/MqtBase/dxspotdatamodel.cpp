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

#include "clustercommon.h"
#include "cutils.h"
#include "htmldelegate.h"
#include "rigutils.h"
#include "dxspotdatamodel.h"

DxSpotDataModel::DxSpotDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{


}

DxSpotDataModel::~DxSpotDataModel()
{

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
                case FREQ_COL_NUM:
                    return tr("Freq");
                case DXSPOT_CALL_COL_NUM:
                    return tr("Dx");
                case DXSPOT_MODE_COL_NUM:
                    return tr("Mode");
                case DXLOC_COL_NUM:
                    return tr("Loc");
                case DXDIST_COL_NUM:
                    return tr("Dist");
                case DXBRG_COL_NUM:
                    return tr("Brg");
                case SPOTTER_CALL_COL_NUM:
                    return tr("Spotter");
                case SPOTTER_LOC_COL_NUM:
                    return tr("Loc");
                case COMMENT_COL_NUM:
                    return tr("Comment");
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
           QSharedPointer<ClusterSpotData> dxSpot = dxSpotData.at(section);
           if (dxSpot->getSentToMemory())
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



    if (role == Qt::DisplayRole)
    {


        int row = index.row();
        if (row < 0 && row >= dxSpotData.size())
        {
            return QVariant();
        }

        QSharedPointer<ClusterSpotData> dxSpot = dxSpotData.at(row);

        QString d;
        switch (col)
        {
            case RXTIME_COL_NUM:
                d = QString::number(dxSpot->getRxTime());
                break;
            case TIME_COL_NUM:
                d = dxSpot->getSpotTime();
            break;
            case FREQ_COL_NUM:
                d = removeHundredHzAndHzDigits(dxSpot->getFreq().convertFreqStrDisp());
            break;
            case DXSPOT_CALL_COL_NUM:
                if (dxSpot->getDxCallWorked())
                {
                    QColor colour = CALLSIGN_WORKED_COLOUR;
                    d = HtmlFontColour(colour);
                }

                d = d + dxSpot->getDxCallStr();
            break;
            case DXSPOT_MODE_COL_NUM:
                d = dxSpot->getMode();
                break;
            case DXLOC_COL_NUM:
                if (dxSpot->getDxLocatorWorked())
                {
                    QColor colour = LOCATOR_WORKED_COLOUR;
                    d = HtmlFontColour(colour);
                }
                if (dxSpot->getDxLocatorIsFromNode())
                {
                    d = d + "<i>" + dxSpot->getDxLocator() + "</i>";
                }
                else
                {
                    d = d + dxSpot->getDxLocator();
                }

            break;
            case DXDIST_COL_NUM:
                d = dxSpot->getDxDist();
            break;

            case DXBRG_COL_NUM:
                d = dxSpot->getDxBrg();
            break;
            case SPOTTER_CALL_COL_NUM:
                d = dxSpot->getSpotterCallStr();
            break;
            case SPOTTER_LOC_COL_NUM:
                d = dxSpot->getSpotterLocator();
            break;
            case COMMENT_COL_NUM:
                d = escapeXML(dxSpot->getSpotComment());
            break;
            default:
                d = "";
        }
        return d;
    }

    return QVariant();
}

// NOTE! This needs modification of the for loop and rowData to truly support multiple rows!!
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
        QSharedPointer<ClusterSpotData> s = dxSpotData.takeAt(row);
        //delete s;
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



