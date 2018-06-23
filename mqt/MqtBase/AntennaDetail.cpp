#include "base_pch.h"
#include "AntennaDetail.h"

AntennaDetail::AntennaDetail(): PubSubValue(AntennaDetailType)
{
    _maxAzimuth.setInitialValue(0);
    _minAzimuth.setInitialValue(0);
    _cwCcwCmdEnable.setInitialValue(false);
}
AntennaDetail::AntennaDetail(QString s):PubSubValue(AntennaDetailType)
{
    qRegisterMetaType< AntennaDetail > ( "AntennaDetail" );
    _maxAzimuth.setInitialValue(0);
    _minAzimuth.setInitialValue(0);
    _cwCcwCmdEnable.setInitialValue(false);
    unpack(s);
}
bool AntennaDetail::isDirty() const
{
    return (_selected.isDirty() || _minAzimuth.isDirty() || _maxAzimuth.isDirty() || _cwCcwCmdEnable.isDirty());
}
void AntennaDetail::clearDirty()
{
    _minAzimuth.clearDirty();
    _maxAzimuth.clearDirty();
    _cwCcwCmdEnable.clearDirty();
    _selected.clearDirty();
}
void AntennaDetail::setDirty()
{
    _minAzimuth.setDirty();
    _maxAzimuth.setDirty();
    _cwCcwCmdEnable.setDirty();
    _selected.setDirty();
}
QString AntennaDetail::pack() const
{
    QJsonObject jv;

    jv.insert(rpcConstants::selected, _selected.pack());
    jv.insert(rpcConstants::rotatorMinAzimuth, minAzimuth().getValue());
    jv.insert(rpcConstants::rotatorMaxAzimuth, maxAzimuth().getValue());
    jv.insert(rpcConstants::rotCwCcwCmdEnable, cwCcwCmdEnable().getValue());

    QJsonDocument json(jv);

    QString message(json.toJson(QJsonDocument::Compact));

    return message;
}
void AntennaDetail::unpack(QString s)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (!err.error)
    {
        QJsonValue selobj = json.object().value(rpcConstants::selected);
        _selected.unpack(selobj);
        _minAzimuth.setValue(json.object().value(rpcConstants::rotatorMinAzimuth).toInt());
        _maxAzimuth.setValue(json.object().value(rpcConstants::rotatorMaxAzimuth).toInt());
        _cwCcwCmdEnable.setValue(json.object().value(rpcConstants::rotCwCcwCmdEnable).toBool());
    }
    else
    {
        trace("Err " + err.errorString() + " Bad Json document " + s);
    }

}

void AntennaDetail::setMinAzimuth(int minAzimuth)
{
    _minAzimuth.setValue( minAzimuth);
}

void AntennaDetail::setMaxAzimuth(int maxAzimuth)
{
    _maxAzimuth.setValue(maxAzimuth);
}
void AntennaDetail::setCwCcwCmdEnable(bool cwCcwCmdEnable)
{
    _cwCcwCmdEnable.setValue(cwCcwCmdEnable);
}
void AntennaDetail::setSelected(const QString &loggeruuid, const QString &selected)
{
    _selected.setSelection(loggeruuid,selected);
}


MinosItem<int> AntennaDetail::minAzimuth() const
{
    return _minAzimuth;
}

MinosItem<int> AntennaDetail::maxAzimuth() const
{
    return _maxAzimuth;
}
MinosItem<bool> AntennaDetail::cwCcwCmdEnable()  const
{
    return _cwCcwCmdEnable;
}
MinosStringItem<QString> AntennaDetail::getSelectedContest(QString loggerUuid) const
{
    return _selected.getSelectedContest(loggerUuid);
}


