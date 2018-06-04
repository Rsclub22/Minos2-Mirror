/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"
#include "rigcontrolmainwindow.h"
#include "rigcontrolrpc.h"

//RigControlRpc *rigControlRpc;

RigControlRpc::RigControlRpc(RigControlMainWindow *parent) : QObject(parent), parent(parent)
{
//    rigControlRpc = this;

    MinosRPC *rpc = MinosRPC::getMinosRPC(rpcConstants::rigControlApp);

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));

    // we aren't subscribing to anything!
}

//--------------------------------------------------------------------------------------------------//

// this publishes the list of radios configured in rigcontrol

void RigControlRpc::publishRadioNames(QStringList radios)
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();

    QString nameList = radios.join(":");
    rigCache.addRigList(nameList);
    rpc->publish( rpcConstants::rigControlCategory, rpcConstants::rigControlRadioList, nameList, psPublished );
}

void RigControlRpc::on_notify( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
    trace( "Notify callback from " + from + ( err ? ":Error" : ":Normal" ) );
    AnalysePubSubNotify an( err, mro );

    // called whenever soemthing we subscribe to changes
    if ( an.getOK() )
    {
        // example from elsewhere
        //      if ( an.getCategory() == rpcConstants::lineControlCategory )
        //      {
        //         lineStates[ an.getKey() ] = ( ( an.getValue() == rpcConstants::lineSet ) ? true : false );
        //         trace( rpcConstants::lineControlCategory + " " + an.getKey() + ":" + an.getValue() );
        //      }
    }
}
//---------------------------------------------------------------------------
void RigControlRpc::on_serverCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
    trace("Rig RPC: Rigcontrol callback from " + from + ( err ? ":Error" : ":Normal" ) );

    if ( !err )
    {
        QSharedPointer<RPCParam> psFreq;
        QSharedPointer<RPCParam> psMode;
        QSharedPointer<RPCParam> psName;
        QSharedPointer<RPCParam> psLoggerUuid;
        QSharedPointer<RPCParam> psSelect;
        QSharedPointer<RPCParam> psRitFreq;
        QSharedPointer<RPCParam> psRitStatus;

        RPCArgs *args = mro->getCallArgs();

        QString sel;
        QString loggeruuid;
        if ( args->getStructArgMember( 0, rpcConstants::loggerUuid, psLoggerUuid ))
        {
            psLoggerUuid->getString( loggeruuid);
        }
        if ( args->getStructArgMember( 0, rpcConstants::selected, psSelect ))
        {
            psSelect->getString( sel );

        }

        if ( args->getStructArgMember( 0, rpcConstants::rigControlFreq, psFreq ))
        {
            if (rigCache.getSelectedContest(loggeruuid) == sel)
            {
                QString freq;
                if ( psFreq->getString( freq ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Freq Command From Logger = %1").arg(freq));
                    emit (setFreq(freq));
                }
            }
        }
        else if (args->getStructArgMember(0, rpcConstants::rigControlRadioName, psName))
        {
            QString name;
            if (psName->getString(name))
            {
                PubSubName psn(name);
                if ( !sel.isEmpty() && !loggeruuid.isEmpty() )
                {
                    PubSubName selected = rigCache.getSelected("");
                    if (selected.isEmpty() || selected.key() == name)
                    {
                        QString mode;
                        if ( args->getStructArgMember( 0, rpcConstants::rigControlMode, psMode ) )
                        {
                            if ( psMode->getString( mode ) )
                            {
                                // here you handle what the logger has sent to us
                                trace(QString("Rig RPC: Mode Command From Logger = %1").arg(mode));
                            }
                        }
                        // here you handle what the logger has sent to us
                        rigCache.setSelected(psn, loggeruuid, sel);
                        trace(QString("Rig RPC: select Command From Logger = %1 psn=%2, sel=%3").arg(sel).arg(psn.toString()).arg(sel));

                        emit selectLoggerRadio(psn, mode);
                    }
                }
            }
        }
        else if ( args->getStructArgMember( 0, rpcConstants::rigControlMode, psMode ) )
        {
            if (rigCache.getSelectedContest(loggeruuid) == sel)
            {
                QString mode;
                if ( psMode->getString( mode ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Mode Command From Logger = %1").arg(mode));
                    emit (setMode(mode));
                }
            }
        }
        if ( args->getStructArgMember( 0, rpcConstants::rigControlRitFreq, psRitFreq ))
        {
            if (rigCache.getSelectedContest(loggeruuid) == sel)
            {
                QString ritFreq;
                if ( psRitFreq->getString( ritFreq ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Rit Freq Command From Logger = %1").arg(ritFreq));
                    emit (setRitFreq(ritFreq));
                }
            }
        }
        else if ( args->getStructArgMember( 0, rpcConstants::rigRitOnOffStatus, psRitStatus ) )
        {
            if (rigCache.getSelectedContest(loggeruuid) == sel)
            {
                bool ritStatus;
                if ( psRitStatus->getBoolean( ritStatus ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Rit Status Command From Logger = %1").arg(ritStatus));
                    emit (setRitStatus(ritStatus));
                }
            }
        }
    }
}
