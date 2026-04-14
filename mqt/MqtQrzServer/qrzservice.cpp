

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
    // TEMP: just call existing MainWindow logic
    return true;
}

bool QRZService::lookupCallsign(const QString& call,
                                QrzCallsignData& result)
{
    // TEMP: DO NOT CHANGE BEHAVIOUR YET
    // we will wire this properly in Step 2

    return false;
}
