/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"

#include "SendRPCDM.h"
#include "tsinglelogframe.h"
#include "tlogcontainer.h"
#include "rigutils.h"
//---------------------------------------------------------------------------
TSendDM::TSendDM(QWidget* Owner )
    : QObject( Owner )
{
    loggerUuid = makeUuid();
    trace("logger uuid is " + loggerUuid);

    MinosRPC *rpc = MinosRPC::getMinosRPC(rpcConstants::loggerApp);
    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));

}
TSendDM::~TSendDM()
{
}
void TSendDM::invalidateCache()
{
    rigCache.invalidate();
    rotatorCache.invalidate();
}
void TSendDM::invalidateRigCache(const PubSubName &name)
{
    rigCache.invalidate(name);
}
void TSendDM::invalidateRotatorCache(const PubSubName &name)
{
    rotatorCache.invalidate(name);
}

//---------------------------------------------------------------------------
void TSendDM::sendKeyerPlay( TSingleLogFrame *tslf, int fno )
{
    if (!keyerApp.isEmpty())
    {
        RPCGeneralClient rpc(rpcConstants::keyerMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        QSharedPointer<RPCParam>sName(new RPCStringParam( rpcConstants::keyerPlayFile ));
        QSharedPointer<RPCParam>iValue(new RPCIntParam( fno ));
        QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
        QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
        st->addMember( logger, rpcConstants::loggerUuid );
        st->addMember( select, rpcConstants::selected );
        st->addMember( sName, rpcConstants::paramName );
        st->addMember( iValue, rpcConstants::paramValue );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( keyerApp );
    }
}
void TSendDM::sendKeyerRecord( TSingleLogFrame *tslf, int fno )
{
    if (!keyerApp.isEmpty())
    {
        RPCGeneralClient rpc(rpcConstants::keyerMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        QSharedPointer<RPCParam>sName(new RPCStringParam( "RecordFile" ));
        QSharedPointer<RPCParam>iValue(new RPCIntParam( fno ));
        QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
        QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
        st->addMember( logger, rpcConstants::loggerUuid );
        st->addMember( select, rpcConstants::selected );
        st->addMember( sName, rpcConstants::paramName );
        st->addMember( iValue, rpcConstants::paramValue );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( keyerApp );
    }
}

void TSendDM::sendKeyerTone(TSingleLogFrame *tslf)
{
    if (!keyerApp.isEmpty())
    {
        RPCGeneralClient rpc(rpcConstants::keyerMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        QSharedPointer<RPCParam>sName(new RPCStringParam( "Tone" ));
        QSharedPointer<RPCParam>iValue(new RPCIntParam( 0 ));
        QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
        QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
        st->addMember( logger, rpcConstants::loggerUuid );
        st->addMember( select, rpcConstants::selected );
        st->addMember( sName, rpcConstants::paramName );
        st->addMember( iValue, rpcConstants::paramValue );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( keyerApp );
    }
}
void TSendDM::sendKeyerTwoTone(TSingleLogFrame *tslf)
{
    if (!keyerApp.isEmpty())
    {
        RPCGeneralClient rpc(rpcConstants::keyerMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        QSharedPointer<RPCParam>sName(new RPCStringParam( "TwoTone" ));
        QSharedPointer<RPCParam>iValue(new RPCIntParam( 0 ));
        QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
        QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
        st->addMember( logger, rpcConstants::loggerUuid );
        st->addMember( select, rpcConstants::selected );
        st->addMember( sName, rpcConstants::paramName );
        st->addMember( iValue, rpcConstants::paramValue );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( keyerApp );
    }
}
void TSendDM::sendKeyerStop(TSingleLogFrame *tslf)
{
    if (!keyerApp.isEmpty())
    {
        RPCGeneralClient rpc(rpcConstants::keyerMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        QSharedPointer<RPCParam>sName(new RPCStringParam( "Stop" ));
        QSharedPointer<RPCParam>iValue(new RPCIntParam( 0 ));
        QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
        QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
        st->addMember( logger, rpcConstants::loggerUuid );
        st->addMember( select, rpcConstants::selected );
        st->addMember( sName, rpcConstants::paramName );
        st->addMember( iValue, rpcConstants::paramValue );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( keyerApp );
    }
}
//---------------------------------------------------------------------------

void TSendDM::sendBandMap(  TSingleLogFrame * tslf, const QString &freq, const QString &call, const QString &utc, const QString &loc, const QString &qth )
{
   RPCGeneralClient rpc(rpcConstants::bandmapMethod);
   QSharedPointer<RPCParam>st(new RPCParamStruct);

   st->addMember( loggerUuid, rpcConstants::loggerUuid );
   st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
   //st->addMember( rpcConstants::bandmapApp, rpcConstants::bandmapParamName );
   st->addMember( freq, rpcConstants::bandmapParamFreq );
   st->addMember( call, rpcConstants::bandmapParamCallsign );
   st->addMember( loc, rpcConstants::bandmapParamLocator );
   st->addDtgMember( utc, rpcConstants::bandmapParamUTC );
   st->addMember( qth, rpcConstants::bandmapParamQTH );

   rpc.getCallArgs() ->addParam( st );
//   rpc.queueCall( tslf->bandMapServerConnectable.remoteAppName + "@" + tslf->bandMapServerConnectable.serverName );
}

void TSendDM::sendRotator(TSingleLogFrame *tslf, rpcConstants::RotateDirection direction, int angle )
{
    RPCGeneralClient rpc(rpcConstants::rotatorMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );
    QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
    st->addMember( select, rpcConstants::selected );
    st->addMember( static_cast<int> (direction), rpcConstants::rotatorParamDirection );
    st->addMember( angle, rpcConstants::rotatorParamAngle );
    rpc.getCallArgs() ->addParam( st );

    PubSubName rotSelected = rotatorCache.getSelected(loggerUuid);
    rpc.queueCall( rotSelected );
}
void TSendDM::changeRotatorSelectionTo(const PubSubName &name, const QString &uuid)
{
    // we should de-select the cached uuid on all rotator apps
    trace(QString("Change rotator selection to %1 %2").arg(name.toString()).arg(uuid));

    PubSubName selected = rotatorCache.getSelected(loggerUuid);

    rotatorCache.setSelected(selected, loggerUuid, ""); // deselect the old one
    if (!selected.isEmpty() && selected != name)
        sendRotatorSelection(selected, "");

    if (!name.isEmpty() && rotatorCache.setSelected(name, loggerUuid, uuid))
    {
        sendRotatorSelection(name, uuid);
    }
    //emit RotatorList(rotators().join(":"));
}
void TSendDM::sendRotatorSelection(const PubSubName &s, const QString &uuid)
{
    RPCGeneralClient rpc(rpcConstants::rotatorMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );

    QSharedPointer<RPCParam>select(new RPCStringParam(uuid ));
    st->addMember( select, rpcConstants::selected );

    st->addMember( s.toString(), rpcConstants::rotatorAntennaName );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( s );
}

void TSendDM::changeRigSelectionTo(const PubSubName &name, const QString &mode, const QString &uuid)
{
    // we should de-select the cached uuid on all rig apps

    trace(QString("Change rig selection to %1 %2 %3").arg(name.toString()).arg(mode).arg(uuid));

    PubSubName selected = rigCache.getSelected(loggerUuid);

    if (!selected.isEmpty() && selected != name)
    {
        sendRigSelection(selected, "", "");
    }
    sendRigSelection(name, mode, uuid);

    //emit setRadioList(rigs().join(":"));    // need to CALL this?

}
void TSendDM::sendRigSelection(const PubSubName &s, const QString &mode, const QString &uuid)
{
    rigCache.setSelected(s, loggerUuid, uuid);
    rigCache.setMode(s, mode);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );
    QSharedPointer<RPCParam>select(new RPCStringParam(uuid ));
    st->addMember( select, rpcConstants::selected );

    st->addMember( s.toString(), rpcConstants::rigControlRadioName );
    st->addMember( mode, rpcConstants::rigControlMode );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( s );

}

void TSendDM::sendRigControlFreq(TSingleLogFrame *tslf,const QString &freq)
{
    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setFreq(rigSelected, convertStrToFreq(freq));
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );

    QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
    st->addMember( select, rpcConstants::selected );

    st->addMember( freq, rpcConstants::rigControlFreq );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
}


void TSendDM::sendRigControlMode(TSingleLogFrame *tslf,const QString &mode)
{
    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setMode(rigSelected, mode);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );
    QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
    st->addMember( select, rpcConstants::selected );
    st->addMember( mode, rpcConstants::rigControlMode );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
}



void TSendDM::sendRigControlRitFreq(TSingleLogFrame *tslf,const QString &freq)
{
    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setFreq(rigSelected, convertStrToFreq(freq));
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );
    QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
    st->addMember( select, rpcConstants::selected );
    st->addMember( freq, rpcConstants::rigControlRitFreq );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
}


void TSendDM::sendRigControlRitStatus(TSingleLogFrame *tslf, const bool &status)
{
    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setRitEnableStatus(rigSelected, status);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );
    QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
    st->addMember( select, rpcConstants::selected );
    st->addMember( status, rpcConstants::rigRitOnOffStatus);
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
}






