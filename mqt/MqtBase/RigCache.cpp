#include "base_pch.h"
#include "RigCache.h"
RigCache::RigCache()
{

}
QString RigCache::getStateString(const PubSubName &name) const
{
    QString val = rigStates[name].pack();
    return val;
}
void RigCache::invalidate()
{
    for(QMap<PubSubName, RigState>::iterator i = rigStates.begin(); i != rigStates.end(); i++ )
    {
        i->setDirty();
    }
    for(QMap<PubSubName, RigDetails>::iterator i = rigDetails.begin(); i != rigDetails.end(); i++ )
    {
        i->setDirty();
    }
}
void RigCache::invalidate(const PubSubName &name)
{
    rigStates[name].setDirty();
    rigDetails[name].setDirty();
}

void RigCache::setStateString(const AnalysePubSubNotify & an)
{
    RigState &as = rigStates[PubSubName(an)];
    as.unpack(an.getValue());
}
QString RigCache::getDetailsString(const PubSubName &name) const
{
    QString val = rigDetails[name].pack();
    return val;
}
void RigCache::setDetailsString(const AnalysePubSubNotify & an)
{
    RigDetails &as = rigDetails[PubSubName(an)];
    as.unpack(an.getValue());
}
void RigCache::addRigList(const QString &s)
{
    // clumsy code - there must be a better way!
    if (s.isEmpty())
        return;
    QStringList list = s.split(":");
    if (list.length())
    {
        // remove all rigs from this app from the rig list
        PubSubName lpsn = PubSubName(list[0]);
        QVector<PubSubName> newRigList;

        foreach(PubSubName psn, rigList)
        {
            if (lpsn.server() != psn.server() || lpsn.appName() != psn.appName())
                newRigList.push_back(psn);
        }
        rigList = newRigList;

    }
    foreach(QString l, list)
    {
        // add all of the list to rig list
        PubSubName psn(l);
        if (!rigList.contains(psn))
        {
            rigList.push_back(psn);
        }
    }
    {
        // clear now non-existant rigs from details
        QMap<PubSubName, RigDetails> newdets;
        QMap<PubSubName, RigDetails>::const_iterator i = rigDetails.constBegin();
        while (i != rigDetails.constEnd())
        {
            if (rigList.contains(i.key()))
            {
                newdets[i.key()] = i.value();
            }
            ++i;
        }
        rigDetails = newdets;
    }
    {
        // clear now non-existant rigs from states
        QMap<PubSubName, RigState> newstates;
        QMap<PubSubName, RigState>::const_iterator j = rigStates.constBegin();
        while (j != rigStates.constEnd())
        {
            if (rigList.contains(j.key()))
            {
                newstates[j.key()] = j.value();
            }
            ++j;
        }
        rigStates = newstates;
    }
    foreach(PubSubName psn, rigList)
    {
        if (!rigDetails.contains(psn))
        {
            rigDetails[psn] = RigDetails();
        }
        if (!rigStates.contains(psn))
        {
            rigStates[psn] = RigState();
        }
    }

    qSort(rigList);
}
RigState &RigCache::getState(const PubSubName &p)
{
    return rigStates[p];
}
RigDetails &RigCache::getDetails(const PubSubName &p)
{
    return rigDetails[p];
}

void RigCache::setState(const PubSubName &name, const RigState &state)
{
    rigStates[name] = state;
}
void RigCache::setDetails(const PubSubName &name, const RigDetails &details)
{
    rigDetails[name] = details;
}

void RigCache::setSelected(const PubSubName &name, const QString &sel)
{
    for(QMap<PubSubName, RigState>::iterator i = rigStates.begin(); i != rigStates.end(); i++ )
    {
        if (i.key() == name)
        {
            i.value().setSelected(sel);
            trace("selecting rig state " + i.key().toString() + "  " + sel);
        }
        else if (!i.value().selected().getValue().isEmpty())
        {
            trace("de-selecting rig state " + i.key().toString());
            i.value().setSelected("");
        }
    }
    for(QMap<PubSubName, RigDetails>::iterator i = rigDetails.begin(); i != rigDetails.end(); i++ )
    {
        if (i.key() == name)
        {
            i.value().setSelected(sel);
            trace("selecting rig details " + i.key().toString() + "  " + sel);
        }
        else if (!i.value().selected().getValue().isEmpty())
        {
            trace("de-selecting rig details " + i.key().toString());
            i.value().setSelected("");
        }
    }
}
PubSubName RigCache::getSelected()
{
    for(QMap<PubSubName, RigState>::iterator i = rigStates.begin(); i != rigStates.end(); i++ )
    {
        if (!i.value().selected().getValue().isEmpty())
        {
            PubSubName psn = i.key();
            return psn;
        }
    }
    return PubSubName();
}
void RigCache::setStatus(const PubSubName &name, const QString &status)
{
    rigStates[name].setStatus(status);
}

void RigCache::setFreq(const PubSubName &name, double freq)
{
    rigStates[name].setFreq(freq);
}
void RigCache::setMode(const PubSubName &name, const QString &mode)
{
    rigStates[name].setMode(mode);
}

void RigCache::setTransverterOffset(const PubSubName &name, double transverterOffset)
{
    rigDetails[name].setTransverterOffset(transverterOffset);
}
void RigCache::setTransverterSwitch(const PubSubName &name, int transverterSwitch)
{
    rigDetails[name].setTransverterSwitch(transverterSwitch);
}
void RigCache::setTransverterStatus(const PubSubName &name, bool transverterStatus)
{
    rigDetails[name].setTransverterStatus(transverterStatus);
}
void RigCache::setBandList(const PubSubName &name, const QString &bands)
{
    rigDetails[name].setBandList(bands);
}
void RigCache::setRitFreq(const PubSubName &name, double freq)
{
    rigStates[name].setRitFreq(freq);
}
void RigCache::setRitEnableStatus(const PubSubName &name, bool ritEnableStatus)
{
    rigDetails[name].setRitEnableStatus(ritEnableStatus);
}
void RigCache::setRitOnOffStatus(const PubSubName &name, bool status)
{
    rigDetails[name].setRitOnOffStatus(status);
}

void RigCache::publishState()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    for(QMap<PubSubName, RigState>::iterator i = rigStates.begin(); i != rigStates.end(); i++ )
    {
        if (i.value().isDirty())
        {
            rpc->publish(rpcConstants::rigStateCategory, i.key().toString(), i.value().pack(), psPublished);
            rigStates[i.key()].clearDirty();
        }
    }

}
void RigCache::publishDetails()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    for(QMap<PubSubName, RigDetails>::iterator i = rigDetails.begin(); i != rigDetails.end(); i++ )
    {
        if (i.value().isDirty())
        {
            rpc->publish(rpcConstants::rigDetailsCategory, i.key().toString(), i.value().pack(), psPublished);
            rigDetails[i.key()].clearDirty();
        }
    }

}
