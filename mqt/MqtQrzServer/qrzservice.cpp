

#include "qrzservice.h"
#include "qrzservermainwindow.h"

QRZService::QRZService(QRZDB* db, QObject* parent)
    : CallsignService(parent)
    , m_db(db)
{
}

QString QRZService::name() const
{
    return "QRZ";
}

void QRZService::login(const QString& user_, const QString& password_)
{
    if (loginInProgress)
        return;

    loginInProgress = true;

    user = user_.trimmed();
    password = password_.trimmed();

    if (user.isEmpty() || password.isEmpty())
    {
        emit loginFailed("Missing credentials");
        loginInProgress = false;
        return;
    }

    QString url = QString("https://xmldata.qrz.com/xml/current/?")
                  + "username=" + user
                  + ";password=" + password
                  + ";agent=Minos";

    emit loginRequest(url);
}

/*
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
*/

bool QRZService::lookupCallsign(const QString& call,
                                QrzCallsignData& result)
{
    QrzCallsignData data = m_db->getRecord(call);

    if (!data.getCallsign().isEmpty())
    {
        result = data;
        return true;
    }

    emit lookupNetworkRequested(call);
    return false;
}
