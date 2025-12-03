#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include "AntennaState.h"
#include "RPCCommandConstants.h"
#include "MTrace.h"

AntennaState::AntennaState()
{
}

AntennaState::AntennaState(QString s)
{
    qRegisterMetaType< AntennaState > ( "AntennaState" );
    unpack(s);
}
bool AntennaState::isDirty() const
{
    return (_selected.isDirty()
            || _status.isDirty()
            || _bearing.isDirty()
            || _skyScanNextStep.isDirty()
            || _skyScanCountDown.isDirty()
            || _skyScanButtonState.isDirty()
            || _skyScanReverseScan.isDirty());
}
void AntennaState::clearDirty()
{
    _selected.clearDirty();
    _status.clearDirty();
    _bearing.clearDirty();
    _skyScanCountDown.clearDirty();
    _skyScanButtonState.clearDirty();
    _skyScanReverseScan.clearDirty();
}
void AntennaState::setDirty()
{
    _selected.setDirty();
    _status.setDirty();
    _bearing.setDirty();
    _skyScanCountDown.setDirty();
    _skyScanButtonState.setDirty();
    _skyScanReverseScan.setDirty();
}

QString AntennaState::pack() const
{
    QJsonObject jv;

    jv.insert(rpcConstants::selected, _selected.pack());
    jv.insert(rpcConstants::rotatorStatus, status().getValue());
    jv.insert(rpcConstants::rotatorBearing, bearing().getValue());
    jv.insert(rpcConstants::skyScanNextStep, skyScanNextStep().getValue());
    jv.insert(rpcConstants::skyScanCountDown, skyScanCountDown().getValue());
    jv.insert(rpcConstants::skyScanButtonState, skyScanButtonState().getValue());
    jv.insert(rpcConstants::skyScanReverseScan, skyScanReverseScan().getValue());

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
        _skyScanNextStep.setValue(json.object().value(rpcConstants::skyScanNextStep).toString());
        _skyScanCountDown.setValue(json.object().value(rpcConstants::skyScanCountDown).toString());
        _skyScanButtonState.setValue(json.object().value(rpcConstants::skyScanButtonState).toInt());
        _skyScanReverseScan.setValue(json.object().value(rpcConstants::skyScanReverseScan).toBool());
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

MinosStringItem<QString> AntennaState::getSelectedContest(QString loggerUuid) const
{
    return _selected.getSelectedContest(loggerUuid);
}

QStringList AntennaState::getSelectedLoggers()
{
    return _selected.getSelectedLoggers();
}
MinosStringItem<QString> AntennaState::skyScanNextStep() const
{
    return _skyScanNextStep;
}
MinosStringItem<QString> AntennaState::skyScanCountDown() const
{
    return _skyScanCountDown;
}
MinosItem<int> AntennaState::skyScanButtonState() const
{
    return _skyScanButtonState;
}
MinosItem<bool> AntennaState::skyScanReverseScan() const
{
    return _skyScanReverseScan;
}

void AntennaState::setSkyScanNextStep(const QString &skyScanNextStep)
{
    _skyScanNextStep.setValue(skyScanNextStep);
}
void AntennaState::setSkyScanCountDown(const QString &skyScanCountDown)
{
    _skyScanCountDown.setValue(skyScanCountDown);
}
void AntennaState::setSkyScanButtonState(int state)
{
    _skyScanButtonState.setValue(state);
}
void AntennaState::setSkyScanReverseScan(bool reverseScan)
{
    _skyScanReverseScan.setValue(reverseScan);
}


