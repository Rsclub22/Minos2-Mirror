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
    void setTransVertStatus(bool status);
    bool getTransVertStatus();
    void setVolumeStatus(bool status);
    bool getVolumeStatus();
    void setRitEnableStatus(bool status);
    bool getRitEnableStatus();
    void setBandList(QString bandList);
    QString getBandList();


private:

double transVerterOffset;
int transVerterSwitch;
bool transVertStatus;
bool volumeStatus;
bool ritEnableStatus;
QString bandList;

};

#endif // RADIODETAILS_H
