/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"
#include "rotatormainwindow.h"
#include "rotatorRpc.h"

//RotatorRpc *rotatorRpc;

RotatorRpc::RotatorRpc(RotatorMainWindow *parent) : QObject(parent), parent(parent)
{
//    rotatorRpc = this;

    MinosRPC *rpc = MinosRPC::getMinosRPC(rpcConstants::rotatorApp);

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));

    // we aren't subscribing to anything!

}
//---------------------------------------------------------------------------
void RotatorRpc::publishAntennaList(QString ants)
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    rotatorCache.addRotList(ants);
    rpc->publish( rpcConstants::RotatorCategory, rpcConstants::rotatorList, ants, psPublished );
}


void RotatorRpc::publishPresetList(QString presets)
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    rpc->publish( rpcConstants::RotatorCategory, rpcConstants::rotPresetList, presets, psPublished );
}

void RotatorRpc::on_notify( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
   trace( "Rot Rpc: Notify callback from " + from + ( err ? ":Error" : ":Normal" ) );
   AnalysePubSubNotify an( err, mro );

   // called whenever soemthing we subscribe to changes
   if ( an.getOK() )
   {
// example from elsewhere
//      if ( an.getCategory() == rpcConstants::lineControlCategory )
//      {
//         lineStates[ an.getKey() ] = ( ( an.getValue() == rpcConstants::lineSet ) ? true : false );
//         trace( rpcConstants::lineControlCategory " " " + an.getKey() + ":" + an.getValue() );
//      }
   }
}
//---------------------------------------------------------------------------
void RotatorRpc::on_serverCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
    trace( "Rot RPC: rotator callback from " + from + ( err ? ":Error" : ":Normal" ) );

    if ( !err )
    {
        QSharedPointer<RPCParam> psDirection;
        QSharedPointer<RPCParam> psAngle;
        QSharedPointer<RPCParam> psAntName;
        QSharedPointer<RPCParam> psSelect;
        QSharedPointer<RPCParam> psLoggerUuid;
        QSharedPointer<RPCParam> psRotPreset;
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
        if ( args->getStructArgMember( 0, rpcConstants::rotatorParamDirection, psDirection )
             && args->getStructArgMember( 0, rpcConstants::rotatorParamAngle, psAngle ) )
        {
            int direction;
            int angle;

            if (rotatorCache.getSelectedContest(loggeruuid) == sel)
            {
                if ( psDirection->getInt( direction ) && psAngle->getInt( angle ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rot RPC: Direction = %1, Angle = %2").arg(QString::number(direction), QString::number(angle)));
                    emit (setRotation(direction, angle));
                }
            }
        }
        else if (args->getStructArgMember(0, rpcConstants::rotatorAntennaName, psAntName))
        {
            // how do we handle deselecting the rotator? Or is it automatic?
            QString name;
            if (psAntName->getString(name))
            {
                PubSubName psn(name);

                if ( !sel.isEmpty() && !loggeruuid.isEmpty() )
                {
                    PubSubName selected = rotatorCache.getSelected("");
                    if (selected.isEmpty() || selected.key() == name)
                    {
                     // here you handle what the logger has sent to us
                        trace(QString("Rig RPC: select Command From Logger = %1").arg(sel));
                        rotatorCache.setSelected(psn, loggeruuid, sel);
                        emit selectAntennaFromLog(psn);
                    }
                }
                else
                {
                    // reply with failure and the current selection
                }
            }
        }
        else if (args->getStructArgMember(0, rpcConstants::rotPreset, psRotPreset))
        {
            QString rotPreset;
            if (psRotPreset->getString(rotPreset))
            {
                if (!rotatorCache.getSelected(loggeruuid).isEmpty())
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: rotPreset Command From Logger = %1").arg(rotPreset));
                    emit setRotPreset(rotPreset);
                }
            }
        }
    }
}

