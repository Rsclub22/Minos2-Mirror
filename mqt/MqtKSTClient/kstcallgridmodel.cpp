#include "kstcallgridmodel.h"
#include "contest.h"
#include "kstmainwindow.h"
#include "cutils.h"

// kst2me sort by
// new before old
// locator
// distance
// call
bool KstUser::operator< ( const KstUser& rhs ) const
{
    // p1 is from list; p2 is the one being searched for

    if (chat  == rhs.chat)
        return call < rhs.call;
    else
        return chat < rhs.chat;
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
void KstCallGridModel::setCallVector(QSharedPointer<QVector<QSharedPointer<KstUser> > > &pcallVector)
{
    beginResetModel();
    callVector = pcallVector;
    for (int i = 0; i < callVector->size(); i++)
    {
        QSharedPointer<KstUser> kstuser = callVector->at(i);
        if (kstuser->distance == -2)
        {
            checkDistBear(kstuser);
        }
    }
    endResetModel();
}
QModelIndex KstCallGridModel::index( int row, int column,
                              const QModelIndex &parent ) const
{
    if (!callVector)
        return QModelIndex();

    if ( row < 0 || row >= rowCount() || ( parent.isValid() && parent.column() != 0 ) )
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
    return callVector->count() + 1;
}

int KstCallGridModel::rawCount(const QModelIndex &/*parent*/) const
{
    if (!callVector)
        return 0;
    return callVector->count();
}

void KstCallGridModel::appendRow(QSharedPointer<KstUser> call)
{
    beginInsertRows(QModelIndex(), rawCount() , rawCount());
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

void KstCallGridModel::setFilterString(QString f)
{
    filterString = f;
}

void KstCallGridModel::setChatFilter(int value)
{
    chatFilter = value;
}
void KstCallGridModel::checkDistBear(QSharedPointer<KstUser> crec) const
{
    if (crec->distance < 0)
    {
        double dist = 0.0;
        int brg;
        double longitude = 0.0;
        double latitude = 0.0;

        BaseContestLog cnt;
        cnt.myloc.setLoc( locator );
        cnt.validateLoc();

        if ( lonlat( crec->loc.toUpper(), longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() ) == LOC_OK )
        {
            cnt.disbeara( longitude, latitude, dist, brg );
            crec->distance = static_cast<int>(dist);
            crec->bearing = brg;
        }
    }
}
QVariant KstCallGridModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    int row = index.row();

    if (role == Qt::DisplayRole)
    {
        if (row >= rawCount())
            return QVariant();

        QSharedPointer<KstUser> crec = callVector->at(row);

        switch(index.column())
        {
        case ecscChat:
        if (crec->chat > 0 && crec->chat <= services.count())
            return services[crec->chat - 1];
        else
            return "Unknown";

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
            checkDistBear(crec);
            return crec->distance;

        }
        case ecscBearing:
        {
            checkDistBear(crec);
            return crec->bearing;
        }
        case ecscName:
            return crec->name;

        case ecscCountryPrefix:
            return crec->prefix;

        case ecscCountryName:
            return crec->country;

        case ecscAirscout:
        {
            if (crec->distance > mainWindow->getASMaxDistance())
                return "&gt;";
            if (crec->distance < mainWindow->getASMinDistance())
                return "&lt;";

            int zcount = 0;
            QString col;
            if (crec->planes.count())
            {
                for (int i = 0; i < crec->planes.count(); i++)
                {
                    if (crec->planes[i].minutes != 0)
                        break;
                    zcount++;
                }
                if (zcount)
                {
//                    100 = magenta,
//                    75 = red,
//                    50 = orange,
//                    <50 = grey
                    switch(crec->planes[0].potential)
                    {
                    case 100:
                        col = "magenta";
                        break;

                    case 75:
                        col = "red";
                        break;

                    case 50:
                        col = "orange";
                        break;

                    default:
                        col = "gray";
                        break;
                    }
                }
            }
            QString cell;
            if (crec->planeResponseSeen)
            {
                if (!col.isEmpty())
                    cell = HtmlFontColour(col) + QString::number(zcount);

                cell +=  HtmlFontColour("black") + "(" + QString::number(crec->planes.count()) + ")";
            }
            else
            {
                cell = "--";
            }
            return cell;
        }
        }
    }
    if (role == Qt::UserRole)
    {
        if (row >= rawCount())
            return QVariant();

        QSharedPointer<KstUser> crec = callVector->at(row);
        switch (index.column())
        {
        case ecscChat:
            return crec->chat;

        case ecscCall:
        {
            QString call = crec->call;
            if (!crec->recent)
            {
                call = "ZZ " + call;    // to force recent to sort first
            }
            return call;
        }

        case ecscLoc:
        {
            QString loc = crec->loc;
            return loc;
        }

        case ecscName:
        {
            QString name = crec->name;
            return name;
        }

        case ecscDistance:
        {
            QVariant cell = data(index, Qt::DisplayRole);
            return cell.toInt();
        }
        case ecscBearing:
        {
            QVariant cell = data(index, Qt::DisplayRole);
            return cell.toInt();
        }

        case ecscCountryPrefix:
            return crec->prefix;

        case ecscCountryName:
            return crec->country;

        case ecscAirscout:
            if (crec->distance > mainWindow->getASMaxDistance())
                return QString("000001");
            if (crec->distance < mainWindow->getASMinDistance())
                return QString("000000");

            int zcount = 0;
            QString col;
            if (crec->planes.count())
            {
                for (int i = 0; i < crec->planes.count(); i++)
                {
                    if (crec->planes[i].minutes != 0)
                        break;
                    zcount++;
                }
                QString z = QString::number(zcount);
                QString a = QString::number(crec->planes.count());
                QString cell = QString("%1%2").arg(z, 2, '0').arg(a, 4, '0');
                return cell;
            }
            return QString("000002");
        }
    }
    if (role == Qt::BackgroundColorRole)
    {
        if (isFiltered())
        {
            return static_cast< QColor> ( 0x00FF80C0 ).lighter(135);        // Pink(ish)
        }
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
        case ecscChat:
            return tr("Chat");

        case ecscCall:
            return tr("Callsign");

        case ecscLoc:
            return tr("Loc");

        case ecscDistance:
            return tr("Dist");

        case ecscBearing:
            return tr("Brg");

        case ecscAirscout:
            return tr("AS");

        case ecscName:
            return tr("Name");

        case ecscCountryPrefix:
            return tr("Prefix");

        case ecscCountryName:
            return tr("Country");
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
void KstCallGridSortFilterModel::setChatFilter(int value)
{
    KstCallGridModel *cgm = dynamic_cast<KstCallGridModel *>(sourceModel());
    if (cgm)
        cgm->setChatFilter(value);
    chatFilter = value;
    invalidateFilter();
}

bool KstCallGridSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{

    KstCallGridModel *cgm = dynamic_cast<KstCallGridModel *>(sourceModel());
    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    if (sourceRow >= cgm->rawCount())
        return isFiltered();

    QSharedPointer<KstUser> call = cgm->callVector->at(sourceRow);

    int m = mainWindow->getMaxDistance();
    if (m > 0 && call->distance > m)
        return false;

    int chat = call->chat;
    if ((chatFilter > 0 && chatFilter == chat) || (chatFilter == 0))
    {
        if (filterString.isEmpty())
            return true;

        if (call->call.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
            return true;

        if (call->loc.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
            return true;

        if (call->name.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
            return true;
    }

    return false;
}

void KstCallGridSortFilterModel::setFilterString(QString f)
{
    KstCallGridModel *cgm = dynamic_cast<KstCallGridModel *>(sourceModel());
    if (cgm)
        cgm->setFilterString(f);
    filterString = f;
    invalidateFilter();
}

bool KstCallGridSortFilterModel::lessThan(const QModelIndex &left,
                      const QModelIndex &right) const
{
    //Model Indices are to the SOURCE model

    KstCallGridModel *cgm = dynamic_cast<KstCallGridModel *>(sourceModel());

    int lrow = left.row();
    int rrow = right.row();

    if (lrow >= cgm->rawCount())
        return false;
    if (rrow >= cgm->rawCount())
        return true;

    QVariant ws1;
    QVariant ws2;
    ws1 = sourceModel()->data(left, Qt::UserRole);
    ws2 = sourceModel()->data(right, Qt::UserRole);

    //need to correct for locator and distance sorting

    if (ws1 == ws2)
    {
        ws1 = sourceModel()->data(createIndex(lrow, ecscCall), Qt::UserRole);
        ws2 = sourceModel()->data(createIndex(rrow, ecscCall), Qt::UserRole);
    }
    if (ws1.type() == QVariant::Type::Int)
    {
        return ws1.toInt() < ws2.toInt();
    }
    else if (ws1.type() == QVariant::Type::String)
    {
        return ws1.toString() < ws2.toString();
    }
    return false;
}
