

#include "qrzservice.h"
#include "qrzservermainwindow.h"

QRZService::QRZService(QrzServerMainWindow* owner)
    : m_owner(owner)
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
    // STEP 1: try DB first (reuse existing logic)
    if (m_owner->askDBCallsignData(call))
    {
        result = m_owner->qrzCallsignData;
        return true;
    }

    // STEP 2: fallback to QRZ network
    m_owner->askCallsignData(call);

    // NOTE:
    // result will be filled later by existing async flow
    // we preserve behaviour exactly as before

    return false;
}
