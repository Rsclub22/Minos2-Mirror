#ifndef QRZSERVICE_H
#define QRZSERVICE_H



#include "CallsignService.h"
#include "qrzdb.h"

class QrzServerMainWindow;

class QRZService : public CallsignService
{
    Q_OBJECT

public:
    explicit QRZService(QRZDB* db, QObject* parent = nullptr);

    QString name() const override;

    void login(const QString& user, const QString& password) override;
    bool lookupCallsign(const QString& call, QrzCallsignData& result) override;



private:
    QRZDB* m_db;

    bool loginInProgress = false;
    QString user;
    QString password;

};

#endif // QRZSERVICE_H
