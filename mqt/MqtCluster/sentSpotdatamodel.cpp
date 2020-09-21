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
#include "sentSpotdatamodel.h"


SentSpotDataModel::SentSpotDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{


}

SentSpotDataModel::~SentSpotDataModel()
{
    foreach(auto s, sentSpotData)
    {
        delete s;
    }
    sentSpotData.clear();
}

QVariant SentSpotDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            switch (section) {
                case SENT_SPOT_TIME_COL_NUM:
                    return tr("UTC");
                case SENT_SPOT_FREQ_STR_COL_NUM:
                    return tr("Freq");
                case SENT_SPOT_DXSPOT_CALL_COL_NUM:
                    return tr("Dx");
                case SENT_SPOT_DXLOC_COL_NUM:
                    return tr("Loc");
                case SENT_SPOT_COMMENT_COL_NUM:
                    return tr("Comment");
                case SENT_SPOT_STATUS_COL_NUM:
                    return tr("Status");
                case SENT_SPOT_REASON_COL_NUM:
                    return tr("Reason");
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


    }
    return QVariant();
}

int SentSpotDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return sentSpotData.count();
}

int SentSpotDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return sentSpotColCount;
}




QVariant SentSpotDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() >= sentSpotData.size() || index.row() < 0)
    {
             return QVariant();
    }

    int col = index.column();



    if (role == Qt::DisplayRole)
    {


        int row = index.row();
        if (row < 0 && row >= sentSpotData.size())
        {
            return QVariant();
        }

        SentSpotData* sentSpot = sentSpotData.at(row);

        QString d;
        switch (col)
        {
            case SENT_SPOT_TIME_COL_NUM:
                d = sentSpot->spotTime;
            break;
            case SENT_SPOT_FREQ_STR_COL_NUM:
                d = sentSpot->dxFreq.str();
            break;
            case SENT_SPOT_DXSPOT_CALL_COL_NUM:
                d = sentSpot->dxCall;
            break;
            case SENT_SPOT_DXLOC_COL_NUM:
                d = sentSpot->dxLocator;
            break;
            case SENT_SPOT_COMMENT_COL_NUM:
                d = escapeXML(sentSpot->spotComment);
            break;
            case SENT_SPOT_RXTIME_COL_NUM:
                d = QString::number(sentSpot->rxTime);
            break;
            case SENT_SPOT_STATUS_COL_NUM:
                if (sentSpot->sentOk)
                {
                    d = tr("OK");
                }
                else
                {
                    d = tr("Failed");
                }

            break;
            case SENT_SPOT_REASON_COL_NUM:
                d = sentSpot->reason;
            break;

            default:
                d = "";
        }
        return d;
    }

    if (role == SentDataStoredRole)
    {

        SentSpotData* sentSpot = sentSpotData.at(index.row());

        QVariant d;
        switch (col)
        {
            case SENT_SPOT_TIME_COL_NUM:
                d = sentSpot->spotTime;
            break;
            case SENT_SPOT_FREQ_STR_COL_NUM:
                d = sentSpot->dxFreq.str();
            break;
            case SENT_SPOT_DXSPOT_CALL_COL_NUM:
                d = sentSpot->dxCall;
            break;
            case SENT_SPOT_DXLOC_COL_NUM:
                d = sentSpot->dxLocator;
            break;
            case SENT_SPOT_COMMENT_COL_NUM:
                d = sentSpot->spotComment;
            break;
            case SENT_SPOT_STATUS_COL_NUM:
                d = sentSpot->sentOk ;
            break;
            case SENT_SPOT_RXTIME_COL_NUM:
                d = sentSpot->rxTime;
            break;
            case SENT_SPOT_REASON_COL_NUM:
                d = sentSpot->reason;
            break;


        }

        return d;
    }

    return QVariant();
}


bool SentSpotDataModel::setData(const QModelIndex & index, const QVariant & value, int role)
{

    int row = index.row();
    int col = index.column();

    if (index.isValid() && role == SentDataStoredRole)
    {

        SentSpotData* sentSpot = sentSpotData.value(row);

        switch (col)
        {
            case SENT_SPOT_TIME_COL_NUM :
                sentSpot->spotTime = value.toString();
            break;
            case SENT_SPOT_FREQ_STR_COL_NUM:
                sentSpot->dxFreq = Frequency(value.toString());
            break;
            case SENT_SPOT_DXSPOT_CALL_COL_NUM:
                sentSpot->dxCall = value.toString();
            break;
            case SENT_SPOT_DXLOC_COL_NUM:
                sentSpot->dxLocator = value.toString();
            break;
            case SENT_SPOT_COMMENT_COL_NUM:
                sentSpot->spotComment = value.toString();
                break;
            case SENT_SPOT_STATUS_COL_NUM:
                sentSpot->sentOk = value.toBool();
            break;
            case SENT_SPOT_REASON_COL_NUM:
                sentSpot->reason = value.toBool();
            break;


            default:
                return false;

        }


        // sentSpotData.replace(row, sentSpot); // not needed
        emit dataChanged(index, index);


            return true;
    }


        return false;
}



// NOTE! This needs modification of the for loop and rowData to trully support multiple rows!!
bool SentSpotDataModel::insertRows(int row, int count, const QModelIndex &index)
{
    Q_UNUSED(index)

    beginInsertRows(QModelIndex(), row , row + count - 1);
    for (int i = 0; i < count; i++)
    {
        sentSpotData.insert(row , rowData);
    }
    endInsertRows();
    return true;
}
// not used......
bool SentSpotDataModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endInsertColumns();
    return true;
}



bool SentSpotDataModel::removeRows(int _row, int count, const QModelIndex &parent)
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
        SentSpotData *s = sentSpotData.takeAt(row);
        delete s;
    }
    endRemoveRows();
    return true;
}

// not used.....
bool SentSpotDataModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    // FIXME: Implement me!
    endRemoveColumns();
    return true;
}



