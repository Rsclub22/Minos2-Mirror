/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2021
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "AppStartup.h"
#include "MinosRPC.h"
#include "RPCCommandConstants.h"
#include "rigcontrolmainwindow.h"
#include "rigcontrolrpc.h"
#include "MTrace.h"

RigControlRpc::RigControlRpc(RigControlMainWindow *parent) : QObject(parent), parent(parent)
{


        trace("rigcontrol rpc init");
        trace(QString("app name %1").arg(getAppStartupName()));
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());

    connect(rpc, &MinosRPC::routerCall, this, &RigControlRpc::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &RigControlRpc::on_notify);

    QStringList sv = {rpcConstants::rigControlCategory
                      };
    rpc->initialiseRouters(sv);
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


void RigControlRpc::publishListChangedRadioNames(QVector<QSharedPointer<RadioNameChange> > listOfRadioNameChanges, QVector<QString> listOfRadiosDataChanged)
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();

    QString rpcData;
    QString listOfRadioNameChangesStr;
    QString listOfRadiosDataChangedStr;

    // package data
    if (!listOfRadioNameChanges.isEmpty())
    {

        for (const auto &lrnc: QASCONST(listOfRadioNameChanges))
        {

            QString rn;
            rn.append(lrnc->oldName + ',');
            rn.append(lrnc->newName);
            if (listOfRadioNameChangesStr.isEmpty())
            {
                listOfRadioNameChangesStr.append(rn);
            }
            else
            {
                listOfRadioNameChangesStr.append(rn.prepend(':'));
            }
        }

    }

    if (!listOfRadiosDataChanged.isEmpty())
    {
        for (const auto &n: QASCONST(listOfRadiosDataChanged))
        {
            if (listOfRadiosDataChangedStr.isEmpty())
            {
               listOfRadiosDataChangedStr.append(n);
            }
            else
            {
               listOfRadiosDataChangedStr.append(':' + n);
            }

        }
    }

    rpcData = listOfRadioNameChangesStr + '#' + listOfRadiosDataChangedStr;

    rpc->publish( rpcConstants::rigControlCategory, rpcConstants::rigControlChangeList, rpcData, psPublished );
}



