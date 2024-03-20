#ifndef RADIODETAILS_H
#define RADIODETAILS_H

#include <QStringList>

class RadioDetails
{
public:
    RadioDetails();
    void setTransVerterOffset(double offset);
    double getTransVerterOffset();
    void setTransVertSwitch(int switchNum);
    int getTransVertSwitch();
    void setTransVertEnabled(bool status);
    bool getTransVertEnabled();
    void setTransVertStatus(bool status);
    bool getTransVertStatus();
    void setVolumeStatus(bool status);
    bool getVolumeStatus();
    void setRitEnableStatus(bool status);
    bool getRitEnableStatus();
    void setBandList(QString bandList);
    void setPttEnabled(bool state);
    bool getPttEnabled();
    void setPttType(int type);
    int getPttType();
    QString getBandList();
    int getBandListCount();



    void setRitMaxKHzFreq(int maxRitFreq_);
    int getRitMaxKHzFreq();


    void setVoiceMemAvail(bool avail);
    bool getVoiceMemAvail();
    void setNumVoiceMessages(int numMsgs);
    int getNumVoiceMessages();
    void setCwMemType(int cwMemType);
    void setRigKeyerSupportStopCmd(bool supportStopCmd);
    bool getRigKeyerSupportStopCmd();
    int getCwMemType();
private:

double transVerterOffset;
int transVerterSwitch;
bool transVertEnabled;
bool transVertStatus;
bool volumeStatus;
bool ritEnableStatus;
int maxRitFreq;
bool pttEnabled;
int pttType;
bool voiceMemAvail;
int numVoiceMessages;
int cwMemType;
bool rigKeyerSupportStopCmd;
QString bandList;

};

#endif // RADIODETAILS_H
