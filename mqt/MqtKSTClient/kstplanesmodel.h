#ifndef KSTPLANESMODEL_H
#define KSTPLANESMODEL_H

#include <QAbstractItemModel>
#include "airscoutlink.h"

enum PlaneColumns {eplcCall, eplcCategory, eplcDistance, eplcPotential, eplcMinutes, eplcMax};

class Aircraft;
class KstPlanesModel : public QAbstractItemModel
{
    Q_OBJECT

    QVector<Aircraft> planes;

public:
    explicit KstPlanesModel(QObject *parent = nullptr);

    void setPlanesVector(QVector<Aircraft> &p);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
};

class KstPlanesGridSortFilterModel : public QSortFilterProxyModel
{
public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

};

#endif // KSTPLANESMODEL_H
