#include "StatisticsDiplay.h"
#include "ui_StatisticsDiplay.h"

#include "base_pch.h"

//============================================================================================
class BandMode
{
public:
    QString band;
    QString mode;

    bool operator==(const BandMode &rhs)
    {
        return band == rhs.band && mode == rhs.mode;
    }
    bool operator<(const BandMode &rhs)
    {
        if (band == rhs.band)
        {
            return mode < rhs.mode;
        }

        return band < rhs.band;
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
QMap<QDateTime, StatisticsSlot> contestSlots;


//============================================================================================
class SlotsModel: public QAbstractTableModel
{
public:
    SlotsModel();
    virtual ~SlotsModel() override;

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};
SlotsModel::SlotsModel()
{}

SlotsModel::~SlotsModel()
{}

QModelIndex SlotsModel::index(int row, int column, const QModelIndex &parent) const
{

}

int SlotsModel::rowCount(const QModelIndex &parent) const
{

}

int SlotsModel::columnCount(const QModelIndex &parent) const
{

}

QVariant SlotsModel::data(const QModelIndex &index, int role) const
{

}

QVariant SlotsModel::headerData(int section, Qt::Orientation orientation, int role) const
{

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

}

StatisticsDiplay::~StatisticsDiplay()
{
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

}

