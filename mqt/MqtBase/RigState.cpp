#include "base_pch.h"
#include "RigState.h"


RigState::RigState()
    :PubSubValue(RigStateType)
{
    qRegisterMetaType< RigState > ( "RigState" );
    _radioFreq.setInitialValue(0.0);
    _logFreq.setInitialValue(0.0);
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
            _radioFreq.isDirty() ||
            _logFreq.isDirty() ||
            _radioMode.isDirty() ||
            _logMode.isDirty() ||
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
    _radioFreq.clearDirty();
    _logFreq.clearDirty();
    _radioMode.clearDirty();
    _logMode.clearDirty();
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
    _radioFreq.setDirty();
    _logFreq.setDirty();
    _radioMode.setDirty();
    _logMode.setDirty();
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

void RigState::setRadioFreq(double freq)
{
    _radioFreq.setValue(freq);
}
void RigState::setLogFreq(double freq)
{
    _logFreq.setValue(freq);
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
void RigState::setRadioMode(const QString &mode)
{
    _radioMode.setValue(mode);
}
void RigState::setLogMode(const QString &mode)
{
    _logMode.setValue(mode);
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
    jv.insert(rpcConstants::rigControlRadioFreq, radioFreq().getValue());
    jv.insert(rpcConstants::rigControlLogFreq, logFreq().getValue());
    jv.insert(rpcConstants::rigControlRadioMode, radioMode().getValue());
    jv.insert(rpcConstants::rigControlLogMode, logMode().getValue());
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
        _radioFreq.setValue(json.object().value(rpcConstants::rigControlRadioFreq).toDouble());
        _logFreq.setValue(json.object().value(rpcConstants::rigControlLogFreq).toDouble());
        _radioMode.setValue(json.object().value(rpcConstants::rigControlRadioMode).toString());
        _logMode.setValue(json.object().value(rpcConstants::rigControlLogMode).toString());
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


MinosItem<double> RigState::radioFreq() const
{
    return _radioFreq;
}
MinosItem<double> RigState::logFreq() const
{
    return _logFreq;
}

MinosStringItem<QString> RigState::radioMode() const
{
    return _radioMode;
}
MinosStringItem<QString> RigState::logMode() const
{
    return _logMode;
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

