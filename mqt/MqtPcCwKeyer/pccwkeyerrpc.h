/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Pc Serial Port DTR CW Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef PCCWKEYERRPC_H
#define PCCWKEYERRPC_H

#include <QObject>

#include "XMPPRPCObj.h"
#include "AnalysePubSubNotify.h"

class PcCwKeyerServer
{
public:
    QString routerName;
    QString app;
    QString publisherProgram;
    PublishState state;

};




class PcCwKeyerRpc : public QObject
{
    Q_OBJECT

public:

    static const char *pcCwKeyerServerStateIndicator[];
    static const char * stateList[];

    explicit PcCwKeyerRpc();
    virtual ~PcCwKeyerRpc();

    static PcCwKeyerRpc *getPcCwKeyerRpc();


    void publishState( const QString &raw, const QString &state );

signals:

    void cwMessageFromLoggerToKeyer(QString);
    void cwStopCommandFromLogger(QString);


private:

    static PcCwKeyerRpc *pcCwKeyerRpc;

    QVector<PcCwKeyerServer> serverList;

private slots:

    void on_notify(AnalysePubSubNotify an, const QString);
    void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);



};




#endif // PCCWKEYERRPC_H
