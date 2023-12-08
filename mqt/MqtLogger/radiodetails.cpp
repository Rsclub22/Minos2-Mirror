#include "radiodetails.h"
#include "rigcommon.h"

RadioDetails::RadioDetails()
{

  setTransVerterOffset(0.0);
  setTransVertSwitch(0);
  setTransVertStatus(false);
  setVolumeStatus(false);
  setRitEnableStatus(false);
  setRitMaxKHzFreq(MAX_RITFREQ);
  setCwMemType(hamlibData::CW_MEMORY_TYPES::NONE);


}


void RadioDetails::setTransVerterOffset(double offset)
{
    transVerterOffset = offset;
}

double RadioDetails::getTransVerterOffset()
{
    return transVerterOffset;
}


void RadioDetails::setTransVertSwitch(int switchNum)
{
    transVerterSwitch = switchNum;
}
int RadioDetails::getTransVertSwitch()
{
    return transVerterSwitch;
}
void RadioDetails::setTransVertEnabled(bool status)
{
    transVertEnabled = status;
}
bool RadioDetails::getTransVertEnabled()
{
    return transVertEnabled;
}
void RadioDetails::setTransVertStatus(bool status)
{
    transVertStatus = status;
}
bool RadioDetails::getTransVertStatus()
{
    return transVertStatus;
}
void RadioDetails::setVolumeStatus(bool status)
{
    volumeStatus = status;
}
bool RadioDetails::getVolumeStatus()
{
    return  volumeStatus;
}
void RadioDetails::setRitEnableStatus(bool status)
{
    ritEnableStatus = status;
}
bool RadioDetails::getRitEnableStatus()
{
    return ritEnableStatus;
}

void RadioDetails::setRitMaxKHzFreq(int maxRitFreq_)
{
    maxRitFreq = maxRitFreq_;
}

int RadioDetails::getRitMaxKHzFreq()
{
    return maxRitFreq;
}
void RadioDetails::setBandList(QString _bandList)
{
    bandList = _bandList;
}
QString RadioDetails::getBandList()
{
    return bandList;
}
int RadioDetails::getBandListCount()
{
    return bandList.size();
}
void RadioDetails::setPttType(int type)
{
    pttType = type;
}
int RadioDetails::getPttType()
{
    return pttType;
}
void RadioDetails::setPttEnabled(bool state)
{
    pttEnabled = state;
}
bool RadioDetails::getPttEnabled()
{
    return pttEnabled;
}
void RadioDetails::setVoiceMemAvail(bool avail)
{
    voiceMemAvail = avail;
}
bool RadioDetails::getVoiceMemAvail()
{
    return voiceMemAvail;
}
void RadioDetails::setNumVoiceMessages(int numMsgs)
{
    numVoiceMessages = numMsgs;
}
int RadioDetails::getNumVoiceMessages()
{
    return numVoiceMessages;
}
void RadioDetails::setCwMemType(int _cwMemType)
{
    cwMemType = _cwMemType;
}
int RadioDetails::getCwMemType()
{
    return cwMemType;
}
void RadioDetails::setNumCwMessages(int numMsgs)
{
    numCwMessages = numMsgs;
}
int RadioDetails::getNumCwMessages()
{
    return numCwMessages;
}



