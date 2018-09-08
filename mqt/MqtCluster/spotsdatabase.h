#ifndef SPOTSDATABASE_H
#define SPOTSDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

class SpotsDatabase
{


public:
    SpotsDatabase();
    void databaseConnect();
    void databaseInit();
    void databasePopulate();
    void databaseQuery();


};

#endif // SPOTSDATABASE_H
