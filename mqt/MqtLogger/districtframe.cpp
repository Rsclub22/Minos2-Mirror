#include "base_pch.h"
#include "MinosLoggerEvents.h"

#include "LoggerContest.h"
#include "ContestApp.h"
#include "htmldelegate.h"
#include "tsinglelogframe.h"
#include "districtframe.h"
#include "ui_districtframe.h"

static GridColumn DistrictTreeColumns[ ectMultMaxCol - 1 ] =
   {
      GridColumn( ectCall, "XXXXXXX", "Code", taLeftJustify ),
      GridColumn( ectWorked, "Wk CtX", "Wkd", taCenter ),
      GridColumn( ectLocator, "MM00MM00", "Locator", taLeftJustify ),
      GridColumn( ectBearing, "BRGXXX", "brg", taCenter ),
      GridColumn( ectName, "This is a Very Very long District", "District", taLeftJustify ),
   };

DistrictFrame::DistrictFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DistrictFrame)
{
    ui->setupUi(this);
}

DistrictFrame::~DistrictFrame()
{
    delete ui;
}
void DistrictFrame::setContest(BaseContestLog *contest)
{
    model.ct = contest;
    delegate = new HtmlDelegate(1.0, 1.0);
    model.delegate = delegate;
    ui->DistrictTable->setItemDelegate(delegate);
    proxyModel.setSourceModel(&model);
    ui->DistrictTable->setModel(&proxyModel);
    if (contest)
    {
        reInitialiseDistricts();
        connect( ui->DistrictTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
                 this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);
    }
    ui->DistrictTable->resizeRowsToContents();
}
void DistrictFrame::reInitialiseDistricts()
{
    model.reset();
    QSettings settings;
    QByteArray state;

    state = settings.value("DistrictTable/state").toByteArray();
    ui->DistrictTable->horizontalHeader()->restoreState(state);

    for(int i = 0; i < proxyModel.rowCount(); i++)
    {
        const QModelIndex index = proxyModel.mapToSource( proxyModel.index(i, 0) );
        int sourceRow = index.row();
        if (sourceRow == proxyModel.scrolledDistrict)
        {
            ui->DistrictTable->setCurrentIndex(proxyModel.index(i, 0));
        }
    }

}
void DistrictFrame::scrollToDistrict( int district_ind, bool makeVisible )
{
    if (makeVisible)
        proxyModel.scrolledDistrict = district_ind;
    else
        proxyModel.scrolledDistrict = -1;
   reInitialiseDistricts();
}
void DistrictFrame::on_sectionResized(int, int , int)
{
    QSettings settings;
    QByteArray state;

    state = ui->DistrictTable->horizontalHeader()->saveState();
    settings.setValue("DistrictTable/state", state);
}

DistrictGridModel::DistrictGridModel():
ct(nullptr)
{}
DistrictGridModel::~DistrictGridModel()
{
}
void DistrictGridModel::reset()
{
    beginResetModel();

    endResetModel();
}

void DistrictGridModel::initialise( )
{
   beginResetModel();

   // pick up the correct District list

   endResetModel();
}
QVariant DistrictGridModel::data( const QModelIndex &index, int role ) const
{

    if (role == Qt::BackgroundRole)
    {
        return QVariant();
    }
    if (role == Qt::TextAlignmentRole)
        return Qt::AlignLeft;

    if (role == Qt::DisplayRole)
    {
        QString disp;
        if (ct)
            disp = MultLists::getMultLists() ->getDistListText( index.row(), DistrictTreeColumns[ index.column() ].fieldId, ct );
        return disp;
    }
    return QVariant();
}
QVariant DistrictGridModel::headerData( int section, Qt::Orientation orientation,
                     int role ) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        QString cell;

        cell = DistrictTreeColumns[section].title;

        return cell;
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

QModelIndex DistrictGridModel::index( int row, int column, const QModelIndex &/*parent*/) const
{
    if ( row < 0 || row >= rowCount()  )
        return QModelIndex();

    return createIndex( row, column );
}

QModelIndex DistrictGridModel::parent( const QModelIndex &/*index*/ ) const
{
    return QModelIndex();
}

int DistrictGridModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    return MultLists::getMultLists() ->getDistListSize();
}

int DistrictGridModel::columnCount( const QModelIndex &/*parent*/ ) const
{
    return ectMultMaxCol - 1;
}
bool DistrictSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    if (scrolledDistrict == sourceRow)
        return true;
    LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>(TContestApp::getContestApp() ->getCurrentContest());

    if (!ct)
        return false;

    int worked = MultLists::getMultLists()->getDistWorked(sourceRow, ct) ;

    bool makeVisible = true;
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

void DistrictFrame::on_DistrictTable_clicked(const QModelIndex &index)
{
    const QModelIndex srcindex = proxyModel.mapToSource( index );
    int sourceRow = srcindex.row();

    QString disp = MultLists::getMultLists() ->getDistListText( sourceRow, 0, model.ct );
    MinosLoggerEvents::SendDistrictSelect(disp, model.ct);
}
