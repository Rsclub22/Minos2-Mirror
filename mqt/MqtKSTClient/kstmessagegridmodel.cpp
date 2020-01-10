#include "kstmessagegridmodel.h"
#include "kstmainwindow.h"
//==========================================================================================

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
    endResetModel();
}
QModelIndex KstMessageGridModel::index( int row, int column,
                              const QModelIndex &parent ) const
{
    if (!messageVector)
        return QModelIndex();

    if ( row < 0 || row >= messageVector->count() || ( parent.isValid() && parent.column() != 0 ) )
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
    return messageVector->count();
}

void KstMessageGridModel::appendLastRow(QSharedPointer<KstMessageLine> msg)
{
    beginInsertRows(QModelIndex(), rowCount() , rowCount());
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
        //QSharedPointer<QVector <QSharedPointer<ChatLine> > > chatVector;
        QSharedPointer<KstMessageLine> crec = messageVector->at(row);

        QString cell;
        switch (column)
        {
        case eccChat:
            if (crec->chat > 0 && crec->chat <= services.count())
                return services[crec->chat - 1];
            else
                return "Unknown";
        case eccDTG:
            cell = crec->dtg;
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
        QSharedPointer<KstMessageLine> crec = messageVector->at(row);
        return crec->fullLine;
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
            return "Chat";
        case eccDTG:
            cell = "Time(Z)";
            break;
        case eccCall:
            cell = "Call";
            break;
        case eccName:
            cell = "Name";
            break;
        case eccOther:
            cell = "Other Call";
            break;
        case eccText:
            cell = "Text";
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
//==========================================================================================
void KstMessageGridSortFilterModel::setShowChat(const QVector<int> &value)
{
    showChat = value;
    invalidateFilter();
}

void KstMessageGridSortFilterModel::setChatFilter(int value)
{
    chatFilter = value;
    invalidateFilter();
}

bool KstMessageGridSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{

    KstMessageGridModel *cgm = dynamic_cast<KstMessageGridModel *>(sourceModel());
    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    QSharedPointer<KstMessageLine> kstmsg = cgm->messageVector->at(sourceRow);

    int chat = kstmsg->chat;
    if ((chatFilter > 0 && chatFilter == chat) || (chatFilter == 0 && showChat.contains(chat)))
    {
        if (filterString.isEmpty())
            return true;

        if (kstmsg->fullLine.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
            return true;
    }

    return false;
}

void KstMessageGridSortFilterModel::setFilterString(QString f)
{
    filterString = f;
    invalidateFilter();
}
//==========================================================================================

bool KstMeepGridSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{

    KstMessageGridModel *cgm = dynamic_cast<KstMessageGridModel *>(sourceModel());
    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    QSharedPointer<KstMessageLine> kstmsg = cgm->messageVector->at(sourceRow);

    if (filterString.isEmpty())
        return true;

    if (kstmsg->fullLine.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

    return false;
}

void KstMeepGridSortFilterModel::setFilterString(QString f)
{
    filterString = f;
    invalidateFilter();
}
