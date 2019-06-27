/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "cutils.h"
#include "contacts.h"
#include "contest.h"
#include "TreeUtils.h"
#include "htmldelegate.h"

GridColumn QSOTreeColumns[ LOGTREECOLS ] =
   {
      GridColumn( egTime, "XXXXXX", "UTC", taLeftJustify ),               // time
      GridColumn( egCall, "MMMMMMMMMMM", "Callsign", taLeftJustify ),         // call
      GridColumn( egRSTTx, "599XXX", "RepTx", taLeftJustify ),                 // RST
      GridColumn( egSNTx, "1234X", "SnTx", taLeftJustify /*taRightJustify*/ ),   // serial
      GridColumn( egRSTRx, "599XXX", "RepRx", taLeftJustify ),                 // RST
      GridColumn( egSNRx, "1234X", "SnRx", taLeftJustify /*taRightJustify*/ ),   // Serial
      GridColumn( egLoc, "MM00MM00X", "Loc", taLeftJustify ),            // LOC
      GridColumn( egScore, "12345XX", "dist", taLeftJustify /*taRightJustify*/ ),  // score
      GridColumn( egBrg, "3601X", "brg", taLeftJustify ),                // bearing
      GridColumn( egExchange, "XXXXXXXXXXXXXXXX", "Exchange", taLeftJustify ),    // QTH
      GridColumn( egComments, "XXXXXXXXXXXXXXXX", "Comments", taLeftJustify ),     // comments
      GridColumn( egFrequency, "1.296.123.456XXX", "Freq", taLeftJustify ),
      GridColumn( egRotatorHeading, "XXXXXX", "Rot Heading", taLeftJustify ),
      GridColumn( egRigName, "XXXXXX", "Rig", taLeftJustify )

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
        if ( ct && column >= 0 && column < columnCount())
        {
           QString line = ct->getField( QSOTreeColumns[ column ].fieldId, contest );
           QColor multhighlight = Qt::red;
           bool setHighlight = false;
           switch ( QSOTreeColumns[ column ].fieldId )
           {
              case egTime:
                 if (!contest->checkTime(ct->time))
                 {
                    setHighlight = true;
                 }
                 break;
              case egCall:
                 if ( contest->countryMult.getValue() && ct->newCtry )
                     setHighlight = true;
                 break;
              case egExchange:
                 if ( contest->districtMult.getValue() && ct->newDistrict )
                     setHighlight = true;
                 break;
              case egLoc:
                 if ( contest->locMult.getValue() && ct->locCount > 0)
                 {
                     setHighlight = true;
                 }
                 else if ( contest->usesBonus.getValue() && ct->bonus > 0)
                 {
                     switch (ct->bonus)
                     {
                     case 500:  //blue
                         multhighlight = Qt::blue;
                         break;
                     case 1000: //green
                         multhighlight = Qt::darkGreen;
                         break;
                     case 2000: //red
                         multhighlight = Qt::red;
                         break;
                     }

                     setHighlight = true;
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
        QString h = QSOTreeColumns[ section ].title;
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
    return  LOGTREECOLS;
}