void TSendDM::sendRotatorPreset(QString s)
{
    RPCGeneralClient rpc(rpcConstants::rotatorMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );

    st->addMember( s, rpcConstants::rotPreset );
    rpc.getCallArgs() ->addParam( st );

    PubSubName rotSelected = rotatorCache.getSelected(loggerUuid);
    rpc.queueCall( rotSelected );
}
//---------------------------------------------------------------------------
void TSendDM::on_notify( bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
    // PubSub notifications
    AnalysePubSubNotify an( err, mro );
    trace( "Notify callback from " + from + ( err ? ":Error " : ":Normal " ) +  an.getPublisherProgram() + "@" + an.getPublisherServer());

    if ( an.getOK())
    {
        if ( an.getState() == psPublished)
        {
            trace(QString("SendRPC category %1 key %2").arg(an.getCategory()).arg(an.getKey()));
            if ( an.getCategory() == rpcConstants::rigStateCategory)
            {
                rigCache.setStateString(an);
            }
            if ( an.getCategory() == rpcConstants::rigDetailsCategory)
            {
                rigCache.setDetailsString(an);
            }
            else if ( an.getCategory() == rpcConstants::rotatorStateCategory )
            {
                rotatorCache.setStateString(an);
            }
            else if ( an.getCategory() == rpcConstants::rotatorDetailCategory )
            {
                rotatorCache.setDetailString(an);
            }
            else if ( an.getCategory() == rpcConstants::rotatorPresetsCategory )
            {
                rotatorCache.setPresetsString(an);
            }
            else if ( an.getCategory() == rpcConstants::rigControlCategory && an.getKey() == rpcConstants::rigControlRadioList )
            {
                trace("SendRPC set rigList and loaded " + an.getValue());
                rigCache.addRigList(an.getValue());
                radioLoaded = true;
                emit setRadioLoaded();
                emit setRadioList(an.getValue());
            }
            else if ( an.getCategory() == rpcConstants::RotatorCategory && an.getKey() == rpcConstants::rotatorList )
            {
                trace("SendRPC set rotList and loaded " + an.getValue());
                rotatorCache.addRotList(an.getValue());
                rotatorLoaded = true;
                emit RotatorLoaded();
                emit RotatorList(an.getValue());
            }
        }

        QVector<TSingleLogFrame *> frames = LogContainer->getLogFrames();
        {
            PubSubName rigSelected = rigCache.getSelected(loggerUuid);
            if (!rigSelected.isEmpty())
            {
                RigState &selState = rigCache.getState(rigSelected);
                QString selStateUuid = selState.selected(loggerUuid).getValue();
                RigDetails &selDetail = rigCache.getDetails(rigSelected);
                QString selDetailsUuid = selDetail.selected(loggerUuid).getValue();
                if (!selStateUuid.isEmpty())
                {

                    for (int i = 0; i < frames.size(); i++)
                    {
                        TSingleLogFrame *tslf = frames[i];
                        QString frameUuid = tslf->getContest()->uuid;

                        if (selStateUuid == frameUuid)
                        {
                            trace("Rig state distribution for " + selStateUuid);
                            if (selState.mode().isDirty())
                            {
                                trace("SendRPC Rig set mode " + selState.mode().getValue());
                                tslf->on_SetMode(selState.mode().getValue());
                            }
                            if (selState.freq().isDirty())
                            {
                                trace("SendRPC Rig set freq " + convertFreqToStr(selState.freq().getValue()));
                                tslf->on_SetFreq(convertFreqToStr(selState.freq().getValue()));
                            }
                            if (selState.status().isDirty())
                            {
                                trace("SendRPC Rig set status " + selState.status().getValue());
                                tslf->on_SetRadioStatus(selState.status().getValue());
                            }
                            selState.clearDirty();
                        }
                        if (selDetailsUuid == frameUuid)
                        {
                            trace("Rig details distribution for " + selDetailsUuid);
                            if (selDetail.bandList().isDirty())
                            {
                                trace("SendRPC Rig set bandList " + selDetail.bandList().getValue());
                                tslf->on_SetBandList(selDetail.bandList().getValue());
                            }
                            if (selDetail.transverterStatus().isDirty())
                            {
                                trace(QString("SendRPC Rig set transverter status ") + (selDetail.transverterStatus().getValue() ? " True" : " False"));
                                tslf->on_SetRadioTxVertState( selDetail.transverterStatus().getValue() );
                            }
                            if (selDetail.ritEnableStatus().isDirty())
                            {
                                trace(QString("SendRPC Rig set ritEnable status ") + (selDetail.ritEnableStatus().getValue() ? " True" : " False"));
                                tslf->on_SetRitEnableState( selDetail.ritEnableStatus().getValue());
                            }
                            selDetail.clearDirty();

                        }
                    }
                }
            }
            {
                PubSubName rotSelected = rotatorCache.getSelected(loggerUuid);
                if (!rotSelected.isEmpty())
                {
                    AntennaState &selState = rotatorCache.getState(rotSelected);
                    QString selStateUuid = selState.selected(loggerUuid).getValue();
                    AntennaDetail &selDetail = rotatorCache.getDetails(rotSelected);
                    QString selDetailUuid = selState.selected(loggerUuid).getValue();
                    if (!selStateUuid.isEmpty())
                    {
                        for (int i = 0; i < frames.size(); i++)
                        {
                            TSingleLogFrame *tslf = frames[i];
                            QString frameUuid = tslf->getContest()->uuid;

                            if (selStateUuid == frameUuid)
                            {
                                trace("Rotator state distribution for " + selStateUuid);

                                if (selState.bearing().isDirty())
                                {
                                    trace("SendRPC Rotator set bearing " + selState.bearing().getValue());
                                    tslf->on_RotatorBearing(selState.bearing().getValue());
                                }
                                if (selState.status().isDirty())
                                {
                                    trace("SendRPC Rotator set status " + selState.status().getValue());
                                    tslf->on_RotatorStatus(selState.status().getValue());
                                }
                                selState.clearDirty();
                            }
                            if (selDetailUuid == frameUuid)
                            {
                                trace("Rotator details distribution for " + selDetailUuid);
                                if (selDetail.maxAzimuth().isDirty())
                                {
                                    trace(QString("SendRPC Rotator set maxAzimuth %1").arg(selDetail.maxAzimuth().getValue()));
                                    tslf->on_RotatorMaxAzimuth(QString::number(selDetail.maxAzimuth().getValue()));
                                }
                                if (selDetail.minAzimuth().isDirty())
                                {
                                    trace(QString("SendRPC Rotator set minAzimuth %1").arg(selDetail.minAzimuth().getValue()));
                                    tslf->on_RotatorMinAzimuth(QString::number(selDetail.minAzimuth().getValue()));
                                }
                                if (selDetail.cwCcwCmdEnable().isDirty())
                                {
                                    trace(QString("SendRPC Rotator set cwCcwCmdEnable %1").arg(selDetail.cwCcwCmdEnable().getValue() ? "True" : "False"));
                                    tslf->on_cwCcwCmdEnable(selDetail.cwCcwCmdEnable().getValue());
                                }
                                selDetail.clearDirty();

                            }
                            if (rotatorCache.rotatorPresetsIsDirty(rotSelected))
                            {
                                trace("SendRPC Rotator set presets " + rotatorCache.getRotatorPresets(rotSelected));
                                tslf->on_RotatorPresetList(rotatorCache.getRotatorPresets(rotSelected));
                            }
                        }
                        rotatorCache.rotatorPresetsClearDirty();
                    }
                }
            }
        }
        for (int i = 0; i < frames.size(); i++)
        {
            //TSingleLogFrame *tslf = frames[i];

            //if (an.getPublisherProgram() == tslf->keyerServerConnectable.remoteAppName && an.getPublisherServer() == tslf->keyerServerConnectable.serverName)
            {
                if ( an.getCategory() == rpcConstants::KeyerCategory && an.getKey() == rpcConstants::keyerReport )
                {
                    if (keyerApp.isEmpty())
                        keyerApp = PubSubName(an);
                    emit setKeyerLoaded();
                    LogContainer->setCaption( an.getValue() );
                    trace( "KeyerReport " + an.getValue() );
                    break;
                }
            }
            /*
            if (an.getPublisherProgram() == tslf->bandMapServerConnectable.remoteAppName && an.getPublisherServer() == tslf->bandMapServerConnectable.serverName)
            {
                if ( an.getCategory() == rpcConstants::BandMapCategory && an.getKey() == rpcConstants::bandmapKeyLoaded )
                {
                    emit setBandMapLoaded();
                    break;
                }
            }
            */
        }
    }
}
//---------------------------------------------------------------------------
void TSendDM::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
    // responds to pull calls from the monitoring client
    trace( "request callback from " + from + ( err ? ":Error" : ":Normal" ) );
    trace("method is " + mro->getMethodName());

    // need to check "from" is correct
    if ( !err )
    {
        QSharedPointer<RPCParam> psLogName;
        QSharedPointer<RPCParam>psStanza;
        RPCArgs *args = mro->getCallArgs();

        QString call = mro->getMethodName();
        if (call == rpcConstants::loggerStanzaRequest)
        {
            if ( args->getStructArgMember( 0, "LogName", psLogName )
                 && args->getStructArgMember( 0, "Stanza", psStanza ) )
            {
                QString LogName;
                int Stanza;
                if ( psLogName->getString( LogName ) && psStanza->getInt( Stanza ) )
                {
                    mro->clearCallArgs();


                    bool callOK = true;
                    // Find Value as a published log
                    // and then send requested stanza from the log
                    // we publish the stanza count; it is up to the monitor to ensure
                    // it has a full set

                    RPCGeneralClient rpc(rpcConstants::loggerStanzaResponse);
                    QSharedPointer<RPCParam>st(new RPCParamStruct);
                    st->addMember( LogName, "LogName" );
                    st->addMember( Stanza, "Stanza" );
                    rpc.getCallArgs() ->addParam( st );

                    // we need to start pushing stanzas from the logfile - we can
                    // only really process a log sequentially
                    // Once it has all been pushed then later stanzas need
                    // to go to all subscribers.

                    // SO it is more in the nature of a "subscribe" but no
                    // get the stanza data from the log and add it as a string
                    TSingleLogFrame * lf = LogContainer ->findContest( LogName );
                    if ( lf )
                    {
                        QString StanzaData;
                        callOK = lf->getStanza( static_cast<unsigned int>(Stanza), StanzaData );
                        if ( callOK )
                        {
                            st->addMember( StanzaData, "StanzaData" );
                        }
                    }
                    else
                    {
                        callOK = false;
                    }

                    st->addMember( callOK, "LoggerResult" );
                    rpc.queueCall( from );

                }
            }
        }
    }
}
QStringList TSendDM::rotators()
{
    QStringList sl;
    QVector<PubSubName> rotlist = rotatorCache.getRotList();
    foreach (PubSubName psn, rotlist)
    {
        QString antname = psn.toString();
        sl.append(antname);
    }
    qSort(sl);
    return  sl;
}
QStringList TSendDM::rigs()
{
    QStringList sl;
    QVector<PubSubName> riglist = rigCache.getRigList();
    foreach (PubSubName psn, riglist)
    {
        QString antname = psn.toString();
        sl.append(antname);
    }
    qSort(sl);
    return  sl;
}
const RigState &TSendDM::getRigState(const QString &name)
{
    return rigCache.getState(PubSubName(name));
}
const RigDetails &TSendDM::getRigDetails(const QString &name)
{
    return rigCache.getDetails(PubSubName(name));
}
//---------------------------------------------------------------------------

