/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      ClusterClientServer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef CLUSTERCLIENTSERVER_H
#define CLUSTERCLIENTSERVER_H

#include <QObject>
#include "base_pch.h"
#include "clustercommon.h"





class ClusterClientServer : public QObject
{
    Q_OBJECT

public:
    explicit ClusterClientServer();
    virtual ~ClusterClientServer();
    static ClusterClientServer  *getClusterClientServer();

    void setLoggerUuid(QString uuid);
private:
    static ClusterClientServer *clusterClientServer;
    QTimer SyncTimer;


    void addSpotQueue(const ClusterMessage spot);

private slots:
    void SyncTimerTimer( );
    void on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from);

signals:
    void ClusterServerList(QVector<ClusterServer>);
    void dxSpot(QVector<ClusterMessage>);

};

#endif // CLUSTERCLIENTSERVER_H