void RigControlRpc::on_notify( AnalysePubSubNotify an, const QString /*from*/ )
{
    //trace( "Notify callback from " + from + ( !an.getOK() ? ":Error" : ":Normal" ) );

    // called whenever soemthing we subscribe to changes

    if ( an.getOK() )
    {
        if ( an.getState() == psPublished)
        {
            if ( an.getCategory() == rpcConstants::rigControlCategory && an.getKey() == rpcConstants::rigControlChangeList )
            {
                // In this case, ANOTHER rig control program has published when it made changes
                // and we need to take note of the changes

                MinosRPC *rpc = MinosRPC::getMinosRPC();

                QString publisherRouter = an.getPublisherRouter();
                QString publisherProgram = an.getPublisherProgram();

                QString s = MinosConfig::getMinosConfig()->getThisRouterName();

                if (publisherRouter == s && publisherProgram != rpc->getAppName())
                {
                    emit rereadConfig();
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void RigControlRpc::on_routerCall( bool err, QSharedPointer<MinosRPCObj>mro, const QString /*from*/ )
{
    //trace("Rig RPC: Rigcontrol callback from " + from + ( err ? ":Error" : ":Normal" ) );

    if ( !err )
    {
        QSharedPointer<RPCParam> psFreq;
        QSharedPointer<RPCParam> psBand;
        QSharedPointer<RPCParam> psMode;
        QSharedPointer<RPCParam> psName;
        QSharedPointer<RPCParam> psLoggerUuid;
        QSharedPointer<RPCParam> psVoiceMessageNum;
        QSharedPointer<RPCParam> psStopVoiceMessage;
        QSharedPointer<RPCParam> psCwMessage;
        QSharedPointer<RPCParam> psSelect;
        QSharedPointer<RPCParam> psRitFreq;
        QSharedPointer<RPCParam> psRitStatus;
        QSharedPointer<RPCParam> psVolLevel;
        QSharedPointer<RPCParam> psReq;
        QSharedPointer<RPCParam> psPttOnOff;



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
            PubSubName psn("test"); // just uses router/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                int volLevel;
                if ( psVolLevel->getInt( volLevel ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Vol Level From Logger = %1").arg(volLevel));
                    emit setVolume(volLevel);
                }
            }
        }

        if (args->getStructArgMember(0, rpcConstants::rigPttOnOff, psPttOnOff))
        {
            PubSubName psn("test"); // just uses router/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                bool pttOnOff;
                if ( psPttOnOff->getBoolean(pttOnOff))
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: PTT On/Off From Logger = %1").arg(pttOnOff ? "On" : "Off"));
                    emit setPttOnOff(pttOnOff);
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
            PubSubName psn("test"); // just uses router/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                QString mode;
                if ( psMode->getString( mode ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Mode Command From Logger = %1").arg(mode));
                    emit setMode(mode);
                }
            }
        }

        else if ( args->getStructArgMember( 0, rpcConstants::rigControlLogFreq, psFreq ))
        {
            PubSubName psn("test"); // just uses router/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                QString sfreq;
                if ( psFreq->getString( sfreq ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Freq Command From Logger = %1").arg(sfreq));
                    emit setFreq(Frequency(sfreq));
                }
            }
        }
        else if ( args->getStructArgMember( 0, rpcConstants::rigControlLogBand, psBand ))
        {
            PubSubName psn("test"); // just uses router/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                QString sBand;
                if ( psBand->getString( sBand ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Band Command From Logger = %1").arg(sBand));
                    emit setBand(sBand);
                }
            }
        }
        else if ( args->getStructArgMember( 0, rpcConstants::rigVoiceMessageNum, psVoiceMessageNum ))
        {
            PubSubName psn("test"); // just uses router/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                QString msgNum;
                if ( psVoiceMessageNum->getString( msgNum ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: VoiceMessage Number From Logger = %1").arg(msgNum));
                    emit setVoiceMessageNum(msgNum);
                }
            }
        }
        else if ( args->getStructArgMember( 0, rpcConstants::rigStopVoiceMessage, psStopVoiceMessage ))
        {
            PubSubName psn("test"); // just uses router/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                QString msg;
                if ( psStopVoiceMessage->getString( msg ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Stop VoiceMessage From Logger = %1").arg(msg));
                    emit setStopVoiceMessage(msg);
                }
            }
        }
        else if ( args->getStructArgMember( 0, rpcConstants::rigCwTxMessage, psCwMessage ))
        {
            PubSubName psn("test"); // just uses router/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                QString cwMsg;
                if ( psCwMessage->getString( cwMsg ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: CW Message Number From Logger = %1").arg(cwMsg));
                    emit setCwTXMessage(cwMsg);
                }
            }
        }

        else if ( args->getStructArgMember( 0, rpcConstants::rigControlLogRitFreq, psRitFreq ))
        {
            PubSubName psn("test"); // just uses router/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                QString ritFreq;
                if ( psRitFreq->getString( ritFreq ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Rit Freq Command From Logger = %1").arg(ritFreq));
                    emit setRitFreq(ShortFreq(ritFreq));
                }
            }
        }
        else if ( args->getStructArgMember( 0, rpcConstants::rigRitOnOffStatus, psRitStatus ) )
        {
            PubSubName psn("test"); // just uses router/appname
            QString cursel = rigCache.getSelectedContest(psn, loggeruuid);
            if (cursel == selContest)
            {
                bool ritStatus;
                if ( psRitStatus->getBoolean( ritStatus ) )
                {
                    // here you handle what the logger has sent to us
                    trace(QString("Rig RPC: Rit Status Command From Logger = %1").arg(ritStatus ? "On" : "Off"));
                    emit setRitStatus(ritStatus);
                }
            }
        }
    }
}
