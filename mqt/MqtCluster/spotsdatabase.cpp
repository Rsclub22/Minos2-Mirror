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


    QSqlQuery query("CREATE TABLE people (id INTEGER PRIMARY KEY, name TEXT)");

    if(!query.isActive())
        qDebug() << "ERROR: " << query.lastError().text();


}


void SpotsDatabase::databasePopulate()
{
    QSqlQuery query;

        if(!query.exec("INSERT INTO people(name) VALUES('Eddie Guerrero')"))
            qWarning() << "MainWindow::DatabasePopulate - ERROR: " << query.lastError().text();
        if(!query.exec("INSERT INTO people(name) VALUES('Gordon Ramsay')"))
            qWarning() << "MainWindow::DatabasePopulate - ERROR: " << query.lastError().text();
        if(!query.exec("INSERT INTO people(name) VALUES('Alan Sugar')"))
            qWarning() << "MainWindow::DatabasePopulate - ERROR: " << query.lastError().text();
        if(!query.exec("INSERT INTO people(name) VALUES('Dana Scully')"))
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
