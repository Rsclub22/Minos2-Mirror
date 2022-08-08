#include "base_pch.h"
#include "MinosLoggerEvents.h"

#include "LoggerContest.h"
#include "ContestApp.h"
#include "htmldelegate.h"
#include "tsinglelogframe.h"
#include "StackedInfoFrame.h"
#include "districtframe.h"
#include "ui_districtframe.h"

QVector<GridColumn> DistrictGridModel::DistrictTreeColumns =
   {
      GridColumn( ectCall, "XXXXXXX", QT_TR_NOOP("Code"), taLeftJustify ),
      GridColumn( ectWorked, "Wk CtX", QT_TR_NOOP("Wkd"), taCenter ),
      GridColumn( ectLocator, "MM00MM00", QT_TR_NOOP("Locator"), taLeftJustify ),
      GridColumn( ectBearing, "BRGXXX", QT_TR_NOOP("brg"), taCenter ),
      GridColumn( ectName, "This is a Very Very long District", QT_TR_NOOP("District"), taLeftJustify )
   };

DistrictFrame::DistrictFrame(StackedInfoFrame *parent) :
    QFrame(parent),
    ui(new Ui::DistrictFrame),
    tslf(parent->tslf)
{
    ui->setupUi(this);
    ui->DistrictTable->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->DistrictTable->horizontalHeader()->setSectionsMovable(true);

    connect( ui->DistrictTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &DistrictFrame::onDistrictGrid_customContextMenuRequested );
    connect( ui->DistrictTable->horizontalHeader(), &QHeaderView::sectionMoved, this, &DistrictFrame::onDistrictGrid_sectionMoved);
    connect( ui->DistrictTable->horizontalHeader(), &QHeaderView::sectionResized, this, &DistrictFrame::on_sectionResized);

    proxyModel.setSourceModel(&model);
    ui->DistrictTable->setModel(&proxyModel);

    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0));
    model.delegate = delegate;

    ui->DistrictTable->setItemDelegate( delegate.data() );
    QSize ms = delegate->docSize("XX");
    ui->DistrictTable->verticalHeader()->setDefaultSectionSize(ms.height() );

    createColumnsMenu(columnsMenu, &model, this,
              [=]{
                    viewColumn();
              });

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::doColumnChanges, this, &DistrictFrame::on_doColumnChanges);

}

DistrictFrame::~DistrictFrame()
{
    delete ui;
}

void DistrictFrame::viewColumn()
{
    // a columnsMenu entry has been clicked... action it
    QAction *act = dynamic_cast<QAction *>(sender());
    if (act)
    {
        int col = act->data().toInt();
        if (col >= 0)
        {
            bool check = act->isChecked();
            ui->DistrictTable->horizontalHeader()->setSectionHidden(col, !check);
        }
        else
        {
            QString fname("./Configuration/LoggerTableHeaders.ini");
            resetHeaderColumns(fname, "DistrictTable", tslf->getCurScreenLayout(), ui->DistrictTable->horizontalHeader());
        }
    }
    saveDistrictTableColumns();
}
void DistrictFrame::saveDistrictTableColumns()
{
    if (!inRestoreColumns)
    {
        QString fname("./Configuration/LoggerTableHeaders.ini");
        saveHeaderColumns(fname, "DistrictTable", tslf->getCurScreenLayout(), ui->DistrictTable->horizontalHeader());
        MinosLoggerEvents::SendColumnsChanged();
    }
}
void DistrictFrame::restoreDistrictTableColumns()
{
    inRestoreColumns = true;
    QString fname("./Configuration/LoggerTableHeaders.ini");
    restoreHeaderColumns(fname, "DistrictTable", tslf->getCurScreenLayout(), ui->DistrictTable->horizontalHeader());
    inRestoreColumns = false;
}
void DistrictFrame::onDistrictGrid_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->DistrictTable->mapToGlobal( pos );
    popupColumnsMenu(columnsMenu, globalPos, ui->DistrictTable->horizontalHeader());
}
void DistrictFrame::onDistrictGrid_sectionMoved(int, int, int)
{
    saveDistrictTableColumns();
}
void DistrictFrame::on_sectionResized(int, int , int)
{
    saveDistrictTableColumns();
}
void DistrictFrame::on_doColumnChanges(BaseContestLog *b)
{
    if (b == model.ct)
    {
        restoreDistrictTableColumns();
    }
}

void DistrictFrame::setContest(BaseContestLog *contest)
{
    model.ct = contest;

    if (contest)
    {
        band = contest->currentBand.getValue();
        model.band = band;
        proxyModel.band = band;

        reInitialiseDistricts();
    }
}

void DistrictFrame::setBand(QString band)
{
    model.band = band;
    proxyModel.band = band;
    model.initialise();
}
void DistrictFrame::doScrollToDistrict()
{
    model.reset();  // or we don't see unworked districts
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

void DistrictFrame::reInitialiseDistricts()
{
    restoreDistrictTableColumns();

    doScrollToDistrict();
}
void DistrictFrame::scrollToDistrict( const QString &cd, bool makeVisible )
{
    if (makeVisible)
        proxyModel.scrolledDistrict = cd;
    else
        proxyModel.scrolledDistrict.clear();

    doScrollToDistrict();
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
            disp = MultLists::getMultLists() ->getDistListText( cd, DistrictTreeColumns[ index.column() ].fieldId, ct, band );
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

        if (section >= 0)
        {
            cell = tr(DistrictTreeColumns[section].title);
        }

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
            if (section >= 0)
            {

                QString s = data(index(section, 0), Qt::DisplayRole).toString();
                QSize r = delegate->docSize(s);
                r.setWidth(0);
                return r;
            }
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
    return DistrictTreeColumns.count();
}
bool DistrictSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>(TContestApp::getContestApp() ->getCurrentContest());

    if (!ct)
        return false;

    QString cd = MultLists::getMultLists() ->getDistList()[sourceRow]->districtCode;
    if (scrolledDistrict == cd)
        return true;

    int worked = MultLists::getMultLists()->getDistWorked(cd, ct, band) ;

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

    QString disp = MultLists::getMultLists() ->getDistListText( cd, 0, model.ct, model.band );
    MinosLoggerEvents::SendDistrictSelect(disp, model.ct);
}
