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
        QSharedPointer<RPCParam> psTpm;
        QSharedPointer<RPCParam> psVolLevel;


        RPCArgs *args = mro->getCallArgs();

        QString selContest;
        QString loggeruuid;
        if ( args->getStructArgMember( 0, rpcConstants::loggerUuid, psLoggerUuid ))
        {
            psLoggerUuid->getString( loggeruuid);
        }
        if ( args->getStructArgMember( 0, rpcConstants::selected, psSelect ))
        {
            psSelect->getString( selContest );

        }

        if ( args->getStructArgMember( 0, rpcConstants::rigTpm, psTpm ) )
        {
            if ( args->getStructArgMember( 0, rpcConstants::rigControlFreq, psFreq ))
            {
                PubSubName psn("test"); // just uses server/appname
                QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
                if (cursel == selContest)
                {
                    QString freq;
                    int t;
                    if ( psFreq->getString( freq ) && psTpm->getInt( t ) )
                    {
                        // here you handle what the logger has sent to us
                        trace(QString("Rig RPC: Tpm From Logger = %1 freq = %2").arg(t).arg(freq));
                        emit (setTpm(t, freq));
                    }
                }
            }
        }
        else if ( args->getStructArgMember( 0, rpcConstants::rigControlFreq, psFreq ))
        {
            PubSubName psn("test"); // just uses server/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
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
        else if ( args->getStructArgMember( 0, rpcConstants::rigVolLevel, psVolLevel ))
        {
            PubSubName psn("test"); // just uses server/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                int volLevel;
                if ( psVolLevel->getInt( volLevel ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Vol Level From Logger = %1").arg(volLevel));
                    emit (setVolume(volLevel));
                }
            }
        }
        else if (args->getStructArgMember(0, rpcConstants::rigControlRadioName, psName))
        {
            QString name;
            if (psName->getString(name))
            {
                trace(QString("Rig RPC: select Command for radio = %1").arg(name));
                PubSubName psn(name);

                if (rigCache.setSelected(psn, loggeruuid, selContest))
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
                    psn = rigCache.getSelectedRadio(psn);
                    emit selectLoggerRadio(psn, mode);
                }
                else
                {
                    // reply with failure and the current selection
                }
            }
        }
        else if ( args->getStructArgMember( 0, rpcConstants::rigControlMode, psMode ) )
        {
            PubSubName psn("test"); // just uses server/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
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
            PubSubName psn("test"); // just uses server/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
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
            PubSubName psn("test"); // just uses server/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
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
