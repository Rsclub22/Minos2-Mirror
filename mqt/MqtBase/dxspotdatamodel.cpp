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
                case FREQ_COL_NUM:
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
                case SPOTTER_CALL_COL_NUM:
                    return tr("Spotter");
                case SPOTTER_LOC_COL_NUM:
                    return tr("Loc");
                case COMMENT_COL_NUM:
                    return tr("Comment");
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
            case TIME_COL_NUM:
                d = dxSpot->getSpotTime();
            break;
            case DATE_COL_NUM:
                d = dxSpot->getSpotDate();
            break;
            case FREQ_COL_NUM:
                d = removeHundredHzAndHzDigits(dxSpot->getFreq().convertFreqStrDisp());
            break;
            case DXSPOT_CALL_COL_NUM:
                if (dxSpot->getDxCallWorked() == BOOL_YES)
                {
                    QColor colour = CALLSIGN_WORKED_COLOUR;
                    d = HtmlFontColour(colour);
                }

                d = d + dxSpot->getDxCallStr();
            break;
            case DXLOC_COL_NUM:
                if (dxSpot->getDxLocatorWorked() == BOOL_YES)
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
            case RXTIME_COL_NUM:
                d = QString::number(dxSpot->getRxTime());
            break;
            case DXSPOT_MODE_COL_NUM:
                d = dxSpot->getMode();
            break;
            case DXSPOT_PROP_MODE_COL_NUM:
                d = dxSpot->getDxPropMode();
            break;
            case DXBANDSTR_COL_NUM:
                d = dxSpot->getBand();
            break;
            case DXLOC_FROM_NODE_FLAG_COL_NUM:
                d = dxSpot->getDxLocatorIsFromNode();
            break;
            default:
                d = "";
        }
        return d;
    }

    if (role == DataStoredRole)
    {

        QSharedPointer<ClusterSpotData> dxSpot = dxSpotData.at(index.row());

        QVariant d;
        switch (col)
        {
            case TIME_COL_NUM:
                d = dxSpot->getSpotTime();
            break;
            case DATE_COL_NUM:
                d = dxSpot->getSpotDate();
            break;
            case FREQ_COL_NUM:
                d.setValue(dxSpot->getFreq());
            break;
            case DXSPOT_CALL_COL_NUM:
                d = dxSpot->getDxCallStr();
            break;
            case DXLOC_COL_NUM:
                d = dxSpot->getDxLocator();
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
                d = dxSpot->getSpotComment();
            break;
            case DXSPOT_CALL_WORKED_COL_NUM:
                d = dxSpot->getDxCallWorked();
            break;
            case DXLOC_WORKED_COL_NUM:
                d = dxSpot->getDxLocatorWorked();
            break;
            case DXSPOT_TO_MEMORY_FLAG_COL_NUM:
                d = dxSpot->getSentToMemory();
            break;
            case RXTIME_COL_NUM:
                d = dxSpot->getRxTime();
            break;
            case DXSPOT_MODE_COL_NUM:
                d = dxSpot->getMode();
            break;
            case DXSPOT_PROP_MODE_COL_NUM:
                d = dxSpot->getDxPropMode();
            break;
            case DXBANDSTR_COL_NUM:
                d = dxSpot->getBand();
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

        QSharedPointer<ClusterSpotData> dxSpot = dxSpotData.value(row);

        switch (col)
        {
            case TIME_COL_NUM :
                dxSpot->setSpotTime(value.toString());
            break;
            case DATE_COL_NUM:
                dxSpot->setSpotDate(value.toString());
            break;
            case FREQ_COL_NUM:
                dxSpot->setFreq(qvariant_cast<Frequency>(value));
            break;
            case DXSPOT_CALL_COL_NUM:
                dxSpot->setDxCall(value.toString());
            break;
            case DXSPOT_CALL_WORKED_COL_NUM:
                dxSpot->setDxCallWorked(value.toBool());
            break;
            case DXLOC_COL_NUM:
                dxSpot->setDxLocator(value.toString());
            break;
            case DXDIST_COL_NUM:
                dxSpot->setDxDist(value.toString());
            break;
            case DXBRG_COL_NUM:
                dxSpot->setDxBrg(value.toString());
            break;
            case DXLOC_WORKED_COL_NUM:
                dxSpot->setDxLocatorWorked(value.toBool());
            break;
            case SPOTTER_CALL_COL_NUM:
                dxSpot->setSpotterCall(value.toString());
                break;
            case SPOTTER_LOC_COL_NUM:
                dxSpot->setSpotterLocator(value.toString());
                break;
            case COMMENT_COL_NUM:
                dxSpot->setSpotComment(value.toString());
                break;
            case DXSPOT_TO_MEMORY_FLAG_COL_NUM:
                dxSpot->setSentToMemory(value.toBool());
            break;
            case DXSPOT_MODE_COL_NUM:
                dxSpot->setMode(value.toString());
            break;
            case DXSPOT_PROP_MODE_COL_NUM:
                dxSpot->setDxPropMode(value.toString());
            break;
            case DXBANDSTR_COL_NUM:
                dxSpot->setBand(value.toString());
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



