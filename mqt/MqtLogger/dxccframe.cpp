#include "base_pch.h"
#include "MinosLoggerEvents.h"

#include "LoggerContest.h"
#include "ContestApp.h"

#include "tsinglelogframe.h"
#include "htmldelegate.h"

#include "dxccframe.h"
#include "ui_dxccframe.h"

extern ContList contlist[ CONTINENTS ];

static GridColumn CountryTreeColumns[ ectMultMaxCol ] =
   {
      GridColumn( ectCall, "XXXXXX", "Call", taLeftJustify ),
      GridColumn( ectWorked, "Wk CtX", "Wkd", taCenter ),
      GridColumn( ectLocator, "MM00MM00", "Locator", taLeftJustify ),
      GridColumn( ectBearing, "BRGXXX", "brg", taCenter ),
      GridColumn( ectName, "This is a very long country", "Country", taLeftJustify ),
      GridColumn( ectOtherCalls, "This is a very very very very long country name", "Other calls", taLeftJustify /*taRightJustify*/ )
   };

DXCCFrame::DXCCFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DXCCFrame)
{
    ui->setupUi(this);
}

DXCCFrame::~DXCCFrame()
{
    delete ui;
}
void DXCCFrame::setContest(LoggerContestLog *contest)
{
    model.ct = contest;
    if (contest)
    {
        int lcf;
        TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
        delegate = new HtmlDelegate(1.0, lcf/100.0);
        model.delegate = delegate;

        proxyModel.setSourceModel(&model);
        ui->DXCCTable->setModel(&proxyModel);
        ui->DXCCTable->setItemDelegate(delegate);
        reInitialiseCountries();
        ui->DXCCTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        connect( ui->DXCCTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
                 this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);
    }
}
void DXCCFrame::reInitialiseCountries()
{
    model.reset();
    QSettings settings;
    QByteArray state;

    state = settings.value("DXCCTable/state").toByteArray();
    ui->DXCCTable->horizontalHeader()->restoreState(state);

    for(int i = 0; i < proxyModel.rowCount(); i++)
    {
        const QModelIndex index = proxyModel.mapToSource( proxyModel.index(i, 0) );
        int sourceRow = index.row();
        if (sourceRow == proxyModel.scrolledCountry)
        {
            ui->DXCCTable->setCurrentIndex(proxyModel.index(i, 0));
        }
    }
}
void DXCCFrame::scrollToCountry( int ctry_ind, bool makeVisible )
{
    if (makeVisible)
        proxyModel.scrolledCountry = ctry_ind;
    else
        proxyModel.scrolledCountry = -1;
   reInitialiseCountries();
}

void DXCCFrame::on_sectionResized(int, int , int)
{
    QSettings settings;
    QByteArray state;

    state = ui->DXCCTable->horizontalHeader()->saveState();
    settings.setValue("DXCCTable/state", state);
}

DXCCGridModel::DXCCGridModel():ct(nullptr)
{}
DXCCGridModel::~DXCCGridModel()
{
}
void DXCCGridModel::reset()
{
    beginResetModel();

    endResetModel();
}

void DXCCGridModel::initialise( )
{
   beginResetModel();

   // pick up the correct DXCC list

   endResetModel();
}
QVariant DXCCGridModel::data( const QModelIndex &index, int role ) const
{

    if (ct)
    {
        if (role == Qt::BackgroundRole)
        {
            return QVariant();
        }
        if ( role != Qt::DisplayRole && role != Qt::EditRole )
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            QString disp = MultLists::getMultLists() ->getCtryListText( index.row(), CountryTreeColumns[ index.column() ].fieldId, ct );
            return disp.trimmed();
        }
        if (role == Qt::TextAlignmentRole)
            return Qt::AlignLeft;
    }
    return QVariant();
}
QVariant DXCCGridModel::headerData( int section, Qt::Orientation orientation,
                     int role ) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        QString cell;

        cell = CountryTreeColumns[section].title;

        return cell.trimmed();
    }
    if (role == Qt::TextAlignmentRole)
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

QModelIndex DXCCGridModel::index( int row, int column, const QModelIndex &/*parent*/) const
{
    if ( row < 0 || row >= rowCount()  )
        return QModelIndex();

    return createIndex( row, column );
}

QModelIndex DXCCGridModel::parent( const QModelIndex &/*index*/ ) const
{
    return QModelIndex();
}

int DXCCGridModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    return MultLists::getMultLists() ->getCtryListSize();
}

int DXCCGridModel::columnCount( const QModelIndex &/*parent*/ ) const
{
    return ectMultMaxCol;
}
bool DXCCSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    if (scrolledCountry == sourceRow)
        return true;
    LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>(TContestApp::getContestApp() ->getCurrentContest());
    if (!ct)
        return false;

    int worked = MultLists::getMultLists()->getCountryWorked(sourceRow, ct) ;

    QSharedPointer<CountryEntry> ce = MultLists::getMultLists() ->getCtryListAt( sourceRow );
    bool makeVisible = false;
    for ( int i = 0; i < CONTINENTS; i++ )
    {
        if ( ce->continent == contlist[ i ].continent )
        {
            makeVisible = contlist[ i ].allow;
            break;
        }
    }

    if ( worked && ct->showUnworked.getValue() && !ct->showWorked.getValue() )
    {
        makeVisible = false;
    }
    else
        if ( !worked && !ct->showUnworked.getValue() && ct->showWorked.getValue() )
        {
            makeVisible = false;
        }


    return makeVisible;
}

void DXCCFrame::on_DXCCTable_clicked(const QModelIndex &index)
{
    const QModelIndex srcindex = proxyModel.mapToSource( index );
    int sourceRow = srcindex.row();

    QString disp = MultLists::getMultLists() ->getCtryListText( sourceRow, 0, model.ct );
    MinosLoggerEvents::SendCountrySelect(disp, model.ct);

}
