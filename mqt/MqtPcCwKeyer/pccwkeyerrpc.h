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

#include "MinosRPC.h"
#include "AnalysePubSubNotify.h"

/*
class PcCwKeyerServer
{
public:
    QString routerName;
    QString app;
    QString publisherProgram;
    PublishState state;

};
*/



class PcCwKeyerRpc : public QObject
{
    Q_OBJECT

public:



    explicit PcCwKeyerRpc();
    virtual ~PcCwKeyerRpc();


    void publishState( const QString &comport, const QString &state, const QString &errorMsg );

    int getServerListCount();
    void publishPttEnable(const bool state);
    void publishTxOn(const QString txOn);

signals:

    void cwMessageFromLoggerToKeyer(QString);
    void cwStopCommandFromLogger(QString);




private slots:

    void on_notify(AnalysePubSubNotify an, const QString);
    void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);
    void on_provider(Provider provider, QString cat);




};




#endif // PCCWKEYERRPC_H
