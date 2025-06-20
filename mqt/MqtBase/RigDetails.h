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

#include "CacheSelection.h"
#include "minositem.h"


class RigDetails
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
    MinosItem<bool> _pttEnabled;
    MinosItem<int> _pttType;
    MinosItem<bool> _voiceMemAvail;
    MinosItem<int> _numVoiceMemMessages;
    MinosItem<int> _cwMemType;
    MinosItem<bool> _rigVoiceKeyerMessageSupportStop;
    MinosItem<bool> _rigCwKeyerMessageSupportStop;
    MinosItem<QString> _rigModel;
    MinosItem<int> _rttyOffset;
    MinosItem<int> _pskOffset;


public:
    RigDetails();
    RigDetails(QString s);
    ~RigDetails()
    {}

    bool isDirty() const;
    void clearDirty();
    void setDirty();

    QString pack() const ;
    void unpack(QString) ;

    MinosStringItem<QString> getSelectedContest(QString loggerUuid) const;
    MinosItem<double> transverterOffset() const;
    MinosItem<int> transverterSwitch() const;
    MinosItem<bool> transverterEnabled() const;
    MinosItem<bool> transverterStatus() const;
    MinosItem<bool> volumeStatus() const;
    MinosItem<bool> ritEnableStatus() const;
    MinosItem<int> ritMaxKHzFreq() const;
    MinosStringItem<QString> bandList() const;
    MinosItem<bool> pttEnabled() const;
    MinosItem<int> pttType() const;
    MinosItem<bool> voiceMemAvail() const;
    MinosItem<int> numVoiceMessages() const;
    MinosItem<int> cwMemType() const;
    MinosItem<bool> rigVoiceKeyerMessageSupportStop() const;
    MinosItem<bool> rigCwKeyerMessageSupportStop() const;
    MinosItem<QString> rigModel() const;
    MinosItem<int> getRttyOffset() const;
    MinosItem<int> getPskOffset() const;


    void setSelected(const QString &loggeruuid, const QString &selected);
    void setTransverterOffset(double transverterOffset);
    void setTransverterSwitch(int transverterSwitch);
    void setTransverterEnabled(bool transverterEnabled);
    void setTransverterStatus(bool transverterStatus);
    void setVolumeStatus(bool volumeStatus);
    void setBandList(const QString &bandList);
    void setRitEnableStatus(bool ritEnableStatus);
    void setRitMaxKHzFreq(int ritMaxKHz);
    void setPttEnabled(bool pttEnabled);
    void setPttType(int pttType);
    void setVoiceMemAvail(bool voiceMemAvail);
    void setNumVoiceMessages(int numMessages);
    void setCwMemType(int cwMemType);
    void setRigVoiceKeyerMessageSupportStop(bool supportStop);
    void setRigCwKeyerMessageSupportStop(bool supportStop);
    void setRigModel(QString);

    void setRttyOffset(int f);
    void setPskOffset(int f);
};


#endif // RIGDETAILS_H
