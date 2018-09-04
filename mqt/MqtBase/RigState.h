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
    MinosItem<int> _volLevel;
    MinosItem<double> _ritFreq;
    MinosItem<bool> _ritOnOffStatus;
    MinosItem<bool> _ritRadioStatus;
    MinosItem<int> _tpm;

public:
    RigState();
    RigState(QString s);

    bool isDirty() const;
    void clearDirty();
    void setDirty();

    virtual QString pack() const;
    virtual void unpack(QString);

    MinosStringItem<QString> getSelectedContest(QString loggerUuid) const;
    MinosStringItem<QString> status() const;
    MinosItem<double> freq() const;
    MinosStringItem<QString> mode() const;
    MinosItem<bool> ritOnOffStatus() const;
    MinosItem<bool> ritRadioStatus() const;
    MinosItem<double> ritFreq() const;
    MinosItem<int> volLevel() const;
    MinosItem<int> tpm() const;

    void setSelected(const QString &loggeruuid, const QString &selected);
    void setFreq(double freq);
    void setMode(const QString &mode);
    void setVolume(int level);
    void setStatus(const QString &status);
    void setRitFreq(double freq);
    void setRitOnOffStatus(const bool status);
    void setRitRadioStatus(const bool status);
    void setTpm(int tpm);

    QStringList getSelectedLoggers();
};

#endif // RIGSTATE_H
