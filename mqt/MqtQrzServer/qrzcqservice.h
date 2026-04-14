#ifndef QRZCQSERVICE_H
#define QRZCQSERVICE_H

#include "callsignservice.h"
#include "qrzdb.h"
#include <QObject>

class QRZCQService : public CallsignService
{
    Q_OBJECT

public:
    explicit QRZCQService(QRZDB* db, QObject* parent = nullptr);
    QString name() const override;
    void login(const QString& user, const QString& password) override;
    bool lookupCallsign(const QString& call, QrzCallsignData& result) override;



private:
    QRZDB* m_db;
};

#endif // QRZCQSERVICE_H
