#include "StatisticsDiplay.h"
#include "ui_StatisticsDiplay.h"

#include "base_pch.h"
#include "contacts.h"
#include "contest.h"

//============================================================================================ (
/*
To Do:
Interval saving
Interval rationalise (30 mins/60 mins/24 hours?)

Column saving
Column dragging and saving
"natural" sort order for bands (and wavelength?)

Export as CSV

Filtering

Is there any way of compressing things? Do we need to? (band tabs!)

Added breaks on operator change

Sort on a column by each component - or separate the columns (would raise column naming problems
Or band tabs (like it!) for each band (and overall)
*/
//============================================================================================
class BandModeSlot
{
public:
    BandModeSlot(const QString m):mode(m){}
    BandModeSlot(){}
    QString mode;
    QStringList ops;
    int QSOs = 0;
    int points = 0;
    int newMults = 0;
    int bonus = 0;

    QString toString()
    {
        if (QSOs == 0)
        {
            return QString();
        }
        QString res = QString("Q %1 P %2 M %3 B %4 %5").arg(QSOs).arg(points).arg(newMults).arg(bonus).arg(ops.join(" "));
        return res;
    }
};
//============================================================================================
class BandMode
{
public:
    BandMode(const QString b):band(b){}
    BandMode(){}
    QString band;
    QMap <QString, BandModeSlot> modes;
};

//============================================================================================
class StatisticsSlot
{
public:
public:
    int slotStart = 0;
    QDateTime dtStart;
    QString sstart;
    QString sstart2;
    int slotDuration = 0;

    QMap<QString, BandMode> modesMap;   // mode slots by band

    StatisticsSlot(){}  // for QVector
    StatisticsSlot ( QDateTime current, int slot, int duration );
};
StatisticsSlot::StatisticsSlot ( QDateTime current, int slot, int duration ) :
        slotStart ( slot )
      ,  dtStart ( current )
      , slotDuration ( duration )
{
    sstart = current.toString ( "hh:mm dd/MM/yyyy" );
    sstart2 = current.toString ( "yyMMddhhmm" );

}

//============================================================================================
class Band
{
public:
    QStringList modes;
};

QMap<QString, Band> bandList;
int curBand = 0;

QVector<StatisticsSlot> contestSlots;

QStringList bandStrings;
QStringList modeStrings;
//============================================================================================
class SlotsModel: public QAbstractTableModel
{
public:
    SlotsModel();
    virtual ~SlotsModel() override;

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    void reset();
};
SlotsModel::SlotsModel()
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
    return contestSlots.size();
}

int SlotsModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 5 * modeStrings.size();   // cols in a band/mode slot
}

QVariant SlotsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() >= contestSlots.size() || index.row() < 0)
    {
             return QVariant();
    }

    int col = index.column();
    int row = index.row();
    if (role == Qt::DisplayRole)
    {
        QVariant cell;
        if (contestSlots.size())
        {
            BandModeSlot &bms = contestSlots[row].modesMap[bandStrings[curBand]].modes[modeStrings[col /5 ] ];
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
            if (contestSlots.size())
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
                cell = modeStrings[section/5] + " " + cell;
            }
        }
        else
        {
            cell = contestSlots[section].sstart;
        }
        return cell;
    }
    return QVariant();
}

//============================================================================================
class SlotsProxyModel: public QSortFilterProxyModel
{
public:
    SlotsProxyModel();
    virtual ~SlotsProxyModel();

    // QSortFilterProxyModel interface
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};
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
    return source_left.row() < source_right.row();
}
//============================================================================================

StatisticsDiplay::StatisticsDiplay(BaseContestLog *ct, QWidget *parent) :
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

    SlotsModel *sm = new SlotsModel();
    SlotsProxyModel *spm = new SlotsProxyModel();

    spm->setSourceModel(sm);
    ui->StatsTable->setModel(spm);

    ui->StatsTable->horizontalHeader()->setVisible(true);
    ui->StatsTable->verticalHeader()->setVisible(true);

    ui->tabBar->setCurrentIndex( 0 );

    connect( ui->tabBar , &QTabBar::currentChanged, this, &StatisticsDiplay::on_currentTabChangedSlot );


}
void StatisticsDiplay::on_currentTabChangedSlot(int index)
{
    curBand = index;
    SlotsProxyModel * spm = dynamic_cast<SlotsProxyModel *>(ui->StatsTable->model());
    if (spm)
    {
        spm->invalidate();
    }
}
StatisticsDiplay::~StatisticsDiplay()
{
    contestSlots.clear();
    delete ui;
}
void StatisticsDiplay::moveEvent(QMoveEvent *event)
{
    QSettings settings;
    settings.setValue("StatisticsDisplay/geometry", saveGeometry());
    QDialog::moveEvent(event);
}
void StatisticsDiplay::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("StatisticsDisplay/geometry", saveGeometry());
    QDialog::resizeEvent(event);
}
void StatisticsDiplay::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("StatisticsDisplay/geometry", saveGeometry());
    }
}

void StatisticsDiplay::on_CloseButton_clicked()
{
    close();
}


void StatisticsDiplay::on_RecalcButton_clicked()
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

    SlotsProxyModel * spm = dynamic_cast<SlotsProxyModel *>(ui->StatsTable->model());
    if (spm)
    {
        spm->invalidate();
    }
}


void StatisticsDiplay::on_MinutesSpinner_textChanged(const QString &)
{
    int interval = ui->MinutesSpinner->value();
    QSettings settings;
    settings.setValue("StatisticsDisplay/interval", interval);

    on_RecalcButton_clicked();
}

