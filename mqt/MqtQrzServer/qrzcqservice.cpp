#include "qrzcqservice.h"

QRZCQService::QRZCQService(QRZDB* db, QObject* parent)
    : CallsignService(parent)
    , m_db(db)
{
}

QString QRZCQService::name() const
{
    return "QRZCQ";
}

void QRZCQService::login(const QString& user_, const QString& password_)
{

}

bool QRZCQService::lookupCallsign(const QString& call, QrzCallsignData& result)
{
    // TEMP: fallback to DB only for now
    result = m_db->getRecord(call);

    if (!result.getCallsign().isEmpty())
        return true;

    emit lookupNetworkRequested(call);
    return false;
}
