#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

#include "AppStartup.h"
#include "MTrace.h"
#include "dtg.h"
#include "bandmapspotdb.h"

BandMapSpotDB::BandMapSpotDB(QObject *parent): QObject(parent)
{
    qdb = QSqlDatabase::addDatabase("QSQLITE");

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString dbName = getDirectoryLocation(dlDB) + "/lspotsdb6.db";
#else
    QString dbName = getDirectoryLocation(dlDB) + "/lspotsdb5.db";
#endif

    qdb.setDatabaseName(dbName);

    if (!qdb.open())
    {
        trace(QString("Error: connection with database %1 failed %2").arg(dbName, qdb.lastError().text()));
    }
    else
    {
        trace("BandMapSpotDB: connection ok");

        QString createQuery = "CREATE TABLE IF NOT EXISTS LSPOTS ("
                              "recno INTEGER PRIMARY KEY AUTOINCREMENT,"
                              "id TEXT,"
                              "type TEXT,"
                              "spottype TEXT,"
                              "callsign TEXT,"
                              "band TEXT,"
                              "mode TEXT,"
                              "loc TEXT,"
                              "dist TEXT,"
                              "freq TEXT,"
                              "dtg TEXT,"
                              "rmOn TEXT,"
                              "offRF TEXT,"
                              "CQResp TEXT,"
                              "mark BOOLEAN"
                              ") ";

        QSqlQuery cquery;
        bool cres = cquery.prepare(createQuery);
        if(cres && cquery.exec())
        {
            trace(QString("BandMapSpotDB %1 database created successfully").arg(dbName));

            {
                QString createIndexQuery = "CREATE INDEX IF NOT EXISTS lspot_idx ON LSPOTS (id)";
                QSqlQuery query;
                bool ipres = query.prepare(createIndexQuery);
                if (ipres)
                {
                    bool qres = query.exec();
                    if (!qres)
                    {
                        QString mess = QString("BandMapSpotDB create index error: %1").arg(query.lastError().text());
                        trace(mess);
                    }
                }
            }
        }
        else
        {
            trace(QString("BandMapSpotDB create DB error: %1").arg(cquery.lastError().text()));
        }

        {
            QSqlQuery amQuery;
            bool amres = amQuery.prepare("ALTER TABLE LSPOTS ADD mark BOOLEAN");
            if (amres)
            {
                bool qres = amQuery.exec();
                if (!qres)
                {
                    QString mess = QString("BandMapSpotDB add field error: %1").arg(amQuery.lastError().text());
                    trace(mess);
                }
            }
        }
        {
            QSqlQuery amQuery;
            bool amres = amQuery.prepare("ALTER TABLE LSPOTS ADD spottype TEXT");
            if (amres)
            {
                bool qres = amQuery.exec();
                if (!qres)
                {
                    QString mess = QString("BandMapSpotDB add field error: %1").arg(amQuery.lastError().text());
                    trace(mess);
                }
            }
        }
        {
            QSqlQuery query;
            query.prepare("SELECT COUNT(*) FROM LSPOTS");
            if (query.exec())
            {
                if (query.next())
                {
                    trace(QString("BandMapSpotDB %1 spot records in DB").arg(query.value(0).toString()));
                }
                else
                {
                    trace(QString("BandMapSpotDB select count(*) next error: %1").arg(query.lastError().text()));
                }
            }
            else
            {
                trace(QString("BandMapSpotDB select count(*) exec error: %1").arg(query.lastError().text()));
            }
        }
    }
}

BandMapSpotDB::~BandMapSpotDB()
{

}

