#include "kstcallgridmodel.h"
KstCallGridModel::KstCallGridModel()
{

}
void KstCallGridModel::setCallVector(QSharedPointer<QStringList > pcallVector)
{
    beginResetModel();
    callVector = pcallVector;
    endResetModel();
}
QModelIndex KstCallGridModel::index( int row, int column,
                              const QModelIndex &parent ) const
{
    if (!callVector)
        return QModelIndex();

    if ( row < 0 || row >= callVector->count() || ( parent.isValid() && parent.column() != 0 ) )
        return QModelIndex();

    return createIndex( row, column, nullptr );
}
QModelIndex KstCallGridModel::parent( const QModelIndex &/*index*/ ) const
{
    return QModelIndex();
}

int KstCallGridModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    if (!callVector)
        return 0;
    return callVector->count();
}

void KstCallGridModel::appendRow(QString call)
{
    beginInsertRows(QModelIndex(), rowCount() , rowCount());
    callVector->push_back(call);
    endInsertRows();
}

QVariant KstCallGridModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    int row = index.row();

    if (role == Qt::DisplayRole)
    {
        QString crec = callVector->at(row);

        return crec;
    }
    return QVariant();
}
QVariant KstCallGridModel::headerData( int /*section*/, Qt::Orientation orientation,
                     int role ) const
{

    QString cell;
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        return "Callsign";
    }
    else if (orientation == Qt::Vertical && role == Qt::SizeHintRole)
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
    return QVariant();
}
int KstCallGridModel::columnCount( const QModelIndex & /*parent*/ ) const
{
    return 1;
}
//==========================================================================================
bool KstCallGridSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    if (filterString.isEmpty())
        return true;

    KstCallGridModel *cgm = dynamic_cast<KstCallGridModel *>(sourceModel());
    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    QString call = cgm->callVector->at(sourceRow);
    if (call.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

    return false;
}

void KstCallGridSortFilterModel::setFilterString(QString f)
{
    filterString = f;
    invalidateFilter();
}

