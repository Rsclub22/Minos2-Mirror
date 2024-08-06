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




#include <QObject>
#include <QObject>
#include "XMPPRPCObj.h"
#include "AnalysePubSubNotify.h"
#include "winKeyerCommon.h"


static bool syncstat = false;

class WinkeyerRpc : public QObject
{
    Q_OBJECT
public:

    static const char * winkeyerServerStateIndicator[];
    static const char * stateList[];

    explicit WinkeyerRpc();
    virtual ~WinkeyerRpc();

    static WinkeyerRpc *getWinkeyerRpc();

signals:

    //void winkeyerMsg(QrzServerMessage);
    //void loggerWinkeyerMsg(QrzServerMessage);

private:

    static WinkeyerRpc *winkeyerServerRpc;
    QVector<WinkeyerServer> serverList;

private slots:

    void on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from);

    void on_notify(AnalysePubSubNotify an, const QString );


};

#endif // WINKEYERRPC_H