bool BandMapSpotDB::createRecord(QSharedPointer<ClusterSpotData> spot, QString id)
{
    return createRecord(spot.data(), id);
}
bool BandMapSpotDB::createRecord(ClusterSpotData *spot, QString id)
{
    int recid = spot->getRecNo();
    if (recid >= 0)
    {
        return true;
    }
    QSqlQuery query;
    QString create =         "INSERT INTO LSPOTS "
                     "(id, type, spottype, callsign, band, mode, loc, dist, freq, dtg, rmOn, offRF, CQResp)"
                     " VALUES "
                     "(:id, :type, :spottype, :callsign, :band, :mode, :loc, :dist, :freq, :dtg, :rmOn, :offRF, :CQResp)";
    bool prepres = query.prepare(create);

    if (prepres)
    {
        query.bindValue(":id", id);
        query.bindValue(":type", spot->getSpotType());
        query.bindValue(":spottype", spot->spotName());
        query.bindValue(":callsign", spot->getDxCall().getFullCall());
        query.bindValue(":band", spot->getBand());
        query.bindValue(":mode", spot->getMode());
        query.bindValue(":loc", spot->getDxLocator());
        query.bindValue(":dist", spot->getDxDist());
        query.bindValue(":freq", spot->getFreq().str());
        dtg sdtg(false);
        sdtg.setDateTime(spot->getSpotDateTime());
        query.bindValue(":dtg", sdtg.getIsoDTG());
        query.bindValue(":rmOn", spot->getRunModeOn());
        query.bindValue(":offRF", spot->getOffRunFreq());
        query.bindValue(":CQResp", spot->getCqResponse());

    // // derived
    // spot->getDxBrg();
    // spot->getDxCallWorked();
    // spot->getDxLocatorWorked();

        if(query.exec())
        {
            //select seq from sqlite_sequence where name="table_name"
            QSqlQuery query;
            bool prepres = query.prepare("SELECT SEQ FROM sqlite_sequence WHERE name = \"LSPOTS\" ");
            if (prepres)
            {
                if (query.exec() && query.next())
                {
                    recid = query.value(0).toInt();
                    spot->setRecNo(recid);
                    trace(QString("BandMapSpotDB::createRecord spot callsign %1 spottype %2 recid %3")
                              .arg(spot->getDxCall().getFullCall(), ClusterSpotData::spotName(spot->getSpotType()))
                              .arg(recid)
                          );
                }
                else
                {
                    trace(QString("BandMapSpotDB::createRecord error:").arg(query.lastError().text()));
                }
            }
            return true;
        }
        else
        {
            trace(create);
            trace(QString("BandMapSpotDB::createRecord error:").arg(query.lastError().text()));
        }
    }
    else
    {
        trace(create);
        trace(QString("BandMapSpotDB::createRecord prepare error:").arg(query.lastError().text()));
    }
    return false;
}
bool BandMapSpotDB::modifyRecord(QSharedPointer<ClusterSpotData> spot)
{
    return modifyRecord(spot.data());
}
bool BandMapSpotDB::modifyRecord(ClusterSpotData *spot)
{
    QSqlQuery query;
    QString mod = "UPDATE LSPOTS SET "
                  "type=:type, spottype=:spottype, callsign=:callsign, band=:band, mode=:mode, loc=:loc, dist=:dist, freq=:freq,"
                  " dtg=:dtg, rmOn=:rmOn, offRF=:offRF, CQResp=:CQResp"
                  " WHERE "
                  " recno=:recId";
    bool prepres = query.prepare(mod);

    if (prepres)
    {
        query.bindValue(":recId", spot->getRecNo());
        query.bindValue(":type", spot->getSpotType());
        query.bindValue(":spottype", spot->spotName());
        query.bindValue(":callsign", spot->getDxCall().getFullCall());
        query.bindValue(":band", spot->getBand());
        query.bindValue(":mode", spot->getMode());
        query.bindValue(":loc", spot->getDxLocator());
        query.bindValue(":dist", spot->getDxDist());
        query.bindValue(":freq", spot->getFreq().str());
        dtg sdtg(false);
        sdtg.setDateTime(spot->getSpotDateTime());
        query.bindValue(":dtg", sdtg.getIsoDTG());
        query.bindValue(":rmOn", spot->getRunModeOn());
        query.bindValue(":offRF", spot->getOffRunFreq());
        query.bindValue(":CQResp", spot->getCqResponse());

        // // derived
        // spot->getDxBrg();
        // spot->getDxCallWorked();
        // spot->getDxLocatorWorked();

        if(query.exec())
        {
            trace(QString("BandMapSpotDB::modifyRecord spot callsign %1 spottype %2 recid %3")
                      .arg(spot->getDxCall().getFullCall(), ClusterSpotData::spotName(spot->getSpotType()))
                      .arg(spot->getRecNo())
                  );
            return true;
        }
        else
        {
            trace(mod);
            trace(QString("BandMapSpotDB::modifyRecord error:").arg(query.lastError().text()));
        }
    }
    else
    {
        trace(mod);
        trace(QString("BandMapSpotDB::modifyRecord prepare error:").arg(query.lastError().text()));
    }
    return false;
}
bool BandMapSpotDB::deleteRecord(QSharedPointer<ClusterSpotData> spot)
{
    return deleteRecord(spot.data());
}
bool BandMapSpotDB::deleteRecord(ClusterSpotData *spot)
{
    QSqlQuery query;
    bool prepres = query.prepare("DELETE FROM LSPOTS WHERE recno=(:recid)");
    if (prepres)
    {
        int recid = spot->getRecNo();
        query.bindValue(":recid", recid);
        if (query.exec())
        {
            trace(QString("BandMapSpotDB::deleteRecord spot callsign %1 spottype %2 recid %3")
                      .arg(spot->getDxCall().getFullCall(), ClusterSpotData::spotName(spot->getSpotType()))
                      .arg(recid));
            return true;
        }
    }
    trace(QString("BandMapSpotDB::deleteRecord prepare error:").arg(query.lastError().text()));
    return false;
}

