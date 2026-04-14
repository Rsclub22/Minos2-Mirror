#ifndef QRZSERVICE_H
#define QRZSERVICE_H



#include "CallsignService.h"
#include "qrzdb.h"

class QrzServerMainWindow;

class QRZService : public QObject, public CallsignService
{
    Q_OBJECT

public:
    explicit QRZService(QRZDB* db, QObject* parent = nullptr);

    QString name() const override;

    bool login() override;
    bool lookupCallsign(const QString& call, QrzCallsignData& result) override;

signals:

    void lookupNetworkRequested(const QString& call);

private:
    QRZDB* m_db;
};

#endif // QRZSERVICE_H
