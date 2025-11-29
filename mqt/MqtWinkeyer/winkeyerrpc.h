/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef WINKEYERRPC_H
#define WINKEYERRPC_H




#include "MinosRPC.h"
#include "AnalysePubSubNotify.h"

#include "winKeyerCommon.h"




class WinkeyerRpc : public QObject
{
    Q_OBJECT
public:


    explicit WinkeyerRpc();
    virtual ~WinkeyerRpc();


    void publishState( const QString &comport, const QString &state, const QString &errorMsg );

    int getServerListCount();
    void publishPttEnable(const bool state);
    void publishTxOn(const QString txOn);

    void publishWpm(const int wpm);

signals:

    void cwMessageFromLoggerToKeyer(QString);
    void cwStopCommandFromLogger(QString);
    void wpmFromLog(int);



private slots:

    void on_notify(AnalysePubSubNotify an, const QString);
    void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);
    void on_provider(Provider provider, QString cat);



};

#endif // WINKEYERRPC_H
