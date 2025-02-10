/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2026
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#include <hamlib/rig.h>
#include "rigcapabilities.h"


RigCapabilities::RigCapabilities()
{
    clear();
}


RigCapabilities::~RigCapabilities()
{

}


RigCapabilities:: RigCapabilities( const RigCapabilities &rigcap)
{
    *this = rigcap;
}





RigCapabilities & RigCapabilities::operator= ( const RigCapabilities &rigcap)
{
    portType = rigcap.portType;
    rigManufacturer = rigcap.rigManufacturer;
    rigModelName = rigcap.rigModelName;
    rigModelNumber = rigcap.rigModelNumber;
    libraryName = rigcap.libraryName;
    libraryVersion = rigcap.libraryVersion;
    detailedLibraryVersion = rigcap.detailedLibraryVersion;
    supportGetSupBands = rigcap.supportGetSupBands;
    supportGetVfo = rigcap.supportGetVfo;
    supportSetVfo = rigcap.supportSetVfo;
    supportGetRit = rigcap.supportGetRit;
    supportSetRit = rigcap.supportSetRit;
    supportGetRitState = rigcap.supportGetRitState;
    supportSetRitState = rigcap.supportSetRitState;
    supportGetRitMax = rigcap.supportGetRitMax;
    supportSMeter = rigcap.supportSMeter;
    supportPttPortType = rigcap.supportPttPortType;
    supportGetPtt = rigcap.supportGetPtt;
    supportSetPtt = rigcap.supportSetPtt;
    supportGetVox = rigcap.supportGetVox;
    supportSetVox = rigcap.supportSetVox;
    supportVolume = rigcap.supportVolume;
    supportAntSw = rigcap.supportAntSw;
    supportRigCtld = rigcap.supportRigCtld;
    supportVoiceMemory = rigcap.supportVoiceMemory;
    startVoiceMemoryNumber = rigcap.startVoiceMemoryNumber;
    endVoiceMemoryNumber = rigcap.endVoiceMemoryNumber;
    supportStopVoiceMemory = rigcap.supportStopVoiceMemory;
    supportCwMemory = rigcap.supportCwMemory;
    startCwMemoryNumber = rigcap.startCwMemoryNumber;
    endCwMemoryNumber = rigcap.endCwMemoryNumber;
    supportCwMemoryStop = rigcap.supportCwMemoryStop;
    supportCwMemoryWait = rigcap.supportCwMemoryWait;
    pollData = rigcap.pollData;
    return *this;
}



void RigCapabilities::clear()
{

    portType = RigCapConstants::none;
    rigManufacturer = "";
    rigName = "";
    rigModelName = "";
    rigModelNumber = 0;
    libraryName = "";
    libraryVersion = "";
    detailedLibraryVersion = "";
    supportGetSupBands = false;
    supportGetVfo = false;
    supportSetVfo = false;
    supportGetRit = false;
    supportSetRit = false;
    supportGetRitState = false;
    supportSetRitState = false;
    supportGetRitMax = false;
    supportSMeter = false;
    supportPttPortType = RigCapConstants::RigPttPortType::RIG_PTT_NONE;
    supportGetPtt = false;
    supportSetPtt = false;
    supportGetVox = false;
    supportSetVox = false;
    supportVolume = false;
    supportAntSw = false;
    supportRigCtld = false;
    supportVoiceMemory = false;
    startVoiceMemoryNumber = 0;
    endVoiceMemoryNumber = 0;
    supportStopVoiceMemory = false;
    supportCwMemory = false;
    startCwMemoryNumber = 0;
    endCwMemoryNumber = 0;
    supportCwMemoryStop = false;
    supportCwMemoryWait = false;
    pollData= RigCapConstants::pollDataOn;
}


void RigCapabilities::setPortType(RigCapConstants::PortType portType_)
{
    portType = portType_;
}
RigCapConstants::PortType RigCapabilities::getPortType() const
{
    return portType;
}

