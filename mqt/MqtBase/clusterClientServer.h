#ifndef CLUSTERCLIENTSERVER_H
#define CLUSTERCLIENTSERVER_H

#include <QObject>
#include "base_pch.h"

extern QString clusterStateIndicator[];
extern QString clusterStateList[];

class ClusterServer
{
public:
    QString serverName;
    QString app;
    PublishState state;
};



class ClusterClientServer : public QObject
{
    Q_OBJECT



public:
    explicit ClusterClientServer();
    virtual ~ClusterClientServer();
    static ClusterClientServer  *getClusterClientServer();
    void sendDxSpot(QString spot);


private:
    static ClusterClientServer *clusterClientServer;
    QVector<ClusterServer> serverList;
    QTimer SyncTimer;

   void addSpotQueue(const QString &spot);
    void syncSpots();
    void syncStations();


private slots:
    void SyncTimerTimer( );
    void on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &);
    void on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from);

signals:
    void ClusterServerList(QVector<ClusterServer>);
    void dxSpot(QVector<QString>);


};

#endif // CLUSTERCLIENTSERVER_H
