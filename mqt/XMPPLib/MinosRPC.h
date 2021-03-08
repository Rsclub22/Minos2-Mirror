#ifndef MINOSRPC_H
#define MINOSRPC_H

#include <QTimer>
#include <QSet>
#include "XMPPRPCObj.h"
#include "RPCPubSub.h"
#include "AnalysePubSubNotify.h"
#include "ConfigFile.h"

class RPCGeneralClient: public MinosRPCClient
{
   public:
    RPCGeneralClient( QString mName) : MinosRPCClient( mName, nullptr, true )
    {}
      RPCGeneralClient( TRPCFunctor *cb ) : MinosRPCClient( "", cb, true )
      {}
      ~RPCGeneralClient();

      virtual QSharedPointer<MinosRPCObj> makeObj()
      {
         return QSharedPointer<MinosRPCObj>(new RPCGeneralClient( callback ));
      }
};
class RPCGeneralRouter: public MinosRPCRouter
{
   public:
    RPCGeneralRouter( ) : MinosRPCRouter( "", nullptr, true )
    {}
      RPCGeneralRouter( TRPCFunctor *cb ) : MinosRPCRouter( "", cb, true )
      {}
      ~RPCGeneralRouter();

      virtual QSharedPointer<MinosRPCObj>makeObj()
      {
         return QSharedPointer<MinosRPCObj>(new RPCGeneralRouter( callback ));
      }

};
class RPCRouter
{
public:
    QString routerName;
    QString app;
    PublishState state;
};
class MinosRPC: public QObject
{
    Q_OBJECT

    MinosRPC(const QString &defaultName, bool useEnvVar);

    static MinosRPC *rpc;

    QString appName;
    bool connected;
    bool subscribed;

    QTimer connectTimer;

    QSet <QString> subscriptions;
    QSet <QPair <QString, QString> > remoteSubscriptions;

    QVector<RPCRouter> routerList;
    QStringList routerSubs;
    QMap<QString,QVector< QSharedPointer<Connectable> > > routerAppCatMap;
    QVector<QString> routers;

    bool routersInitialised = false;


    void setAppName(const QString &);
    void notifyCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );
    void routerCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );

    void routerNotify(AnalysePubSubNotify &an);
public:

    static MinosRPC *getMinosRPC(QString defaultName = QString(), bool useEnvVar = true)
    {
        if (!rpc && !defaultName.isEmpty())
        {
            rpc = new MinosRPC(defaultName, useEnvVar);
        }
        return rpc;
    }
    static MinosRPC *validMinosRPC()
    {
        return rpc;
    }
    ~MinosRPC() override
    {
        rpc = nullptr;
    }

    QString getAppName();

    void subscribe(const QString &);
    void subscribeRemote(const QString &router, const QString &);

    void publish( const QString &category, const QString &key, const QString &value, PublishState pState );

    void setRouterAppCatMap(QMap<QString,QVector< QSharedPointer<Connectable> > > &sacm);
    void initialiseRouters(QStringList subs);
    QVector<RPCRouter> getRouterList()
    {
        return routerList;
    }
signals:

    void notify( AnalysePubSubNotify an, const QString from);
    void routerCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString from);

private slots:

    void on_connectedTimeout();
};

#endif // MINOSRPC_H
