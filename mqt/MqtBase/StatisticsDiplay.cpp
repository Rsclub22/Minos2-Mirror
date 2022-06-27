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
"natural" sort order for columns (and wavelength?)

Export as CSV

Filtering

Is there any way of compressing things? Do we need to?

Added breaks on operator change
*/
//============================================================================================
class BandMode
{
public:
    BandMode(){}
    BandMode(const QString &b, const QString &m):band(b), mode(m){}
    QString band;
    QString mode;

    bool operator==(const BandMode &rhs) const
    {
        return band == rhs.band && mode == rhs.mode;
    }
    bool operator<(const BandMode &rhs) const
    {
        if (band == rhs.band)
        {
            return mode < rhs.mode;
        }

        return band < rhs.band;
    }
    QString toString()
    {
        return band + "-" + mode;
    }
};
//============================================================================================
class BandModeSlot
{
public:
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
class StatisticsSlot
{
public:
public:
    int slotStart = 0;
    QDateTime dtStart;
    QString sstart;
    QString sstart2;
    int slotDuration = 0;

    QMap<BandMode, BandModeSlot> bmSlots;
    QVector<BandModeSlot> bmSlotVector;

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
QVector<BandMode> bandModeList;
QVector<StatisticsSlot> contestSlots;


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
    return bandModeList.size();
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
            cell = contestSlots[row].bmSlotVector[col].toString();
        }
        return cell;
    }
    return QVariant();
}

QVariant SlotsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        QVariant cell;
        if ( orientation == Qt::Horizontal)
        {
            if (contestSlots.size())
            {
                cell = bandModeList[section].toString();
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

    SlotsModel *sm = new SlotsModel();
    SlotsProxyModel *spm = new SlotsProxyModel();

    spm->setSourceModel(sm);
    ui->StatsTable->setModel(spm);

    ui->StatsTable->horizontalHeader()->setVisible(true);
    ui->StatsTable->verticalHeader()->setVisible(true);

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

            BandMode bm(band, c.wt->mode.getValue());
            if (!bandModeList.contains(bm))
            {
                bandModeList.push_back(bm);
            }

            contestSlots[sno].bmSlots[bm].QSOs++;
            contestSlots[sno].bmSlots[bm].points += c.wt->contactScore.getValue();
            contestSlots[sno].bmSlots[bm].bonus += c.wt->bonus;
            contestSlots[sno].bmSlots[bm].newMults += c.wt->multCount;
            QString op = c.wt->op1.getValue();
            if (!contestSlots[sno].bmSlots[bm].ops.contains(op))
            {
                contestSlots[sno].bmSlots[bm].ops.append(op);
            }
        }

    }
    std::sort(bandModeList.begin(), bandModeList.end());

    for (auto &c:contestSlots)
    {
        for (auto &b:bandModeList)
        {
            c.bmSlotVector.push_back(c.bmSlots[b]);
        }
    }

    SlotsProxyModel * spm = dynamic_cast<SlotsProxyModel *>(ui->StatsTable->model());
    if (spm)
    {
        spm->invalidate();
    }
}

