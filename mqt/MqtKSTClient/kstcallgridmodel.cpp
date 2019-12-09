#include "kstcallgridmodel.h"
#include "contest.h"
// kst2me sort by
// new before old
// locator
// distance
// call
bool KstUser::operator< ( const KstUser& rhs ) const
{
    // p1 is from list; p2 is the one being searched for

    return call < rhs.call;
}
bool KstUserCompare (QSharedPointer<KstUser> i, QSharedPointer<KstUser> j)
{
    return (*i<*j);
}
KstCallGridModel::KstCallGridModel()
{

}
void KstCallGridModel::reset()
{
    beginResetModel();
    callVector->clear();
    endResetModel();
}
void KstCallGridModel::setCallVector(QSharedPointer<QVector<QSharedPointer<KstUser> > > pcallVector)
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

void KstCallGridModel::appendRow(QSharedPointer<KstUser> call)
{
    beginInsertRows(QModelIndex(), rowCount() , rowCount());
    callVector->push_back(call);
    endInsertRows();
}
void KstCallGridModel::insertRow( int row, QSharedPointer<KstUser> call)
{
    beginInsertRows(QModelIndex(), row , row);
    callVector->insert(row, call);
    endInsertRows();
}
void KstCallGridModel::removeRow(int _row)
{
    beginRemoveRows(QModelIndex(), _row, _row);
    callVector->removeAt(_row);
    endRemoveRows();
}
QVariant KstCallGridModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    int row = index.row();

    if (role == Qt::DisplayRole)
    {
        QSharedPointer<KstUser> crec = callVector->at(row);

        switch(index.column())
        {
        case ecscCall:
        {
            QString call = crec->call;
            if (crec->away)
                call = "(" + call + ")";
            if (crec->recent)
                call = "*" + call + "*";

            return call;
        }

        case ecscLoc:
        {
            QString loc = crec->loc;

            return loc;
        }

        case ecscDistance:
        {
            if (crec->distance > 0)
            {
                return crec->distance;
            }
            double dist = 0.0;
            int brg;
            double longitude = 0.0;
            double latitude = 0.0;

            BaseContestLog cnt;
            cnt.myloc.loc.setValue( locator.toUpper() );
            cnt.validateLoc();

            if ( lonlat( crec->loc.toUpper(), longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() ) == LOC_OK )
            {
                cnt.disbeara( longitude, latitude, dist, brg );
                crec->distance = static_cast<int>(dist);
                return QString::number(crec->distance);
            }
            return "--";

        }
        case ecscName:
            return crec->name;

        }
    }
    if (role == Qt::UserRole)
    {
        QSharedPointer<KstUser> crec = callVector->at(row);
        if (index.column() == ecscCall)
        {
            QString call = crec->call;
            if (!crec->recent)
            {
                call = "ZZ " + call;    // to force recent to sort first
            }
            return call;
        }
        if (index.column() == ecscLoc)
        {
            QString loc = crec->loc;
            return loc;
        }
        QVariant cell = data(index, Qt::DisplayRole);
        return cell.toInt();
    }
    return QVariant();
}
QVariant KstCallGridModel::headerData( int section, Qt::Orientation orientation,
                     int role ) const
{

    QString cell;
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        switch(section)
        {
        case ecscCall:
            return "Callsign";

        case ecscLoc:
            return "Loc";

        case ecscDistance:
            return "Dist";
        }
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
    return ecscMaxColumn;
}
//==========================================================================================
bool KstCallGridSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    if (filterString.isEmpty())
        return true;

    KstCallGridModel *cgm = dynamic_cast<KstCallGridModel *>(sourceModel());
    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    QSharedPointer<KstUser> call = cgm->callVector->at(sourceRow);

    if (call->call.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

    if (call->loc.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

    if (call->name.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

    return false;
}

void KstCallGridSortFilterModel::setFilterString(QString f)
{
    filterString = f;
    invalidateFilter();
}

bool KstCallGridSortFilterModel::lessThan(const QModelIndex &left,
                      const QModelIndex &right) const
{
    //Model Indices are to the SOURCE model

    int lrow = left.row();
    int rrow = right.row();

    if (lrow >= sourceModel()->rowCount())
        return false;
    if (rrow >= sourceModel()->rowCount())
        return false;

    QVariant ws1;
    QVariant ws2;
    ws1 = sourceModel()->data(left, Qt::UserRole);
    ws2 = sourceModel()->data(right, Qt::UserRole);

    //need to correct for locator and distance sorting

    return ws1 < ws2;
}
