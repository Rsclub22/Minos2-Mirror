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
    qdb.setDatabaseName(getDirectoryLocation(dlDB) + "/lspotsdb6.db");
#else
    qdb.setDatabaseName(getDirectoryLocation(dlDB) + "/lspotsdb5.db");
#endif

    if (!qdb.open())
    {
        trace("BandMapSpotDB Error: connection with database failed");
    }
    else
    {
        trace("BandMapSpotDB: connection ok");

        QString createQuery = "CREATE TABLE IF NOT EXISTS LSPOTS ("
                              "recno INTEGER PRIMARY KEY AUTOINCREMENT,"
                              "id TEXT,"
                              "type TEXT,"
                              "callsign TEXT,"
                              "band TEXT,"
                              "mode TEXT,"
                              "loc TEXT,"
                              "dist TEXT,"
                              "freq TEXT,"
                              "dtg TEXT,"
                              "rmOn TEXT,"
                              "offRF TEXT,"
                              "CQResp TEXT"
                              ") ";

        // you should check if args are ok first...
        QSqlQuery cquery;
        bool cres = cquery.prepare(createQuery);
        if(cres && cquery.exec())
        {
            trace("BandMapSpotDB LSPOTS database created successfully");
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
            QSqlQuery query;
            query.prepare("SELECT COUNT(*) FROM LSPOTS");
            if (query.exec())
            {
                if (query.next())
                {
                    trace(QString("BandMapSpotDB %1 spot records retrieved").arg(query.value(0).toString()));
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
        else
        {
            trace(QString("BandMapSpotDB create DB error: %1").arg(cquery.lastError().text()));
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
    QSqlQuery query;
    bool prepres = query.prepare(
        "INSERT INTO LSPOTS "
        "(id, type, callsign, band, mode, loc, dist, freq, dtg, rmOn, offRF, CQResp)"
        " VALUES "
        "(:id, :type, :callsign, :band, :mode, :loc, :dist, :freq, :dtg, :rmOn, :offRF, :CQResp)"
        );

    if (prepres)
    {
        query.bindValue(":id", id);
        query.bindValue(":type", spot->getSpotType());
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
            trace(QString("BandMapSpotDB::createRecord spot callsign %1 spottype %2").arg(spot->getDxCall().getFullCall(), ClusterSpotData::spotName(spot->getSpotType())));
            return true;
        }
        else
        {
            trace(QString("BandMapSpotDB::createRecord error:").arg(query.lastError().text()));
        }
    }
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
