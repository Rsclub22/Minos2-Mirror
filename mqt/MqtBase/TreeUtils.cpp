/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "cutils.h"
#include "contacts.h"
#include "contest.h"
#include "TreeUtils.h"
#include "htmldelegate.h"

QVector<GridColumn> QSOGridModel::QSOTreeColumns =
   {
      GridColumn( egDate, "XX/XX/XX", QT_TR_NOOP("Date"), taLeftJustify ),               // time
      GridColumn( egTime, "XXXXXX", QT_TR_NOOP("UTC"), taLeftJustify ),               // time
      GridColumn( egBand, "XXXXXX", QT_TR_NOOP("Band"), taLeftJustify ),               // time
      GridColumn( egMode, "XXXX", QT_TR_NOOP("Mode"), taLeftJustify ),               // time
      GridColumn( egCall, "MMMMMMMMMMM", QT_TR_NOOP("Callsign"), taLeftJustify ),         // call
      GridColumn( egRSTTx, "599XXX", QT_TR_NOOP("RepTx"), taLeftJustify ),                 // RST
      GridColumn( egSNTx, "1234X", QT_TR_NOOP("SnTx"), taLeftJustify /*taRightJustify*/ ),   // serial
      GridColumn( egRSTRx, "599XXX", QT_TR_NOOP("RepRx"), taLeftJustify ),                 // RST
      GridColumn( egSNRx, "1234X", QT_TR_NOOP("SnRx"), taLeftJustify /*taRightJustify*/ ),   // Serial
      GridColumn( egLoc, "MM00MM00X", QT_TR_NOOP("Loc"), taLeftJustify ),            // LOC
      GridColumn( egScore, "12345XX", QT_TR_NOOP("dist"), taLeftJustify /*taRightJustify*/ ),  // score
      GridColumn( egBrg, "3601X", QT_TR_NOOP("brg"), taLeftJustify ),                // bearing
      GridColumn( egExchange, "XXXXXXXXXXXXXXXX", QT_TR_NOOP("Exchange"), taLeftJustify ),    // QTH
      GridColumn( egComments, "XXXXXXXXXXXXXXXX", QT_TR_NOOP("Comments"), taLeftJustify ),     // comments
      GridColumn( egFrequency, "1.296.123.456XXX", QT_TR_NOOP("Freq"), taLeftJustify ),
      GridColumn( egRotatorHeading, "XXXXXX", QT_TR_NOOP("Rot Heading"), taLeftJustify ),
      GridColumn( egRigName, "XXXXXX", QT_TR_NOOP("Rig"), taLeftJustify ),
      GridColumn( egOperator, "XXXXXX", QT_TR_NOOP("Op"), taLeftJustify )

   };

QSOGridModel::QSOGridModel():contest(nullptr)
{}
QSOGridModel::~QSOGridModel()
{}
void QSOGridModel::reset()
{
    beginResetModel();
    endResetModel();
}

void QSOGridModel::initialise(BaseContestLog * pcontest )
{
   contest = pcontest;
}
QVariant QSOGridModel::data( const QModelIndex &index, int role ) const
{
    int row = index.row();
    int column = index.column();

    if ( row >= rowCount() )
        return QVariant();

    if (contest == nullptr)
        return QVariant();

    QSharedPointer<BaseContact> ct = contest->pcontactAt( row);
    if (!ct)
        return QVariant();

    if (role == Qt::BackgroundRole)
    {
        if ( ct->contactFlags.getValue() & FORCE_LOG )
        {
           return static_cast< QColor> ( 0x00FF80C0 );        // Pink(ish)
        }
        else
        {
           if ( ct->getModificationCount() > 1 )
           {
               return static_cast< QColor> ( 0x00C0DCC0 );    // "money green"
           }
        }
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
        return Qt::AlignLeft;           // but HtmlDelegate overrides

    if (role == Qt::DisplayRole)
    {
        if ( column >= 0 && column < columnCount())
        {
           QString line = ct->getField( QSOTreeColumns[ column ].fieldId, contest );
           QColor multhighlight = Qt::red;
           bool setHighlight = false;
           switch ( QSOTreeColumns[ column ].fieldId )
           {
              case egDate:
              case egTime:
                 if (!contest->checkTime(ct->timeOff))
                 {
                    setHighlight = true;
                 }
                 break;
              case egBand:
                   break;
              case egMode:
                   break;
              case egCall:
                 if ( contest->countryMult.getValue() && ct->newCtry )
                     setHighlight = true;
                 else if ( contest->usesBonus.getValue() && ct->countryBonus > 0)
                 {
                     multhighlight = Qt::blue;
                     setHighlight = true;
                 }
                 break;
              case egExchange:
                 if ( contest->districtMult.getValue() && ct->newDistrict )
                     setHighlight = true;
                 else if ( contest->usesBonus.getValue() && ct->distBonus > 0)
                 {
                     multhighlight = Qt::blue;
                     setHighlight = true;
                 }
                 break;
              case egLoc:
                 if ( contest->locMult.getValue() && ct->locCount > 0)
                 {
                     setHighlight = true;
                 }
                 else if ( contest->usesBonus.getValue() && ct->locBonus > 0)
                 {
                    multhighlight = Qt::blue;
                     setHighlight = true;
                 }
                 break;
              case egFrequency:
                 {
                    if (ct->cqResponse.getValue())
                        line += "(CQ)";
                 }
                 break;
           }
           if (setHighlight)
               line = HtmlFontColour(multhighlight) + "<b>" + line;
           return line;
        }
    }
    return QVariant();
}
QVariant QSOGridModel::headerData( int section, Qt::Orientation orientation,
                     int role ) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        QString h = tr(QSOTreeColumns[ section ].title);
        return h;
    }
    else if (role == Qt::TextAlignmentRole)
    {
        return Qt::AlignLeft;
    }
    else if (orientation == Qt::Vertical && role == Qt::SizeHintRole)
    {
        if (delegate)
        {
            QString s = data(index(section, 0), Qt::DisplayRole).toString();
            QSize r = delegate->docSize(s);
            r.setWidth(0);
            return r;
        }
    }
    return QVariant();
}

QModelIndex QSOGridModel::index( int row, int column, const QModelIndex &parent) const
{
    if ( row < 0 || row >= rowCount() || ( parent.isValid() && parent.column() != 0 ) )
        return QModelIndex();

    return createIndex( row, column );
}

QModelIndex QSOGridModel::parent( const QModelIndex &/*index*/ ) const
{
    return QModelIndex();
}

int QSOGridModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    if (!contest)
        return 0;

    return contest->ctList.size() + 1;
}

int QSOGridModel::columnCount( const QModelIndex &/*parent*/ ) const
{
    return  QSOGridModel::QSOTreeColumns.count();
}

bool QSOGridModel::insertRows(int row, int count, const QModelIndex &index)
{
    beginInsertRows(index, row, row + count - 1);
    endInsertRows();
    return true;
}
void QSOGridModel::changeRow(int row)
{
    emit dataChanged(index(row, 0), index(row, columnCount()));
}
