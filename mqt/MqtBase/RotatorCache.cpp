#include "base_pch.h"
#include "rotatorcommon.h"
#include "RotatorCache.h"

RotatorCache::RotatorCache()
{

}
void RotatorCache::invalidate()
{
    for(auto &i: rotStates )
    {
        i.setDirty();
    }
    for(auto &i: rotDetails )
    {
        i.setDirty();
    }
    for(auto &i: rotPresets )
    {
        i.setDirty();
    }

}
void RotatorCache::invalidate(const PubSubName &name)
{
    rotStates[name].setDirty();
    rotDetails[name].setDirty();

    PubSubName n(name);
    n.setKey("");
    rotPresets[n].setDirty();
}
void RotatorCache::addRotList(const QString &s)
{
    // clumsy code - there must be a better way!
    if (s.isEmpty())
        return;
    QStringList list = s.split(":");
    if (list.length())
    {
        // remove all rots from this app from the rot list
        PubSubName lpsn = PubSubName(list[0]);
        QVector<PubSubName> newRotList;

        for(auto const &psn: qAsConst(rotList))
        {
            if (lpsn.server() != psn.server() || lpsn.appName() != psn.appName())
                newRotList.push_back(psn);
        }
        rotList = newRotList;

    }
    for(auto const &l: qAsConst(list))
    {
        // add all of the list to rot list
        PubSubName psn(l);
        if (!rotList.contains(psn))
        {
            rotList.push_back(psn);
        }
    }
    {
        // clear now non-existant rots from details
        QMap<PubSubName, AntennaDetail> newdets;
        QMap<PubSubName, AntennaDetail>::const_iterator i = rotDetails.constBegin();
        while (i != rotDetails.constEnd())
        {
            if (rotList.contains(i.key()))
            {
                newdets[i.key()] = i.value();
            }
            ++i;
        }
        rotDetails = newdets;
    }
    {
        // clear now non-existant rots from states
        QMap<PubSubName, AntennaState> newstates;
        QMap<PubSubName, AntennaState>::const_iterator j = rotStates.constBegin();
        while (j != rotStates.constEnd())
        {
            if (rotList.contains(j.key()))
            {
                newstates[j.key()] = j.value();
            }
            ++j;
        }
        rotStates = newstates;
    }
    for(auto const &psn: qAsConst(rotList))
    {
        if (!rotDetails.contains(psn))
        {
            rotDetails[psn] = AntennaDetail();
        }
        if (!rotStates.contains(psn))
        {
            rotStates[psn] = AntennaState();
        }
    }
    std::sort(rotList.begin(), rotList.end());
}

AntennaState &RotatorCache::getState(const PubSubName &p)
{
    return rotStates[p];
}
AntennaDetail &RotatorCache::getDetails(const PubSubName &p)
{
    return rotDetails[p];
}

QString RotatorCache::getDetailString(const PubSubName &name) const
{
    QString val = rotDetails[name].pack();
    return val;
}
void RotatorCache::setDetailString(const AnalysePubSubNotify & an)
{
    AntennaDetail &ad = rotDetails[PubSubName(an)];
    ad.unpack(an.getValue());
}
void RotatorCache::setStateString(const AnalysePubSubNotify &an)
{
    AntennaState &as = rotStates[PubSubName(an)];
    as.unpack(an.getValue());
}

void RotatorCache::setStateDisconnected(const AnalysePubSubNotify &an)
{
    AntennaState &as = rotStates[PubSubName(an)];
    as.setStatus(ROT_STATUS_DISCONNECTED);
}
QString RotatorCache::getPresetsString(const PubSubName &name) const
{
    PubSubName n(name);
    n.setKey("");
    QString val = rotPresets[n].getValue();

    QJsonObject jv;

    jv.insert(rpcConstants::rotPresetList, val);

    QJsonDocument json(jv);

    QString message(json.toJson(QJsonDocument::Compact));

    return message;

}
void RotatorCache::setPresetsString(const AnalysePubSubNotify & an)
{
    PubSubName n(an);
    n.setKey("");
    QString s = an.getValue();

    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (!err.error)
    {
        rotPresets[n].setValue(json.object().value(rpcConstants::rotPresetList).toString());
    }
    else
    {
        trace("Err " + err.errorString() + " Bad Json document " + s);
    }
}
bool RotatorCache::rotatorPresetsIsDirty(const PubSubName &name)
{
    PubSubName n(name);
    n.setKey("");
    return rotPresets[n].isDirty();

}
void RotatorCache::rotatorPresetsClearDirty()
{
    for(auto &i: rotPresets )
    {
        i.clearDirty();
    }
}

void RotatorCache::setDetail(const PubSubName &name, const AntennaDetail &detail)
{
    rotDetails[name] = detail;
}
void RotatorCache::setState(const PubSubName &name, const AntennaState &state)
{
    rotStates[name] = state;
}

