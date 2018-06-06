/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2018
//
/////////////////////////////////////////////////////////////////////////////


#ifndef ROTATORCACHE_H
#define ROTATORCACHE_H

#include "base_pch.h"
#include "AntennaDetail.h"
#include "AntennaState.h"

class RotatorCache
{
    QMap<PubSubName, AntennaDetail> rotDetails;
    QMap<PubSubName, AntennaState> rotStates;
    QMap<PubSubName, MinosStringItem<QString> > rotPresets;
    QVector<PubSubName> rotList;
    void publishState( );
    void publishDetails( );
    void publishPresets();
public:
    RotatorCache();

    void invalidate();
    void invalidate(const PubSubName &name);
    void publish()
    {
        publishState();
        publishDetails();
        publishPresets();
    }


    AntennaState &getState(const PubSubName &p);
    AntennaDetail &getDetails(const PubSubName &p);

    QString getDetailString(const PubSubName &name) const;
    void setDetailString(const AnalysePubSubNotify &an);

    QString getStateString(const PubSubName &name) const;
    void setStateString(const AnalysePubSubNotify & an);

    QString getPresetsString(const PubSubName &name) const;
    void setPresetsString(const AnalysePubSubNotify & an);
    bool rotatorPresetsIsDirty(const PubSubName &name);

    void setDetail(const PubSubName &name, const AntennaDetail &detail);
    void setState(const PubSubName &name, const AntennaState &state);

    PubSubName getSelected(QString loggerUuid);
    QString getSelectedContest(QString loggerUuid);

    void setSelected(const PubSubName &name, const QString &loggeruuid, const QString &sel);
    void setStatus(const PubSubName &name, const QString &state);
    void setBearing(const PubSubName &name, const QString &bearing);

    void setMinAzimuth(const PubSubName &name, int minaz);
    void setMaxAzimuth(const PubSubName &name, int maxaz);
    void setCwCcwCmdEnable(const PubSubName &name, bool cwCcwCmdEnable);

    void setRotatorPresets(const PubSubName &name, const QString &p);
    QString getRotatorPresets(const PubSubName &name);

    void addRotList(const QString &s);
    QVector<PubSubName> getRotList()
    {
        return rotList;
    }

    bool rotatorPresetsClearDirty();
};

#endif // ROTATORCACHE_H
