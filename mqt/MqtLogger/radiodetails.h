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
    QString getBandList();
    int getBandListCount();



    void setRitMaxKHzFreq(int maxRitFreq_);
    int getRitMaxKHzFreq();

private:

double transVerterOffset;
int transVerterSwitch;
bool transVertEnabled;
bool transVertStatus;
bool volumeStatus;
bool ritEnableStatus;
int maxRitFreq;
QString bandList;

};

#endif // RADIODETAILS_H
