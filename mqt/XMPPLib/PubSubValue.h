#ifndef PUBSUBVALUE_H
#define PUBSUBVALUE_H
#include "XMPPRPCParams.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

//#include "AnalysePubSubNotify.h"
class AnalysePubSubNotify;
class PubSubValue
{
protected:
    QString _psType;
public:
    PubSubValue(QString t);
    virtual ~PubSubValue();

    virtual QString pack() const = 0;
    virtual void unpack(QString) = 0;
    QString psType() const;
};

class PubSubName
{
private:
    QString _server;
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
    QString server() const;
    void setServer(const QString &server);
    QString appName() const;
    void setAppName(const QString &appName);
    QString key() const;
    void setKey(const QString &key);
};

#endif // PUBSUBVALUE_H
