#include "base_pch.h"
#include "contacts.h"
#include "contest.h"
#include "cutils.h"

#include "StatisticsDisplay.h"
#include "ui_StatisticsDisplay.h"
//============================================================================================ (
/*
To Do:
Interval saving
Interval rationalise (30 mins/60 mins/24 hours?)

band saving

Column saving
Column dragging and saving
"natural" sort order for bands (and wavelength?)

Export as CSV

Filtering

Added breaks on operator change
*/

//============================================================================================
StatisticsSlot::StatisticsSlot ( QDateTime current, int slot, int duration ) :
        slotStart ( slot )
      ,  dtStart ( current )
      , slotDuration ( duration )
{
    sstart = current.toString ( "hh:mm dd/MM/yyyy" );
    sstart2 = current.toString ( "yyMMddhhmm" );

}

//============================================================================================
SlotsModel::SlotsModel(StatisticsDisplay *s):sd(s)
{}

SlotsModel::~SlotsModel()
{}
void SlotsModel::reset()
{
    beginResetModel();
    endResetModel();
}

int SlotsModel::rowCount(const QModelIndex &/*parent*/) const
{
    return sd->contestSlots.size();
}

int SlotsModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 5 * sd->modeStrings.size();   // cols in a band/mode slot
}

QVariant SlotsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() >= sd->contestSlots.size() || index.row() < 0)
    {
             return QVariant();
    }

    int col = index.column();
    int row = index.row();
    if (role == Qt::DisplayRole)
    {
        QVariant cell;
        if (sd->contestSlots.size())
        {
            BandModeSlot &bms = sd->contestSlots[row].modesMap[sd->bandStrings[sd->curBand]].modes[sd->modeStrings[col /5 ] ];
            if (bms.QSOs > 0)
            {
                switch(col % 5)
                {
                case 0:
                    cell = bms.QSOs;
                    break;
                case 1:
                    cell = bms.points;
                    break;
                case 2:
                    cell = bms.newMults;
                    break;
                case 3:
                    cell = bms.bonus;
                    break;
                case 4:
                    cell = bms.ops.join(" ");
                    break;
                }
            }
        }
        return cell;
    }
    return QVariant();
}

QVariant SlotsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        QString cell;
        if ( orientation == Qt::Horizontal)
        {
            if (sd->contestSlots.size())
            {
                switch(section % 5)
                {
                case 0:
                    cell = "QSOs";
                    break;
                case 1:
                    cell = "Points";
                    break;
                case 2:
                    cell = "Mults";
                    break;
                case 3:
                    cell = "Bonus";
                    break;
                case 4:
                    cell = "Operators";
                    break;
                }
                cell = sd->modeStrings[section/5] + " " + cell;
            }
        }
        else
        {
            cell = sd->contestSlots[section].sstart;
        }
        return cell;
    }
    return QVariant();
}

//============================================================================================
SlotsProxyModel::SlotsProxyModel()
{}

SlotsProxyModel::~SlotsProxyModel()
{}

bool SlotsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    return true;
}

bool SlotsProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    //Model Indices are to the SOURCE model

    QAbstractItemModel *aim = sourceModel();
    SlotsModel *sm = dynamic_cast<SlotsModel *>(aim);

    QVariant l = sm->data(source_left, Qt::DisplayRole);
    QVariant r = sm->data(source_right, Qt::DisplayRole);

    if (source_right.column() == 5)
    {
        return (l.toString() < r.toString());
    }
    else
    {
        return (l.toInt() < r.toInt());
    }

}
//============================================================================================

StatisticsDisplay::StatisticsDisplay(BaseContestLog *ct, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StatisticsDiplay),
    ct(ct)
{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("StatisticsDisplay/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    int interval = settings.value("StatisticsDisplay/interval", 60).toInt();
    ui->MinutesSpinner->setValue(interval);

    sm = new SlotsModel(this);
    spm = new SlotsProxyModel();

    spm->setSourceModel(sm);
    ui->StatsTable->setModel(spm);

    ui->StatsTable->horizontalHeader()->setVisible(true);
    ui->StatsTable->verticalHeader()->setVisible(true);

    ui->tabBar->setCurrentIndex( 0 );

    connect( ui->tabBar , &QTabBar::currentChanged, this, &StatisticsDisplay::on_currentTabChangedSlot );

    ui->StatsTable->horizontalHeader()->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->StatsTable->horizontalHeader()->setSectionsMovable(true);

    connect( ui->StatsTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &StatisticsDisplay::onStatisticsGrid_customContextMenuRequested );
    connect( ui->StatsTable->horizontalHeader(), &QHeaderView::sectionMoved, this, &StatisticsDisplay::onStatisticsGrid_sectionMoved);
    connect( ui->StatsTable->horizontalHeader(), &QHeaderView::sectionResized, this, &StatisticsDisplay::on_sectionResized);

    createColumnsMenu(columnsMenu, sm, this,
              [=]{
                    viewColumn();
              });

    restoreStatisticsTableColumns();

}
StatisticsDisplay::~StatisticsDisplay()
{
    delete ui;

    delete sm;
    sm = nullptr;
    delete spm;
    spm = nullptr;
}
void StatisticsDisplay::on_currentTabChangedSlot(int index)
{
    curBand = index;
    if (spm)
    {
        spm->invalidate();
    }
}
void StatisticsDisplay::moveEvent(QMoveEvent *event)
{
    QSettings settings;
    settings.setValue("StatisticsDisplay/geometry", saveGeometry());
    QDialog::moveEvent(event);
}
void StatisticsDisplay::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("StatisticsDisplay/geometry", saveGeometry());
    QDialog::resizeEvent(event);
}
void StatisticsDisplay::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("StatisticsDisplay/geometry", saveGeometry());
    }
}
void StatisticsDisplay::viewColumn()
{
    // a columnsMenu entry has been clicked... action it
    QAction *act = dynamic_cast<QAction *>(sender());
    if (act)
    {
        int col = act->data().toInt();
        if (col >= 0)
        {
            bool check = act->isChecked();
            ui->StatsTable->horizontalHeader()->setSectionHidden(col, !check);
        }
        else
        {
            QString fname("./Configuration/LoggerTableHeaders.ini");
            resetHeaderColumns(fname, "StatisticsTable", "Statistics", ui->StatsTable->horizontalHeader());
        }
    }
    saveStatisticsTableColumns();
}

