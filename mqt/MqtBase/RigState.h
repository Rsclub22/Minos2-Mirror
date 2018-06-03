/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2018
//
/////////////////////////////////////////////////////////////////////////////

#ifndef RIGSTATE_H
#define RIGSTATE_H
#include "base_pch.h"

const QString RigStateType("RigState");
class RigState: public PubSubValue
{
    CacheSelection _selected;
    MinosStringItem<QString> _status;
    MinosItem<double> _freq;
    MinosStringItem<QString> _mode;
    MinosItem<double> _ritFreq;
    MinosStringItem<QString> _ritEnableStatus;


public:
    RigState();
    RigState(QString s);
    //RigState(const QString &status, const QString &sel, int f, const QString &m);
    RigState(const QString &status, const QString &loggeruuid, const QString &sel, int f, const QString &m, double ritF, QString &rites);

    bool isDirty() const;
    void clearDirty();
    void setDirty();

    virtual QString pack() const;
    virtual void unpack(QString);

    MinosStringItem<QString> selected(QString loggerUuid) const;
    MinosStringItem<QString> status() const;
    MinosItem<double> freq() const;
    MinosStringItem<QString> mode() const;
    MinosStringItem<QString> ritEnableStatus() const;
    MinosItem<double> ritFreq() const;

    void setSelected(const QString &loggeruuid, const QString &selected);
    void setFreq(double freq);
    void setMode(const QString &mode);
    void setStatus(const QString &status);
    void setRitFreq(double freq);
    void setRitEnableStatus(const QString &status);
};

#endif // RIGSTATE_H
