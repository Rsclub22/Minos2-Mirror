

#include "qrzservice.h"
#include "qrzservermainwindow.h"

QRZService::QRZService(QRZDB* db, QObject* parent)
    : QObject(parent)
    , m_db(db)
{
}

QString QRZService::name() const
{
    return "QRZ";
}

bool QRZService::login()
{
    // still handled by MainWindow for now
    return true;
}


bool QRZService::lookupCallsign(const QString& call,
                                QrzCallsignData& result)
{
    // --------------------------
    // DB LOOKUP (service owns DB)
    // --------------------------
    if (m_db && m_db->getRecord(call) .getCallsign() == call)
    {
        result = m_db->getRecord(call);
        return true;
    }

    // --------------------------
    // NETWORK FALLBACK
    // --------------------------
    // Service should NOT call UI
    // So we emit a request OR call a network layer (next step)

    emit lookupNetworkRequested(call);  // OR callback style

    return false;
}
