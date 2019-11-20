#include "kstmessagegridmodel.h"
//==========================================================================================

KstMessageGridModel::KstMessageGridModel()
{

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

void KstMessageGridModel::appendLastRow()
{
    // data must have already been added tothe shared message vector
    beginInsertRows(QModelIndex(), rowCount() - 1 , rowCount() - 1);
    endInsertRows();
}

enum ChatColumns {eccSrc = 0, eccDTG, eccCall, eccName, eccOther, eccText, eccMaxColumn};
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
        case eccSrc:
            cell = crec->source;
            break;
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
        case eccSrc:
            cell = "Source";
            break;
        case eccDTG:
            cell = "DTG";
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
int KstMessageGridModel::columnCount( const QModelIndex & /*parent*/ ) const
{
    return eccMaxColumn;
}
//==========================================================================================
bool KstMessageGridSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    if (filterString.isEmpty())
        return true;

    KstMessageGridModel *cgm = dynamic_cast<KstMessageGridModel *>(sourceModel());
    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    QSharedPointer<KstMessageLine> kstmsg = cgm->messageVector->at(sourceRow);
    if (kstmsg->fullLine.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

    return false;
}

void KstMessageGridSortFilterModel::setFilterString(QString f)
{
    filterString = f;
    invalidateFilter();
}
