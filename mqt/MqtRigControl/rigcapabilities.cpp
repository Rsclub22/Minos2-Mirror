/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2023
//
//
//
//
/////////////////////////////////////////////////////////////////////////////




#include "rigcapabilities.h"
#include <hamlib/rig.h>

RigCapabilities::RigCapabilities(RigCapConstants::PortType portType_,
                                 QString rigManufacturer_,
                                 QString rigName_,
                                 QString rigModelName_,
                                 int rigModelNumber_,
                                 bool supportGetSupBands_,
                                 bool supportGetVfo_,
                                 bool supportSetVfo_,
                                 bool supportGetRit_,
                                 bool supportSetRit_,
                                 bool supportGetRitState_,
                                 bool supportSetRitState_,
                                 bool supportGetRitMax_,
                                 bool supportSMeter_,
                                 RigCapConstants::PttPortType supportPttPortType_,
                                 bool supportGetPtt_,
                                 bool supportSetPtt_,
                                 bool supportGetVox_,
                                 bool supportSetVox_,
                                 bool supportVolume_,
                                 bool supportAntSw_,
                                 bool supportRigCtld_,
                                 bool supportVoiceMemory_,
                                 int startVoiceMemoryNumber_,
                                 int endVoiceMemoryNumber_,
                                 bool supportStopVoiceMemory_,
                                 bool supportCwMemory_,
                                 int startCwMemoryNumber_,
                                 int endCwMemoryNumber_,
                                 bool supportCwMemoryStop_,
                                 bool supportCwMemoryWait_,
                                 bool pollData_ )
    : portType (portType_),
      rigManufacturer (rigManufacturer_),
      rigName (rigName_),
      rigModelName (rigModelName_),
      rigModelNumber (rigModelNumber_),
      supportGetSupBands (supportGetSupBands_),
      supportGetVfo(supportGetVfo_),
      supportSetVfo(supportSetVfo_),
      supportGetRit (supportGetRit_),
      supportSetRit (supportSetRit_),
      supportGetRitState (supportGetRitState_),
      supportSetRitState (supportSetRitState_),
      supportGetRitMax (supportGetRitMax_),
      supportSMeter (supportSMeter_),
      supportPttPortType(supportPttPortType_),
      supportGetPtt (supportGetPtt_),
      supportSetPtt (supportSetPtt_),
      supportGetVox(supportGetVox_),
      supportSetVox(supportSetVox_),
      supportVolume (supportVolume_),
      supportAntSw (supportAntSw_),
      supportRigCtld (supportRigCtld_),
      supportVoiceMemory(supportVoiceMemory_),
      startVoiceMemoryNumber(startVoiceMemoryNumber_),
      endVoiceMemoryNumber(endVoiceMemoryNumber_),
      supportStopVoiceMemory (supportStopVoiceMemory_),
      supportCwMemory(supportCwMemory_),
      startCwMemoryNumber(startCwMemoryNumber_),
      endCwMemoryNumber(endCwMemoryNumber_),
      supportCwMemoryStop(supportCwMemoryStop_),
      supportCwMemoryWait(supportCwMemoryWait_),
      pollData (pollData_)
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



