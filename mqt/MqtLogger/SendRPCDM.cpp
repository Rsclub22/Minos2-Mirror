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
    connect(rpc, &MinosRPC::routerCall, this, &TSendDM::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &TSendDM::on_notify);

}
TSendDM::~TSendDM()
{
    delete MinosRPC::getMinosRPC(getAppStartupName());
}
//---------------------------------------------------------------------------
void TSendDM::getRouterAppCatMap()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    MinosConfig *config = MinosConfig::getMinosConfig();

    QVector<QSharedPointer<Connectable> > connectables;
    connectables = config->getConnectables();

    QMap<QString,QVector< QSharedPointer<Connectable> > > routerAppCatMap;
    for ( const auto &i: qAsConst(connectables))
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
            routerAppCatMap[rpcConstants::KeyerCategory].push_back(i);
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
            routerAppCatMap[rpcConstants::rigControlCategory].push_back(i);
            routerAppCatMap[rpcConstants::rigDetailsCategory].push_back(i);
            routerAppCatMap[rpcConstants::rigStateCategory].push_back(i);
        }
        else if (i->appType == "Rotator")
        {
            routerAppCatMap[rpcConstants::RotatorCategory].push_back(i);
            routerAppCatMap[rpcConstants::rotatorDetailCategory].push_back(i);
            routerAppCatMap[rpcConstants::rotatorStateCategory].push_back(i);
            routerAppCatMap[rpcConstants::rotatorPresetsCategory].push_back(i);
        }
        else if (i->appType == "Server")
        {
//            routerAppCatMap[rpcConstants::LocalStationCategory].push_back(i);
//            routerAppCatMap[rpcConstants::StationCategory].push_back(i);
        }
        else if (i->appType == "Cluster")
        {
            routerAppCatMap[rpcConstants::clusterClientServer].push_back(i);
            routerAppCatMap[rpcConstants::clusterCategory].push_back(i);
        }
        else if (i->appType == "KSTClient")
        {

        }
    }
    rpc->setRouterAppCatMap(routerAppCatMap);
}
void TSendDM::subscribeApps()
{
    // This is called whenever the possible set of apps may have changed

    traceMsg("subscribeApps");
    invalidateCache();
    getRouterAppCatMap();
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
        st->addMember( loggerUuid, rpcConstants::loggerUuid );
        st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
        st->addMember( rpcConstants::keyerPlayFile, rpcConstants::paramName );
        st->addMember( fno, rpcConstants::paramValue );
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
        st->addMember( loggerUuid, rpcConstants::loggerUuid );
        st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
        st->addMember( QString("RecordFile"), rpcConstants::paramName );
        st->addMember( fno, rpcConstants::paramValue );
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
        st->addMember( loggerUuid, rpcConstants::loggerUuid );
        st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
        st->addMember( QString("Tone"), rpcConstants::paramName );
        st->addMember( 0, rpcConstants::paramValue );
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
        st->addMember( loggerUuid, rpcConstants::loggerUuid );
        st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
        st->addMember( QString("TwoTone"), rpcConstants::paramName );
        st->addMember( 0, rpcConstants::paramValue );
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
        st->addMember( loggerUuid, rpcConstants::loggerUuid );
        st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
        st->addMember( QString("Stop"), rpcConstants::paramName );
        st->addMember( 0, rpcConstants::paramValue );
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

        st->addMember( rpcConstants::txSpotToCluster, rpcConstants::paramName );
        st->addMember( freq.str(), rpcConstants::txSpotParamFreq );
        st->addMember( call, rpcConstants::txSpotParamCallsign );
        st->addMember( loc, rpcConstants::txSpotParamLocator );

        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( clusterApp  );
    }
}

void TSendDM::sendHfFlagToClusterServer(const bool state)
{
    if (!clusterApp.isEmpty())
    {
        traceMsg(QString("Send HfFlag To Cluster Server - state = %1").arg(state ? "True": "False"));
        RPCGeneralClient rpc(rpcConstants::clusterMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);

        st->addMember( rpcConstants::hfFlagToCluster, rpcConstants::paramName );
        st->addMember( state, rpcConstants::clusterHfFlag );

        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( clusterApp  );
    }
}

void TSendDM::sendReconnectFlagToClusterServer(const bool state)
{

    if (!clusterApp.isEmpty())
    {
        traceMsg(QString("Send Reconnect Flag To Cluster Server - state = %1").arg(state ? "True": "False"));
        RPCGeneralClient rpc(rpcConstants::clusterMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);

        st->addMember( rpcConstants::clusterReconnect, rpcConstants::paramName );
        st->addMember( state, rpcConstants::clusterReconnect );

        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( clusterApp  );
    }


}


