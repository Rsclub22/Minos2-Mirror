#include "base_pch.h"
#include "RigState.h"


RigState::RigState()
    :PubSubValue(RigStateType)
{
    qRegisterMetaType< RigState > ( "RigState" );
    _freq.setInitialValue(0.0);
    _tpm.setInitialValue(0);
    _volLevel.setInitialValue(0);
}

RigState::RigState(QString s)
    :PubSubValue(RigStateType)
{
    unpack(s);
}

bool RigState::isDirty() const
{
    return  _status.isDirty() ||
            _selected.isDirty() ||
            _freq.isDirty() ||
            _mode.isDirty() ||
            _volLevel.isDirty() ||
            _ritFreq.isDirty() ||
            _ritOnOffStatus.isDirty() ||
            _ritRadioStatus.isDirty() ||
            _tpm.isDirty();
}
void RigState::clearDirty()
{
    _status.clearDirty();
    _selected.clearDirty();
    _freq.clearDirty();
    _mode.clearDirty();
    _volLevel.clearDirty();
    _ritFreq.clearDirty();
    _ritOnOffStatus.clearDirty();
    _ritRadioStatus.clearDirty();
    _tpm.clearDirty();
}
void RigState::setDirty()
{
    _status.setDirty();
    _selected.setDirty();
    _freq.setDirty();
    _mode.setDirty();
    _volLevel.setDirty();
    _ritFreq.setDirty();
    _ritOnOffStatus.setDirty();
    _ritRadioStatus.setDirty();
    _tpm.setDirty();
}
void RigState::setSelected(const QString &loggeruuid, const QString &selected)
{
    _selected.setSelection(loggeruuid, selected);
}

void RigState::setFreq(double freq)
{
    _freq.setValue(freq);
}
void RigState::setRitFreq(int freq)
{
    _ritFreq.setValue(freq);
}

void RigState::setRitOnOffStatus(bool status)
{
    _ritOnOffStatus.setValue(status);
}

void RigState::setRitRadioStatus(const bool status)
{
    _ritRadioStatus.setValue(status);
}
void RigState::setMode(const QString &mode)
{
    _mode.setValue(mode);
}
void RigState::setVolume(int level)
{
    _volLevel.setValue(level);
}
void RigState::setStatus(const QString &status)
{
    _status.setValue(status);
}

void RigState::setTpm(int tpm)
{
    _tpm.setValue(tpm);
}
QString RigState::pack() const
{
    QJsonObject jv;

    jv.insert(rpcConstants::selected, _selected.pack());
    jv.insert(rpcConstants::rigControlStatus, status().getValue());
    jv.insert(rpcConstants::rigControlFreq, freq().getValue());
    jv.insert(rpcConstants::rigControlMode, mode().getValue());
    jv.insert(rpcConstants::rigControlRitFreq, ritFreq().getValue());
    jv.insert(rpcConstants::rigVolLevel, volLevel().getValue());
    jv.insert(rpcConstants::rigRitOnOffStatus, ritOnOffStatus().getValue());
    jv.insert(rpcConstants::rigRitRadioStatus, ritRadioStatus().getValue());
    jv.insert(rpcConstants::rigTpm, tpm().getValue());

    QJsonDocument json(jv);

    QString message(json.toJson(QJsonDocument::Compact));

    return message;
}
void RigState::unpack(QString s)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (!err.error)
    {
        QJsonValue selobj = json.object().value(rpcConstants::selected);
        _selected.unpack(selobj);
        _status.setValue(json.object().value(rpcConstants::rigControlStatus).toString());
        _freq.setValue(json.object().value(rpcConstants::rigControlFreq).toDouble());
        _mode.setValue(json.object().value(rpcConstants::rigControlMode).toString());
        _volLevel.setValue(json.object().value(rpcConstants::rigVolLevel).toInt());
        _ritFreq.setValue(json.object().value(rpcConstants::rigControlRitFreq).toInt());
        _ritOnOffStatus.setValue(json.object().value(rpcConstants::rigRitOnOffStatus).toBool());
        _ritRadioStatus.setValue(json.object().value(rpcConstants::rigRitRadioStatus).toBool());
        _tpm.setValue(json.object().value(rpcConstants::rigTpm).toInt());
    }
    else
    {
        trace("Err " + err.errorString() + " Bad Json document " + s);
    }

}
MinosStringItem<QString> RigState::getSelectedContest(QString loggerUuid) const
{
    return _selected.getSelectedContest(loggerUuid);
}
MinosStringItem<QString> RigState::status() const
{
    return _status;
}


MinosItem<double> RigState::freq() const
{
    return _freq;
}

MinosStringItem<QString> RigState::mode() const
{
    return _mode;
}


MinosItem<int> RigState::ritFreq() const
{
    return _ritFreq;
}

MinosItem<int> RigState::volLevel() const
{
    return _volLevel;
}

MinosItem<bool> RigState::ritOnOffStatus() const
{
    return _ritOnOffStatus;
}


MinosItem<bool> RigState::ritRadioStatus() const
{
    return _ritRadioStatus;
}

MinosItem<int> RigState::tpm() const
{
    return _tpm;
}
QStringList RigState::getSelectedLoggers()
{
    return _selected.getSelectedLoggers();
}

