/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"
#include <QHostInfo>


#include "SendRPCDM.h"
#include "tsinglelogframe.h"
#include "tlogcontainer.h"
#include "rigutils.h"
//---------------------------------------------------------------------------
TSendDM::TSendDM(QWidget* Owner )
    : QObject( Owner )
{
    QString h = QHostInfo::localHostName();
    loggerUuid = /*makeUuid()*/h;
    trace("logger uuid is " + loggerUuid);

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
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
PubSubName TSendDM::getSelectedRig(QString loggerUuid)
{
    return rigCache.getSelected(loggerUuid);
}
PubSubName TSendDM::getSelectedRot(QString loggerUuid)
{
    return rotatorCache.getSelected(loggerUuid);
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

    st->addMember( s.toString(), rpcConstants::rotatorSelectAntennaName );
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

    st->addMember( s.toString(), rpcConstants::rigControlSelectRadioName );
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
    rigCache.setRitFreq(rigSelected, convertStrToFreq(freq));
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
    rigCache.setRitOnOffStatus(rigSelected, status);
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


void TSendDM::sendRigControlVolumeLevel(TSingleLogFrame *tslf, int level)
{
    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setVolume(rigSelected, level);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );
    QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
    st->addMember( select, rpcConstants::selected );
    st->addMember( level, rpcConstants::rigVolLevel);
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
}

void TSendDM::sendRigControlTpm(TSingleLogFrame *tslf, int tpm, QString &freq)
{
    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setTpm(rigSelected, tpm);
    rigCache.setFreq(rigSelected, convertStrToFreq(freq));

    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );
    QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
    st->addMember( select, rpcConstants::selected );
    st->addMember( tpm, rpcConstants::rigTpm);
    st->addMember( freq, rpcConstants::rigControlFreq );
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
void TSendDM::notifyRigChanges()
{
    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    if (!rigSelected.isEmpty())
    {
        RigState &selState = rigCache.getState(rigSelected);
        QString selStateUuid = selState.getSelectedContest(loggerUuid).getValue();
        RigDetails &selDetail = rigCache.getDetails(rigSelected);
        QString selDetailsUuid = selDetail.getSelectedContest(loggerUuid).getValue();
        if (!selStateUuid.isEmpty())
        {
            QVector<TSingleLogFrame *> frames = LogContainer->getLogFrames();
            for (int i = 0; i < frames.size(); i++)
            {
                TSingleLogFrame *tslf = frames[i];
                QString frameUuid = tslf->getContest()->uuid;

                if (selStateUuid == frameUuid)
                {
                    trace("Rig state distribution for " + selStateUuid);
                    if (selState.tpm().isDirty())
                    {
                        trace("SendRPC Rig set tpm " + QString::number(selState.tpm().getValue()));
                        tslf->on_SetRadioTpm(selState.tpm().getValue());
                    }
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
                    if (selState.ritFreq().isDirty())
                    {
                        trace("SendRPC Rig set ritFreq " + convertRitFreqToStr(selState.ritFreq().getValue()));
                        tslf->on_SetRitFreq(convertRitFreqToStr(selState.ritFreq().getValue()));
                    }
                    if (selState.ritRadioStatus().isDirty())
                    {
                        QString s;
                        trace("SendRPC Rig set ritRadioStatus " + (s = selState.ritRadioStatus().getValue() ? "On" : "Off"));
                        tslf->on_SetRitRadioStatus(selState.ritRadioStatus().getValue());
                    }
                    if (selState.volLevel().isDirty())
                    {
                        trace("SendRPC Rig set volume " + QString::number(selState.volLevel().getValue()));
                        tslf->on_SetVolume(selState.volLevel().getValue());
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
                    if (selDetail.volumeStatus().isDirty())
                    {
                        trace(QString("SendRPC Rig set volume status ") + (selDetail.volumeStatus().getValue() ? " True" : " False"));
                        tslf->on_SetRadioVolumeState( selDetail.volumeStatus().getValue() );
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
}

void TSendDM::notifyRotChanges()
{
    PubSubName rotSelected = rotatorCache.getSelected(loggerUuid);
    if (!rotSelected.isEmpty())
    {
        AntennaState &selState = rotatorCache.getState(rotSelected);
        QString selStateUuid = selState.getSelectedContest(loggerUuid).getValue();
        AntennaDetail &selDetail = rotatorCache.getDetails(rotSelected);
        QString selDetailUuid = selState.getSelectedContest(loggerUuid).getValue();
        if (!selStateUuid.isEmpty())
        {
            QVector<TSingleLogFrame *> frames = LogContainer->getLogFrames();
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

void TSendDM::on_notify( bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
    // PubSub notifications
    AnalysePubSubNotify an( err, mro );
    trace( "Notify callback from " + from + ( err ? ":Error " : ":Normal " ) +  an.getPublisherProgram() + "@" + an.getPublisherServer());

    // Need to check that the server/app is in the category map; if not, don't pass it on
    if ( an.getOK())
    {
        QString category = an.getCategory();
        if (category != rpcConstants::LocalStationCategory && category != rpcConstants::StationCategory)
        {
            bool notificationOK = false;
            for ( QVector <QSharedPointer<Connectable> >::iterator j = catMap[category].begin(); j != catMap[category].end(); j++ )
            {
                if ((*j)->runType == RunLocal)
                {
                    if (an.getPublisherServer() != (*j)->serverName)
                    {
                        //trace("RunLocal server " + an.getPublisherServer() + " " + (*j)->serverName);
                        continue;
                    }
                    if (an.getPublisherProgram() != (*j)->appName)
                    {
                        //trace("RunLocal appName " + an.getPublisherProgram() + " " + (*j)->appName);
                        continue;
                    }

                    notificationOK = true;
                    break;
                }
                else if ((*j)->runType == ConnectServer)
                {
                    if ((*j)->serverName.isEmpty())
                    {
                        notificationOK = true;
                        break;
                    }
                    else if (an.getPublisherServer() != (*j)->serverName)
                    {
                        //trace("ConnectServer server " + an.getPublisherServer() + " " + (*j)->serverName);
                        continue;
                    }
                    if ((*j)->remoteAppName.isEmpty())
                    {
                        notificationOK = true;
                        break;
                    }
                    else if (an.getPublisherProgram() != (*j)->remoteAppName)
                    {
                        //trace("ConnectServer appName " + an.getPublisherProgram() + " " + (*j)->appName);
                        continue;
                    }

                    notificationOK = true;
                    break;
                }
            }

            if (!notificationOK)
                return;

        }
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
                emit setRadioList();
            }
            else if ( an.getCategory() == rpcConstants::RotatorCategory && an.getKey() == rpcConstants::rotatorList )
            {
                trace("SendRPC set rotList and loaded " + an.getValue());
                rotatorCache.addRotList(an.getValue());
                rotatorLoaded = true;
                emit RotatorLoaded();
                emit RotatorList();
            }
            else if ( an.getCategory() == rpcConstants::LocalStationCategory)
            {
                //localServerName = an.getKey();
//                if (!servers.contains(an.getKey()))
//                    RPCPubSub::subscribe( rpcConstants::StationCategory );  //want ALL keys - but do it once we know who WE are!
            }
            else if ( an.getCategory() == rpcConstants::StationCategory)
            {
                QString server = an.getKey();
                if (!servers.contains(server))
                {
                    servers.append(server);
                    for ( QMap<QString,QVector< QSharedPointer<Connectable> > >::iterator i = catMap.begin(); i != catMap.end(); i++)
                    {
                        for ( QVector <QSharedPointer<Connectable> >::iterator j = (*i).begin(); j != (*i).end(); j++ )
                        {
                            if ((*j)->runType == ConnectServer && (*j)->serverName.isEmpty())
                            {
                                RPCPubSub::subscribeRemote(server, i.key());
                            }
                        }
                    }
                }
            }
        }

        notifyRigChanges();
        notifyRotChanges();

        QVector<TSingleLogFrame *> frames = LogContainer->getLogFrames();
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
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    if (tslf)
        tslf->checkConnections();
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

void TSendDM::subscribeApps()
{
    /*
        for each type of interest (i.e. not chat or monitor)

        We need to subscribe to all server names - cf chatserver

    rpc->subscribe(rpcConstants::LocalStationCategory);

        NB we should try to integrate chat and monitor into this part

        look for all config entries

        If local, then we subscribe to it

        if remote and server is empty, then we want all servers as they become available
        if remote and a named server, then subscribe to that server only

        We need to save all this, and restrict on the app name as well

        So, we need some structures

        ?? key a list by category subscribed - each entry a chain of entries?

        type of app
        server name
        app name
        state

        When we get a LocalStationCategory notification, we need to look down the list
        and if this servername or server name is blank, then subcribe to the relevant
        category on this server. Extra subscriptions are harmless(I am pretty certain -
        maybe they will force a set of notifications).

        When we get an "other category" notification we need to find the relevant entries
        and check the app name before responding to it.

    */
    trace("subscribeApps");
    invalidateCache();

    catMap.clear();
    connectables.clear();
    servers.clear();

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    MinosConfig *config = MinosConfig::getMinosConfig();

    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = config->elelist.begin(); i != config->elelist.end(); i++ )
    {
        if (!(*i)->deleted)
        {
            QSharedPointer<Connectable> res = (*i)->connectable();
            connectables.push_back(res);
        }
    }

    for ( QVector <QSharedPointer<Connectable> >::iterator i = connectables.begin(); i != connectables.end(); i++ )
    {
        if ((*i)->appType == "None")
        {
            // no action
        }
        else if ((*i)->appType == "AppStarter")
        {
            // no action
        }
        else if ((*i)->appType == "BandMap")
        {
            // no action
        }
        else if ((*i)->appType == "Chat")
        {
            // no action - done in chat server
        }
        else if ((*i)->appType == "Keyer")
        {
            catMap[rpcConstants::KeyerCategory].push_back((*i));
        }
        else if ((*i)->appType == "LineControl")
        {
            // no action except in keyer
        }
        else if ((*i)->appType == "Logger")
        {
            // no action
        }
        else if ((*i)->appType == "Monitor")
        {
            // no action
        }
         else if ((*i)->appType == "Other")
        {
            // no action
        }
        else if ((*i)->appType == "RigControl")
        {
            catMap[rpcConstants::rigControlCategory].push_back((*i));
            catMap[rpcConstants::rigDetailsCategory].push_back((*i));
            catMap[rpcConstants::rigStateCategory].push_back((*i));
        }
        else if ((*i)->appType == "Rotator")
        {
            catMap[rpcConstants::RotatorCategory].push_back((*i));
            catMap[rpcConstants::rotatorDetailCategory].push_back((*i));
            catMap[rpcConstants::rotatorStateCategory].push_back((*i));
            catMap[rpcConstants::rotatorPresetsCategory].push_back((*i));
        }
        else if ((*i)->appType == "Server")
        {
            catMap[rpcConstants::LocalStationCategory].push_back((*i));
            catMap[rpcConstants::StationCategory].push_back((*i));
        }
    }

    if (!servers.contains(config->getThisServerName()))
    {
        servers.append(config->getThisServerName());
        for ( QMap<QString,QVector< QSharedPointer<Connectable> > >::iterator i = catMap.begin(); i != catMap.end(); i++)
        {
            for ( QVector <QSharedPointer<Connectable> >::iterator j = (*i).begin(); j != (*i).end(); j++ )
            {
                if ((*j)->runType == RunLocal)
                {
                    rpc->subscribeRemote(config->getThisServerName(), i.key());
                }
                else if ((*j)->runType == ConnectServer && !(*j)->serverName.isEmpty())
                {
                    rpc->subscribeRemote((*j)->serverName, i.key());
                }
            }
        }
    }
}
