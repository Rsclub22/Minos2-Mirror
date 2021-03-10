#ifndef RIGCACHE_H
#define RIGCACHE_H

#include "base_pch.h"
#include "RigState.h"
#include "RigDetails.h"

class RigCache
{
    QMap<PubSubName, RigState> rigStates;
    QMap<PubSubName, RigDetails> rigDetails;
    QVector<PubSubName> rigList;
    void publishState();
    void publishDetails();
public:
    RigCache();

    void invalidate();
    void invalidate(const PubSubName &name);
    void publish()
    {
        publishState();
        publishDetails();
    }

    int getRigDetailCount()
    {
        return rigDetails.count();
    }

    int getRigListCount()
    {
        return rigList.count();
    }
    void setStateString(const AnalysePubSubNotify & an);
    void setStateDisconnected(const AnalysePubSubNotify & an);
    QString getDetailsString(const PubSubName &name) const;
    void setDetailsString(const AnalysePubSubNotify & an);

    void setState(const PubSubName &name, const RigState &state);
    void setDetails(const PubSubName &name, const RigDetails &details);

    PubSubName getSelected(QString loggerUuid);
    QString getSelectedContest(PubSubName psn, QString loggerUuid);
    QStringList getSelectedLoggers(PubSubName psn);
    PubSubName getSelectedRadio(PubSubName psn);

    RigState &getState(const PubSubName &p);
    RigDetails &getDetails(const PubSubName &p);
    bool setSelected(const PubSubName &name, const QString &loggeruuid, const QString &selected);
    void setStatus(const PubSubName &name, const QString &status);

    void setRadioFreq(const PubSubName &name, Frequency freq);
    void setLogFreq(const PubSubName &name, Frequency freq);

    void setVoiceMessageNum(const PubSubName &name, const QString &msgNum);

    void setRadioVolume(const PubSubName &name, const int level);
    void setLogVolume(const PubSubName &name, const int level);
    void setRadioMode(const PubSubName &name, const QString &mode);
    void setLogMode(const PubSubName &name, const QString &mode);
    void setTransverterOffset(const PubSubName &name, double transverterOffset);
    void setTransverterSwitch(const PubSubName &name, int transverterSwitch);
    void setTransverterEnabled(const PubSubName &name, bool transverterEnabled);
    void setTransverterStatus(const PubSubName &name, bool transverterStatus);
    void setVolumeStatus(const PubSubName &name, bool volumeStatus);
    void setBandList(const PubSubName &name, const QString &bands);
    void setRadioRitFreq(const PubSubName &name, const ShortFreq &freq);
    void setLogRitFreq(const PubSubName &name, ShortFreq freq);
    void setRitMaxKHzFreq(const PubSubName &name, int ritMaxKHzFreq);
    void setRitEnableStatus(const PubSubName &name, bool ritEnableStatus);
    void setRitOnOffStatus(const PubSubName &name, bool status);
    void setRadioRitStatus(const PubSubName &name, bool status);
    void addRigList(const QString &s);
    void setIgnorePresetFreq(const PubSubName &name, bool ignorePresetFreqFlag);
    void setIgnorePreviousFreq(const PubSubName &name, bool ignorePreviousFreqFlag);


    void setPttEnabled(const PubSubName &name, bool state);
    void setPttType(const PubSubName &name, int type);
    void setPttState(const PubSubName &name, bool state);

    QVector<PubSubName> &getRigList()
    {
        return rigList;
    }
    

    Frequency getRadioFreq(const PubSubName &name);
    void setLogBand(const PubSubName &name, QString band);
    void setHfFlag(const PubSubName &name, bool state);

};

#endif // RIGCACHE_H
