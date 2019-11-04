/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      DXSpot Data Model
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
///
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef SENDSPOTDATAMODEL_H
#define SENDSPOTDATAMODEL_H

#include <QAbstractTableModel>
#include "sentSpotdata.h"
#include "clustercommon.h"

const int sentSpotColCount = 8;

const int SentDataStoredRole = Qt::UserRole + 0;

const int SENT_SPOT_TIME_COL_NUM = 0;
const int SENT_SPOT_FREQ_STR_COL_NUM = 1;
const int SENT_SPOT_DXSPOT_CALL_COL_NUM = 2;
const int SENT_SPOT_DXLOC_COL_NUM = 3;
const int SENT_SPOT_COMMENT_COL_NUM = 4;
const int SENT_SPOT_STATUS_COL_NUM = 5;
const int SENT_SPOT_RXTIME_COL_NUM = 6;
const int SENT_SPOT_REASON_COL_NUM = 7;



class HtmlDelegate;

class SentSpotDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SentSpotDataModel(QObject *parent = nullptr);
    ~SentSpotDataModel() override;

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // get data
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    // set data
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &index = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    SentSpotData* rowData = nullptr;
    QSharedPointer<HtmlDelegate> delegate ;

private:

    QVector<SentSpotData*> sentSpotData;


};

#endif // DXSPOTDATAMODEL_H
