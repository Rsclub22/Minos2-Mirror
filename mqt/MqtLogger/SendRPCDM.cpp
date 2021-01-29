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
    traceMsg("logger uuid is " + loggerUuid);

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));

}
TSendDM::~TSendDM()
{
    delete MinosRPC::getMinosRPC(getAppStartupName());
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




void TSendDM::sendSpotToClusterServer(const Frequency &freq, const QString &call, const QString &loc )
{

    if (!clusterApp.isEmpty())
    {
        traceMsg(QString("Send Spot To Cluster Server - call = %1").arg(call));
        RPCGeneralClient rpc(rpcConstants::clusterMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        QSharedPointer<RPCParam>sName(new RPCStringParam( rpcConstants::txSpotToCluster ));
        QSharedPointer<RPCParam>freqStr(new RPCStringParam(freq.str()));
        QSharedPointer<RPCParam>callStr(new RPCStringParam(call));
        QSharedPointer<RPCParam>locStr(new RPCStringParam(loc));
        //QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));

        //st->addMember( logger, rpcConstants::loggerUuid );
        //st->addMember( select, rpcConstants::selected );
        st->addMember( sName, rpcConstants::paramName );
        st->addMember( freqStr, rpcConstants::txSpotParamFreq );
        st->addMember( callStr, rpcConstants::txSpotParamCallsign );
        st->addMember( locStr, rpcConstants::txSpotParamLocator );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( clusterApp  );
    }


}


void TSendDM::sendRequestSpotsResentFromClusterServer( resendFrameId id, const QString &cmd, const int bandMask, const QString &uuid )
{

    if (!clusterApp.isEmpty())
    {
        traceMsg(QString("Send Request Spot Resend Command = %1, loggerUuid %2").arg(cmd).arg(uuid));
        RPCGeneralClient rpc(rpcConstants::clusterMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        QSharedPointer<RPCParam>sName(new RPCStringParam( rpcConstants::clusterResendSpots));
        QSharedPointer<RPCParam>logUuid(new RPCStringParam(uuid ));
        QSharedPointer<RPCParam>resendCmd(new RPCStringParam(cmd));
        QSharedPointer<RPCParam>bandmask(new RPCIntParam(bandMask));
        QSharedPointer<RPCParam>frameId(new RPCIntParam(id));

        st->addMember( sName, rpcConstants::paramName );
        st->addMember( resendCmd, rpcConstants::clusterResendSpotsCmd );
        st->addMember( frameId, rpcConstants::clusterFrameId ); // cluster or bandmap frame
        st->addMember(bandmask, rpcConstants::clusterBandmask);
        st->addMember(logUuid, rpcConstants::loggerUuid);

        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( clusterApp  );
    }


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
    traceMsg(QString("Change rotator selection to %1 %2").arg(name.toString()).arg(uuid));

    PubSubName selected = rotatorCache.getSelected(loggerUuid);

    rotatorCache.setSelected(selected, loggerUuid, ""); // deselect the old one
    if (!selected.isEmpty() && selected != name)
        sendRotatorSelection(selected, "");

    if (!name.isEmpty() && rotatorCache.setSelected(name, loggerUuid, uuid))
    {
        sendRotatorSelection(name, uuid);
    }
}
void TSendDM::sendRotatorSelection(const PubSubName &s, const QString &uuid)
{
    traceMsg(QString("Send rotator selection to %1 %2").arg(s.toString()).arg(uuid));

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

void TSendDM::changeRigSelectionTo(const PubSubName &name, const Frequency &freq, const QString &mode, const QString &uuid)
{
    // we should de-select the cached uuid on all rig apps

    trace(QString("Change rig selection to name = %1, freq = %2, mode = %3, uuid = %4").arg(name.toString()).arg(freq.traceStr()).arg(mode).arg(uuid));

    PubSubName selected = rigCache.getSelected(loggerUuid);

    if (!selected.isEmpty() && selected != name)
    {
        sendRigSelection(selected, Frequency(),"", "");
    }
    sendRigSelection(name, freq, mode, uuid);
}
void TSendDM::sendRigSelection(const PubSubName &s, const Frequency &freq, const QString &mode, const QString &uuid)
{
    rigCache.setSelected(s, loggerUuid, uuid);
    rigCache.setLogMode(s, mode);
    rigCache.setLogFreq(s, freq);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );
    QSharedPointer<RPCParam>select(new RPCStringParam(uuid ));
    st->addMember( select, rpcConstants::selected );

    st->addMember( s.toString(), rpcConstants::rigControlSelectRadioName );
    st->addMember(freq.str(), rpcConstants::rigControlLogFreq);
    st->addMember( mode, rpcConstants::rigControlLogMode );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( s );

}



void TSendDM::sendRigControlFreq(TSingleLogFrame *tslf, const Frequency &freq)
{

    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setLogFreq(rigSelected, freq);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );

    QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
    st->addMember( select, rpcConstants::selected );

    st->addMember( freq.str(), rpcConstants::rigControlLogFreq );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
    traceMsg(QString("SendRigControlFreq = %1 uuid = %2").arg(freq.traceStr()).arg(tslf->getContest()->uuid));
}


void TSendDM::sendRigControlMode(TSingleLogFrame *tslf,const QString &mode)
{
    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setLogMode(rigSelected, mode);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );
    QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
    st->addMember( select, rpcConstants::selected );
    st->addMember( mode, rpcConstants::rigControlLogMode );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
}



void TSendDM::sendRigControlRitFreq(TSingleLogFrame *tslf, ShortFreq freq)
{
    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setLogRitFreq(rigSelected, freq);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );
    QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
    st->addMember( select, rpcConstants::selected );
    st->addMember( freq.str(), rpcConstants::rigControlLogRitFreq );
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
    rigCache.setLogVolume(rigSelected, level);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
    st->addMember( logger, rpcConstants::loggerUuid );
    QSharedPointer<RPCParam>select(new RPCStringParam(tslf->getContest()->uuid ));
    st->addMember( select, rpcConstants::selected );
    st->addMember( level, rpcConstants::rigLogVolLevel);
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
void TSendDM::notifyRigDetailChanges()
{
    QVector<PubSubName> riglist = rigCache.getRigList();
    QVector<TSingleLogFrame *> frames = LogContainer->getLogFrames();
    for (auto const &psn: riglist)
    {
        RigDetails& selDetail = rigCache.getDetails(psn);
        if (selDetail.isDirty())
        {
            traceMsg(QString("notifyRigDetailChanges: %1 is dirty, send to rigcontrol").arg(psn.toString()));
            if (selDetail.transverterOffset().isDirty())
            {
                for (auto const &tslf: frames)
                {
                    tslf->on_SetTransVertOffset(selDetail.transverterOffset().getValue(), psn);
                }
            }
            if (selDetail.transverterSwitch().isDirty())
            {
                for (auto const &tslf: frames)
                {
                    tslf->on_SetTransVertSwitch(selDetail.transverterSwitch().getValue(), psn);
                }
            }
            if (selDetail.transverterEnabled().isDirty())
            {
                for (auto const &tslf: frames)
                {
                    tslf->on_SetTransVertEnabled(selDetail.transverterEnabled().getValue(), psn);
                }
            }
            if (selDetail.transverterStatus().isDirty())
            {
                for (auto const &tslf: frames)
                {
                    tslf->on_SetTransVertStatus(selDetail.transverterStatus().getValue(), psn);
                }
            }
            if (selDetail.volumeStatus().isDirty())
            {
                for (auto const &tslf: frames)
                {
                    tslf->on_SetVolumeStatus(selDetail.volumeStatus().getValue(), psn);
                }
            }
            if (selDetail.ritEnableStatus().isDirty())
            {
                for (auto const &tslf: frames)
                {
                    tslf->on_SetRitEnableStatus(selDetail.ritEnableStatus().getValue(), psn);
                }
            }
            if (selDetail.ritMaxKHzFreq().isDirty())
            {
                for (auto const &tslf: frames)
                {
                    tslf->on_SetRitMaxKHzFreq(selDetail.ritMaxKHzFreq().getValue(), psn);

                }
            }

            if (selDetail.bandList().isDirty())
            {
                for (auto const &tslf: frames)
                {
                    tslf->on_SetBandList(selDetail.bandList().getValue(), psn);
                }
            }

        }
        selDetail.clearDirty();
    }
}




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
            for (auto const &tslf: frames)
            {
                if (tslf->getContest() == nullptr)
                {
                    continue;
                }
                QString frameUuid = tslf->getContest()->uuid;

                if (selStateUuid == frameUuid)
                {
                    traceMsg(QString("Rig state distribution for %1").arg(selStateUuid));
                    if (selState.radioMode().isDirty())
                    {
                        traceMsg(QString("Rig set mode = %1, uuid = %2").arg(selState.radioMode().getValue()).arg(selStateUuid));
                        tslf->on_SetMode(selState.radioMode().getValue());
                    }
                    if (selState.radioFreq().isDirty())
                    {

                        traceMsg(QString("Rig set freq = %1, uuid = %2").arg(selState.radioFreq().getValue().traceStr()).arg(selStateUuid));
                        tslf->on_SetFreq(selState.radioFreq().getValue());

                    }
                    if (selState.radioRitFreq().isDirty())
                    {
                        traceMsg(QString("Rig set ritFreq = %1, uuid = %2").arg(selState.radioRitFreq().getValue().traceStr()).arg(selStateUuid));
                        tslf->on_SetRitFreq(selState.radioRitFreq().getValue());
                    }
                    if (selState.ritRadioStatus().isDirty())
                    {
                        QString s;
                        traceMsg(QString("Rig set ritRadioStatus = %1, uuid = %2").arg((s = selState.ritRadioStatus().getValue() ? "On" : "Off")).arg(selStateUuid));
                        tslf->on_SetRitRadioStatus(selState.ritRadioStatus().getValue());
                    }
                    if (selState.radioVolLevel().isDirty())
                    {
                        traceMsg(QString("Rig set volume =  %1, uuid = %2").arg(QString::number(selState.radioVolLevel().getValue())).arg(selStateUuid));
                        tslf->on_SetVolume(selState.radioVolLevel().getValue());
                    }
                    if (selState.status().isDirty())
                    {
                        traceMsg(QString("Rig set status = %1, uuid = %2").arg(selState.status().getValue()).arg(selStateUuid));
                        tslf->on_SetRadioStatus(selState.status().getValue());
                    }
                    selState.clearDirty();
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
            for (auto const &tslf: frames)
            {
                if (tslf->getContest() == nullptr)
                {
                    continue;
                }
                QString frameUuid = tslf->getContest()->uuid;

                if (selStateUuid == frameUuid)
                {
                    traceMsg(QString("Rotator state distribution for %1").arg(selStateUuid));

                    if (selState.bearing().isDirty())
                    {
                        traceMsg(QString("Rotator set bearing = %1, uuid = %2").arg(selState.bearing().getValue()).arg(selStateUuid));
                        tslf->on_RotatorBearing(selState.bearing().getValue());
                    }
                    if (selState.status().isDirty())
                    {
                        traceMsg(QString("Rotator set status = %1, uuid = %2").arg(selState.status().getValue()).arg(selStateUuid));
                        tslf->on_RotatorStatus(selState.status().getValue());
                    }
                    selState.clearDirty();
                }
                if (selDetailUuid == frameUuid)
                {
                    traceMsg(QString("Rotator details distribution for %1").arg(selDetailUuid));
                    if (selDetail.maxAzimuth().isDirty())
                    {
                        traceMsg(QString("Rotator set maxAzimuth = %1, uuid = %2").arg(selDetail.maxAzimuth().getValue()).arg(selStateUuid));
                        tslf->on_RotatorMaxAzimuth(selDetail.maxAzimuth().getValue());
                    }
                    if (selDetail.minAzimuth().isDirty())
                    {
                        traceMsg(QString("Rotator set minAzimuth = %1, uuid = %2").arg(selDetail.minAzimuth().getValue()).arg(selStateUuid));
                        tslf->on_RotatorMinAzimuth(selDetail.minAzimuth().getValue());
                    }
                    if (selDetail.cwCcwCmdEnable().isDirty())
                    {
                        traceMsg(QString("SendRPC Rotator set cwCcwCmdEnable = %1, uuid = %2").arg(selDetail.cwCcwCmdEnable().getValue() ? "True" : "False").arg(selStateUuid));
                        tslf->on_cwCcwCmdEnable(selDetail.cwCcwCmdEnable().getValue());
                    }
                    if (selDetail.supportStopCommand().isDirty())
                    {
                        traceMsg(QString("Rotator set supportStopCommand = %1, uuid = %2").arg(selDetail.supportStopCommand().getValue() ? "True" :"False").arg(selStateUuid));
                        tslf->on_SupportStopCommand(selDetail.supportStopCommand().getValue());
                    }
                    selDetail.clearDirty();

                }
                if (rotatorCache.rotatorPresetsIsDirty(rotSelected))
                {
                    traceMsg(QString("Rotator set presets = %1 - %2").arg(rotatorCache.getRotatorPresets(rotSelected)).arg(selStateUuid));
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
    traceMsg( "Notify callback from " + from + ( err ? ":Error " : ":Normal " ) +  an.getPublisherProgram() + "@" + an.getPublisherServer());

    // Need to check that the server/app is in the category map; if not, don't pass it on
    if ( an.getOK())
    {
        QString category = an.getCategory();
        if (category != rpcConstants::LocalStationCategory && category != rpcConstants::StationCategory)
        {
            bool notificationOK = false;
            for ( auto const &j: catMap[category] )
            {
                if (j->runType == RunLocal)
                {
                    if (an.getPublisherServer() != j->serverName)
                    {
                        //trace("RunLocal server " + an.getPublisherServer() + " " + (*j)->serverName);
                        continue;
                    }
                    if (an.getPublisherProgram() != j->appName)
                    {
                        //trace("RunLocal appName " + an.getPublisherProgram() + " " + (*j)->appName);
                        continue;
                    }

                    notificationOK = true;
                    break;
                }
                else if (j->runType == ConnectServer)
                {
                    if (j->serverName.isEmpty())
                    {
                        notificationOK = true;
                        break;
                    }
                    else if (an.getPublisherServer() != j->serverName)
                    {
                        //trace("ConnectServer server " + an.getPublisherServer() + " " + (*j)->serverName);
                        continue;
                    }
                    if (j->remoteAppName.isEmpty())
                    {
                        notificationOK = true;
                        break;
                    }
                    else if (an.getPublisherProgram() != j->remoteAppName)
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
            traceMsg(QString("Category %1 key %2").arg(an.getCategory()).arg(an.getKey()));
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
                traceMsg(QString("Set rigList and loaded %1").arg(an.getValue()));
                rigCache.addRigList(an.getValue());
                radioLoaded = true;
                emit setRadioLoaded();
                emit setRadioList();
            }
            else if ( an.getCategory() == rpcConstants::RotatorCategory && an.getKey() == rpcConstants::rotatorList )
            {
                traceMsg(QString("SendRPC set rotList and loaded %1").arg(an.getValue()));
                rotatorCache.addRotList(an.getValue());
                rotatorLoaded = true;
                emit RotatorLoaded();
                emit RotatorList();
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
        else if (an.getState() == psRevoked)
        {
            // on far end close,
            // all published cats/values get revoked
            // StationCategory goes to psNotConnected
//            e.g.
//           14:21:59.563 RX Minos:PubSub:ClientNotify id 91 from Shack :
//                        Server<'Stone'> Publisher<'RigControl@Stone'>
//                        Category<'RigState'>
//                        Key<'Stone/RigControl/rigctl'>
//                        Value<''> State<1>

//            rotStates[name].setStatus(ROT_STATUS_DISCONNECTED);

            if ( an.getCategory() == rpcConstants::rigStateCategory)
            {
                rigCache.setStateDisconnected(an);
            }
            else if ( an.getCategory() == rpcConstants::rotatorStateCategory )
            {
                rotatorCache.setStateDisconnected(an);
            }
        }
        else if (an.getState() == psNotConnected)
        {
            // when far end closes,
            // all published cats/values get revoked
            // StationCategory goes to psNotConnected
        }

        notifyRigDetailChanges();
        notifyRigChanges();
        notifyRotChanges();

        if ( an.getCategory() == rpcConstants::KeyerCategory && an.getKey() == rpcConstants::keyerReport )
        {
            if (keyerApp.isEmpty())
                keyerApp = PubSubName(an);
            emit setKeyerLoaded();
            LogContainer->setCaption( an.getValue() );
            traceMsg( "KeyerReport " + an.getValue() );
        }

        if ( an.getCategory() == rpcConstants::clusterCategory  && an.getKey() == rpcConstants::clusterReport )
        {
            if (clusterApp.isEmpty())
            {
                clusterApp = PubSubName(an);
                emit setClusterServerLoaded();
            }

            emit setClusterState(an.getValue());
        }
        else if ( an.getCategory() == rpcConstants::clusterCategory  && an.getKey() == rpcConstants::clusterTXSpotEnableState )
        {
            emit setClusterTXSpotEnableState(an.getValue());
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
    traceMsg( "request callback from " + from + ( err ? ":Error" : ":Normal" ) );
    traceMsg("method is " + mro->getMethodName());

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
    for(auto const &psn: rotlist)
    {
        QString antname = psn.toString();
        sl.append(antname);
    }
    sl.sort();
    return  sl;
}
QStringList TSendDM::rigs()
{
    QStringList sl;
    QVector<PubSubName> riglist = rigCache.getRigList();
    for (auto const &psn: riglist)
    {
        QString rigname = psn.toString();
        sl.append(rigname);
    }
    sl.sort();
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
    traceMsg("subscribeApps");
    invalidateCache();

    catMap.clear();
    connectables.clear();
    servers.clear();

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    MinosConfig *config = MinosConfig::getMinosConfig();

    for ( auto const &i: config->elelist )
    {
        if (!i->deleted)
        {
            QSharedPointer<Connectable> res = i->connectable();
            connectables.push_back(res);
        }
    }

    for ( auto const &i: connectables)
    {
        if (i->appType == "None")
        {
            // no action
        }
        else if (i->appType == "AppStarter")
        {
            // no action
        }
        else if (i->appType == "BandMap")
        {
            // no action
        }
        else if (i->appType == "Chat")
        {
            // no action - done in chat server
        }
        else if (i->appType == "Keyer")
        {
            catMap[rpcConstants::KeyerCategory].push_back(i);
        }
        else if (i->appType == "LineControl")
        {
            // no action except in keyer
        }
        else if (i->appType == "Logger")
        {
            // no action
        }
        else if (i->appType == "Monitor")
        {
            // no action
        }
         else if (i->appType == "Other")
        {
            // no action
        }
        else if (i->appType == "RigControl")
        {
            catMap[rpcConstants::rigControlCategory].push_back(i);
            catMap[rpcConstants::rigDetailsCategory].push_back(i);
            catMap[rpcConstants::rigStateCategory].push_back(i);
        }
        else if (i->appType == "Rotator")
        {
            catMap[rpcConstants::RotatorCategory].push_back(i);
            catMap[rpcConstants::rotatorDetailCategory].push_back(i);
            catMap[rpcConstants::rotatorStateCategory].push_back(i);
            catMap[rpcConstants::rotatorPresetsCategory].push_back(i);
        }
        else if (i->appType == "Server")
        {
            catMap[rpcConstants::LocalStationCategory].push_back(i);
            catMap[rpcConstants::StationCategory].push_back(i);
        }
        else if (i->appType == "Cluster")
        {
            catMap[rpcConstants::clusterClientServer].push_back(i);
            catMap[rpcConstants::clusterCategory].push_back(i);
        }
        else if (i->appType == "KSTClient")
        {

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


void TSendDM::traceMsg(QString msg)
{
    trace(QString("[SendRPCDM] %1").arg(msg));
}
