#ifndef CLUSTERRPC_H
#define CLUSTERRPC_H

#include <QObject>
#include "base_pch.h"



class ClusterRpc : public QObject
{
    Q_OBJECT



public:
    explicit ClusterRpc();

signals:

private:

    PubSubName clusterApp;

    void sendSpot(QString spot);


private slots:
    void on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &);
    void on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from);

};

#endif // CLUSTERRPC_H
