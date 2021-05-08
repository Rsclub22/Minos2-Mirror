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

//    BandmapData* bandmapSpot = new BandmapData();

    if (role == Qt::DisplayRole)
    {


        int row = index.row();
        if (row < 0 && row >= bandmapData.size())
        {
            return QVariant();
        }

        QSharedPointer<BandmapSpotData> bandmapSpot = bandmapData.at(row);

        QString d;
        switch (col)
        {
            case TIME_COL_NUM:
                d = bandmapSpot->getSpotTime();
                break;
            case FREQ_COL_NUM:
                d = bandmapSpot->getFreq().convertFreqStrDisp();
                break;
            case DXBANDSTR_COL_NUM:
                d = bandmapSpot->getBand();
            break;
            case DXSPOT_CALL_COL_NUM:
                if (bandmapSpot->getDxCallWorked() == BMP_BOOL_YES)
                {
                    d = HtmlFontColour(CALLSIGN_WORKED_COLOUR);
                }
                d = d + bandmapSpot->getDxCallStr();
                if (bandmapSpot->getDxCallWorked() == BMP_BOOL_YES)
                {
                    d = d + HtmlFontColour(NOT_WORKED_COLOUR);
                }
            break;

            case DXSPOT_MODE_COL_NUM:
                d = bandmapSpot->getMode();
            break;

            case DXLOC_COL_NUM:
                if (bandmapSpot->getDxLocatorWorked() == BOOL_NO && bandmapSpot->getDxCallWorked() == BMP_BOOL_YES)
                {
                    d = HtmlFontColour(NOT_WORKED_COLOUR);
                }
                else if (bandmapSpot->getDxLocatorWorked() == BMP_BOOL_YES)
                {
                    d = HtmlFontColour(LOCATOR_WORKED_COLOUR);
                }

                d = d + bandmapSpot->getDxLocator() + HtmlFontColour(NOT_WORKED_COLOUR);
            break;
            case DXDIST_COL_NUM:
                //d = HtmlFontColour(NOT_WORKED_COLOUR);
                //d = d + bandmapSpot->dxDist;
                d = bandmapSpot->getDxDist();
            break;
            case DXBRG_COL_NUM:
                d = bandmapSpot->getDxBrg();
            break;
            case SPOTTER_CALL_COL_NUM:
                d = bandmapSpot->getSpotterCallStr();
            break;
            case SPOTTER_LOC_COL_NUM:
                d = bandmapSpot->getSpotterLocator();
            break;
            case COMMENT_COL_NUM:
                d = escapeXML(bandmapSpot->getSpotComment());
            break;
            case RXTIME_COL_NUM:
                d = QString::number(bandmapSpot->getRxTime());
            break;
            case SPOT_TYPE_COL_NUM:
                d = bandmapSpot->getSpotType();
            break;
            case ROT_BEARING_COL_NUM:
                d = bandmapSpot->getRotBrg() + QChar('R');
            break;
            case ROT_CONNECTED_COL_NUM:
                d = bandmapSpot->getRotConnected();
            break;
            case RUN_MODE_ON_COL_NUM:
                d = bandmapSpot->getRunModeOn();
            break;
            case OFF_RUN_FREQ_COL_NUM:
                d = bandmapSpot->getOffRunFreq();
            break;
            case CQ_RESPONSE_COL:
                d = bandmapSpot->getCqResponse();
            break;
            case DXLOC_FROM_NODE_FLAG_COL_NUM:
                d = bandmapSpot->getDxLocatorIsFromNode();
            break;
            case DX_DISTRICT_COL_NUM:
                d = bandmapSpot->getDistrict();
            break;
            case DX_DISTRICT_WORKED_COL_NUM:
                d = bandmapSpot->getDistrictWorked();
            break;
            default:
                d = "";
        }
        return d;
    }

    if (role == BMP_DataStoredRole)
    {

        QSharedPointer<BandmapSpotData> bandmapSpot = bandmapData.at(index.row());

        QVariant d;
        switch (col)
        {
            case TIME_COL_NUM:
                d = bandmapSpot->getSpotTime();
            break;
            case FREQ_COL_NUM:
                d.setValue( bandmapSpot->getFreq());
                break;
            case DXBANDSTR_COL_NUM:
                d = bandmapSpot->getBand();
            break;
            case DXSPOT_CALL_COL_NUM:
                d = bandmapSpot->getDxCallStr();
            break;
            case DXLOC_COL_NUM:
                d = bandmapSpot->getDxLocator();
            break;
            case DXDIST_COL_NUM:
                d = bandmapSpot->getDxDist();
            break;
            case DXBRG_COL_NUM:
                d = bandmapSpot->getDxBrg();
            break;
            case DXSPOT_MODE_COL_NUM:
                d = bandmapSpot->getMode();
            break;
            case SPOTTER_CALL_COL_NUM:
                d = bandmapSpot->getSpotterCallStr();
            break;
            case SPOTTER_LOC_COL_NUM:
                d = bandmapSpot->getSpotterLocator();
            break;
            case COMMENT_COL_NUM:
                d = bandmapSpot->getSpotComment();
            break;
            case DXSPOT_CALL_WORKED_COL_NUM:
                d = bandmapSpot->getDxCallWorked();
            break;
            case DXLOC_WORKED_COL_NUM:
                d = bandmapSpot->getDxLocatorWorked();
            break;
            case DXSPOT_TO_MEMORY_FLAG_COL_NUM:
                d = bandmapSpot->getSentToMemory();
            break;
            case RXTIME_COL_NUM:
                d = bandmapSpot->getRxTime();
            break;
            case SPOT_TYPE_COL_NUM:
                d = bandmapSpot->getSpotType();
            break;
            case SPOT_IS_SELECTED_COL_NUM:
                d = bandmapSpot->getIsSelected();
            break;
            case ROT_BEARING_COL_NUM:
                d = bandmapSpot->getRotBrg();
            break;
            case ROT_CONNECTED_COL_NUM:
                d = bandmapSpot->getRotConnected();
            break;
            case RUN_MODE_ON_COL_NUM:
                d = bandmapSpot->getRunModeOn();
            break;
            case OFF_RUN_FREQ_COL_NUM:
                d = bandmapSpot->getOffRunFreq();
            break;
            case CQ_RESPONSE_COL:
                d = bandmapSpot->getCqResponse();
            break;
            case DX_DISTRICT_COL_NUM:
                d = bandmapSpot->getDistrict();
            break;
            case DX_DISTRICT_WORKED_COL_NUM:
                d = bandmapSpot->getDistrictWorked();
            break;
            case DXCLUSTER_SPOT_TYPE:
                d = bandmapSpot->getClusterSpotType();
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

        QSharedPointer<BandmapSpotData> bandmapSpot = bandmapData.value(row);

        switch (col)
        {
            case TIME_COL_NUM :
                bandmapSpot->setSpotTime(value.toString());
            break;
            case FREQ_COL_NUM:
                bandmapSpot->setFreq(qvariant_cast<Frequency>(value));
            break;
            case DXBANDSTR_COL_NUM:
                bandmapSpot->setBand(value.toString());
            break;
            case DXSPOT_CALL_COL_NUM:
                bandmapSpot->setDxCall(value.toString());
            break;
            case DXSPOT_CALL_WORKED_COL_NUM:
                bandmapSpot->setDxCallWorked(value.toBool());
            break;
            case DXSPOT_MODE_COL_NUM:
                bandmapSpot->setMode(value.toString());
            break;
            case DXLOC_COL_NUM:
                bandmapSpot->setDxLocator(value.toString());
            break;
            case DXDIST_COL_NUM:
                bandmapSpot->setDxDist(value.toString());
            break;
            case DXBRG_COL_NUM:
                bandmapSpot->setDxBrg(value.toString());
            break;
            case DXLOC_WORKED_COL_NUM:
                bandmapSpot->setDxLocatorWorked(value.toBool());
            break;
            case SPOTTER_CALL_COL_NUM:
                bandmapSpot->setSpotterCall(value.toString());
                break;
            case SPOTTER_LOC_COL_NUM:
                bandmapSpot->setSpotterLocator(value.toString());
                break;
            case COMMENT_COL_NUM:
                bandmapSpot->setSpotComment(value.toString());
                break;
            case DXSPOT_TO_MEMORY_FLAG_COL_NUM:
                bandmapSpot->setSentToMemory(value.toBool());
                break;
            case SPOT_TYPE_COL_NUM:
                bandmapSpot->setSpotType(static_cast<bandmapSpotType::SPOT_TYPE>(value.toInt()));
                break;
            case SPOT_IS_SELECTED_COL_NUM:
                bandmapSpot->setIsSelected(value.toBool());
                break;
            case ROT_BEARING_COL_NUM:
                bandmapSpot->setRotBrg(value.toString());
                break;
            case ROT_CONNECTED_COL_NUM:
                bandmapSpot->setRotConnected(value.toBool());
                break;
            case RUN_MODE_ON_COL_NUM:
                bandmapSpot->setRunModeOn(value.toBool());
            break;
            case OFF_RUN_FREQ_COL_NUM:
                bandmapSpot->setOffRunFreq(value.toBool());
            break;
            case CQ_RESPONSE_COL:
                bandmapSpot->setCqResponse(value.toBool());
            break;
            case DX_DISTRICT_COL_NUM:
                bandmapSpot->setDistrict(value.toString());
            break;
            case DX_DISTRICT_WORKED_COL_NUM:
                bandmapSpot->setDistrictWorked(value.toBool());
            break;
            case DXCLUSTER_SPOT_TYPE:
                bandmapSpot->setClusterSpotType(value.toString());
            break;

            default:
                return false;

        }


        //bandmapData.replace(row, bandmapSpot);    // not needed as bandmapspot points to the data anyway
        emit dataChanged(index, index);


        return true;
    }


        return false;
}



