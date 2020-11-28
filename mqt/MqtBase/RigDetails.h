/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2018
//
/////////////////////////////////////////////////////////////////////////////



#ifndef RIGDETAILS_H
#define RIGDETAILS_H
#include "base_pch.h"

const QString RigDetailsType("RigDetails");
class RigDetails: public PubSubValue
{
    CacheSelection _selected;
    MinosItem<double> _transverterOffset;
    MinosItem<int> _transverterSwitch;
    MinosItem<bool> _transverterEnabled;
    MinosItem<bool> _transverterStatus;
    MinosItem<bool> _volumeStatus;
    MinosItem<bool> _ritEnableStatus;
    MinosItem<int> _ritMaxKHzFreq;
    MinosStringItem<QString> _bandList;
    MinosItem<bool> _hfFlag;

public:
    RigDetails();
    RigDetails(QString s);
    virtual ~RigDetails() override
    {}

    bool isDirty() const;
    void clearDirty();
    void setDirty();

    virtual QString pack() const override;
    virtual void unpack(QString) override;

    MinosStringItem<QString> getSelectedContest(QString loggerUuid) const;
    MinosItem<double> transverterOffset() const;
    MinosItem<int> transverterSwitch() const;
    MinosItem<bool> transverterEnabled() const;
    MinosItem<bool> transverterStatus() const;
    MinosItem<bool> volumeStatus() const;
    MinosItem<bool> ritEnableStatus() const;
    MinosItem<int> ritMaxKHzFreq() const;
    MinosStringItem<QString> bandList() const;
    MinosItem<bool> hfFlag() const;

    void setSelected(const QString &loggeruuid, const QString &selected);
    void setTransverterOffset(double transverterOffset);
    void setTransverterSwitch(int transverterSwitch);
    void setTransverterEnabled(bool transverterEnabled);
    void setTransverterStatus(bool transverterStatus);
    void setVolumeStatus(bool volumeStatus);
    void setBandList(const QString &bandList);
    void setRitEnableStatus(bool ritEnableStatus);
    void setRitMaxKHzFreq(int ritMaxKHz);
    void setHfFlag(bool state);



};


#endif // RIGDETAILS_H
