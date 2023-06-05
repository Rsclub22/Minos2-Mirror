#include <QSettings>
#include "regsettings.h"
#include "contacts.h"
#include "contest.h"
#include "cutils.h"
#include "BandList.h"

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
StatisticsSlot::StatisticsSlot (QDateTime current, int duration ) :
      dtStart ( current )
      , slotDuration ( duration )
{
    sstart = current.toString ( "hh:mm dd/MM/yyyy" );
}

//============================================================================================
SlotsModel::SlotsModel(StatisticsDisplay *s):sd(s)
{}

SlotsModel::~SlotsModel()
{}

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
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
    {
        QString cell;
        if (sd->contestSlots.size())
        {
            BandModeSlot &bms = sd->contestSlots[row].modesMap[sd->bandStrings[sd->curBand]].modes[sd->modeStrings[col /5 ] ];
            if (bms.QSOs > 0)
            {
                switch(col % 5)
                {
                case 0:
                    cell = QString::number(bms.QSOs);
                    break;
                case 1:
                    cell = QString::number(bms.points);
                    break;
                case 2:
                    cell = QString::number(bms.newMults);
                    break;
                case 3:
                    cell = QString::number(bms.bonus);
                    break;
                case 4:
                    cell = bms.ops.join(" ");
                    break;
                }
            }
        }
        if (role == Qt::ToolTipRole)
        {
            QString hdr = headerData(col, Qt::Horizontal, role).toString();
            cell = hdr + QString(" ") + cell;
        }
        return cell;
    }
    return QVariant();
}

