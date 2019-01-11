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
    MinosItem<double> _radioFreq;
    MinosItem<double> _logFreq;
    MinosStringItem<QString> _radioMode;
    MinosStringItem<QString> _logMode;
    MinosItem<int> _radioVolLevel;
    MinosItem<int> _logVolLevel;
    MinosItem<int> _radioRitFreq;
    MinosItem<int> _logRitFreq;
    MinosItem<bool> _ritOnOffStatus;
    MinosItem<bool> _ritRadioStatus;
    MinosItem<int> _tpm;

public:
    RigState();
    RigState(QString s);
    virtual ~RigState() override
    {}

    bool isDirty() const;
    void clearDirty();
    void setDirty();

    virtual QString pack() const override;
    virtual void unpack(QString) override;

    MinosStringItem<QString> getSelectedContest(QString loggerUuid) const;
    MinosStringItem<QString> status() const;
    MinosItem<double> radioFreq() const;
    MinosItem<double> logFreq() const;
    MinosStringItem<QString> radioMode() const;
    MinosStringItem<QString> logMode() const;
    MinosItem<bool> ritOnOffStatus() const;
    MinosItem<bool> ritRadioStatus() const;
    MinosItem<int> radioRitFreq() const;
    MinosItem<int> logRitFreq() const;
    MinosItem<int> radioVolLevel() const;
    MinosItem<int> logVolLevel() const;
    MinosItem<int> tpm() const;

    void setSelected(const QString &loggeruuid, const QString &selected);
    void setRadioFreq(double freq);
    void setLogFreq(double freq);
    void setRadioMode(const QString &mode);
    void setLogMode(const QString &mode);
    void setRadioVolume(int level);
    void setLogVolume(int level);
    void setStatus(const QString &status);
    void setRadioRitFreq(int freq);
    void setLogRitFreq(int freq);
    void setRitOnOffStatus(const bool status);
    void setRitRadioStatus(const bool status);
    void setTpm(int tpm);

    QStringList getSelectedLoggers();
};

#endif // RIGSTATE_H
