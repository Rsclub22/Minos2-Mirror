/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
///
//
//
/////////////////////////////////////////////////////////////////////////////

#include "spotsdatabase.h"
#include <QDebug>




SpotsDatabase::SpotsDatabase()
{

}


void SpotsDatabase::databaseConnect()
{
    const QString DRIVER("QSQLITE");

    if(QSqlDatabase::isDriverAvailable(DRIVER))
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER);
        //db.setDatabaseName(SPOTDB_PATH + SPOTDB_FILENAME);
        db.setDatabaseName(":memory:");
        if(!db.open())
            qDebug() << "ERROR: " << db.lastError();
    }
    else
    {
        qDebug() << QString("Sqlite Driver not available!");
    }
}

void SpotsDatabase::databaseInit()
{


    QSqlQuery query("CREATE TABLE spotDb (id INTEGER PRIMARY KEY AUTOINCREMENT, time VARCHAR(10), freq VARCHAR(12), dxCallsign VARCHAR(20), spotterCallsign VARCHAR(20), locator VARCHAR(6), comment VARCHAR(40), spotType INTEGER)");

    if(!query.isActive())
        qDebug() << "ERROR: " << query.lastError().text();


}


bool SpotsDatabase::databaseInsert( QString time, QString freq, QString callsign, QString spotterCallsign, QString locator, QString comment, SpotType spotType)
{
    QSqlQuery query;

    query.prepare("INSERT INTO spotDb(name) (freq, dxCallsign, spotterCallsign, locator, comment, spotType)"
                  "VALUES (:time, :callsign, :spotterCallsign, :locator, :comment, :spotType)");
    query.bindValue(":time", time);
    query.bindValue(":freq", freq);
    query.bindValue(":callsign", callsign);
    query.bindValue("spotterCallsign", spotterCallsign);
    query.bindValue(":locator", locator);
    query.bindValue(":comment", comment);
    query.bindValue(":spotType", spotType);
    if (query.exec())
    {
        qDebug() << "Insert Success " << callsign << " " << spotterCallsign;
        return true;
    }
    else
    {
        qDebug() << "insert fail";
        return false;
    }


}



void SpotsDatabase::databaseQuery()
{
    QSqlQuery query;
    query.prepare("SELECT name FROM people WHERE id = ?");
//	query.addBindValue(mInputText->text().toInt());

    if(!query.exec())
        qWarning() << "MainWindow::OnSearchClicked - ERROR: " << query.lastError().text();

//	if(query.first())
//		mOutputText->setText(query.value(0).toString());
//	else
//		mOutputText->setText("person not found");
}
