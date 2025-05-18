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
    MinosItem<QString> _rotatorStopSouthStopOffset;
    MinosItem<bool> _skyScanVisible;
    MinosItem<int> _skyScanStartBearing;
    MinosItem<int> _skyScanEndBearing;
    MinosItem<int> _skyScanRotatorStartBearing;
    MinosItem<int> _skyScanRotatorEndBearing;

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
    MinosItem<QString> rotatorStopSouthStopOffset() const;
    MinosStringItem<QString> getSelectedContest(QString loggerUuid) const;
    void setMinAzimuth(int minAzimuth);
    void setMaxAzimuth(int maxAzimuth);
    void setSupportStopCommand(bool state);
    void setEndStopSouthStopOffset(QString data);

    void setCwCcwCmdEnable(bool cwCcwCmdEnable);

    void setSelected(const QString &loggeruuid, const QString &selected);

    MinosItem<bool> skyScanVisible() const;
    MinosItem<int> skyScanStartBearing() const;
    MinosItem<int> skyScanEndBearing() const;
    MinosItem<int> skyScanRotatorStartBearing() const;
    MinosItem<int> skyScanRotatorEndBearing() const;



    void setSkyScanVisible(bool state);
    void setSkyScanStartBearing(int startBearing);
    void setSkyScanEndBearing(int endBearing);
    void setSkyScanRotatorStartBearing(int rotatorStartBearing);
    void setSkyScanRotatorEndBearing(int rotatorEndBearing);
};

#endif // ANTENNADETAIL_H
