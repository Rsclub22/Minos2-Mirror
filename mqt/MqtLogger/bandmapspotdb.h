#ifndef BANDMAPSPOTDB_H
#define BANDMAPSPOTDB_H

#include <QObject>
#include <QSqlDatabase>

#include "spotbasedata.h"

class BandMapSpotDB: public QObject
{
    Q_OBJECT
public:
    explicit BandMapSpotDB(QObject *parent = nullptr);
    virtual ~BandMapSpotDB() override;

    bool createRecord(QSharedPointer<ClusterSpotData>csData, QString id);
    bool createRecord(ClusterSpotData *spot, QString id);

    QVector<QSharedPointer<ClusterSpotData> > getRecords(const QString id);

    int getRecordCount();

    void resetDB();

private:
    QSqlDatabase qdb;
};

#endif // BANDMAPSPOTDB_H
