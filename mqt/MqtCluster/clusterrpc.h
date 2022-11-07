#ifndef CLUSTERRPC_H
#define CLUSTERRPC_H

#include "AnalysePubSubNotify.h"
#include "MinosRPC.h"
#include "frequency.h"

class Clusterrpc : public QObject
{
    Q_OBJECT

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
    void qrzServerLoggedState(bool, QString);


private slots:
    void on_notify(AnalysePubSubNotify an, const QString);
    void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);
    void on_provider(Provider provider, QString cat);
};

#endif // CLUSTERRPC_H
