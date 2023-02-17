#include <QProcessEnvironment>
#include "ServerEvent.h"
#include "MinosConnection.h"
#include "MTrace.h"
#include "RPCCommandConstants.h"
#include "RPCPubSub.h"
#include "MinosRPC.h"

/*static*/ MinosRPC *MinosRPC::rpc = nullptr;

RPCGeneralClient::~RPCGeneralClient()
{}
RPCGeneralRouter::~RPCGeneralRouter()
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

    connect(&connectTimer, &QTimer::timeout, this, &MinosRPC::on_connectedTimeout);
    connectTimer.start(100);
}
void MinosRPC::on_connectedTimeout()
{
    if ( !connected && checkRouterReady() )
    {
        MinosRPCObj::addRouterObj( QSharedPointer<MinosRPCObj>(new RPCGeneralRouter( new TRPCCallback <MinosRPC> ( this, &MinosRPC::routerCallback ) ) ) );
        XMPPInitialise( appName );
        connected = true;
    }
    if ( connected && !subscribed )
    {
        RPCPubSub::initialisePubSub( new TRPCCallback <MinosRPC> ( this, &MinosRPC::notifyCallback ) );
        RPCPubSub::subscribe(rpcConstants::StationCategory);

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
void MinosRPC::subscribeRemote(const QString &router, const QString &cat)
{
    remoteSubscriptions.insert(QPair<QString, QString>(router, cat));
    if (subscribed)
    {
        RPCPubSub::subscribeRemote(router , cat );
    }
}
void MinosRPC::publish( const QString &category, const QString &key, const QString &value, PublishState pState )
{
    RPCPubSub::publish( category, key, value, pState );
}

void MinosRPC::setRouterAppCatMap(QMap<QString, QVector<QSharedPointer<Connectable> > > &sacm)
{
    routerAppCatMap = sacm;

    routers.clear();    // so StationCategory can re-populate it

    // Named routers get connected here; "any" routers get connected as and when they connect
    for ( QMap<QString,QVector< QSharedPointer<Connectable> > >::iterator i = routerAppCatMap.begin(); i != routerAppCatMap.end(); i++)
    {
        for ( QVector <QSharedPointer<Connectable> >::iterator j = (*i).begin(); j != (*i).end(); j++ )
        {
            if ((*j)->runType == RunLocal)
            {
                MinosConfig *config = MinosConfig::getMinosConfig();
                rpc->subscribeRemote(config->getThisRouterName(), i.key());
            }
            else if ((*j)->runType == ConnectRouter && !(*j)->routerName.isEmpty())
            {
                rpc->subscribeRemote((*j)->routerName, i.key());
            }
        }
    }

}

void MinosRPC::initialiseRouters(QStringList subs)
{
    routerSubs += subs;
    routerSubs.removeDuplicates();

    routers.clear();    // so StationCategory can re-populate it

}
void MinosRPC::findProviders(QString sub, QStringList psubs)
{
    providers[sub]; // create the element if it isn't there already
    postSubs[sub] += psubs;
    postSubs[sub].removeDuplicates();

}
void MinosRPC::routerNotify( AnalysePubSubNotify &an)
{
    if ( an.getOK() )
    {
        if (an.getCategory() == rpcConstants::StationCategory)
        {
            QString router = an.getKey();

            if (!routers.contains(router))
            {
                routers.append(router);

                // connect all "initialiseServers" subscriptions and any findProviders

                for(auto &cat:routerSubs)
                {
                    RPCPubSub::subscribeRemote(router, cat);
                }
                for(QMap<QString, QVector<Provider> >::iterator i = providers.begin(); i != providers.end(); i++)
                {
                    QString cat = i.key();
                    RPCPubSub::subscribeRemote(router, cat);
                }

                // This connects up those connectables that are "any" server
                // we can't do this until we know the  name

                for ( QMap<QString,QVector< QSharedPointer<Connectable> > >::iterator i = routerAppCatMap.begin(); i != routerAppCatMap.end(); i++)
                {
                    for ( QVector <QSharedPointer<Connectable> >::iterator j = (*i).begin(); j != (*i).end(); j++ )
                    {
                        if ((*j)->runType == ConnectRouter && (*j)->routerName.isEmpty())
                        {
                            RPCPubSub::subscribeRemote(router, i.key());
                        }
                    }
                }
            }

        }
        for(QMap<QString, QVector<Provider> >::iterator i = providers.begin(); i != providers.end(); i++)
        {
            QString sub = i.key();
            if (sub == an.getCategory())
            {
                Provider prov(an);
                int p = providers[sub].indexOf(prov);
                if (p >= 0)
                {
                    providers[sub][p] = prov;
                }
                else
                {
                    providers[sub].push_back(prov);
                }
                for(const auto &s: qAsConst(postSubs[sub]))
                {
                    RPCPubSub::subscribeRemote(prov.routerName, s);
                }
                emit provider(prov, sub);
            }
        }

    }

}
void MinosRPC::notifyCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
   //trace( "MinosRPC Notify callback from " + from + ( err ? ":Error" : ":Normal" ) );

   AnalysePubSubNotify an( err, mro );
   routerNotify(an );

   QString category = an.getCategory();
   if (routerAppCatMap.count() && category != rpcConstants::LocalStationCategory && category != rpcConstants::StationCategory)
   {
       bool notificationOK = false;
       for ( auto &j: routerAppCatMap[category] )
       {
           if (j->runType == RunLocal)
           {
               if (an.getPublisherRouter() != j->routerName)
               {
                   //trace("RunLocal  " + an.getPublisherRouter() + " " + (*j).routerName);
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
           else if (j->runType == ConnectRouter)
           {
               if (j->routerName.isEmpty())
               {
                   notificationOK = true;
                   break;
               }
               else if (an.getPublisherRouter() != j->routerName)
               {
                   //trace("ConnectRouter  " + an.getPublisherRouter() + " " + (*j).routerName);
                   continue;
               }
               if (j->remoteAppName.isEmpty())
               {
                   notificationOK = true;
                   break;
               }
               else if (an.getPublisherProgram() != j->remoteAppName)
               {
                   //trace("ConnectRouter appName " + an.getPublisherProgram() + " " + (*j).appName);
                   continue;
               }

               notificationOK = true;
               break;
           }
       }

       //trace(QString("notificationOK is %1").arg(notificationOK));
       for (const auto &s:qAsConst(routerSubs))
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
void MinosRPC::routerCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
   //trace( "rpc  callback from " + from + ( err ? ":Error" : ":Normal" ) );

   emit routerCall(err, mro, from);
}
