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



#ifndef RIGCAPABILITIES_H
#define RIGCAPABILITIES_H

#include <QString>

namespace RigCapConstants
{
    enum PortType {none, serial, network, usb};
    enum PollData {pollDataOn = true, pollDataOff = false};
}



class RigCapabilities
{
public:
    RigCapabilities(RigCapConstants::PortType portType_ = RigCapConstants::none,
                    QString rigManufacturer_ = "",
                    QString rigName_ = "",
                    QString rigModelName_ = "",
                    int rigModelNumber_ = 0,
                    bool supportGetSupBands_ = false,
                    bool supportGetRit_ = false,
                    bool supportSetRit_ = false,
                    bool supportGetRitState = false,
                    bool supportSetRitState = false,
                    bool supportGetRitMax = false,
                    bool supportSMeter_ = false,
                    bool supportGetPtt_ = false,
                    bool supportSetPtt_ = false,
                    bool supportVolume_ = false,
                    bool supportAntSw_ = false,
                    bool supportRigCtld_ = false,
                    bool supportVoiceMemory_ = false,
                    bool supportCwMemory_ = false,
                    bool pollData_= RigCapConstants::pollDataOn);



    RigCapConstants::PortType portType;
    QString rigManufacturer;
    QString rigName;
    QString rigModelName;       // combined manufacturer and rig names
    int rigModelNumber;
    bool supportGetSupBands;
    bool supportGetRit;
    bool supportSetRit;
    bool supportGetRitState;
    bool supportSetRitState;
    bool supportGetRitMax;
    bool supportSMeter;
    bool supportGetPtt;
    bool supportPtt;
    bool supportSetPtt;
    bool supportVolume;
    bool supportAntSw;
    bool supportRigCtld;
    bool supportVoiceMemory;
    bool supportCwMemory;
    bool pollData;

  RigCapabilities( const RigCapabilities &rigcap);


};

#endif // RIGCAPABILITIES_H
