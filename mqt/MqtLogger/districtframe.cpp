#include "base_pch.h"
#include "MinosLoggerEvents.h"

#include "LoggerContest.h"
#include "ContestApp.h"
#include "htmldelegate.h"
#include "tsinglelogframe.h"
#include "districtframe.h"
#include "ui_districtframe.h"

GridColumn DistrictGridModel::DistrictTreeColumns[ ectMultMaxCol - 1 ] =
   {
      GridColumn( ectCall, "XXXXXXX", QT_TR_NOOP("Code"), taLeftJustify ),
      GridColumn( ectWorked, "Wk CtX", QT_TR_NOOP("Wkd"), taCenter ),
      GridColumn( ectLocator, "MM00MM00", QT_TR_NOOP("Locator"), taLeftJustify ),
      GridColumn( ectBearing, "BRGXXX", QT_TR_NOOP("brg"), taCenter ),
      GridColumn( ectName, "This is a Very Very long District", QT_TR_NOOP("District"), taLeftJustify ),
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
    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0));
    model.delegate = delegate;
    ui->DistrictTable->setItemDelegate(delegate.data());
    proxyModel.setSourceModel(&model);
    ui->DistrictTable->setModel(&proxyModel);
    if (contest)
    {
        reInitialiseDistricts();
        connect( ui->DistrictTable->horizontalHeader(), &QHeaderView::sectionResized,
                 this, &DistrictFrame::on_sectionResized, Qt::UniqueConnection);
    }
    ui->DistrictTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
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
        QString cd = MultLists::getMultLists() ->getDistList()[sourceRow]->districtCode;
        if (cd == proxyModel.scrolledDistrict)
        {
            ui->DistrictTable->setCurrentIndex(proxyModel.index(i, 0));
        }
    }
}
void DistrictFrame::scrollToDistrict( const QString &cd, bool makeVisible )
{
    if (makeVisible)
        proxyModel.scrolledDistrict = cd;
    else
        proxyModel.scrolledDistrict.clear();
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
        {
            QString cd = MultLists::getMultLists() ->getDistList()[index.row()]->districtCode;
            disp = MultLists::getMultLists() ->getDistListText( cd, DistrictTreeColumns[ index.column() ].fieldId, ct );
        }
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

        cell = tr(DistrictTreeColumns[section].title);

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
    if ( !ct || row < 0 || row >= rowCount()  )
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
    LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>(TContestApp::getContestApp() ->getCurrentContest());

    if (!ct)
        return false;

    QString cd = MultLists::getMultLists() ->getDistList()[sourceRow]->districtCode;
    if (scrolledDistrict == cd)
        return true;

    int worked = MultLists::getMultLists()->getDistWorked(cd, ct) ;

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

    QString cd = MultLists::getMultLists() ->getDistList()[sourceRow]->districtCode;

    QString disp = MultLists::getMultLists() ->getDistListText( cd, 0, model.ct );
    MinosLoggerEvents::SendDistrictSelect(disp, model.ct);
}
