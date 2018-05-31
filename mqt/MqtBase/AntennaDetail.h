#ifndef ANTENNADETAIL_H
#define ANTENNADETAIL_H
#include "base_pch.h"

const QString AntennaDetailType("AntennaDetail");


class AntennaDetail: public PubSubValue
{
    MinosItem<int> _minAzimuth;
    MinosItem<int> _maxAzimuth;
    MinosItem<bool> _cwCcwCmdEnable;
    MinosStringItem<QString> _selected;
public:
    AntennaDetail();
    AntennaDetail(QString s);
    AntennaDetail(int minA, int maxA, const QString &sel, bool cwCcwCmdEnable):
        PubSubValue(AntennaDetailType)
    {
          _minAzimuth.setValue(minA);
          _maxAzimuth.setValue(maxA);
          _cwCcwCmdEnable.setValue(cwCcwCmdEnable);
          _selected.setValue(sel);
    }

    bool isDirty() const;
    void clearDirty();
    void setDirty();

    virtual QString pack() const;
    virtual void unpack(QString);

    MinosItem<int> minAzimuth() const;
    MinosItem<int> maxAzimuth() const;
    MinosItem<bool> cwCcwCmdEnable() const;
    MinosStringItem<QString> selected() const;
    void setMinAzimuth(int minAzimuth);
    void setMaxAzimuth(int maxAzimuth);
    void setCwCcwCmdEnable(bool cwCcwCmdEnable);
    void setSelected(const QString &selected);
};

#endif // ANTENNADETAIL_H
