#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QVariant>
#include <QDir>
#include "AppStartup.h"
#include "MTrace.h"
#include "fileutils.h"
#include "qrzserverminosparameters.h"
#include "qrzdb.h"

const char *connectionName = "QRZDB";

QRZDB::QRZDB(QObject *parent)
    : QObject{parent}
{
    QSqlDatabase qdb = QSqlDatabase::addDatabase("QSQLITE", "QRZDB");

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QString dbName = getDirectoryLocation(dlDB) + "/qrzdb6";
#else
    QString dbName = getDirectoryLocation(dlDB) + "/qrzdb5";
#endif

    if (!FileExists(dbName))
    {
        trace(QString("%1 doesn't exist").arg(dbName));
    }
    else
    {
        if (!FileExists(dbName + ".db"))
        {
            QDir r(getDirectoryLocation(dlDB));
            if ( !r.rename( dbName, dbName + ".db" ) )
            {
                QrzServerMinosParameters::getMinosParameters() ->mshowMessage( tr( "Failed to rename\n%1\n as \n%2\n\nPlease choose a new name.").arg(dbName, dbName + ".db") );
            }
        }
    }
    dbName += ".db";

    qdb.setDatabaseName(dbName);

    if (!qdb.open())
    {
        trace(QString("Error: connection with database %1 failed %2").arg(dbName, qdb.lastError().text()));
    }
    else
    {
       trace("Database: connection ok");

       QString createQuery = "CREATE TABLE IF NOT EXISTS QRZ ("
       "callsign TEXT PRIMARY KEY,"
       "dataSource TEXT,"
       "firstName TEXT,"
       "name TEXT,"
       "addr1 TEXT,"
       "addr2 TEXT,"
       "county TEXT,"
       "country TEXT,"
       "lat TEXT,"
       "lon TEXT,"
       "qra TEXT,"
       "cqZone TEXT,"
       "ituZone TEXT,"
       "moddate TEXT,"
       "dbdate TEXT,"
       "message TEXT"
       ") ";

       QSqlQuery cquery(QSqlDatabase::database(connectionName));
          cquery.prepare(createQuery);
          if(cquery.exec())
          {
              trace(QString("QRZ database %1 created successfully").arg(dbName));
              // Original table def didn't have "message" column, so add it now in case
              QSqlQuery rquery(QSqlDatabase::database(connectionName));
              rquery.prepare("ALTER TABLE QRZ ADD message TEXT");
              if (rquery.exec())
              {
                  trace("message added to QRZ table");
              }
              else
              {
                  trace(QString("failed to add message to QRZ table error ; %1").arg(rquery.lastError().text()));
              }

              QSqlQuery query(QSqlDatabase::database(connectionName));
              query.prepare("SELECT COUNT(*) FROM QRZ");
              if (query.exec())
              {
                  if (query.next())
                  {
                      trace(QString("%1 callsign records retrieved").arg(query.value(0).toString()));
                  }
                  else
                  {
                      trace(QString("select count(*) next error: %1").arg(query.lastError().text()));
                  }
              }
              else
              {
                  trace(QString("select count(*) exec error: %1").arg(query.lastError().text()));
              }
          }
          else
          {
               trace(QString("create DB error: %1").arg(cquery.lastError().text()));
          }
    }
}

