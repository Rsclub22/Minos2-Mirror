#ifndef ANTENNADETAIL_H
#define ANTENNADETAIL_H
#include "base_pch.h"

const QString AntennaDetailType("AntennaDetail");

class AntennaDetail: public PubSubValue
{
    MinosItem<int> _minAzimuth;
    MinosItem<int> _maxAzimuth;
    MinosItem<bool> _cwCcwCmdEnable;
    CacheSelection _selected;
public:
    AntennaDetail();
    AntennaDetail(QString s);
    virtual ~AntennaDetail() override
    {}

    bool isDirty() const;
    void clearDirty();
    void setDirty();

    virtual QString pack() const override;
    virtual void unpack(QString) override;

    MinosItem<int> minAzimuth() const;
    MinosItem<int> maxAzimuth() const;
    MinosItem<bool> cwCcwCmdEnable() const;
    MinosStringItem<QString> getSelectedContest(QString loggerUuid) const;
    void setMinAzimuth(int minAzimuth);
    void setMaxAzimuth(int maxAzimuth);
    void setCwCcwCmdEnable(bool cwCcwCmdEnable);
    void setSelected(const QString &loggeruuid, const QString &selected);
};

#endif // ANTENNADETAIL_H