bool BandMapSpotDB::deleteAllRecords(QString id)
{
    QSqlQuery query;
    bool prepres = query.prepare("DELETE FROM LSPOTS WHERE id=(:id)");
    if (prepres)
    {
        query.bindValue(":id", id);
        if (query.exec())
        {
            trace(QString("BandMapSpotDB::deleteAllRecords spot id %1")
                      .arg(id));
            return true;
        }
    }
    trace(QString("BandMapSpotDB::deleteAllRecords error:").arg(query.lastError().text()));
    return false;
}

QVector<QSharedPointer<ClusterSpotData> > BandMapSpotDB::getRecords(const QString id)
{
    QVector<QSharedPointer<ClusterSpotData> > spots;

    QSqlQuery query;
    bool prepres = query.prepare("SELECT * FROM LSPOTS WHERE id=(:id) ORDER BY recno ASC");
    if (prepres)
    {
        query.bindValue(":id", id);
        if (query.exec())
        {
            int recno = query.record().indexOf("recno");
            int idCallsign = query.record().indexOf("callsign");
            int idType = query.record().indexOf("type");
            int idBand = query.record().indexOf("band");
            int idMode = query.record().indexOf("mode");
            int idLoc = query.record().indexOf("loc");
            int idDist = query.record().indexOf("dist");
            int idFreq = query.record().indexOf("freq");
            int idDtg = query.record().indexOf("dtg");
            int idRmOn = query.record().indexOf("rmOn");
            int idOffRF = query.record().indexOf("offRF");
            int idCQResp = query.record().indexOf("CQResp");

            while (query.next())
            {
                QString type = query.value(idType).toString();
                QSharedPointer<ClusterSpotData> spot(new ClusterSpotData(static_cast<bandmapSpotType::SPOT_TYPE>(type.toInt())));

                spot->setRecNo(query.value(recno).toInt());
                spot->setDxCall(query.value(idCallsign).toString());
                spot->setBand(query.value(idBand).toString());
                spot->setMode(query.value(idMode).toString());
                spot->setDxLocator(query.value(idLoc).toString());
                spot->setDistrict(query.value(idDist).toString());
                spot->setFreq(query.value(idFreq).toString());
                QString sdtg = query.value(idDtg).toString();
                dtg sdt(false);
                sdt.setIsoDTG(sdtg);
                spot->setSpotDateTime(sdt.getQDT());
                spot->setRunModeOn(query.value(idRmOn).toBool());
                spot->setOffRunFreq(query.value(idOffRF).toBool());
                spot->setCqResponse(query.value(idCQResp).toBool());

                spots.append(spot);
            }
        }
        else
        {
            QString mess = QString("BandMapSpotDB::getRecords exec error:%1 for %2").arg(query.lastError().text(), id);
            trace(mess);
        }
    }
    return spots;
}

int BandMapSpotDB::getRecordCount()
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM LSPOTS");
    if (query.exec() && query.next())
    {
        int recs = query.value(0).toInt();
        return recs;
    }
    return -1;
}

QVector<ContestDbEntry > BandMapSpotDB::getContests()
{
    QVector<ContestDbEntry > contests;
    QSqlQuery query;
    query.prepare("SELECT distinct id FROM LSPOTS");
    if (query.exec())
    {
        while (query.next())
        {
            QString name = query.value(0).toString();
            ContestDbEntry cde;
            cde.name = name;

            QSqlQuery cquery;
            cquery.prepare(QString("SELECT COUNT(*) FROM LSPOTS WHERE id='%1'").arg(name));
            if (cquery.exec() && cquery.next())
            {
                int recs = cquery.value(0).toInt();
                cde.entries = recs;
            }
            else
            {
                QString mess = QString("BandMapSpotDB::getContests error:%1 for %2").arg(cquery.lastError().text(), name);
                trace(mess);
            }
            contests.push_back(cde);
        }
    }
    return contests;
}

void BandMapSpotDB::deleteRecords(QStringList nameList)
{
    QSqlQuery query;
    bool prepres = query.prepare("DELETE FROM LSPOTS WHERE id=(:id)");
    if (prepres)
    {
        for(auto &s:nameList)
        {
            query.bindValue(":id", s);
            if (query.exec())
            {
                trace(QString("BandMapSpotDB::deleteRecordsname OK %1")
                          .arg(s));
            }
            else
            {
                trace(QString("BandMapSpotDB::deleteRecordsnames failed %1 %2")
                          .arg(s, query.lastError().text()));
            }
        }
    }
    else
    {
    trace(QString("BandMapSpotDB::deleteRecords prepare error: %1").arg(query.lastError().text()));
    }
}

void BandMapSpotDB::resetDB()
{
    QSqlQuery rquery;
    rquery.prepare("DROP TABLE LSPOTS");
    if (rquery.exec())
    {
        trace("LSPOTS table dropped");
    }
    else
    {
        trace(QString("failed to drop LSPOTS table error ; %1").arg(rquery.lastError().text()));
    }

}
