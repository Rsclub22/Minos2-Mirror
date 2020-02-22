/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////




#include "rigcapabilities.h"

RigCapabilities::RigCapabilities(int modelNumber_,
                                 RigCapConstants::PortType portType_,
                                 QString rigManufacturer_,
                                 QString rigName_,
                                 QString rigModelName_,
                                 bool supportGetRit_,
                                 bool supportSetRit_,
                                 bool supportGetRitState_,
                                 bool supportSetRitState_,
                                 bool supportSMeter_,
                                 bool supportGetPtt_,
                                 bool supportSetPtt_,
                                 bool supportVolume_,
                                 bool supportAntSw_,
                                 bool supportRigCtld_,
                                 bool pollData_ )
    : modelNumber (modelNumber_),
      portType (portType_),
      rigManufacturer (rigManufacturer_),
      rigName (rigName_),
      rigModelName (rigModelName_),
      supportGetRit (supportGetRit_),
      supportSetRit (supportSetRit_),
      supportGetRitState (supportGetRitState_),
      supportSetRitState (supportSetRitState_),
      supportSMeter (supportSMeter_),
      supportGetPtt (supportGetPtt_),
      supportSetPtt (supportSetPtt_),
      supportVolume (supportVolume_),
      supportAntSw (supportAntSw_),
      supportRigCtld (supportRigCtld_),
      pollData (pollData_)
{

}


RigCapabilities:: RigCapabilities( const RigCapabilities &rigcap)
{
    modelNumber = rigcap.modelNumber;
    portType = rigcap.portType;
    rigManufacturer = rigcap.rigManufacturer;
    rigModelName = rigcap.rigModelName;
    rigModelNumber = rigcap.rigModelNumber;
    supportGetRit = rigcap.supportGetRit;
    supportSetRit = rigcap.supportSetRit;
    supportGetRitState = rigcap.supportGetRitState;
    supportSetRitState = rigcap.supportSetRitState;
    supportSMeter = rigcap.supportSMeter;
    supportGetPtt = rigcap.supportGetPtt;
    supportPtt = rigcap.supportPtt;
    supportSetPtt = rigcap.supportSetPtt;
    supportVolume = rigcap.supportVolume;
    supportAntSw = rigcap.supportAntSw;
    supportRigCtld = rigcap.supportRigCtld;
    pollData = rigcap.pollData;

}
