#include "base_pch.h"
#include "MinosLoggerEvents.h"

#include "LoggerContest.h"
#include "ContestApp.h"

#include "qvariant.h"
#include "tsinglelogframe.h"
#include "htmldelegate.h"

#include "dxccframe.h"
#include "ui_dxccframe.h"

QVector<GridColumn> DXCCGridModel::CountryTreeColumns =
{
    GridColumn( ectCall, "XXXXXX", QT_TR_NOOP("Call"), taLeftJustify ),
    GridColumn( ectWorked, "Wk CtX", QT_TR_NOOP("Wkd"), taCenter ),
    GridColumn( ectLocator, "MM00MM00", QT_TR_NOOP("Locator"), taLeftJustify ),
    GridColumn( ectBearing, "BRGXXX", QT_TR_NOOP("brg"), taCenter ),
    GridColumn( ectName, "This is a very long country", QT_TR_NOOP("Country"), taLeftJustify ),
    GridColumn( ectCQZone, "1234", QT_TR_NOOP("CQ"), taCenter ),
    GridColumn( ectITUZone, "1234", QT_TR_NOOP("ITU"), taCenter ),
    GridColumn( ectOtherCalls, "This is a very very very very long country name", QT_TR_NOOP("Other calls"), taLeftJustify /*taRightJustify*/ )
};

DXCCFrame::DXCCFrame(StackedInfoFrame *parent) :
    QFrame(parent),
    ui(new Ui::DXCCFrame),
    tslf(parent->tslf)
{
    ui->setupUi(this);
    ui->DXCCTable->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->DXCCTable->horizontalHeader()->setSectionsMovable(true);

    connect( ui->DXCCTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &DXCCFrame::onDXCCGrid_customContextMenuRequested );
    connect( ui->DXCCTable->horizontalHeader(), &QHeaderView::sectionMoved, this, &DXCCFrame::onDXCCGrid_sectionMoved);
    connect( ui->DXCCTable->horizontalHeader(), &QHeaderView::sectionResized, this, &DXCCFrame::on_sectionResized);

    proxyModel.setSourceModel(&model);
    ui->DXCCTable->setModel(&proxyModel);

    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    delegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate(1.0, lcf/100.0));
    model.delegate = delegate;

    ui->DXCCTable->setItemDelegate( delegate.data() );
    QSize ms = delegate->docSize("XX");
    ui->DXCCTable->verticalHeader()->setDefaultSectionSize(ms.height() );

    createColumnsMenu(columnsMenu, &model, this,
              [=]{
                    viewColumn();
              });
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::doColumnChanges, this, &DXCCFrame::on_doColumnChanges);

}
void DXCCFrame::viewColumn()
{
    // a columnsMenu entry has been clicked... action it
    QAction *act = dynamic_cast<QAction *>(sender());
    if (act)
    {
        int col = act->data().toInt();
        if (col >= 0)
        {
            bool check = act->isChecked();
            ui->DXCCTable->horizontalHeader()->setSectionHidden(col, !check);
        }
        else
        {
            QString fname("./Configuration/loggerTableHeaders.ini");
            resetHeaderColumns(fname, "DXCCTable", tslf->getCurScreenLayout(), ui->DXCCTable->horizontalHeader());
        }
    }
    saveDXCCTableColumns();
}
void DXCCFrame::saveDXCCTableColumns()
{
    if (!inRestoreColumns)
    {
        QString fname("./Configuration/loggerTableHeaders.ini");
        saveHeaderColumns(fname, "DXCCTable", tslf->getCurScreenLayout(), ui->DXCCTable->horizontalHeader());
        MinosLoggerEvents::SendColumnsChanged();
    }
}
void DXCCFrame::restoreDXCCTableColumns()
{
    inRestoreColumns = true;
    QString fname("./Configuration/loggerTableHeaders.ini");
    restoreHeaderColumns(fname, "DXCCTable", tslf->getCurScreenLayout(), ui->DXCCTable->horizontalHeader());
    inRestoreColumns = false;
}
void DXCCFrame::onDXCCGrid_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->DXCCTable->mapToGlobal( pos );
    popupColumnsMenu(columnsMenu, globalPos, ui->DXCCTable->horizontalHeader());
}
void DXCCFrame::onDXCCGrid_sectionMoved(int, int, int)
{
    saveDXCCTableColumns();
}
void DXCCFrame::on_sectionResized(int, int , int)
{
    saveDXCCTableColumns();
}
void DXCCFrame::on_doColumnChanges(BaseContestLog *b)
{
    if (b == model.ct)
    {
        restoreDXCCTableColumns();
    }
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
        band = contest->currentBand.getValue();
        model.band = band;
        proxyModel.band = band;

        reInitialiseCountries();

    }
}

void DXCCFrame::setBand(QString pband)
{
    band = pband;
    model.band = band;
    proxyModel.band = band;
    model.initialise();
}
void DXCCFrame::doScrollToCountry()
{
    model.reset(); // or we don't see unworked countries
    for(int i = 0; i < proxyModel.rowCount(); i++)
    {
        const QModelIndex index = proxyModel.mapToSource( proxyModel.index(i, 0) );
        int sourceRow = index.row();
        QSharedPointer<CountryEntry> ce = MultLists::getMultLists() ->getCountryList()[sourceRow];
        QString bp = ce->getBasePrefix();

        if (bp == proxyModel.scrolledCountry)
        {
            ui->DXCCTable->setCurrentIndex(proxyModel.index(i, 0));
        }
    }
}

void DXCCFrame::reInitialiseCountries()
{
    restoreDXCCTableColumns();

    doScrollToCountry();
}
void DXCCFrame::scrollToCountry( const QString &bp, bool makeVisible )
{
    if (makeVisible)
        proxyModel.scrolledCountry = bp;
    else
        proxyModel.scrolledCountry.clear();
   doScrollToCountry();
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
        if (role == Qt::DisplayRole)
        {
            QString bp = MultLists::getMultLists() ->getCountryList()[index.row()]->getBasePrefix();
            int ic = index.column();
            QString disp = MultLists::getMultLists() ->getCtryListText( bp, CountryTreeColumns[ ic].fieldId, ct, band );
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

        if (section >= 0)
        {
            cell = tr(CountryTreeColumns[section].title);
        }

        return cell;
    }
    if (role == Qt::TextAlignmentRole)
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

QModelIndex DXCCGridModel::index( int row, int column, const QModelIndex &/*parent*/) const
{
    if ( !ct || row < 0 || row >= rowCount()  )
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
    return CountryTreeColumns.count();
}
bool DXCCSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    LoggerContestLog * ct = dynamic_cast<LoggerContestLog *>(TContestApp::getContestApp() ->getCurrentContest());
    if (!ct)
        return false;

    QSharedPointer<CountryEntry> ce = MultLists::getMultLists() ->getCountryList()[sourceRow];

    QString bp = ce->getBasePrefix();
    if (scrolledCountry == bp)
        return true;

    //int worked = MultLists::getMultLists()->getCountryWorked(bp, ct) ;
    int worked = ct->getCountriesWorked(band, bp);

    bool makeVisible = false;
    for ( auto const &c: qAsConst(contlist ))
    {
        if ( ce->getContinent() == c.continent )
        {
            makeVisible = c.allow;
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

    QString bp = MultLists::getMultLists() ->getCountryList()[sourceRow]->getBasePrefix();
    QString disp = MultLists::getMultLists() ->getCtryListText( bp, 0, model.ct, model.band );
    MinosLoggerEvents::SendCountrySelect(disp, model.ct);

}
