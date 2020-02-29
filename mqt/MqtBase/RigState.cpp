#include "base_pch.h"
#include "RigState.h"


RigState::RigState()
    :PubSubValue(RigStateType)
{
    qRegisterMetaType< RigState > ( "RigState" );
    _radioFreq.setInitialValue(0.0);
    _logFreq.setInitialValue(0.0);
    _radioVolLevel.setInitialValue(0);
    _logVolLevel.setInitialValue(0);
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
            _radioVolLevel.isDirty() ||
            _logVolLevel.isDirty() ||
            _radioRitFreq.isDirty() ||
            _logRitFreq.isDirty() ||
            _ritOnOffStatus.isDirty() ||
            _ritRadioStatus.isDirty();
}
void RigState::clearDirty()
{
    _status.clearDirty();
    _selected.clearDirty();
    _radioFreq.clearDirty();
    _logFreq.clearDirty();
    _radioMode.clearDirty();
    _logMode.clearDirty();
    _radioVolLevel.clearDirty();
    _logVolLevel.clearDirty();
    _radioRitFreq.clearDirty();
    _logRitFreq.clearDirty();
    _ritOnOffStatus.clearDirty();
    _ritRadioStatus.clearDirty();
}
void RigState::setDirty()
{
    _status.setDirty();
    _selected.setDirty();
    _radioFreq.setDirty();
    _logFreq.setDirty();
    _radioMode.setDirty();
    _logMode.setDirty();
    _radioVolLevel.setDirty();
    _logVolLevel.setDirty();
    _radioRitFreq.setDirty();
    _logRitFreq.setDirty();
    _ritOnOffStatus.setDirty();
    _ritRadioStatus.setDirty();
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
void RigState::setRadioRitFreq(int freq)
{
    _radioRitFreq.setValue(freq);
}
void RigState::setLogRitFreq(int freq)
{
    _logRitFreq.setValue(freq);
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
void RigState::setRadioVolume(int level)
{
    _radioVolLevel.setValue(level);
}
void RigState::setLogVolume(int level)
{
    _logVolLevel.setValue(level);
}
void RigState::setStatus(const QString &status)
{
    _status.setValue(status);
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
    jv.insert(rpcConstants::rigControlRadioRitFreq, radioRitFreq().getValue());
    jv.insert(rpcConstants::rigControlLogRitFreq, logRitFreq().getValue());
    jv.insert(rpcConstants::rigRadioVolLevel, radioVolLevel().getValue());
    jv.insert(rpcConstants::rigLogVolLevel, logVolLevel().getValue());
    jv.insert(rpcConstants::rigRitOnOffStatus, ritOnOffStatus().getValue());
    jv.insert(rpcConstants::rigRitRadioStatus, ritRadioStatus().getValue());

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
        _radioVolLevel.setValue(json.object().value(rpcConstants::rigRadioVolLevel).toInt());
        _logVolLevel.setValue(json.object().value(rpcConstants::rigLogVolLevel).toInt());
        _radioRitFreq.setValue(json.object().value(rpcConstants::rigControlRadioRitFreq).toInt());
        _logRitFreq.setValue(json.object().value(rpcConstants::rigControlLogRitFreq).toInt());
        _ritOnOffStatus.setValue(json.object().value(rpcConstants::rigRitOnOffStatus).toBool());
        _ritRadioStatus.setValue(json.object().value(rpcConstants::rigRitRadioStatus).toBool());
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


MinosItem<int> RigState::radioRitFreq() const
{
    return _radioRitFreq;
}
MinosItem<int> RigState::logRitFreq() const
{
    return _logRitFreq;
}

MinosItem<int> RigState::radioVolLevel() const
{
    return _radioVolLevel;
}

MinosItem<int> RigState::logVolLevel() const
{
    return _logVolLevel;
}

MinosItem<bool> RigState::ritOnOffStatus() const
{
    return _ritOnOffStatus;
}


MinosItem<bool> RigState::ritRadioStatus() const
{
    return _ritRadioStatus;
}

QStringList RigState::getSelectedLoggers()
{
    return _selected.getSelectedLoggers();
}