// This isn't quite right; each CONTEST will be different! Maybe save to .minos
// rather than file? So long as it isn't protected!

void StatisticsDisplay::saveStatisticsTableColumns()
{
    if (!inRestoreColumns)
    {
        QString fname("./Configuration/LoggerTableHeaders.ini");
        saveHeaderColumns(fname, "StatisticsTable", "Statistics", ui->StatsTable->horizontalHeader());
    }
}
void StatisticsDisplay::restoreStatisticsTableColumns()
{
    inRestoreColumns = true;
    QString fname("./Configuration/LoggerTableHeaders.ini");
    restoreHeaderColumns(fname, "StatisticsTable", "Statistics", ui->StatsTable->horizontalHeader());
    inRestoreColumns = false;
}
void StatisticsDisplay::onStatisticsGrid_customContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = ui->StatsTable->mapToGlobal( pos );
    popupColumnsMenu(columnsMenu, globalPos, ui->StatsTable->horizontalHeader());
}
void StatisticsDisplay::onStatisticsGrid_sectionMoved(int, int, int)
{
    saveStatisticsTableColumns();
}
void StatisticsDisplay::on_sectionResized(int, int , int)
{
    saveStatisticsTableColumns();
}
void StatisticsDisplay::on_CloseButton_clicked()
{
    close();
}


void StatisticsDisplay::on_RecalcButton_clicked()
{
    bandList.clear();
    contestSlots.clear();
    bandStrings.clear();
    modeStrings.clear();

    int istart =  DTGToInt ( ct->DTGStart.getValue() );

    int interval = ui->MinutesSpinner->value();

    QDateTime  contestStart = CanonicalToTDT(ct->DTGStart.getValue());
    QDateTime  contestEnd = CanonicalToTDT(ct->DTGEnd.getValue());
    QDateTime current = contestStart;
    int istart2 = istart;
    while ( current < contestEnd )
    {
        // we have already got interval from the constructor
        StatisticsSlot ss ( current, istart2, interval );
        contestSlots.push_back ( ss );
        current = current.addSecs( interval * 60 );
        istart2 += interval;
    }

    // First scan the contest to get all the time slots and all the band/mode slots
    for ( auto const &c: qAsConst(ct->ctList ))
    {
        int cdtg = DTGToInt ( c.wt->timeOff.getQDT() );
        int sno = cdtg - istart;
        sno /= interval;
        if ( sno >= 0 && sno < contestSlots.size() )
        {
            QString band;
            ct->getTxFreqBand(c.wt->frequency.getValue(), band);

            if (!contestSlots[sno].modesMap.contains(band))
            {
                contestSlots[sno].modesMap[band] = BandMode(band);
            }

            QString mode = c.wt->mode.getValue();

            if (!contestSlots[sno].modesMap[band].modes.contains(mode))
            {
                contestSlots[sno].modesMap[band].modes[mode] = BandModeSlot(mode);
            }

            BandModeSlot &bms = contestSlots[sno].modesMap[band].modes[mode];
            bms.QSOs++;
            bms.points += c.wt->contactScore.getValue();
            bms.bonus += c.wt->bonus;
            bms.newMults += c.wt->multCount;
            QString op = c.wt->op1.getValue();
            if (!bms.ops.contains(op))
            {
                bms.ops.append(op);
            }
        }

    }

    for (auto const &c:qAsConst(contestSlots))
    {
        for (auto const &b: qAsConst(c.modesMap))
        {
            QString band = b.band;
            if (!bandStrings.contains(band))
            {
                bandStrings.append(band);
                bandStrings.sort();
            }
            for (auto const &m: qAsConst(b.modes))
            {
                QString mode = m.mode;
                if (!modeStrings.contains(mode))
                {
                    modeStrings.append(mode);
                    modeStrings.sort();
                }
                if (!bandList[band].modes.contains(mode))
                {
                    bandList[band].modes.append(mode);
                    bandList[band].modes.sort();
                }
            }
        }
    }

    while ( ui->tabBar->count() > 0 )
    {
        ui->tabBar->removeTab( ui->tabBar->count() - 1 );
    }

    curBand = 0;
    if (bandStrings.size() > 1)
    {
        for(auto const &b:qAsConst(bandStrings))
        {
            ui->tabBar->addTab( b );
        }
        ui->tabBar->setVisible(true);
    }
    else
    {
        ui->tabBar->setVisible(false);
    }

    if (spm)
    {
        spm->invalidate();
    }
}


void StatisticsDisplay::on_MinutesSpinner_textChanged(const QString &)
{
    int interval = ui->MinutesSpinner->value();
    QSettings settings;
    settings.setValue("StatisticsDisplay/interval", interval);

    on_RecalcButton_clicked();
}

