#ifndef QRZDB_H
#define QRZDB_H

#include <QObject>
#include <QSqlDatabase>
#include "qrzServerCommon.h"

class QRZDB : public QObject
{
    Q_OBJECT
public:
    explicit QRZDB(QObject *parent = nullptr);

    bool createRecord(const QrzCallsignData &csData);

    QrzCallsignData getRecord(const QString cs);

    int getRecordCount();

    void resetDB();

private:
    QSqlDatabase qdb;
};

#endif // QRZDB_H
