#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include "RPCCommandConstants.h"
#include "RigDetails.h"
#include "rigcommon.h"
#include "MTrace.h"

RigDetails::RigDetails()
{
    qRegisterMetaType< RigDetails > ( "RigDetails" );
    _transverterOffset.setInitialValue(0.0);
    _transverterSwitch.setInitialValue(0);
    _transverterEnabled.setInitialValue(false);
    _transverterStatus.setInitialValue(false);
    _volumeStatus.setInitialValue(false);
    _ritEnableStatus.setInitialValue(false);
    _ritMaxKHzFreq.setInitialValue(MAX_RITFREQ);
    _pttEnabled.setInitialValue(false);
    _pttType.setInitialValue(static_cast<int>(serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_CAT));
    _voiceMemAvail.setInitialValue(false);
    _cwMemType.setInitialValue(hamlibData::CW_MEMORY_TYPES::NONE);
    _rigVoiceKeyerMessageSupportStop.setInitialValue(true);
    _rigCwKeyerMessageSupportStop.setInitialValue(true);
    _rigModel.setInitialValue("");
}

RigDetails::RigDetails(QString s)
{
    unpack(s);
}

bool RigDetails::isDirty() const
{
    return
            _selected.isDirty() ||
            _transverterOffset.isDirty() ||
            _transverterSwitch.isDirty() ||
            _transverterEnabled.isDirty() ||
            _transverterStatus.isDirty() ||
            _volumeStatus.isDirty() ||
            _bandList.isDirty() ||
            _ritEnableStatus.isDirty() ||
            _ritMaxKHzFreq.isDirty() ||
            _pttEnabled.isDirty() ||
            _pttType.isDirty() ||
            _voiceMemAvail.isDirty() ||
            _cwMemType.isDirty() ||
           _rigVoiceKeyerMessageSupportStop.isDirty() ||
           _rigCwKeyerMessageSupportStop.isDirty() ||
           _rigModel.isDirty();


}
void RigDetails::clearDirty()
{
    _selected.clearDirty();
    _transverterOffset.clearDirty();
    _transverterSwitch.clearDirty();
    _transverterEnabled.clearDirty();
    _transverterStatus.clearDirty();
    _volumeStatus.clearDirty();
    _bandList.clearDirty();
    _ritEnableStatus.clearDirty();
    _ritMaxKHzFreq.clearDirty();
    _pttEnabled.clearDirty();
    _pttType.clearDirty();
    _voiceMemAvail.clearDirty();
    _cwMemType.clearDirty();
    _rigVoiceKeyerMessageSupportStop.clearDirty();
    _rigCwKeyerMessageSupportStop.clearDirty();
    _rigModel.clearDirty();


}
void RigDetails::setDirty()
{
    _selected.setDirty();
    _transverterOffset.setDirty();
    _transverterSwitch.setDirty();
    _transverterEnabled.setDirty();
    _transverterStatus.setDirty();
    _volumeStatus.setDirty();
    _bandList.setDirty();
    _ritEnableStatus.setDirty();
    _ritMaxKHzFreq.setDirty();
    _pttEnabled.setDirty();
    _pttType.setDirty();
    _voiceMemAvail.setDirty();
    _cwMemType.setDirty();
    _rigVoiceKeyerMessageSupportStop.setDirty();
    _rigCwKeyerMessageSupportStop.setDirty();
    _rigModel.setDirty();



}

void RigDetails::setSelected(const QString &loggeruuid, const QString &selected)
{
    _selected.setSelection(loggeruuid, selected);
}
void RigDetails::setTransverterOffset(double transverterOffset)
{
    _transverterOffset.setValue(transverterOffset);
}

void RigDetails::setTransverterSwitch(int transverterSwitch)
{
    _transverterSwitch.setValue(transverterSwitch);
}

void RigDetails::setTransverterEnabled(bool transverterEnabled)
{
    _transverterEnabled.setValue(transverterEnabled);
}
void RigDetails::setTransverterStatus(bool transverterStatus)
{
    _transverterStatus.setValue(transverterStatus);
}

void RigDetails::setVolumeStatus(bool volumeStatus)
{
    _volumeStatus.setValue(volumeStatus);
}

void RigDetails::setRitEnableStatus(bool ritEnableStatus)
{
    _ritEnableStatus.setValue(ritEnableStatus);
}

void RigDetails::setRitMaxKHzFreq(int maxKHzFreq)
{
    _ritMaxKHzFreq.setValue(maxKHzFreq);
}

void RigDetails::setBandList(const QString &bandList)
{
    _bandList.setValue( bandList);
}


void RigDetails::setPttEnabled(bool pttEnabled)
{
    _pttEnabled.setValue(pttEnabled);
}
void RigDetails::setPttType(int type)
{
    _pttType.setValue(type);
}
void RigDetails::setVoiceMemAvail(bool voiceMemAvail)
{
    _voiceMemAvail.setValue(voiceMemAvail);
}
void RigDetails::setNumVoiceMessages(int numMessages)
{
    _numVoiceMemMessages.setValue(numMessages);
}
void RigDetails::setCwMemType(int cwMemType)
{
    _cwMemType.setValue(cwMemType);

}
void RigDetails::setRigVoiceKeyerMessageSupportStop(bool supportStopCmd)
{
    _rigVoiceKeyerMessageSupportStop.setValue(supportStopCmd);
}
void RigDetails::setRigCwKeyerMessageSupportStop(bool supportStopCmd)
{
    _rigCwKeyerMessageSupportStop.setValue(supportStopCmd);
}
void RigDetails::setRigModel(QString rigModel)
{
    _rigModel.setValue(rigModel);
}



