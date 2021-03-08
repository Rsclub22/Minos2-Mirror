#ifndef CLUSTERRPC_H
#define CLUSTERRPC_H

#include <QObject>
#include "base_pch.h"
//#include "clusterClientServer.h"
#include "MinosLoggerEvents.h"



class Clusterrpc : public QObject
{
    Q_OBJECT

    QVector<ClusterServer> serverList;


public:
    explicit Clusterrpc();
    ~Clusterrpc();
    void sendDXSpot(QString spot, QString uuid, int frameId);

    int getServerListCount();
    void publishState(const QString &raw, const QString &state);

    void publishTXEnable(const QString txOnOff);

    void askQrzServerForQra(QString dxCall, QString spotterCall);

signals:

    void sendSpotToDXCluster(Frequency, QString, QString);
    void resendSpotToClients(int, QString, QString, QString);
    void reconnectCmdFromLog(bool);
    void clusterQrzResponse(QString, QString, QString, QString, QString, QString);


private slots:
    void on_notify(AnalysePubSubNotify an, const QString);
    void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);
};

#endif // CLUSTERRPC_H
