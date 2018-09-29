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


    QSqlQuery query("CREATE TABLE spotDb (id INTEGER PRIMARY KEY, time VARCHAR(10), freq VARCHAR(12), dxCallsign VARCHAR(20), spotterCallsign VARCHAR(20), locator VARCHAR(6), comment VARCHAR(40), spotType INTEGER)");

    if(!query.isActive())
        qDebug() << "ERROR: " << query.lastError().text();


}


void SpotsDatabase::databaseInsert(QString time, QString freq, QString callsign, QString spotterCallsign, QString locator, QString comment, SpotType spotType)
{
    QSqlQuery query;

        if(!query.exec("INSERT INTO spotDb(name) VALUES(time, freq, callsign, spotterCallsign, locator, comment, spotType)"))
            qWarning() << "MainWindow::DatabasePopulate - ERROR: " << query.lastError().text();

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
