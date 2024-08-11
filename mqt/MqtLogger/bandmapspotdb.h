#ifndef BANDMAPSPOTDB_H
#define BANDMAPSPOTDB_H

#include <QObject>
#include <QSqlDatabase>

#include "spotbasedata.h"

class ContestDbEntry
{
public:
    QString name;
    int entries = 0;
    bool selected = false;
};

class BandMapSpotDB: public QObject
{
    Q_OBJECT
public:
    explicit BandMapSpotDB(QObject *parent = nullptr);
    virtual ~BandMapSpotDB() override;

    bool createRecord(QSharedPointer<ClusterSpotData>csData, QString id);
    bool createRecord(ClusterSpotData *spot, QString id);

    bool deleteRecord(QSharedPointer<ClusterSpotData> spot);
    bool deleteRecord(ClusterSpotData *spot);

    bool deleteAllRecords(QString id);

    bool modifyRecord(QSharedPointer<ClusterSpotData> spot);
    bool modifyRecord(ClusterSpotData *spot);

    QVector<QSharedPointer<ClusterSpotData> > getRecords(const QString id);

    int getRecordCount();
    QVector<ContestDbEntry> getContests();
    void deleteRecords(QStringList nameList);

    void resetDB();

private:
    QSqlDatabase qdb;
};

#endif // BANDMAPSPOTDB_H