QVariant SlotsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole)
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
        if (role == Qt::ToolTipRole)
        {
            cell = sd->bandStrings[sd->curBand] + " " + cell;
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

bool SlotsProxyModel::filterAcceptsRow(int /*source_row*/, const QModelIndex &/*source_parent*/) const
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
    ui(new Ui::StatisticsDisplay),
    ct(ct)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    trAll = tr("ALL");
    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("StatisticsDisplay/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    int interval = settings.getSettings().value("StatisticsDisplay/interval", 60).toInt();
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

    doRecalc();

    createColumnsMenu(columnsMenu, sm, this,
              [=]{
                    viewColumn();
              });

    restoreStatisticsTableColumns();

    connect( ui->StatsTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, [=]()
        {
            saveStatisticsTableColumns();
        }
    );


    if (spm)
    {
        spm->invalidate();
    }

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
    if (!rebuildingTabs)
    {
        curBand = index;
        if (spm)
        {
            spm->invalidate();
        }
    }
}
void StatisticsDisplay::moveEvent(QMoveEvent *event)
{
    RegSettings settings;
    settings.getSettings().setValue("StatisticsDisplay/geometry", saveGeometry());
    QDialog::moveEvent(event);
}
void StatisticsDisplay::resizeEvent(QResizeEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue("StatisticsDisplay/geometry", saveGeometry());
    QDialog::resizeEvent(event);
}
void StatisticsDisplay::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        RegSettings settings;
        settings.getSettings().setValue("StatisticsDisplay/geometry", saveGeometry());
    }
}
QString StatisticsDisplay::getIniKey()
{
    QString key = "Statistics";
    if (ct)
    {
        if (ct->contestBands.getValue() == allHF)
        {
            key = "MultiBand";
        }
        else
        {
            key = "SingleBand";
        }
    }
    return key;
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
            resetHeaderColumns(fname, "StatisticsTable", getIniKey(), ui->StatsTable->horizontalHeader());
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
        saveHeaderColumns(fname, "StatisticsTable", getIniKey(), ui->StatsTable->horizontalHeader());
    }
}
void StatisticsDisplay::restoreStatisticsTableColumns()
{
    inRestoreColumns = true;
    QString fname("./Configuration/LoggerTableHeaders.ini");
    restoreHeaderColumns(fname, "StatisticsTable", getIniKey(), ui->StatsTable->horizontalHeader());
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


void StatisticsDisplay::doRecalc()
{
    bandList.clear();
    contestSlots.clear();
    bandStrings.clear();
    modeStrings.clear();

    int interval = ui->MinutesSpinner->value();

    QDateTime  contestStart = CanonicalToTDT(ct->DTGStart.getValue());
    QDateTime  contestEnd = CanonicalToTDT(ct->DTGEnd.getValue());
    QDateTime current = contestStart;
    while ( current < contestEnd )
    {
        // we have already got interval from the constructor
        StatisticsSlot ss ( current, interval );
        contestSlots.push_back ( ss );
        current = current.addSecs( interval * 60 );
    }

    // First scan the contest to get all the time slots and all the band/mode slots
    for ( auto const &c: qAsConst(ct->ctList ))
    {
        QDateTime cdtg = c.wt->timeOff.getQDT();
        int sno = contestStart.secsTo(cdtg)/60;
        sno /= interval;
        if ( sno >= 0 && sno < contestSlots.size() )
        {
            QString band = c.wt->band;

            if (!contestSlots[sno].modesMap.contains(band))
            {
                contestSlots[sno].modesMap[band] = BandMode(band);
            }
            if (!contestSlots[sno].modesMap.contains(trAll))
            {
                contestSlots[sno].modesMap[trAll] = BandMode(trAll);
            }

            QString mode = c.wt->mode.getValue();

            if (!contestSlots[sno].modesMap[band].modes.contains(mode))
            {
                contestSlots[sno].modesMap[band].modes[mode] = BandModeSlot(band, mode);
            }
            if (!contestSlots[sno].modesMap[band].modes.contains(trAll))
            {
                contestSlots[sno].modesMap[band].modes[trAll] = BandModeSlot(band, trAll);
            }
            if (!contestSlots[sno].modesMap[trAll].modes.contains(trAll))
            {
                contestSlots[sno].modesMap[trAll].modes[trAll] = BandModeSlot(trAll, trAll);
            }
            if (!contestSlots[sno].modesMap[trAll].modes.contains(mode))
            {
                contestSlots[sno].modesMap[trAll].modes[mode] = BandModeSlot(trAll, mode);
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
            if (!bandStrings.contains(trAll))
            {
                bandStrings.append(trAll);
            }
            QString band = b.band;
            if (!bandStrings.contains(band))
            {
                bandStrings.append(band);
                bandStrings.sort();
            }
            for (auto const &m: qAsConst(b.modes))
            {
                QString mode = m.mode;
                if (!modeStrings.contains(trAll))
                {
                    modeStrings.append(trAll);
                }
                if (!modeStrings.contains(mode))
                {
                    modeStrings.append(mode);
                    modeStrings.sort();
                }
                if (!bandList[band].modes.contains(trAll))
                {
                    bandList[band].modes.append(trAll);
                }
                if (!bandList[band].modes.contains(mode))
                {
                    bandList[band].modes.append(mode);
                    bandList[band].modes.sort();
                }
            }
        }
    }
//    BandModeSlot &bms = contestSlots[row].modesMap[bandStrings[curBand]].modes[modeStrings[moffset ] ];
    for (auto &c:contestSlots)
    {
        BandMode &allBandsModes = c.modesMap[trAll];  // map of modes for band
        BandModeSlot &allBandsallModesSlot = allBandsModes.modes[trAll];
        for (auto &b: c.modesMap)
        {
            if (b.band != trAll)
            {
                BandModeSlot &allModesSlot = b.modes[trAll];
                for (auto &m: b.modes)
                {
                    if (m.mode != trAll)
                    {
                        allBandsModes.modes[m.mode].QSOs += m.QSOs;
                        allBandsallModesSlot.QSOs += m.QSOs;
                        allModesSlot.QSOs += m.QSOs;

                        allBandsModes.modes[m.mode].bonus += m.bonus;
                        allBandsallModesSlot.bonus += m.bonus;
                        allModesSlot.bonus += m.bonus;

                        allBandsModes.modes[m.mode].newMults += m.newMults;
                        allBandsallModesSlot.newMults += m.newMults;
                        allModesSlot.newMults += m.newMults;

                        allBandsModes.modes[m.mode].bonus += m.bonus;
                        allBandsallModesSlot.bonus += m.bonus;
                        allModesSlot.bonus += m.bonus;

                        allBandsModes.modes[m.mode].points += m.points;
                        allBandsallModesSlot.points += m.points;
                        allModesSlot.points += m.points;

                        //AND we need to merge operators

                        allBandsModes.modes[m.mode].ops.append(m.ops);
                        allBandsModes.modes[m.mode].ops.sort();
                        allBandsModes.modes[m.mode].ops.removeDuplicates();

                        allBandsallModesSlot.ops.append(m.ops);
                        allBandsallModesSlot.ops.sort();
                        allBandsallModesSlot.ops.removeDuplicates();

                        allModesSlot.ops.append(m.ops);
                        allModesSlot.ops.sort();
                        allModesSlot.ops.removeDuplicates();
                    }
                }
            }
        }
    }

    QVector<QSharedPointer<BandInfo>> vbi;
    // need to get bandinfo for each bandstring, then sort it decently in frequency order

    BandList &blist = BandList::getBandList();
    for(auto const &b:qAsConst(bandStrings))
    {
        QSharedPointer<BandInfo>  bi;
        bool bandOK = blist.findBand(b, bi);
        if (bandOK)
        {
            vbi.push_back(bi);
        }
        else
        {
            // what if we can't find the band?
            QSharedPointer<BandInfo> badbi(new BandInfo());
            badbi->uk = b;
            vbi.push_back(badbi);
        }
    }

    std::sort(vbi.begin(), vbi.end(),
          [=](const QSharedPointer<BandInfo> a, const QSharedPointer<BandInfo> b)->bool
            {
                return *a < *b;
            }
          );
    bandStrings.clear();

    for(const auto &b:vbi)
    {
        bandStrings.push_back(b->name());
    }

    rebuildingTabs = true;

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
    rebuildingTabs = false;
}
void StatisticsDisplay::on_RecalcButton_clicked()
{
    doRecalc();

    if (spm)
    {
        spm->invalidate();
    }
}


void StatisticsDisplay::on_MinutesSpinner_textChanged(const QString &)
{
    int interval = ui->MinutesSpinner->value();
    RegSettings settings;
    settings.getSettings().setValue("StatisticsDisplay/interval", interval);

}

