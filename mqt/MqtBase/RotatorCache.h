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

    void setStateString(const AnalysePubSubNotify & an);
    void setStateDisconnected(const AnalysePubSubNotify & an);
    void setStateDisconnected();

    QString getPresetsString(const PubSubName &name) const;
    void setPresetsString(const AnalysePubSubNotify & an);
    bool rotatorPresetsIsDirty(const PubSubName &name);

    void setDetail(const PubSubName &name, const AntennaDetail &detail);
    void setState(const PubSubName &name, const AntennaState &state);

    PubSubName getSelected(QString logger);
    PubSubName getSelectedAntenna(PubSubName psn);
    QStringList getSelectedLoggers(PubSubName psn);
    QString getSelectedContest(PubSubName psn, QString loggerid);

    bool setSelected(const PubSubName &name, const QString &loggeruuid, const QString &contestuuid);
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

    void rotatorPresetsClearDirty();
    void setSupportStopCommand(const PubSubName &name, bool state);
    void setEndStopSouthStopOffset(const PubSubName &name, QString data);
    void setSkyScanVisible(const PubSubName &name, const bool visible);
    void setSkyScanStartBearing(const PubSubName &name, const int startBearing);
    void setSkyScanEndBearing(const PubSubName &name, const int endBearing);
    void setSkyScanNextStep(const PubSubName &name, QString bearing);
    void setSkyScanCountDown(const PubSubName &name, QString countDown);
    void setSkyScanButtonState(const PubSubName &name, int state);
    void setSkyScanReverseScan(const PubSubName &name, bool state);
    void setSkyScanRotatorStartBearing(const PubSubName &name, const int rotatorStartBearing);
    void setSkyScanRotatorEndBearing(const PubSubName &name, const int rotatorEndBearing);

};

#endif // ROTATORCACHE_H
