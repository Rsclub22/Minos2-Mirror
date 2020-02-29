#include "kstplanesmodel.h"
#include "airscoutlink.h"

KstPlanesModel::KstPlanesModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

void KstPlanesModel::setPlanesVector(QVector<Aircraft> &p)
{
    beginResetModel();
    planes = p;
    endResetModel();
}


QModelIndex KstPlanesModel::index(int row, int column, const QModelIndex &parent) const
{
     if ( row < 0 || row >= rowCount() || ( parent.isValid() && parent.column() != 0 ) )
        return QModelIndex();

    return createIndex( row, column, nullptr );
}

QModelIndex KstPlanesModel::parent(const QModelIndex &/*index*/) const
{
    return QModelIndex();
}

int KstPlanesModel::rowCount(const QModelIndex &/*parent*/) const
{
    return planes.size();
}

int KstPlanesModel::columnCount(const QModelIndex &/*parent*/) const
{
    return eplcMax;
}


QVariant KstPlanesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        switch(section)
        {
        case eplcCall:
            return tr("Flight");

        case eplcCategory:
            return tr("Category");

        case eplcDistance:
            return tr("Dist");

        case eplcPotential:
            return tr("Pot");

        case eplcMinutes:
            return tr("Mins");
        }
    }
    return QVariant();
}

QVariant KstPlanesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();

    if ( role == Qt::DisplayRole )
    {
        switch(index.column())
        {
        case eplcCall:
            return planes[row].call;

        case eplcCategory:
            return planes[row].category;

        case eplcDistance:
            return planes[row].distance;

        case eplcPotential:
            return planes[row].potential;

        case eplcMinutes:
            return planes[row].minutes;
        }
    }
    return QVariant();
}


bool KstPlanesGridSortFilterModel::filterAcceptsRow(int /*sourceRow*/, const QModelIndex &/*sourceParent*/) const
{
    return true;
}