void RigCapabilities::setRigManufacturer(QString rigManufacturer_)
{
    rigManufacturer = rigManufacturer_;
}
QString RigCapabilities::getRigManufacturer() const
{
    return rigManufacturer;
}

void RigCapabilities::setRigName(QString rigName_)
{
   rigName = rigName_;
}
QString RigCapabilities::getRigName() const
{
    return rigName;
}

void RigCapabilities::setRigModelName(QString rigModelName_)
{
    rigModelName = rigModelName_;
}
QString RigCapabilities::getRigModelName() const
{
    return rigModelName;
}

void RigCapabilities::setRigModelNumber(int rigModelNumber_)
{
    rigModelNumber = rigModelNumber_;
}
int RigCapabilities::getRigModelNumber() const
{
    return rigModelNumber;
}

void RigCapabilities::setLibraryName(QString libraryName_)
{
    libraryName = libraryName_;
}
QString RigCapabilities::getLibraryName()
{
    return libraryName;
}

void RigCapabilities::setLibraryVersion(QString libraryVersion_)
{
    libraryVersion = libraryVersion_;
}
QString RigCapabilities::getLibraryVersion()
{
    return libraryVersion;
}
void RigCapabilities::setDetailedLibraryVersion(QString detailedVersion_)
{
    detailedLibraryVersion = detailedVersion_;
}
QString RigCapabilities::getDetailedLibraryVersion()
{
    return detailedLibraryVersion;
}
void RigCapabilities::setSupportGetSupBands(bool supportGetSupBands_)
{
    supportGetSupBands = supportGetSupBands_;
}
bool RigCapabilities::getSupportGetSupBands() const
{
    return supportGetSupBands;
}

void RigCapabilities::setSupportGetVfo(bool supportGetVfo_)
{
    supportGetVfo = supportGetVfo_;
}
bool RigCapabilities::getSupportGetVfo() const
{
    return supportGetVfo;
}

void RigCapabilities::setSupportSetVfo(bool supportSetVfo_)
{
    supportSetVfo = supportSetVfo_;
}
bool RigCapabilities::getSupportSetVfo() const
{
    return supportSetVfo;
}

void RigCapabilities::setSupportGetRit(bool supportGetRit_)
{
   supportGetRit = supportGetRit_;
}
bool RigCapabilities::getSupportGetRit() const
{
    return supportGetRit;
}

void RigCapabilities::setSupportSetRit(bool supportSetRit_)
{
    supportSetRit = supportSetRit_;
}
bool RigCapabilities::getSupportSetRit() const
{
    return supportSetRit;
}

void RigCapabilities::setSupportGetRitState(bool supportGetRitState_)
{
    supportGetRitState = supportGetRitState_;
}
bool RigCapabilities::getSupportGetRitState() const
{
    return supportGetRitState;
}

void RigCapabilities::setSupportSetRitState(bool supportSetRitState_)
{
   supportSetRitState = supportSetRitState_;
}
bool RigCapabilities::getSupportSetRitState() const
{
    return supportSetRitState;
}

void RigCapabilities::setSupportGetRitMax(bool supportGetRitMax_)
{
   supportGetRitMax = supportGetRitMax_;
}
bool RigCapabilities::getSupportGetRitMax() const
{
    return supportGetRitMax;
}

void RigCapabilities::setSupportSMeter(bool supportSMeter_)
{
    supportSMeter = supportSMeter_;
}
bool RigCapabilities::getSupportSMeter() const
{
    return supportSMeter;
}

void RigCapabilities::setSupportPttPortType(RigCapConstants::RigPttPortType supportPttPortType_)
{
    supportPttPortType = supportPttPortType_;
}
RigCapConstants::RigPttPortType  RigCapabilities::getSupportPttPortType()
{
    return supportPttPortType;
}

void RigCapabilities::setSupportGetPtt(bool supportGetPtt_)
{
    supportGetPtt = supportGetPtt_;
}
bool RigCapabilities::getSupportGetPtt() const
{
    return supportGetPtt;
}

