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
                                 QString rigModelName_,
                                 bool supportGetRit_,
                                 bool supportSetRit_,
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
      rigModelName (rigModelName_),
      supportGetRit (supportGetRit_),
      supportSetRit (supportSetRit_),
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
    supportSMeter = rigcap.supportSMeter;
    supportGetPtt = rigcap.supportGetPtt;
    supportPtt = rigcap.supportPtt;
    supportSetPtt = rigcap.supportSetPtt;
    supportVolume = rigcap.supportVolume;
    supportAntSw = rigcap.supportAntSw;
    supportRigCtld = rigcap.supportRigCtld;
    pollData = rigcap.pollData;

}
