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

#ifndef BANDMAPDATAMODEL_H
#define BANDMAPDATAMODEL_H

#include <QAbstractTableModel>
#include "bandmapdata.h"
#include "clustercommon.h"
#include "bandmapcommon.h"
#include "cutils.h"

const int bandmapColCount = 26;
const int BMP_DataStoredRole = Qt::UserRole + 0;

class BandmapDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit BandmapDataModel(QObject *parent = nullptr);

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

    QSharedPointer<BandmapData> getBandmapDataRow(int row);

    QSharedPointer<BandmapData> rowData;

private:

    QVector< QSharedPointer<BandmapData> > bandmapData;


};

#endif // BANDMAPDATAMODEL_H