// NOTE! This needs modification of the for loop and rowData to trully support multiple rows!!
bool BandmapDataModel::insertRows(int row, int count, const QModelIndex &index)
{
    Q_UNUSED(index)

    beginInsertRows(QModelIndex(), row , row + count - 1);
    for (int i = 0; i < count; i++)
    {
        bandmapData.insert(row , rowData);
    }
    std::sort(bandmapData.begin(), bandmapData.end(),
              [=](const QSharedPointer<BandmapSpotData> a, const QSharedPointer<BandmapSpotData> b)->bool
                {
                    if (a->getFreq() == b->getFreq())
                    {
                        return a->getDxCall() < b->getDxCall();
                    }
                    return a->getFreq() < b->getFreq();
                }
    );

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

    for (int row = _row + count - 1; row > (_row - 1); row--)
    {
        QSharedPointer<BandmapSpotData> spotData = bandmapData[row];
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

QSharedPointer<BandmapSpotData> BandmapDataModel::getBandmapDataRow(int row)
{
    return bandmapData[row];
}

void BandmapDataModel::sortModel()
{
    beginResetModel();

    std::sort(bandmapData.begin(), bandmapData.end(),
              [=](const QSharedPointer<BandmapSpotData> a, const QSharedPointer<BandmapSpotData> b)->bool
                {
                    if (a->getFreq() == b->getFreq())
                    {
                        return a->getDxCall() < b->getDxCall();
                    }
                    return a->getFreq() < b->getFreq();
                }
    );

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

    QSharedPointer<BandmapSpotData> spotData = cgm->getBandmapDataRow(sourceRow);


    QString call = cgm->data(cgm->index(sourceRow, DXSPOT_CALL_COL_NUM ),  BMP_DataStoredRole).toString();
    QString loc = cgm->data(cgm->index(sourceRow, DXLOC_COL_NUM ),  BMP_DataStoredRole).toString();
//    QString spotterCall = cgm->data(cgm->index(sourceRow, SPOTTER_CALL_COL_NUM ),  BMP_DataStoredRole).toString();
//    QString spotterLoc = cgm->data(cgm->index(sourceRow, SPOTTER_LOC_COL_NUM ),  BMP_DataStoredRole).toString();
//    QString comment = cgm->data(cgm->index(sourceRow, COMMENT_COL_NUM ),  BMP_DataStoredRole).toString();

    if (call.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;
    if (loc.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

//    if (spotterCall.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
//        return true;
//    if (spotterLoc.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
//        return true;
//    if (comment.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
//        return true;

    return false;
}

void BandmapSortFilterProxyModel::setFilterString(QString f)
{
    filterString = f;
    invalidateFilter();
}

bool BandmapSortFilterProxyModel::lessThan(const QModelIndex &left,
                      const QModelIndex &right) const
{
    //Model Indices are to the SOURCE model

    BandmapDataModel *cgm = dynamic_cast<BandmapDataModel *>(sourceModel());

    int lrow = left.row();
    int rrow = right.row();


    Frequency ws1;
    Frequency ws2;
    ws1 = qvariant_cast<Frequency>(cgm->data(left, BMP_DataStoredRole));
    ws2 = qvariant_cast<Frequency>(cgm->data(right, BMP_DataStoredRole));

    if (ws1 == ws2)
    {
        QString ss1 = cgm->data(createIndex(lrow, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toString();
        QString ss2 = cgm->data(createIndex(rrow, DXSPOT_CALL_COL_NUM), BMP_DataStoredRole).toString();
        return ss1 < ss2;
    }
    else
    {
        return ws1 < ws2;
    }

    return false;
}
