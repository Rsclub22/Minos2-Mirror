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
class RPCGeneralServer: public MinosRPCServer
{
   public:
    RPCGeneralServer( ) : MinosRPCServer( "", nullptr, true )
    {}
      RPCGeneralServer( TRPCFunctor *cb ) : MinosRPCServer( "", cb, true )
      {}
      ~RPCGeneralServer();

      virtual QSharedPointer<MinosRPCObj>makeObj()
      {
         return QSharedPointer<MinosRPCObj>(new RPCGeneralServer( callback ));
      }

};
class RPCServer
{
public:
    QString serverName;
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

    QVector<RPCServer> serverList;
    QStringList serverSubs;
    QMap<QString,QVector< QSharedPointer<Connectable> > > serverAppCatMap;
    QVector<QString> servers;

    bool serversInitialised = false;


    void setAppName(const QString &);
    void notifyCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );
    void serverCallback( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from );

    void serverNotify(AnalysePubSubNotify &an);
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
    void subscribeRemote(const QString &, const QString &);

    void publish( const QString &category, const QString &key, const QString &value, PublishState pState );

    void setServerAppCatMap(QMap<QString,QVector< QSharedPointer<Connectable> > > &sacm);
    void initialiseServers(QStringList subs);
    QVector<RPCServer> getServerList()
    {
        return serverList;
    }
signals:
    void notify( AnalysePubSubNotify an, const QString from);
    void serverCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString from);
private slots:
    void on_connectedTimeout();
};

#endif // MINOSRPC_H
