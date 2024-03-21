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

#include "AnalysePubSubNotify.h"
#include "CacheSelection.h"
#include "frequency.h"

class RigState
{
    CacheSelection _selected;
    MinosStringItem<QString> _status;
    MinosFrequencyItem<Frequency> _radioFreq;
    MinosFrequencyItem<Frequency> _logFreq;
    MinosStringItem<QString> _logBand;
    MinosStringItem<QString> _radioVoiceMessageNum;
    MinosStringItem<QString> _radioStopVoiceMessage;
    MinosStringItem<QString> _cwTxMessage;
    MinosStringItem<QString> _radioMode;
    MinosStringItem<QString> _logMode;
    MinosItem<int> _radioVolLevel;
    MinosItem<int> _logVolLevel;
    MinosItem<ShortFreq> _radioRitFreq;
    MinosItem<ShortFreq> _logRitFreq;
    MinosItem<bool> _ritOnOffStatus;
    MinosItem<bool> _ritRadioStatus;
    MinosItem<bool> _pttState;
    MinosItem<bool> _setPttOnOff;


public:
    RigState();
    RigState(QString s);
    ~RigState()
    {}

    bool isDirty() const;
    void clearDirty();
    void setDirty();

    QString pack() const ;
    void unpack(QString) ;

    MinosStringItem<QString> getSelectedContest(QString loggerUuid) const;
    MinosStringItem<QString> status() const;
    MinosFrequencyItem<Frequency> radioFreq() const;
    MinosFrequencyItem<Frequency> logFreq() const;
    MinosItem<QString> logBand() const;
    MinosStringItem<QString> radioMode() const;
    MinosStringItem<QString> radioVoiceMessageNum() const;
    MinosStringItem<QString> radioStopVoiceMessage() const;
    MinosStringItem<QString> cwTxMessage() const;
    MinosStringItem<QString> logMode() const;
    MinosItem<bool> ritOnOffStatus() const;
    MinosItem<bool> ritRadioStatus() const;
    MinosItem<ShortFreq> radioRitFreq() const;
    MinosItem<ShortFreq> logRitFreq() const;
    MinosItem<int> radioVolLevel() const;
    MinosItem<int> logVolLevel() const;
    MinosItem<bool> pttState() const;
    MinosItem<bool> getPttOnOff() const;


    void setSelected(const QString &loggeruuid, const QString &selected);
    void setRadioFreq(Frequency freq);
    void setLogFreq(Frequency freq);
    void setRadioMode(const QString &mode);
    void setLogMode(const QString &mode);
    void setVoiceMessageNum(const QString msgNum);
    void setStopVoiceMessage(const QString msg);
    void setCwTxMessage(const QString msg);
    void setRadioVolume(int level);
    void setLogVolume(int level);
    void setStatus(const QString &status);
    void setRadioRitFreq(const ShortFreq &freq);
    void setLogRitFreq(const ShortFreq &freq);
    void setRitOnOffStatus(const bool status);
    void setRitRadioStatus(const bool status);
    void setPttState(const bool state);
    void setPttOnOff(const bool on);


    QStringList getSelectedLoggers();
    Frequency getRadioFreq();
    void setLogBand(QString band);


};

#endif // RIGSTATE_H
