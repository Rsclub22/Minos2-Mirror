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
    MinosItem<bool> _transverterStatus;
    MinosItem<bool> _ritEnableStatus;
    MinosItem<bool> _ritOnOffStatus;
    MinosStringItem<QString> _bandList;

public:
    RigDetails();
    RigDetails(QString s);

    bool isDirty() const;
    void clearDirty();
    void setDirty();

    virtual QString pack() const;
    virtual void unpack(QString);

    MinosStringItem<QString> getSelectedContest(QString loggerUuid) const;
    MinosItem<double> transverterOffset() const;
    MinosItem<int> transverterSwitch() const;
    MinosItem<bool> transverterStatus() const;
    MinosItem<bool> ritEnableStatus() const;
    MinosItem<bool> ritOnOffStatus() const;
    MinosStringItem<QString> bandList() const;

    void setSelected(const QString &loggeruuid, const QString &selected);
    void setTransverterOffset(double transverterOffset);
    void setTransverterSwitch(int transverterSwitch);
    void setTransverterStatus(bool transverterStatus);
    void setBandList(const QString &bandList);
    void setRitEnableStatus(bool ritEnableStatus);
    void setRitOnOffStatus(bool status);
};


#endif // RIGDETAILS_H
