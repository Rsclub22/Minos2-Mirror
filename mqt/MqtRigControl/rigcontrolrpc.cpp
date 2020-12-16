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

        trace("rigcontrol rpc init");
        trace(QString("app name %1").arg(getAppStartupName()));
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());

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
        QSharedPointer<RPCParam> psBand;
        QSharedPointer<RPCParam> psMode;
        QSharedPointer<RPCParam> psName;
        QSharedPointer<RPCParam> psLoggerUuid;
        QSharedPointer<RPCParam> psVoiceMessageNum;
        QSharedPointer<RPCParam> psSelect;
        QSharedPointer<RPCParam> psRitFreq;
        QSharedPointer<RPCParam> psRitStatus;
        QSharedPointer<RPCParam> psVolLevel;
        QSharedPointer<RPCParam> psReq;


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



        if ( args->getStructArgMember( 0, rpcConstants::rigLogVolLevel, psVolLevel ))
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
        else if (args->getStructArgMember(0, rpcConstants::rigControlSelectRadioName, psName))
        {
            QString name;
            if (psName->getString(name))
            {
                trace(QString("Rig RPC: select Command for radio = %1").arg(name));
                PubSubName psn(name);

                if (rigCache.setSelected(psn, loggeruuid, selContest))
                {
                    QString mode;
                    if ( args->getStructArgMember( 0, rpcConstants::rigControlLogMode, psMode ) )
                    {
                        if ( psMode->getString( mode ) )
                        {
                            // here you handle what the logger has sent to us
                            trace(QString("Rig RPC: Select Radio Mode Command From Logger = %1").arg(mode));
                        }
                    }

                    QString sfreq;
                    if ( args->getStructArgMember(0, rpcConstants::rigControlLogFreq, psFreq))
                    {
                        if (psFreq->getString(sfreq))
                        {
                            trace(QString("Rig RPC: Select Radio Freq Command From Logger = %1").arg(sfreq));
                        }
                    }

                    QString sBand;
                    if ( args->getStructArgMember(0, rpcConstants::rigControlLogBand, psBand))
                    {
                        if (psBand->getString(sBand))
                        {
                            trace(QString("Rig RPC: Select Radio Band Command From Logger = %1").arg(sBand));
                        }
                    }

                    psn = rigCache.getSelectedRadio(psn);
                    emit selectLoggerRadio(psn, sBand, Frequency(sfreq), mode);
                }
                else
                {
                    // reply with failure and the current selection
                }
            }
        }
        else if ( args->getStructArgMember( 0, rpcConstants::rigControlLogMode, psMode ) )
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

        else if ( args->getStructArgMember( 0, rpcConstants::rigControlLogFreq, psFreq ))
        {
            PubSubName psn("test"); // just uses server/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                QString sfreq;
                if ( psFreq->getString( sfreq ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Freq Command From Logger = %1").arg(sfreq));
                    emit (setFreq(Frequency(sfreq)));
                }
            }
        }
        else if ( args->getStructArgMember( 0, rpcConstants::rigVoiceMessageNum, psVoiceMessageNum ))
        {
            PubSubName psn("test"); // just uses server/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                QString msgNum;
                if ( psVoiceMessageNum->getString( msgNum ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: VoiceMessage Number From Logger = %1").arg(msgNum));
                    emit (setVoiceMessageNum(msgNum));
                }
            }
        }

        else if ( args->getStructArgMember( 0, rpcConstants::rigControlLogRitFreq, psRitFreq ))
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
                    emit (setRitFreq(ShortFreq(ritFreq)));
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
                    trace(QString("Rig RPC: Rit Status Command From Logger = %1").arg(ritStatus ? "On" : "Off"));
                    emit (setRitStatus(ritStatus));
                }
            }
        }
    }
}