QString RigDetails::pack() const
{
    QJsonObject jv;

    jv.insert(rpcConstants::selected, _selected.pack());
    jv.insert(rpcConstants::rigControlTxVertOffsetFreq, transverterOffset().getValue());
    jv.insert(rpcConstants::rigControlTxVertSwitch, transverterSwitch().getValue());
    jv.insert(rpcConstants::rigControlTxVertEnabled, transverterEnabled().getValue());
    jv.insert(rpcConstants::rigControlTxVertStatus, transverterStatus().getValue());
    jv.insert(rpcConstants::rigVolStatus, volumeStatus().getValue());
    jv.insert(rpcConstants::rigControlBandList, bandList().getValue());
    jv.insert(rpcConstants::rigRitEnableStatus, ritEnableStatus().getValue());
    jv.insert(rpcConstants::rigRitMaxKHz, ritMaxKHzFreq().getValue());
    jv.insert(rpcConstants::rigPttEnabled, pttEnabled().getValue());
    jv.insert(rpcConstants::rigPttType, pttType().getValue());
    jv.insert(rpcConstants::rigVoiceMemAvail, voiceMemAvail().getValue());
    jv.insert(rpcConstants::rigNumberVoiceMessages, numVoiceMessages().getValue());
    jv.insert(rpcConstants::rigCwMemType, cwMemType().getValue());
    jv.insert(rpcConstants::rigVoiceKeyerMessageSupportStop, rigVoiceKeyerMessageSupportStop().getValue());
    jv.insert(rpcConstants::rigCwKeyerMessageSupportStop, rigCwKeyerMessageSupportStop().getValue());
    jv.insert(rpcConstants::rigRigModel, rigModel().getValue());

    QJsonDocument json(jv);

    QString message(json.toJson(QJsonDocument::Compact));

    return message;
}
void RigDetails::unpack(QString s)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (!err.error)
    {
        QJsonValue selobj = json.object().value(rpcConstants::selected);
        _selected.unpack(selobj);
        _transverterOffset.setValue(json.object().value(rpcConstants::rigControlTxVertOffsetFreq).toDouble());
        _transverterSwitch.setValue(json.object().value(rpcConstants::rigControlTxVertSwitch).toInt());
        _transverterEnabled.setValue(json.object().value(rpcConstants::rigControlTxVertEnabled).toBool());
        _transverterStatus.setValue(json.object().value(rpcConstants::rigControlTxVertStatus).toBool());
        _volumeStatus.setValue(json.object().value(rpcConstants::rigVolStatus).toBool());
        _bandList.setValue(json.object().value(rpcConstants::rigControlBandList).toString());
        _ritEnableStatus.setValue(json.object().value(rpcConstants::rigRitEnableStatus).toBool());
        _ritMaxKHzFreq.setValue(json.object().value(rpcConstants::rigRitMaxKHz).toInt());
        _pttEnabled.setValue(json.object().value(rpcConstants::rigPttEnabled).toBool());
        _pttType.setValue(json.object().value(rpcConstants::rigPttType).toInt());
        _voiceMemAvail.setValue(json.object().value(rpcConstants::rigVoiceMemAvail).toBool());
        _numVoiceMemMessages.setValue(json.object().value(rpcConstants::rigNumberVoiceMessages).toInt());
        _cwMemType.setValue(json.object().value(rpcConstants::rigCwMemType).toInt());
        _rigVoiceKeyerMessageSupportStop.setValue(json.object().value(rpcConstants::rigVoiceKeyerMessageSupportStop).toBool());
        _rigCwKeyerMessageSupportStop.setValue(json.object().value(rpcConstants::rigCwKeyerMessageSupportStop).toBool());
        _rigModel.setValue(json.object().value(rpcConstants::rigRigModel).toString());
    }
    else
    {
        trace("Err " + err.errorString() + " Bad Json document " + s);
    }

}
MinosStringItem<QString> RigDetails::getSelectedContest(QString loggerUuid) const
{
    return _selected.getSelectedContest(loggerUuid);
}
MinosItem<double> RigDetails::transverterOffset() const
{
    return _transverterOffset;
}

MinosItem<int> RigDetails::transverterSwitch() const
{
    return _transverterSwitch;
}

MinosItem<bool> RigDetails::transverterEnabled() const
{
    return _transverterEnabled;
}

MinosItem<bool> RigDetails::transverterStatus() const
{
    return _transverterStatus;
}
MinosItem<bool> RigDetails::volumeStatus() const
{
    return _volumeStatus;
}
MinosStringItem<QString> RigDetails::bandList() const
{
    return _bandList;
}
MinosItem<bool> RigDetails::ritEnableStatus() const
{
    return _ritEnableStatus;
}
MinosItem<int> RigDetails::ritMaxKHzFreq() const
{
    return _ritMaxKHzFreq;
}


MinosItem<bool> RigDetails::pttEnabled() const
{
    return _pttEnabled;
}
MinosItem<int> RigDetails::pttType() const
{
    return _pttType;
}
MinosItem<bool> RigDetails::voiceMemAvail() const
{
    return _voiceMemAvail;
}
MinosItem<int> RigDetails::numVoiceMessages() const
{
    return _numVoiceMemMessages;
}
MinosItem<int> RigDetails::cwMemType() const
{
    return _cwMemType;
}
MinosItem<bool> RigDetails::rigVoiceKeyerMessageSupportStop() const
{
    return _rigVoiceKeyerMessageSupportStop;
}
MinosItem<bool> RigDetails::rigCwKeyerMessageSupportStop() const
{
    return _rigCwKeyerMessageSupportStop;
}
MinosItem<QString> RigDetails::rigModel() const
{
    return _rigModel;
}

