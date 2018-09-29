#ifndef SPOTSDATABASE_H
#define SPOTSDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

const QString SPOTDB_PATH = "./SpotDb/";
const QString SPOTDB_FILENAME = "spots.db";

enum SpotType  { DXCLUSTER, KSTCLUSTER, BANDMAP };

class SpotsDatabase
{


public:
    SpotsDatabase();
    void databaseConnect();
    void databaseInit();
    void databaseQuery();
    void databaseInsert(QString time, QString freq, QString callsign, QString spotterCallsign, QString locator, QString comment, SpotType spotType);




};

#endif // SPOTSDATABASE_H
