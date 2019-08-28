/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      DXSpot Data Model
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef DXSPOTDATAMODEL_H
#define DXSPOTDATAMODEL_H

#include <QAbstractTableModel>
#include "spotdata.h"
#include "clustercommon.h"

const int dxSpotColCount = 17;

const int DataStoredRole = Qt::UserRole + 0;


class HtmlDelegate;

class DxSpotDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit DxSpotDataModel(QObject *parent = nullptr);
    ~DxSpotDataModel() override;

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
    SpotData* rowData = nullptr;
    QSharedPointer<HtmlDelegate> delegate ;

private:

    QVector<SpotData*> dxSpotData;


};

#endif // DXSPOTDATAMODEL_H
