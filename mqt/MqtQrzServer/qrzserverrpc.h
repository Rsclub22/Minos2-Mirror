/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Qrz Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2021
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef QRZSERVERRPC_H
#define QRZSERVERRPC_H

#include <QObject>
#include "base_pch.h"
#include "ConfigFile.h"
#include "clustercommon.h"
#include "qrzServerCommon.h"


class QrzServerRpc : public QObject
{
    Q_OBJECT
public:
    static const char * stateIndicator[];
    static const char * stateList[];

    explicit QrzServerRpc();
    virtual ~QrzServerRpc();

    static QrzServerRpc *getQrzServerRpc();

    void sendQrzResponseToClusterServer(QString dxCall, QString dxQra, QString dxCallStatus, QString spotterCall, QString spotterQra, QString spotterCallStatus);
    void sendQrzResponseToLoggerDisplay(QrzCallsignData qrzCallsignData, QString state);


signals:

    //void qrzRequestQueue(QVector<QrzServerMessage>);
    void clusterQrzMsg(QrzServerMessage);
    void loggerQrzMsg(QrzServerMessage);

private:

    static QrzServerRpc *qrzServerRpc;
    //QTimer SyncTimer;

     QVector<QrzServer> serverList;


private slots:

    //void SyncTimerTimer( );
    void on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);

    void on_notify(AnalysePubSubNotify an, const QString );



};

#endif // QRZSERVERRPC_H
