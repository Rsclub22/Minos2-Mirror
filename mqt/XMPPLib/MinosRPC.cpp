#include <QProcessEnvironment>
#include "MinosRPC.h"
#include "ServerEvent.h"
#include "MinosConnection.h"
#include "MTrace.h"
/*static*/ MinosRPC *MinosRPC::rpc = nullptr;

RPCGeneralClient::~RPCGeneralClient()
{}
RPCGeneralServer::~RPCGeneralServer()
{}
static void xmppsendAction( XStanza *a )
{
   if ( MinosAppConnection::minosAppConnection )
   {
      MinosAppConnection::minosAppConnection->sendAction( a );
   }
}
MinosRPC::MinosRPC(const QString &defaultName, bool useEnvVar):
    connected(false), subscribed(false)
{
    setSendAction(xmppsendAction);
    QString rpcName = defaultName;
    if (useEnvVar)
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        rpcName = env.value("MQTRPCNAME", defaultName);

        trace("Value of MQTRPCNAME for " + defaultName + " is " + rpcName);
    }
    setAppName(rpcName);

    connect(&connectTimer, SIGNAL(timeout()), this, SLOT(on_connectedTimeout()));
    connectTimer.start(100);
}
void MinosRPC::on_connectedTimeout()
{
    if ( !connected && checkServerReady() )
    {
        MinosRPCObj::addServerObj( QSharedPointer<MinosRPCObj>(new RPCGeneralServer( new TRPCCallback <MinosRPC> ( this, &MinosRPC::serverCallback ) ) ) );
        XMPPInitialise( appName );
        connected = true;
    }
    if ( connected && !subscribed )
    {
        RPCPubSub::initialisePubSub( new TRPCCallback <MinosRPC> ( this, &MinosRPC::notifyCallback ) );

        for (auto const &p: remoteSubscriptions)
        {
            RPCPubSub::subscribeRemote( p.first, p.second );
        }

        for (auto const &c: subscriptions)
        {
            RPCPubSub::subscribe( c );
        }

        subscribed = true;
    }
}

void MinosRPC::setAppName(const QString &n)
{
    appName = n;
}
QString MinosRPC::getAppName()
{
    return appName;
}


void MinosRPC::subscribe(const QString &c)
{
    subscriptions.insert(c);
    if (subscribed)
    {
        RPCPubSub::subscribe( c );
    }
}
void MinosRPC::subscribeRemote(const QString &server, const QString &cat)
{
    remoteSubscriptions.insert(QPair<QString, QString>(server, cat));
    if (subscribed)
    {
        RPCPubSub::subscribeRemote( server, cat );
    }
}
void MinosRPC::publish( const QString &category, const QString &key, const QString &value, PublishState pState )
{
   RPCPubSub::publish( category, key, value, pState );
}
void MinosRPC::notifyCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
   trace( "Notify callback from " + from + ( err ? ":Error" : ":Normal" ) );

    emit notify(err, mro, from);
}

//---------------------------------------------------------------------------
void MinosRPC::serverCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
   trace( "rpc server callback from " + from + ( err ? ":Error" : ":Normal" ) );

   emit serverCall(err, mro, from);
}
