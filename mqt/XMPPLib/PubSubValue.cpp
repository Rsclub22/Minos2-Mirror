#include "XMPP_pch.h"
#include "PubSubValue.h"
#include "ConfigFile.h"
#include "RPCPubSub.h"
#include "AnalysePubSubNotify.h"

QString PubSubValue::psType() const
{
    return _psType;
}

PubSubValue::PubSubValue(QString t):_psType(t)
{

}
PubSubValue::~PubSubValue()
{}
//=======================================================================
PubSubName::PubSubName()
{}

PubSubName::PubSubName(const AnalysePubSubNotify &an)
{
    _router = an.getPublisherRouter();
    _appName = an.getPublisherProgram();
    _key = PubSubName(an.getKey()).key();
}
PubSubName::PubSubName(const QString &s)
{
    if (s.isEmpty())
        return;

    QStringList sl = s.split('/');
    std::reverse(sl.begin(), sl.end());
    if (sl.size())
    {
        _key = sl[0];
        if (sl.size() > 1)
        {
            _appName = sl[1];
            if (sl.size() > 2)
            {
                _router = sl[2];
            }
        }

        if (_key.isEmpty())
            return;
        if (_router.isEmpty())
        {
            _router = MinosConfig::getMinosConfig()->getThisRouterName();
        }
        if (_appName.isEmpty())
        {
            MinosRPC *rpc = MinosRPC::validMinosRPC();
            if (rpc)
            {
                _appName = rpc->getAppName();
            }
        }
    }
}
bool PubSubName::isEmpty() const
{
    return _router.isEmpty() && _appName.isEmpty() && _key.isEmpty();
}
QString PubSubName::toString() const
{
    if (isEmpty())
        return QString();

    return _router + "/" + _appName + "/" + _key;
}
bool PubSubName::operator< ( const PubSubName& rhs ) const
{
    int scomp = _router.compare(rhs.router(), Qt::CaseInsensitive);
    if (scomp == 0)
    {
        int acomp = _appName.compare(rhs.appName(), Qt::CaseInsensitive);
        if (acomp == 0)
            return _key.compare(rhs.key(), Qt::CaseInsensitive) < 0;
        return acomp < 0;
    }
    else
        return scomp < 0;
}
bool PubSubName::operator== ( const PubSubName& rhs ) const
{
    int scomp = _router.compare(rhs.router(), Qt::CaseInsensitive);
    if (scomp == 0)
    {
        int acomp = _appName.compare(rhs.appName(), Qt::CaseInsensitive);
        if (acomp == 0)
            return _key.compare(rhs.key(), Qt::CaseInsensitive) == 0;
    }
    return false;
}
bool PubSubName::operator!= ( const PubSubName& rhs ) const
{
    return !(*this == rhs);
}

QString PubSubName::appName() const
{
    return _appName;
}

void PubSubName::setAppName(const QString &appName)
{
    _appName = appName;
}

QString PubSubName::key() const
{
    return _key;
}

void PubSubName::setKey(const QString &key)
{
    _key = key;
}

QString PubSubName::getRouterApp() const
{
    return router() + "/" + appName();
}

QString PubSubName::router() const
{
    return _router;
}

void PubSubName::setRouter(const QString &router)
{
    _router = router;
}
