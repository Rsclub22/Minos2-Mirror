#include <QProcessEnvironment>
#include "MinosRPC.h"
#include "ServerEvent.h"
#include "MinosConnection.h"
#include "MTrace.h"
#include "RPCCommandConstants.h"

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

        for (auto const &p: qAsConst(remoteSubscriptions))
        {
            RPCPubSub::subscribeRemote( p.first, p.second );
        }

        for (auto const &c: qAsConst(subscriptions))
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

void MinosRPC::setServerAppCatMap(QMap<QString, QVector<QSharedPointer<Connectable> > > &sacm)
{
    serverAppCatMap = sacm;

    servers.clear();    // so StationCategory can re-populate it

    RPCPubSub::subscribe(rpcConstants::LocalStationCategory);   // this might not be needed?
    RPCPubSub::subscribe(rpcConstants::StationCategory);


    // Named servers get connected here; "any" servers get connected as and when they connect
    for ( QMap<QString,QVector< QSharedPointer<Connectable> > >::iterator i = serverAppCatMap.begin(); i != serverAppCatMap.end(); i++)
    {
        for ( QVector <QSharedPointer<Connectable> >::iterator j = (*i).begin(); j != (*i).end(); j++ )
        {
            if ((*j)->runType == RunLocal)
            {
                MinosConfig *config = MinosConfig::getMinosConfig();
                rpc->subscribeRemote(config->getThisServerName(), i.key());
            }
            else if ((*j)->runType == ConnectServer && !(*j)->serverName.isEmpty())
            {
                rpc->subscribeRemote((*j)->serverName, i.key());
            }
        }
    }

}

void MinosRPC::initialiseServers(QStringList subs)
{
    // we need to add (and subscribe to) any new subs
    serversInitialised = true;
    RPCPubSub::subscribe(rpcConstants::LocalStationCategory);
    RPCPubSub::subscribe(rpcConstants::StationCategory);

    serverSubs = subs;

}
void MinosRPC::serverNotify( AnalysePubSubNotify &an)
{
    if ( an.getOK() )
    {
        if ( an.getCategory() == rpcConstants::LocalStationCategory)
        {
            QString server = an.getKey();
            bool pubNeeded = true;
            QString a = MinosRPC::getMinosRPC()->getAppName();
            for ( auto const &stat: qAsConst(serverList) )
            {
                if (stat.app == a + "@" + server)
                {
                    pubNeeded = false;
                    break;
                }
            }
            if (pubNeeded)
            {
                RPCPubSub::publish(rpcConstants::ChatServer,  a + "@" + server, "", psPublished);
            }
        }
        if (an.getCategory() == rpcConstants::StationCategory)
        {
            QString server = an.getKey();
            bool subNeeded = true;
            for ( auto const &stat: qAsConst(serverList) )
            {
                if (stat.serverName == server)
                {
                    subNeeded = false;
                    break;
                }
            }
            if (subNeeded)
            {
                for(auto &cat:serverSubs)
                {
                    RPCPubSub::subscribeRemote(server, cat);
                }
            }
            // This connects up those connectables that are "any" server
            // we can't do this until we know the server name

            if (!servers.contains(server))
            {
                servers.append(server);
                for ( QMap<QString,QVector< QSharedPointer<Connectable> > >::iterator i = serverAppCatMap.begin(); i != serverAppCatMap.end(); i++)
                {
                    for ( QVector <QSharedPointer<Connectable> >::iterator j = (*i).begin(); j != (*i).end(); j++ )
                    {
                        if ((*j)->runType == ConnectServer && (*j)->serverName.isEmpty())
                        {
                            RPCPubSub::subscribeRemote(server, i.key());
                        }
                    }
                }
            }

        }
    }

}
void MinosRPC::notifyCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
   trace( "MinosRPC Notify callback from " + from + ( err ? ":Error" : ":Normal" ) );

   AnalysePubSubNotify an( err, mro );
   if (serversInitialised)
   {
       serverNotify(an );
   }
   QString category = an.getCategory();
   if (serverAppCatMap.count() && category != rpcConstants::LocalStationCategory && category != rpcConstants::StationCategory)
   {
       bool notificationOK = false;
       for ( auto &j: serverAppCatMap[category] )
       {
           if (j->runType == RunLocal)
           {
               if (an.getPublisherServer() != j->serverName)
               {
                   //trace("RunLocal server " + an.getPublisherServer() + " " + (*j).serverName);
                   continue;
               }
               if (an.getPublisherProgram() != j->appName)
               {
                   //trace("RunLocal appName " + an.getPublisherProgram() + " " + (*j).appName);
                   continue;
               }

               notificationOK = true;
               break;
           }
           else if (j->runType == ConnectServer)
           {
               if (j->serverName.isEmpty())
               {
                   notificationOK = true;
                   break;
               }
               else if (an.getPublisherServer() != j->serverName)
               {
                   //trace("ConnectServer server " + an.getPublisherServer() + " " + (*j).serverName);
                   continue;
               }
               if (j->remoteAppName.isEmpty())
               {
                   notificationOK = true;
                   break;
               }
               else if (an.getPublisherProgram() != j->remoteAppName)
               {
                   //trace("ConnectServer appName " + an.getPublisherProgram() + " " + (*j).appName);
                   continue;
               }

               notificationOK = true;
               break;
           }
       }

       trace(QString("notificationOK is %1").arg(notificationOK));
       for (const auto &s:qAsConst(serverSubs))
       {
           //trace(QString("Category %1 s %2").arg(category).arg(s));
           if (category == s)
           {
               notificationOK = true;
               break;
           }
       }
       if (!notificationOK)
           return;

   }

   emit notify(an, from);
}

//---------------------------------------------------------------------------
void MinosRPC::serverCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
   trace( "rpc server callback from " + from + ( err ? ":Error" : ":Normal" ) );

   emit serverCall(err, mro, from);
}
