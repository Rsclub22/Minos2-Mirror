#include "AppStartup.h"
#include "MinosLoggerEvents.h"

#include "LoggerContest.h"
#include "ContestApp.h"

#include "qvariant.h"
#include "tsinglelogframe.h"
#include "htmldelegate.h"

#include "StackedInfoFrame.h"
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
static QVector<ContList> contlist =
    {
        ContList("EU", true),
        ContList("AS", false),
        ContList("AF", false),
        ContList("OC", false),
        ContList("SA", false),
        ContList("NA", false)
};

DXCCFrame::DXCCFrame(StackedInfoFrame *parent) :
    QFrame(parent),
    tslf(parent->tslf),
    ui(new Ui::DXCCFrame)
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
    delegate = QSharedPointer<HtmlDelegate>(new HtmlDelegate("DXCCFrame", 1.0, lcf/100.0, this));
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
            QString fname(getDirectoryLocation(dlConfiguration) + "/LoggerTableHeaders.ini");
            resetHeaderColumns(fname, "DXCCTable", tslf->getCurScreenLayout(), ui->DXCCTable->horizontalHeader());
        }
    }
    saveDXCCTableColumns();
}
void DXCCFrame::saveDXCCTableColumns()
{
    if (!inRestoreColumns)
    {
        QString fname(getDirectoryLocation(dlConfiguration) + "/LoggerTableHeaders.ini");
        saveHeaderColumns(fname, "DXCCTable", tslf->getCurScreenLayout(), ui->DXCCTable->horizontalHeader());
        MinosLoggerEvents::SendColumnsChanged();
    }
}
void DXCCFrame::restoreDXCCTableColumns()
{
    inRestoreColumns = true;
    QString fname(getDirectoryLocation(dlConfiguration) + "/LoggerTableHeaders.ini");
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
    ct = contest;
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
    ui->c1Button->setText(tr("EU"));
    ui->c2Button->setText(tr("AS"));
    ui->c3Button->setText(tr("AF"));
    ui->c4Button->setText(tr("OC"));
    ui->c5Button->setText(tr("SA"));
    ui->c6Button->setText(tr("NA"));
    ui->wkdButton->setText(tr("wkd"));
    ui->unwkdButton->setText(tr("unwkd"));
    if (ct)
    {
        contlist[ 0 ].allow = ct->showContinentEU.getValue();
        contlist[ 1 ].allow = ct->showContinentAS.getValue();
        contlist[ 2 ].allow = ct->showContinentAF.getValue();
        contlist[ 3 ].allow = ct->showContinentOC.getValue();
        contlist[ 4 ].allow = ct->showContinentSA.getValue();
        contlist[ 5 ].allow = ct->showContinentNA.getValue();

        ui->c1Button->setChecked(ct->showContinentEU.getValue());
        ui->c2Button->setChecked(ct->showContinentAS.getValue());
        ui->c3Button->setChecked(ct->showContinentAF.getValue());
        ui->c4Button->setChecked(ct->showContinentOC.getValue());
        ui->c5Button->setChecked(ct->showContinentSA.getValue());
        ui->c6Button->setChecked(ct->showContinentNA.getValue());

        ui->wkdButton->setChecked(ct->showWorkedCountries.getValue());
        ui->unwkdButton->setChecked(ct->showUnworkedCountries.getValue());
    }
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
        if (role == Qt::ToolTipRole)
        {
            QSharedPointer<CountryEntry> ce = MultLists::getMultLists() ->getCountryList()[index.row()];
            QString bp = ce->getBasePrefix();
            QString disp = MultLists::getMultLists() ->getCtryListText( bp, ectCall, ct, band );
            QString cont = ce->getContinent();
            disp = disp + "(" + cont + ")";
            return disp;
        }
        if (role == Qt::DisplayRole)
        {
            QSharedPointer<CountryEntry> ce = MultLists::getMultLists() ->getCountryList()[index.row()];
            QString bp = ce->getBasePrefix();
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

    int worked = ct->getCountriesWorked(band, bp);

    bool makeVisible = false;
    for ( auto const &c: QASCONST(contlist ))
    {
        if ( ce->getContinent() == c.continent )
        {
            makeVisible = c.allow;
            break;
        }
    }

    if ( worked && ct->showUnworkedCountries.getValue() && !ct->showWorkedCountries.getValue() )
    {
        makeVisible = false;
    }
    else
        if ( !worked && !ct->showUnworkedCountries.getValue() && ct->showWorkedCountries.getValue() )
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

void DXCCFrame::on_c1Button_clicked()
{
    ct->showContinentEU.setValue(!ct->showContinentEU.getValue());
    ct->commonSave(false);
    reInitialiseCountries();
}
void DXCCFrame::on_c2Button_clicked()
{
    ct->showContinentAS.setValue(!ct->showContinentAS.getValue());
    ct->commonSave(false);
    reInitialiseCountries();
}
void DXCCFrame::on_c3Button_clicked()
{
    ct->showContinentAF.setValue(!ct->showContinentAF.getValue());
    ct->commonSave(false);
    reInitialiseCountries();
}
void DXCCFrame::on_c4Button_clicked()
{
    ct->showContinentOC.setValue(!ct->showContinentOC.getValue());
    ct->commonSave(false);
    reInitialiseCountries();
}
void DXCCFrame::on_c5Button_clicked()
{
    ct->showContinentSA.setValue(!ct->showContinentSA.getValue());
    ct->commonSave(false);
    reInitialiseCountries();
}
void DXCCFrame::on_c6Button_clicked()
{
    ct->showContinentNA.setValue(!ct->showContinentNA.getValue());
    ct->commonSave(false);
    reInitialiseCountries();
}
void DXCCFrame::on_wkdButton_clicked()
{
    ct->showWorkedCountries.setValue(!ct->showWorkedCountries.getValue());
    ct->commonSave(false);
    reInitialiseCountries();
}
void DXCCFrame::on_unwkdButton_clicked()
{
    ct->showUnworkedCountries.setValue(!ct->showUnworkedCountries.getValue());
    ct->commonSave(false);
    reInitialiseCountries();
}

