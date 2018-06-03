#include "base_pch.h"
#include "AntennaState.h"

AntennaState::AntennaState():PubSubValue(AntennaStateType)
{
}
AntennaState::AntennaState(const QString &st, const QString &loggeruuid, const QString &sel, const QString &b):PubSubValue(AntennaStateType)
{
    _bearing.setValue(b);
    _status.setValue(st);
    _selected.setSelection(loggeruuid, sel);
}
AntennaState::AntennaState(QString s):PubSubValue(AntennaStateType)
{
    qRegisterMetaType< AntennaState > ( "AntennaState" );
    unpack(s);
}
bool AntennaState::isDirty() const
{
    return (_selected.isDirty() || _status.isDirty() || _bearing.isDirty());
}
void AntennaState::clearDirty()
{
    _selected.clearDirty();
    _status.clearDirty();
    _bearing.clearDirty();
}
void AntennaState::setDirty()
{
    _selected.setDirty();
    _status.setDirty();
    _bearing.setDirty();
}

QString AntennaState::pack() const
{
    QJsonObject jv;

    jv.insert(rpcConstants::selected, _selected.pack());
    jv.insert(rpcConstants::rotatorStatus, status().getValue());
    jv.insert(rpcConstants::rotatorBearing, bearing().getValue());

    QJsonDocument json(jv);

    QString message(json.toJson(QJsonDocument::Compact));

    return message;
}
void AntennaState::unpack(QString s)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (!err.error)
    {
        QJsonValue selobj = json.object().value(rpcConstants::selected);
        _selected.unpack(selobj);
        _bearing.setValue(json.object().value(rpcConstants::rotatorBearing).toString());
        _status.setValue(json.object().value(rpcConstants::rotatorStatus).toString());
    }
    else
    {
        trace("Err " + err.errorString() + " Bad Json document " + s);
    }

}
void AntennaState::setBearing(const QString &bearing)
{
    _bearing.setValue(bearing);
}

void AntennaState::setStatus(const QString &status)
{
    _status.setValue(status);
}

void AntennaState::setSelected(const QString &loggeruuid, const QString &selected)
{
    _selected.setSelection(loggeruuid, selected);
}

MinosStringItem<QString> AntennaState::status() const
{
    return _status;
}

MinosStringItem<QString> AntennaState::bearing() const
{
    return _bearing;
}

MinosStringItem<QString> AntennaState::selected(QString loggerUuid) const
{
    return _selected.selected(loggerUuid);
}

