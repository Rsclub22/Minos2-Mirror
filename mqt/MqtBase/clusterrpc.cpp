/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2018
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include "base_pch.h"
#include "clusterrpc.h"

ClusterRpc::ClusterRpc()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));

    // we aren't subscribing to anything!
}


void ClusterRpc::on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &/*from*/ )
{
    AnalysePubSubNotify an( err, mro );

    if ( an.getOK() )
    {

    }
}


void ClusterRpc::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
    //trace( "chat callback from " + from + ( err ? ":Error" : ":Normal" ) );

    // Should we use QMap to give a list of name/value pairs?
    // BUT the value isn't always the same type - should it be?
    // We could use QVariant, of course...

    if ( !err )
    {
        RPCArgs *args = mro->getCallArgs();

        if (args)
        {
            QSharedPointer<RPCParam> psSpot;
            if (args->getStructArgMember(0, rpcConstants::SendClusterSpot, psSpot))
            {
                QString pSpot;
                if (psSpot->getString(pSpot))
                {
                    // got spot

                }
            }
        }
    }
}



void ClusterRpc::sendSpot(QString spot)
{
    RPCGeneralClient rpc(rpcConstants::clusterMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);
    st->addMember( spot, rpcConstants::SendClusterSpot );
    rpc.getCallArgs() ->addParam( st );
    rpc.queueCall( clusterApp );

}