void RigCapabilities::setSupportSetPtt(bool supportSetPtt_)
{
    supportSetPtt = supportSetPtt_;
}
bool RigCapabilities::getSupportSetPtt() const
{
    return supportSetPtt;
}

void RigCapabilities::setSupportGetVox(bool supportGetVox_)
{
    supportGetVox = supportGetVox_;
}
bool RigCapabilities::getSupportGetVox() const
{
    return supportGetVox;
}

void RigCapabilities::setSupportSetVox(bool supportSetVox_)
{
    supportSetVox = supportSetVox_;
}
bool RigCapabilities::getSupportSetVox() const
{
    return supportSetVox;
}

void RigCapabilities::setSupportVolume(bool supportVolume_)
{
   supportVolume = supportVolume_;
}
bool RigCapabilities::getSupportVolume() const
{
    return supportVolume;
}

void RigCapabilities::setSupportAntSw(bool supportAntSw_)
{
    supportAntSw = supportAntSw_;
}
bool RigCapabilities::getSupportAntSw() const
{
    return supportAntSw;
}

void RigCapabilities::setSupportRigCtld(bool supportRigCtld_)
{
    supportRigCtld = supportRigCtld_;
}
bool RigCapabilities::getSupportRigCtld() const
{
    return supportRigCtld;
}

void RigCapabilities::setSupportVoiceMemory(bool supportVoiceMemory_)
{
    supportVoiceMemory = supportVoiceMemory_;
}
bool RigCapabilities::getSupportVoiceMemory() const
{
    return supportVoiceMemory;
}

void RigCapabilities::setStartVoiceMemoryNumber(int startVoiceMemoryNumber_)
{
    startVoiceMemoryNumber = startVoiceMemoryNumber_;
}
int RigCapabilities::getStartVoiceMemoryNumber() const
{
    return startVoiceMemoryNumber;
}

void RigCapabilities::setEndVoiceMemoryNumber(int endVoiceMemoryNumber_)
{
    endVoiceMemoryNumber = endVoiceMemoryNumber_;
}
int RigCapabilities::getEndVoiceMemoryNumber() const
{
    return endVoiceMemoryNumber;
}

void RigCapabilities::setSupportStopVoiceMemory(bool supportStopVoiceMemory_)
{
    supportStopVoiceMemory = supportStopVoiceMemory_;
}
bool RigCapabilities::getSupportStopVoiceMemory() const
{
    return supportStopVoiceMemory;
}

void RigCapabilities::setSupportCwMemory(bool supportCwMemory_)
{
    supportCwMemory = supportCwMemory_;
}
bool RigCapabilities::getSupportCwMemory() const
{
    return supportCwMemory;
}

void RigCapabilities::setStartCwMemoryNumber(int startCwMemoryNumber_)
{
    startCwMemoryNumber = startCwMemoryNumber_;
}
int RigCapabilities::getStartCwMemoryNumber() const
{
    return startCwMemoryNumber;
}

void RigCapabilities::setEndCwMemoryNumber(int endCwMemoryNumber_)
{
     endCwMemoryNumber =  endCwMemoryNumber_;
}
int RigCapabilities::getEndCwMemoryNumber() const
{
    return  endCwMemoryNumber;
}

void RigCapabilities::setSupportCwMemoryStop(bool supportCwMemoryStop_)
{
    supportCwMemoryStop = supportCwMemoryStop_;
}
bool RigCapabilities::getSupportCwMemoryStop() const
{
    return supportCwMemoryStop;
}

void RigCapabilities::setSupportCwMemoryWait(bool supportCwMemoryWait_)
{
    supportCwMemoryWait = supportCwMemoryWait_;
}
bool RigCapabilities::getSupportCwMemoryWait() const
{
    return supportCwMemoryWait;
}

void RigCapabilities::setPollData(bool pollData_)
{
   pollData = pollData_;
}
bool RigCapabilities::getPollData() const
{
    return pollData;
}



