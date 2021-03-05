#include <QProcessEnvironment>
#include "MinosRPC.h"
#include "ServerEvent.h"
#include "MinosConnection.h"
#include "MTrace.h"
#include "RPCCommandConstants.h"

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

    connect(&connectTimer, SIGNAL(timeout()), this, SLOT(on_connectedTimeout()));
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

//    RPCPubSub::subscribe(rpcConstants::LocalStationCategory);   // this might not be needed?
    RPCPubSub::subscribe(rpcConstants::StationCategory);


    // Named s get connected here; "any" s get connected as and when they connect
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
    // we need to add (and subscribe to) any new subs
    routersInitialised = true;
    //RPCPubSub::subscribe(rpcConstants::LocalStationCategory);
    RPCPubSub::subscribe(rpcConstants::StationCategory);

    routerSubs = subs;

}
void MinosRPC::routerNotify( AnalysePubSubNotify &an)
{
    if ( an.getOK() )
    {
//        if ( an.getCategory() == rpcConstants::LocalStationCategory)
//        {
//            QString router = an.getKey();
//            bool pubNeeded = true;
//            QString a = MinosRPC::getMinosRPC()->getAppName();
//            for ( auto const &stat: qAsConst(routerList) )
//            {
//                if (stat.app == a + "@" + router)
//                {
//                    pubNeeded = false;
//                    break;
//                }
//            }
//            if (pubNeeded)
//            {
//                RPCPubSub::publish(rpcConstants::ChatServer,  a + "@" + router, "", psPublished);
//            }
//        }
        if (an.getCategory() == rpcConstants::StationCategory)
        {
            QString router = an.getKey();
            bool subNeeded = true;
            for ( auto const &stat: qAsConst(routerList) )
            {
                if (stat.routerName == router)
                {
                    subNeeded = false;
                    break;
                }
            }
            if (subNeeded)
            {
                for(auto &cat:routerSubs)
                {
                    RPCPubSub::subscribeRemote(router, cat);
                }
            }
            // This connects up those connectables that are "any"
            // we can't do this until we know the  name

            if (!routers.contains(router))
            {
                routers.append(router);
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
    }

}
void MinosRPC::notifyCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
   trace( "MinosRPC Notify callback from " + from + ( err ? ":Error" : ":Normal" ) );

   AnalysePubSubNotify an( err, mro );
   if (routersInitialised)
   {
       routerNotify(an );
   }
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

       trace(QString("notificationOK is %1").arg(notificationOK));
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
   trace( "rpc  callback from " + from + ( err ? ":Error" : ":Normal" ) );

   emit routerCall(err, mro, from);
}
