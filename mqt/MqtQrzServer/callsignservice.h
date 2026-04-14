#ifndef CALLSIGNSERVICE_H
#define CALLSIGNSERVICE_H



#include "qobject.h"
#include <QString>

class QrzCallsignData;

class CallsignService : public QObject
{

    Q_OBJECT

public:

    explicit CallsignService(QObject* parent = nullptr)
        : QObject(parent) {}

    virtual ~CallsignService() = default;

    virtual QString name() const = 0;

    virtual void login(const QString& user, const QString& password) = 0;

    virtual bool lookupCallsign(const QString& call, QrzCallsignData& result) = 0;

signals:
    void lookupNetworkRequested(const QString& call);
    void loginRequest(const QString& url);
    void loginFailed(const QString& error);
    void loginSucceeded(const QString& sessionKey);

    void loginRequested(const QString& url);
};
#endif // CALLSIGNSERVICE_H
