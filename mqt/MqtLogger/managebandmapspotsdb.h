#ifndef MANAGEBANDMAPSPOTSDB_H
#define MANAGEBANDMAPSPOTSDB_H

#include "bandmapspotdb.h"
#include <QDialog>
#include <QAbstractItemModel>

namespace Ui {
class manageBandmapSpotsDb;
}

class ContestDbEntry;

enum dbColumns {edbcName, edbcItems, eDBcMax};

class ManageBandmapSpotsDbModel : public QAbstractItemModel
{
    Q_OBJECT

    QVector <ContestDbEntry> *contests = nullptr;

public:
    explicit ManageBandmapSpotsDbModel(QObject *parent = nullptr);

    void setContests(QVector <ContestDbEntry> *pContests);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
};

class ManageBandmapSpotsDb : public QDialog
{
    Q_OBJECT
    QSharedPointer<BandMapSpotDB> bmsdb;
    QVector <ContestDbEntry> contests;

    ManageBandmapSpotsDbModel dbModel;

public:
    explicit ManageBandmapSpotsDb(QWidget *parent = nullptr);
    ~ManageBandmapSpotsDb();
public slots:
    void reject() override;
    void accept() override;
private slots:


    void on_OKButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::manageBandmapSpotsDb *ui;
    void doCloseEvent();

};

#endif // MANAGEBANDMAPSPOTSDB_H
