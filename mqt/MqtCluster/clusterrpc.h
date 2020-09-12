#ifndef CLUSTERRPC_H
#define CLUSTERRPC_H

#include <QObject>
#include "base_pch.h"
//#include "clusterClientServer.h"
#include "MinosLoggerEvents.h"

class ClusterServer
{
public:
    QString serverName;
    QString app;
    PublishState state;
};

class Clusterrpc : public QObject
{
    Q_OBJECT

    QVector<ClusterServer> serverList;

public:
    explicit Clusterrpc();
    ~Clusterrpc();
    void sendDXSpot(QString spot);

    int getServerListCount();
    void publishState(const QString &raw, const QString &state);

    void publishTXEnable(const QString txOnOff);
    void sendDXSpotToClient(QString spot, ClusterServer s);
signals:

    void sendSpotToDXCluster(QString, QString, QString);
    void newClusterClient(ClusterServer);

private slots:
    void on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &);
    void on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from);
};

#endif // CLUSTERRPC_H
