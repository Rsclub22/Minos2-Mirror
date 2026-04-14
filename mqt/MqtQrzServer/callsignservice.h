#ifndef CALLSIGNSERVICE_H
#define CALLSIGNSERVICE_H



#include <QString>

class QrzCallsignData;

class CallsignService
{
public:
    virtual ~CallsignService() = default;

    virtual QString name() const = 0;

    virtual bool login() = 0;

    virtual bool lookupCallsign(const QString& call, QrzCallsignData& result) = 0;
};
#endif // CALLSIGNSERVICE_H
