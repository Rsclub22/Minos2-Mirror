#ifndef PUBSUBNAME_H
#define PUBSUBNAME_H
#include "XMPPRPCParams.h"

class AnalysePubSubNotify;

class PubSubName
{
private:
    QString _router;
    QString _appName;
    QString _key;
    bool operator== ( const QString& rhs ) const ;   // not implemented
public:
    PubSubName();
    PubSubName(const QString &);
    PubSubName(const AnalysePubSubNotify &an);

    bool operator< ( const PubSubName& rhs ) const;
    bool operator== ( const PubSubName& rhs ) const;
    bool operator!= ( const PubSubName& rhs ) const;

    bool isEmpty() const;
    QString toString() const;
    QString router() const;
    void setRouter(const QString &router);
    QString appName() const;
    void setAppName(const QString &appName);
    QString key() const;
    void setKey(const QString &key);

    QString getRouterApp() const;
    QString getLocalName() const;
};

#endif // PUBSUBNAME_H
