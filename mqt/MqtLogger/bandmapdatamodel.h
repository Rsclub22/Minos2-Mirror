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
#include <QSortFilterProxyModel>

#include "spotbasedata.h"

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

    // Add data:
    bool insertRows(int row, int count, const QModelIndex &index = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    // Remove data:
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    QSharedPointer<ClusterSpotData> getBandmapDataRow(int row);

    QVector< QSharedPointer<ClusterSpotData> > rowData;

    QSharedPointer<ClusterSpotData> getSpotData(int row){return bandmapData[row];};

    void sortModel();

private:

    QVector< QSharedPointer<ClusterSpotData>>  bandmapData;


    void sortBandmapModel();
};

class BandmapSortFilterProxyModel: public QSortFilterProxyModel
{
    QString filterString;
public:
    explicit BandmapSortFilterProxyModel(QObject *parent = nullptr);

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setFilterString(QString f);
    QString getFilterString();
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    QSharedPointer<ClusterSpotData> getBandmapDataRow(int row);

};

#endif // BANDMAPDATAMODEL_H
