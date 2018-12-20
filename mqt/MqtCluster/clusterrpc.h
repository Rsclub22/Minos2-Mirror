#ifndef CLUSTERRPC_H
#define CLUSTERRPC_H

#include <QObject>
#include "base_pch.h"
#include "clusterClientServer.h"
#include "MinosLoggerEvents.h"

class Clusterrpc : public QObject
{
    Q_OBJECT


public:
    explicit Clusterrpc();
    ~Clusterrpc();
    void setStandAlone();
    void sendDXSpot(QString spot, QString appName);

signals:
    void clientConnected();

private slots:


    void clusterClientServerList(QVector<ClusterServer>);
    void dxSpots(QVector<QString>);


};

#endif // CLUSTERRPC_H
