#ifndef ANTENNADETAIL_H
#define ANTENNADETAIL_H

#include "AnalysePubSubNotify.h"
#include "CacheSelection.h"
#include "minositem.h"

class AntennaDetail
{
    MinosItem<int> _minAzimuth;
    MinosItem<int> _maxAzimuth;
    MinosItem<bool> _cwCcwCmdEnable;
    MinosItem<bool> _supportStopCommand;
    MinosItem<bool> _skyScanEnabled;
    MinosItem<int> _skyScanStartBearing;
    MinosItem<int> _skyScanEndBearing;
    CacheSelection _selected;
public:
    AntennaDetail();
    AntennaDetail(QString s);
    ~AntennaDetail()
    {}

    bool isDirty() const;
    void clearDirty();
    void setDirty();

    QString pack() const ;
    void unpack(QString) ;

    MinosItem<int> minAzimuth() const;
    MinosItem<int> maxAzimuth() const;
    MinosItem<bool> cwCcwCmdEnable() const;
    MinosItem<bool> supportStopCommand() const;
    MinosStringItem<QString> getSelectedContest(QString loggerUuid) const;
    void setMinAzimuth(int minAzimuth);
    void setMaxAzimuth(int maxAzimuth);
    void setSupportStopCommand(bool state);
    void setCwCcwCmdEnable(bool cwCcwCmdEnable);

    void setSelected(const QString &loggeruuid, const QString &selected);

    MinosItem<bool> skyScanEnabled() const;
    MinosItem<int> skyScanStartBearing() const;
    MinosItem<int> skyScanEndBearing() const;

    void setSkyScanEnabled(bool state);
    void setSkyScanStartBearing(int startBearing);
    void setSkyScanEndBearing(int endBearing);
};

#endif // ANTENNADETAIL_H
