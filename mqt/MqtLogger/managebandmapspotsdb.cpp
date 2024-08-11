#include <QSettings>
#include "bandmapspotdb.h"
#include "managebandmapspotsdb.h"
#include "regsettings.h"
#include "ui_managebandmapspotsdb.h"

ManageBandmapSpotsDb::ManageBandmapSpotsDb(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::manageBandmapSpotsDb)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->contestTable->setSelectionMode(QAbstractItemView::ExtendedSelection);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("ManageBMSpotsDb/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    // get all contests from DB
    bmsdb = QSharedPointer<BandMapSpotDB>(new BandMapSpotDB());

    contests = bmsdb->getContests();

    dbModel.setContests(&contests);

    ui->contestTable->setModel(&dbModel);
    ui->contestTable->resizeColumnsToContents();

}

ManageBandmapSpotsDb::~ManageBandmapSpotsDb()
{
    delete ui;
}
void ManageBandmapSpotsDb::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("ManageBMSpotsDb/geometry", saveGeometry());
}
void ManageBandmapSpotsDb::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void ManageBandmapSpotsDb::accept()
{
    doCloseEvent();
    QDialog::accept();
}
void ManageBandmapSpotsDb::on_OKButton_clicked()
{
    // delete entries for selected contests
    QItemSelectionModel *qism = ui->contestTable->selectionModel();
    QModelIndexList mil;
    if (qism)
        mil =  qism->selectedRows();
    QStringList names;
    for(auto &s:mil)
    {
        int r = s.row();
        ContestDbEntry dbe = contests.at(r);
        names.append(dbe.name);
    }
    bmsdb->deleteRecords(names);
    accept();
}

void ManageBandmapSpotsDb::on_cancelButton_clicked()
{
    // do nothing
    reject();
}

ManageBandmapSpotsDbModel::ManageBandmapSpotsDbModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

void ManageBandmapSpotsDbModel::setContests(QVector<ContestDbEntry> *pContests)
{
    contests = pContests;
}


QModelIndex ManageBandmapSpotsDbModel::index(int row, int column, const QModelIndex &parent) const
{
    if ( row < 0 || row >= rowCount() || ( parent.isValid() && parent.column() != 0 ) )
        return QModelIndex();

    return createIndex( row, column, nullptr );
}

QModelIndex ManageBandmapSpotsDbModel::parent(const QModelIndex &/*index*/) const
{
    return QModelIndex();
}

int ManageBandmapSpotsDbModel::rowCount(const QModelIndex &/*parent*/) const
{
    return contests->size();
}

int ManageBandmapSpotsDbModel::columnCount(const QModelIndex &/*parent*/) const
{
    return eDBcMax;
}


QVariant ManageBandmapSpotsDbModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        switch(section)
        {
        case edbcName:
            return tr("DB Name");

        case edbcItems:
            return tr("Entries");

        }
    }
    return QVariant();
}

QVariant ManageBandmapSpotsDbModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();

    if ( role == Qt::DisplayRole )
    {
        switch(index.column())
        {
        case edbcName:
            return (*contests)[row].name;

        case edbcItems:
            return (*contests)[row].entries;

        }
    }
    return QVariant();
}


