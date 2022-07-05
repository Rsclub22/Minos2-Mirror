#ifndef STATISTICSDISPLAY_H
#define STATISTICSDISPLAY_H

#include "base_pch.h"
#include <QMenu>
#include <QDialog>

namespace Ui {
class StatisticsDiplay;
}

class BaseContestLog;

//============================================================================================
class BandModeSlot
{
public:
    BandModeSlot(const QString &b, const QString m):band(b), mode(m){}
    BandModeSlot(){}
    QString band;
    QString mode;
    QStringList ops;
    int QSOs = 0;
    int points = 0;
    int newMults = 0;
    int bonus = 0;

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
    int slotStart = 0;
    QDateTime dtStart;
    QString sstart;
    int slotDuration = 0;

    QMap<QString, BandMode> modesMap;   // mode slots by band

    StatisticsSlot(){}  // for QVector
    StatisticsSlot ( QDateTime current, int duration );
};

//============================================================================================
class Band
{
public:
    QStringList modes;
};
//============================================================================================
class StatisticsDisplay;

class SlotsModel: public QAbstractTableModel
{
    StatisticsDisplay *sd;
public:
    SlotsModel(StatisticsDisplay *s);
    virtual ~SlotsModel() override;

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    void reset();
};
class SlotsProxyModel: public QSortFilterProxyModel
{
public:
    SlotsProxyModel();
    virtual ~SlotsProxyModel();

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};
//============================================================================================

class StatisticsDisplay : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDisplay(BaseContestLog * ct, QWidget *parent = nullptr);
    ~StatisticsDisplay();

    QMap<QString, Band> bandList;
    int curBand = 0;

    QVector<StatisticsSlot> contestSlots;

    QStringList bandStrings;
    QStringList modeStrings;
private slots:
    void on_CloseButton_clicked();

    void on_RecalcButton_clicked();

    void on_currentTabChangedSlot(int index);
    void on_MinutesSpinner_textChanged(const QString &arg1);

    void onStatisticsGrid_customContextMenuRequested(const QPoint &pos);
    void onStatisticsGrid_sectionMoved(int, int, int);
    void on_sectionResized(int, int, int);
private:
    Ui::StatisticsDiplay *ui;
    BaseContestLog * ct = nullptr;
    QString trAll;

    QMenu columnsMenu;
    bool inRestoreColumns = false;
    bool rebuildingTabs = false;

    SlotsModel *sm = nullptr;
    SlotsProxyModel *spm = nullptr;

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void viewColumn();
    void saveStatisticsTableColumns();
    void restoreStatisticsTableColumns();
    void doRecalc();
    QString getIniKey();
};

#endif // STATISTICSDISPLAY_H
