#include "kstmessagegridmodel.h"
#include "kstmainwindow.h"
//==========================================================================================
bool compMessages ( QSharedPointer<KstMessageLine> q1, const QSharedPointer<KstMessageLine> q2 )
{
    if (q1->dtg == q2->dtg)
        return q1->sequence < q2->sequence;
    else
        return q1->dtg < q2->dtg;
}

KstMessageGridModel::KstMessageGridModel():cacheSize(10, 10)
{
}
void KstMessageGridModel::reset()
{
    beginResetModel();
    messageVector->clear();
    endResetModel();
}
void KstMessageGridModel::setChatVector(QSharedPointer<QVector <QSharedPointer<KstMessageLine> > > pchatVector)
{
    beginResetModel();
    messageVector = pchatVector;
    for ( auto const &kstline: qAsConst(*messageVector))
    {
        if (kstline->distance == -2)
        {
            kstline->distance = mainWindow->calcDistance(kstline->call);
        }
        if (kstline->otherDistance == -2)
        {
            kstline->otherDistance = mainWindow->calcDistance(kstline->otherCall);
        }
    }

    endResetModel();
}
QModelIndex KstMessageGridModel::index( int row, int column,
                              const QModelIndex &parent ) const
{
    if (!messageVector)
        return QModelIndex();

    if ( row < 0 || row >= rowCount() || ( parent.isValid() && parent.column() != 0 ) )
        return QModelIndex();

    return createIndex( row, column, nullptr );
}
QModelIndex KstMessageGridModel::parent( const QModelIndex &/*index*/ ) const
{
    return QModelIndex();
}

int KstMessageGridModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    if (!messageVector)
        return 0;
    return messageVector->count() + 1;
}
int KstMessageGridModel::rawCount( const QModelIndex &/*parent*/ ) const
{
    if (!messageVector)
        return 0;
    return messageVector->count();
}
void KstMessageGridModel::appendLastRow(QSharedPointer<KstMessageLine> msg)
{
    beginInsertRows(QModelIndex(), rawCount() , rawCount());
    messageVector->push_back(msg);
    endInsertRows();
}

void KstMessageGridModel::setCacheSize()
{
    QString s = "Memxx";
    cacheSize = delegate->docSize(s);
}

QVariant KstMessageGridModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    int row = index.row();
    int column = index.column();

    if (role == Qt::DisplayRole)
    {
        if (row >= rawCount())
            return QVariant();

        QSharedPointer<KstMessageLine> crec = messageVector->at(row);

        if (crec->distance == -2)
        {
            crec->distance = mainWindow->calcDistance(crec->call);
            crec->otherDistance = mainWindow->calcDistance(crec->otherCall);
        }

        QString cell;
        switch (column)
        {
        case eccChat:
            if (crec->chat > 0 && crec->chat <= services.count())
                return services[crec->chat - 1];
            else
                return tr("Unknown");
        case eccDTG:
            cell = crec->dtg.toString("HH:mm");
            break;
        case eccCall:
            cell = crec->call;
            break;
        case eccName:
            cell = crec->name;
            break;
        case eccOther:
            cell = crec->otherCall;
            break;
        case eccText:
            cell = crec->message;
            break;
        }
        return cell;
    }
    else if (role == Qt::ToolTipRole)
    {
        if (row >= rawCount())
        {
            return QVariant();
        }
        QSharedPointer<KstMessageLine> crec = messageVector->at(row);
        return crec->message;
    }
    else if (role == Qt::BackgroundRole)
    {
        if (isFiltered())
        {
            return static_cast< QColor> ( 0x00FF80C0 ).lighter(135);        // Pink(ish)
        }
    }
    return QVariant();
}
QVariant KstMessageGridModel::headerData( int section, Qt::Orientation orientation,
                     int role ) const
{

    QString cell;
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        switch (section)
        {
        case eccChat:
            return tr("Chat");
        case eccDTG:
            cell = tr("Time(Z)");
            break;
        case eccCall:
            cell = tr("Call");
            break;
        case eccName:
            cell = tr("Name");
            break;
        case eccOther:
            cell = tr("Other Call");
            break;
        case eccText:
            cell = tr("Text");
            break;
        }
        return cell;
    }
    else if (orientation == Qt::Vertical && role == Qt::SizeHintRole)
    {
        if (delegate)
        {
            return cacheSize;
        }
    }
    return QVariant();
}
int KstMessageGridModel::columnCount( const QModelIndex & /*parent*/ ) const
{
    return eccMaxColumn;
}
void KstMessageGridModel::setFilterString(QString f)
{
    filterString = f;
}

void KstMessageGridModel::setChatFilter(int value)
{
    chatFilter = value;
}
//==========================================================================================
void KstMessageGridSortFilterModel::setChatFilter(int value)
{
    KstMessageGridModel *cgm = dynamic_cast<KstMessageGridModel *>(sourceModel());
    if (cgm)
        cgm->setChatFilter(value);

    chatFilter = value;
    invalidateFilter();
}

bool KstMessageGridSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{

    KstMessageGridModel *cgm = dynamic_cast<KstMessageGridModel *>(sourceModel());
    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    if (sourceRow >= cgm->rawCount())
        return isFiltered();

    QSharedPointer<KstMessageLine> kstmsg = cgm->messageVector->at(sourceRow);

    int m = mainWindow->getMaxDistance();
    // How do we go about filtering for distance?
    if (m == 0 || ((kstmsg->distance > 0 && kstmsg->distance < m)
            || (kstmsg->otherDistance > 0 && kstmsg->otherDistance < m)
            || (kstmsg->distance < 0 && kstmsg->otherDistance < 0)))
    {
        int chat = kstmsg->chat;
        if ((chatFilter > 0 && chatFilter == chat) || (chatFilter == 0 ))
        {
            if (filterString.isEmpty())
                return true;

            if (kstmsg->call.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
                return true;
            if (kstmsg->name.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
                return true;
            if (kstmsg->otherCall.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
                return true;
            if (kstmsg->message.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
                return true;
        }
    }
    return false;
}

void KstMessageGridSortFilterModel::setFilterString(QString f)
{
    KstMessageGridModel *cgm = dynamic_cast<KstMessageGridModel *>(sourceModel());
    if (cgm)
        cgm->setFilterString(f);
    filterString = f;
    invalidateFilter();
}

//==========================================================================================

bool KstMeepGridSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{

    KstMessageGridModel *cgm = dynamic_cast<KstMessageGridModel *>(sourceModel());
    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    if (sourceRow >= cgm->rawCount())
        return false;

    QSharedPointer<KstMessageLine> kstmsg = cgm->messageVector->at(sourceRow);

    if (filterString.isEmpty())
        return false;

    if (kstmsg->call.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;
    if (kstmsg->name.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;
    if (kstmsg->otherCall.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;
    if (kstmsg->message.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

    return false;
}

void KstMeepGridSortFilterModel::setFilterString(QString f)
{
    filterString = f;
    invalidateFilter();
}

QVariant KstMeepGridSortFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::BackgroundRole)
    {
        return QColor(Qt::green).lighter(135);
    }

    QModelIndex sourceIndex = mapToSource(index);
     KstMessageGridModel *cgm = dynamic_cast<KstMessageGridModel *>(sourceModel());
     return cgm->data(sourceIndex, role);
}