void TSendDM::sendRequestSpotsResentFromClusterServer( resendFrameId id, const QString &cmd, const QString bandMask, const QString &uuid )
{

    if (!clusterApp.isEmpty())
    {
        traceMsg(QString("Send Request Spot Resend Command = %1, loggerUuid %2").arg(cmd, uuid));
        RPCGeneralClient rpc(rpcConstants::clusterMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);

        st->addMember( rpcConstants::clusterResendSpots, rpcConstants::paramName );
        st->addMember( cmd, rpcConstants::clusterResendSpotsCmd );
        st->addMember( static_cast<int>(id), rpcConstants::clusterFrameId ); // cluster or bandmap frame
        st->addMember(bandMask, rpcConstants::clusterBandmask);
        st->addMember(uuid, rpcConstants::loggerUuid);

        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( clusterApp  );
    }


}


void TSendDM::sendRotator(TSingleLogFrame *tslf, rpcConstants::RotateDirection direction, int angle )
{
    RPCGeneralClient rpc(rpcConstants::rotatorMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    st->addMember( loggerUuid, rpcConstants::loggerUuid );
    st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
    st->addMember( static_cast<int> (direction), rpcConstants::rotatorParamDirection );
    st->addMember( angle, rpcConstants::rotatorParamAngle );
    rpc.getCallArgs() ->addParam( st );

    PubSubName rotSelected = rotatorCache.getSelected(loggerUuid);
    rpc.queueCall( rotSelected );
}
void TSendDM::changeRotatorSelectionTo(const PubSubName &name, const QString &uuid)
{
    // we should de-select the cached uuid on all rotator apps
    traceMsg(QString("Change rotator selection to %1 %2").arg(name.toString(), uuid));

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
    traceMsg(QString("Send rotator selection to %1 %2").arg(s.toString(), uuid));

    RPCGeneralClient rpc(rpcConstants::rotatorMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);
    st->addMember( loggerUuid, rpcConstants::loggerUuid );
    st->addMember( uuid, rpcConstants::selected );
    st->addMember( s.toString(), rpcConstants::rotatorSelectAntennaName );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( s );
}

void TSendDM::changeRigSelectionTo(const PubSubName &name, const QString &band, const Frequency &freq, const QString &mode, const QString &uuid)
{
    // we should de-select the cached uuid on all rig apps

    trace(QString("Change rig selection to name = %1, band = %2, freq = %3, mode = %4, uuid = %5").arg(name.toString(), band, freq.traceStr(), mode, uuid));

    PubSubName selected = rigCache.getSelected(loggerUuid);

    if (!selected.isEmpty() && selected != name)
    {
        sendRigSelection(selected, "", Frequency(),"", "");
    }
    sendRigSelection(name, band, freq, mode, uuid);
}
void TSendDM::sendRigSelection(const PubSubName &s, const QString &band, const Frequency &freq, const QString &mode, const QString &uuid)
{
    rigCache.setSelected(s, loggerUuid, uuid);
    rigCache.setLogMode(s, mode);
    rigCache.setLogFreq(s, freq);
    rigCache.setLogBand(s, band);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    st->addMember( loggerUuid, rpcConstants::loggerUuid );
    st->addMember( uuid, rpcConstants::selected );
    st->addMember( s.toString(), rpcConstants::rigControlSelectRadioName );
    st->addMember(band, rpcConstants::rigControlLogBand);
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

    st->addMember( loggerUuid, rpcConstants::loggerUuid );
    st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
    st->addMember( freq.str(), rpcConstants::rigControlLogFreq );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
    traceMsg(QString("SendRigControlFreq = %1 uuid = %2").arg(freq.traceStr(), tslf->getContest()->uuid));
}

void TSendDM::sendRigControlBand(TSingleLogFrame *tslf, const QString &band)
{
    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setLogBand(rigSelected, band);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    st->addMember( loggerUuid, rpcConstants::loggerUuid );
    st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
    st->addMember( band, rpcConstants::rigControlLogBand );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
    traceMsg(QString("SendRigControlBand = %1 uuid = %2").arg(band, tslf->getContest()->uuid));
}


void TSendDM::sendRigTxVoiceMessage(TSingleLogFrame *tslf, const QString &msgNum)
{

    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setVoiceMessageNum(rigSelected, msgNum);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    st->addMember( loggerUuid, rpcConstants::loggerUuid );
    st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
    st->addMember( msgNum, rpcConstants::rigVoiceMessageNum );
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
    traceMsg(QString("SendRigVocieMessageNum = %1 uuid = %2").arg(msgNum, tslf->getContest()->uuid));
}




void TSendDM::sendRigControlMode(TSingleLogFrame *tslf,const QString &mode)
{
    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setLogMode(rigSelected, mode);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    st->addMember( loggerUuid, rpcConstants::loggerUuid );
    st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
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

    st->addMember( loggerUuid, rpcConstants::loggerUuid );
    st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
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

    st->addMember( loggerUuid, rpcConstants::loggerUuid );
    st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
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

    st->addMember( loggerUuid, rpcConstants::loggerUuid );
    st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
    st->addMember( level, rpcConstants::rigLogVolLevel);
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
}

void TSendDM::sendRigControlHfFlag(TSingleLogFrame *tslf, const bool &status)
{

    PubSubName rigSelected = rigCache.getSelected(loggerUuid);
    rigCache.setHfFlag(rigSelected, status);
    RPCGeneralClient rpc(rpcConstants::rigControlMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    st->addMember( loggerUuid, rpcConstants::loggerUuid );
    st->addMember( tslf->getContest()->uuid, rpcConstants::selected );
    st->addMember( status, rpcConstants::rigHfFlag);
    rpc.getCallArgs() ->addParam( st );

    rpc.queueCall( rigSelected );
}



void TSendDM::sendRotatorPreset(QString s)
{
    RPCGeneralClient rpc(rpcConstants::rotatorMethod);
    QSharedPointer<RPCParam>st(new RPCParamStruct);

    st->addMember( loggerUuid, rpcConstants::loggerUuid );
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
    for (auto const &psn: qAsConst(riglist))
    {
        RigDetails& selDetail = rigCache.getDetails(psn);
        if (selDetail.isDirty())
        {
            traceMsg(QString("notifyRigDetailChanges: %1 is dirty, send to rigcontrol").arg(psn.toString()));
            if (selDetail.transverterOffset().isDirty())
            {
                for (auto const &tslf: qAsConst(frames))
                {
                    tslf->on_SetTransVertOffset(selDetail.transverterOffset().getValue(), psn);
                }
            }
            if (selDetail.transverterSwitch().isDirty())
            {
                for (auto const &tslf: qAsConst(frames))
                {
                    tslf->on_SetTransVertSwitch(selDetail.transverterSwitch().getValue(), psn);
                }
            }
            if (selDetail.transverterEnabled().isDirty())
            {
                for (auto const &tslf: qAsConst(frames))
                {
                    tslf->on_SetTransVertEnabled(selDetail.transverterEnabled().getValue(), psn);
                }
            }
            if (selDetail.transverterStatus().isDirty())
            {
                for (auto const &tslf: qAsConst(frames))
                {
                    tslf->on_SetTransVertStatus(selDetail.transverterStatus().getValue(), psn);
                }
            }
            if (selDetail.volumeStatus().isDirty())
            {
                for (auto const &tslf: qAsConst(frames))
                {
                    tslf->on_SetVolumeStatus(selDetail.volumeStatus().getValue(), psn);
                }
            }
            if (selDetail.ritEnableStatus().isDirty())
            {
                for (auto const &tslf: qAsConst(frames))
                {
                    tslf->on_SetRitEnableStatus(selDetail.ritEnableStatus().getValue(), psn);
                }
            }
            if (selDetail.ritMaxKHzFreq().isDirty())
            {
                for (auto const &tslf: qAsConst(frames))
                {
                    tslf->on_SetRitMaxKHzFreq(selDetail.ritMaxKHzFreq().getValue(), psn);

                }
            }

            if (selDetail.bandList().isDirty())
            {
                for (auto const &tslf: qAsConst(frames))
                {
                    tslf->on_SetBandList(selDetail.bandList().getValue(), psn);
                }
            }
            if (selDetail.pttEnabled().isDirty())
            {
                for (int i = 0; i < frames.size(); i++)
                {
                    TSingleLogFrame *tslf = frames[i];
                    tslf->onSetPttEnabled(selDetail.pttEnabled().getValue(), psn);
                }
            }
            if (selDetail.pttType().isDirty())
            {
                for (int i = 0; i < frames.size(); i++)
                {
                    TSingleLogFrame *tslf = frames[i];
                    tslf->onSetPttType(selDetail.pttType().getValue(), psn);
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
            for (auto const &tslf: qAsConst(frames))
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
                    if (selState.pttState().isDirty())
                    {
                        traceMsg(QString("Rig ptt state = %1, uuid = %2").arg(selState.pttState().getValue() ? "Tx" : "Rx"));
                        tslf->on_SetPttState(selState.pttState().getValue());
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
            for (auto const &tslf: qAsConst(frames))
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

void TSendDM::on_notify( AnalysePubSubNotify an, const QString from )
{
    // PubSub notifications
    traceMsg( "Notify callback from " + from + ( !an.getOK() ? ":Error " : ":Normal " ) +  an.getPublisherProgram() + "@" + an.getPublisherRouter());

    if ( an.getOK())
    {
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
void TSendDM::on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
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
    for(auto const &psn: qAsConst(rotlist))
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
    for (const auto &psn: qAsConst(riglist))
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

void TSendDM::traceMsg(QString msg)
{
    trace(QString("[SendRPCDM] %1").arg(msg));
}