bool QRZDB::createRecord(const QrzCallsignData &csData)
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare(
      "INSERT INTO QRZ "
      "(dataSource, callsign, firstName, name, addr1, addr2, county, country, lat, lon, qra, cqZone, ituZone, moddate, dbdate, message)"
      " VALUES "
      "(:dataSource, :callsign, :firstName, :name, :addr1, :addr2, :county, :country, :lat, :lon, :qra, :cqZone, :ituZone, :moddate, :dbdate, :message)"
       );
    query.bindValue(":dataSource", csData.getDataSource());
    query.bindValue(":callsign", csData.getCallsign());
    query.bindValue(":firstName", csData.getFirstName());
    query.bindValue(":name", csData.getName());
    query.bindValue(":addr1", csData.getAddr1());
    query.bindValue(":addr2", csData.getAddr2());
    query.bindValue(":county", csData.getCounty());
    query.bindValue(":country", csData.getCountry());
    query.bindValue(":lat", csData.getLat());
    query.bindValue(":lon", csData.getLon());
    query.bindValue(":qra", csData.getQra());
    query.bindValue(":cqZone", csData.getCqZone());
    query.bindValue(":ituZone", csData.getItuZone());
    query.bindValue(":moddate", csData.getModDate());
    query.bindValue(":dbdate", csData.getDBDate());
    query.bindValue(":message", csData.getMessage());
    if(query.exec())
    {
        trace(QString("Record created for csData callsign %1").arg(csData.getCallsign()));
        return true;
    }
    else
    {
         trace(QString("createRecord error:").arg(query.lastError().text()));
    }
    return false;
}

QrzCallsignData QRZDB::getRecord(const QString cs)
{
    QrzCallsignData csData;
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare("SELECT * FROM QRZ WHERE callsign=(:callsign)");
    query.bindValue(":callsign", cs);
    if (query.exec())
    {

        int idDataSource = query.record().indexOf("dataSource");
        int idCallsign = query.record().indexOf("callsign");
        int idFirstName = query.record().indexOf("firstName");
        int idname = query.record().indexOf("name");
        int idaddr1 = query.record().indexOf("addr1");
        int idaddr2 = query.record().indexOf("addr2");
        int idcounty = query.record().indexOf("county");
        int idcountry = query.record().indexOf("country");
        int idlat = query.record().indexOf("lat");
        int idlon = query.record().indexOf("lon");
        int idqra = query.record().indexOf("qra");
        int idcqZone = query.record().indexOf("cqZone");
        int idituZone = query.record().indexOf("ituZone");
        int idmoddate = query.record().indexOf("moddate");
        int iddbdate = query.record().indexOf("dbdate");
        int idmessage = query.record().indexOf("message");
        if (query.next())
        {
           csData.setDataSource(query.value(idDataSource).toString());
           csData.setCallsign(query.value(idCallsign).toString());
           csData.setFirstName(query.value(idFirstName).toString());
           csData.setName(query.value(idname).toString());
           csData.setAddr1(query.value(idaddr1).toString());
           csData.setAddr2(query.value(idaddr2).toString());
           csData.setCounty(query.value(idcounty).toString());
           csData.setCountry(query.value(idcountry).toString());
           csData.setLat(query.value(idlat).toString());
           csData.setLon(query.value(idlon).toString());
           csData.setQra(query.value(idqra).toString());
           csData.setCqZone(query.value(idcqZone).toString());
           csData.setItuZone(query.value(idituZone).toString());
           csData.setModDate(query.value(idmoddate).toString());
           csData.setDBDate(query.value(iddbdate).toString());
           csData.setMessage(query.value(idmessage).toString());

           trace(QString("Record retrieved for csData callsign %1").arg(csData.getCallsign()));

        }
        else
        {
            trace(QString("getRecord error:%1 for %2").arg(query.lastError().text(), cs));
        }
    }
    else
    {
        trace(QString("getRecord exec error:%1 for %2").arg(query.lastError().text(), cs));
    }
    return csData;
}

int QRZDB::getRecordCount()
{
    QSqlQuery query(QSqlDatabase::database(connectionName));
    query.prepare("SELECT COUNT(*) FROM QRZ");
    if (query.exec() && query.next())
    {
        int recs = query.value(0).toInt();
        return recs;
    }
    return -1;
}

void QRZDB::resetDB()
{
    QSqlQuery rquery(QSqlDatabase::database(connectionName));
    rquery.prepare("DROP TABLE QRZ");
    if (rquery.exec())
    {
        trace("QRZ table dropped");
    }
    else
    {
        trace(QString("failed to drop QRZ table error ; %1").arg(rquery.lastError().text()));
    }
}