bool RotatorCache::setSelected(const PubSubName &name, const QString &loggeruuid, const QString &contestuuid)
{
    trace("setSelected rotator " + name.toString()+ " logger " + loggeruuid + " contest " + contestuuid);
    PubSubName psnSelected = getSelectedAntenna(name);
    QStringList loggers = getSelectedLoggers(psnSelected);
    bool selOK = false;

    if (contestuuid.isEmpty())
    {
        // de-select this contest/logger in name
        trace("deselecting rotator; always OK");
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
        trace("selecting rotator " + name.toString()+ " logger " + loggeruuid + " contest " + contestuuid);
        if (!name.isEmpty())
        {
            rotStates[name].setSelected(loggeruuid, contestuuid);
            rotDetails[name].setSelected(loggeruuid, contestuuid);
            if (contestuuid.isEmpty())
            {
                rotStates[name].setStatus(ROT_STATUS_DISCONNECTED);
            }
        }
    }
    return selOK;
}
PubSubName RotatorCache::getSelected(QString logger)
{
    for(QMap<PubSubName, AntennaState>::iterator i = rotStates.begin(); i != rotStates.end(); i++ )
    {
        if (!i.value().getSelectedContest(logger).getValue().isEmpty())
        {
            PubSubName psn = i.key();
            return psn;
        }
    }
    return PubSubName();
}
QString RotatorCache::getSelectedContest(PubSubName psn, QString loggerid)
{
    for(QMap<PubSubName, AntennaState>::iterator i = rotStates.begin(); i != rotStates.end(); i++ )
    {
        if (i.value().getSelectedLoggers().count() > 0
                && i.key().server() == psn.server()
                && i.key().appName() == psn.appName())
        {
            return i.value().getSelectedContest(loggerid).getValue(); // antenna selected on this app
        }
    }
    return QString();
}
QStringList RotatorCache::getSelectedLoggers(PubSubName psn)
{
    AntennaState &s = rotStates[psn];
    QStringList loggers = s.getSelectedLoggers();
    return loggers;
}
PubSubName RotatorCache::getSelectedAntenna(PubSubName psn)
{
    for(QMap<PubSubName, AntennaState>::iterator i = rotStates.begin(); i != rotStates.end(); i++ )
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
void RotatorCache::setStatus(const PubSubName &name, const QString &state)
{
    rotStates[name].setStatus(state);
}
void RotatorCache::setBearing(const PubSubName &name, const QString &bearing)
{
    rotStates[name].setBearing(bearing);
}
void RotatorCache::setMinAzimuth(const PubSubName &name, int minaz)
{
    rotDetails[name].setMinAzimuth(minaz);
}
void RotatorCache::setMaxAzimuth(const PubSubName &name, int maxaz)
{
    rotDetails[name].setMaxAzimuth(maxaz);
}
void RotatorCache::setCwCcwCmdEnable(const PubSubName &name, bool cwCcwCmdEnable)
{
    rotDetails[name].setCwCcwCmdEnable(cwCcwCmdEnable);
}
void RotatorCache::setSupportStopCommand(const PubSubName &name, bool state)
{
    rotDetails[name].setSupportStopCommand(state);
}
void RotatorCache::setRotatorPresets(const PubSubName &name, const QString &p)
{
    PubSubName n(name);
    n.setKey("");
    rotPresets[n].setValue(p);
}
QString RotatorCache::getRotatorPresets(const PubSubName &name)
{
    PubSubName n(name);
    n.setKey("");
    return rotPresets[n].getValue();
}

void RotatorCache::publishState()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    for(QMap<PubSubName, AntennaState>::iterator i = rotStates.begin(); i != rotStates.end(); i++ )
    {
        if (i.value().isDirty())
        {
            if (!i.key().isEmpty())
            {
                rpc->publish(rpcConstants::rotatorStateCategory, i.key().toString(), i.value().pack(), psPublished);
            }
            rotStates[i.key()].clearDirty();
        }
    }

}
void RotatorCache::publishDetails()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    for(QMap<PubSubName, AntennaDetail>::iterator i = rotDetails.begin(); i != rotDetails.end(); i++ )
    {
        if (i.value().isDirty())
        {
            if (!i.key().isEmpty())
            {
                rpc->publish(rpcConstants::rotatorDetailCategory, i.key().toString(), i.value().pack(), psPublished);
            }
            rotDetails[i.key()].clearDirty();
        }
    }

}
void RotatorCache::publishPresets()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    for(QMap<PubSubName, MinosStringItem<QString> >::iterator i = rotPresets.begin(); i != rotPresets.end(); i++ )
    {
        if (i.value().isDirty())
        {
            QString packed = getPresetsString(i.key());
            rpc->publish(rpcConstants::rotatorPresetsCategory, i.key().toString(), packed, psPublished);
            rotDetails[i.key()].clearDirty();
        }
    }

}
