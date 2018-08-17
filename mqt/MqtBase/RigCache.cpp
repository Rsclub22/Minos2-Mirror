#include "base_pch.h"
#include "rigcommon.h"
#include "RigCache.h"
RigCache::RigCache()
{

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

bool RigCache::setSelected(const PubSubName &name, const QString &loggeruuid, const QString &contestuuid)
{
    PubSubName psnSelected = getSelectedRadio(name);
    QStringList loggers = getSelectedLoggers(psnSelected);
    bool selOK = false;

    if (contestuuid.isEmpty())
    {
        // de-select this contest/logger in name
        trace("deselecting radio; always OK");
        selOK = true;
    }
    else if (name.isEmpty())
    {
        selOK = true;
        trace ("selection OK; name empty");
    }
    else if (psnSelected.isEmpty())
    {
        selOK = true;
        trace ("selection OK; current selection empty");
    }
    else if (psnSelected == name)
    {
        selOK = true;
        trace ("selection OK; selecting current rotator");
    }
    else if ((loggers.size() == 0) || (loggers.size() == 1 && loggers[0] == loggeruuid))
    {
        selOK = true;
        trace ("selection OK; selection for this logger");

    }
    else
    {
        trace (QString("selection NOT OK; name %1 psnselected %2 loggers %3 ")
               .arg(name.toString()).arg(psnSelected.toString()).arg(loggers.join(";")));

    }

    if (selOK)
    {
        trace("selecting radio " + name.toString()+ " logger " + loggeruuid + " contest " + contestuuid);
        if (!name.isEmpty())
        {
            rigStates[name].setSelected(loggeruuid, contestuuid);
            rigDetails[name].setSelected(loggeruuid, contestuuid);
            if (contestuuid.isEmpty())
            {
                rigStates[name].setStatus(RIG_STATUS_DISCONNECTED);
            }
        }
    }
    return selOK;
}
PubSubName RigCache::getSelected(QString loggerUuid)
{
    for(QMap<PubSubName, RigState>::iterator i = rigStates.begin(); i != rigStates.end(); i++ )
    {
        if (!i.value().getSelectedContest(loggerUuid).getValue().isEmpty())
        {
            PubSubName psn = i.key();
            return psn;
        }
    }
    return PubSubName();
}
QString RigCache::getSelectedContest(PubSubName psn, QString loggerUuid)
{
    for(QMap<PubSubName, RigState>::iterator i = rigStates.begin(); i != rigStates.end(); i++ )
    {
        if (i.value().getSelectedLoggers().count() > 0
                && i.key().server() == psn.server()
                && i.key().appName() == psn.appName())
        {
            return i.value().getSelectedContest(loggerUuid).getValue(); // antenna selected on this app
        }
    }
    return QString();
}
QStringList RigCache::getSelectedLoggers(PubSubName psn)
{
    RigState &s = rigStates[psn];
    QStringList loggers = s.getSelectedLoggers();
    return loggers;
}
PubSubName RigCache::getSelectedRadio(PubSubName psn)
{
    for(QMap<PubSubName, RigState>::iterator i = rigStates.begin(); i != rigStates.end(); i++ )
    {
        if (i.value().getSelectedLoggers().count() > 0
                && i.key().server() == psn.server()
                && i.key().appName() == psn.appName())
        {
            return i.key(); // antenna selected on this app
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
void RigCache::setVolume(const PubSubName &name, const int level)
{
    rigStates[name].setVolume(level);
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
void RigCache::setVolumeStatus(const PubSubName &name, bool volumeStatus)
{
    rigDetails[name].setVolumeStatus(volumeStatus);
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
void RigCache::setTpm(const PubSubName &name, int tpm)
{
    rigStates[name].setTpm(tpm);
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
