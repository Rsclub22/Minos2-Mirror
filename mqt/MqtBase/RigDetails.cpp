#include "base_pch.h"
#include "RigDetails.h"


RigDetails::RigDetails()
    :PubSubValue(RigDetailsType)
{
    qRegisterMetaType< RigDetails > ( "RigDetails" );
    _transverterOffset.setInitialValue(0.0);
    _transverterSwitch.setInitialValue(0);
    _transverterStatus.setInitialValue(false);
    _ritEnableStatus.setInitialValue(false);
}
RigDetails::RigDetails(double tvo, int tvsw, bool tvst, const QString &blist, const QString &sel, const bool &ritst)
    :PubSubValue(RigDetailsType)
{
    _transverterOffset.setValue(tvo);
    _transverterSwitch.setValue(tvsw);
    _transverterStatus.setValue(tvst);
    _bandList.setValue(blist);
    _selected.setValue(sel);
    _ritEnableStatus.setValue(ritst);
}
RigDetails::RigDetails(QString s)
    :PubSubValue(RigDetailsType)
{
    unpack(s);
}

bool RigDetails::isDirty() const
{
    return
            _selected.isDirty() ||
            _transverterOffset.isDirty() ||
            _transverterSwitch.isDirty() ||
            _transverterStatus.isDirty() ||
            _bandList.isDirty() ||
            _ritEnableStatus.isDirty();
}
void RigDetails::clearDirty()
{
    _selected.clearDirty();
    _transverterOffset.clearDirty();
    _transverterSwitch.clearDirty();
    _transverterStatus.clearDirty();
    _bandList.clearDirty();
    _ritEnableStatus.clearDirty();
}
void RigDetails::setDirty()
{
    _selected.setDirty();
    _transverterOffset.setDirty();
    _transverterSwitch.setDirty();
    _transverterStatus.setDirty();
    _bandList.setDirty();
    _ritEnableStatus.setDirty();
}

void RigDetails::setSelected(const QString &selected)
{
    _selected.setValue(selected);
}
void RigDetails::setTransverterOffset(double transverterOffset)
{
    _transverterOffset.setValue(transverterOffset);
}

void RigDetails::setTransverterSwitch(int transverterSwitch)
{
    _transverterSwitch.setValue(transverterSwitch);
}

void RigDetails::setTransverterStatus(bool transverterStatus)
{
    _transverterStatus.setValue(transverterStatus);
}

void RigDetails::setRitEnableStatus(bool ritEnableStatus)
{
    _ritEnableStatus.setValue(ritEnableStatus);
}
void RigDetails::setBandList(const QString &bandList)
{
    _bandList.setValue( bandList);
}

QString RigDetails::pack() const
{
    QJsonObject jv;

    jv.insert(rpcConstants::selected, selected().getValue());
    jv.insert(rpcConstants::rigControlTxVertOffsetFreq, transverterOffset().getValue());
    jv.insert(rpcConstants::rigControlTxVertSwitch, transverterSwitch().getValue());
    jv.insert(rpcConstants::rigControlTxVertStatus, transverterStatus().getValue());
    jv.insert(rpcConstants::rigControlBandList, bandList().getValue());
    jv.insert(rpcConstants::rigRitEnableStatus, ritEnableStatus().getValue());

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
        _selected.setValue(json.object().value(rpcConstants::selected).toString());
        _transverterOffset.setValue(json.object().value(rpcConstants::rigControlTxVertOffsetFreq).toDouble());
        _transverterSwitch.setValue(json.object().value(rpcConstants::rigControlTxVertSwitch).toInt());
        _transverterStatus.setValue(json.object().value(rpcConstants::rigControlTxVertStatus).toBool());
        _bandList.setValue(json.object().value(rpcConstants::rigControlBandList).toString());
        _ritEnableStatus.setValue(json.object().value(rpcConstants::rigRitEnableStatus).toBool());
    }
    else
    {
        trace("Err " + err.errorString() + " Bad Json document " + s);
    }

}
MinosStringItem<QString> RigDetails::selected() const
{
    return _selected;
}
MinosItem<double> RigDetails::transverterOffset() const
{
    return _transverterOffset;
}

MinosItem<int> RigDetails::transverterSwitch() const
{
    return _transverterSwitch;
}

MinosItem<bool> RigDetails::transverterStatus() const
{
    return _transverterStatus;
}
MinosStringItem<QString> RigDetails::bandList() const
{
    return _bandList;
}
MinosItem<bool> RigDetails::ritEnableStatus() const
{
    return _ritEnableStatus;
}
