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



#ifndef RIGCAPABILITIES_H
#define RIGCAPABILITIES_H

#include <QString>

namespace RigCapConstants
{
    enum PortType {none, serial, network, usb};


    // These are the PTT capabilities from hamlib. Most radios are RIG_PTT_NONE, RIG_PTT_RIG and RIG_PTT_RIG_MICDATA
    // we will use RIG_PTT_NONE to disable CAT PTT and allow Hardware PTT
    enum PttPortType {RIG_PTT_NONE = 0,       /*!< No PTT available */
                      RIG_PTT_RIG,            /*!< Legacy PTT (CAT PTT) */
                      RIG_PTT_SERIAL_DTR,     /*!< PTT control through serial DTR signal */
                      RIG_PTT_SERIAL_RTS,     /*!< PTT control through serial RTS signal */
                      RIG_PTT_PARALLEL,       /*!< PTT control through parallel port */
                      RIG_PTT_RIG_MICDATA,    /*!< Legacy PTT (CAT PTT), supports RIG_PTT_ON_MIC/RIG_PTT_ON_DATA */
                      RIG_PTT_CM108,          /*!< PTT control through CM108 GPIO pin */
                      RIG_PTT_GPIO,           /*!< PTT control through GPIO pin */
                      RIG_PTT_GPION          /*!< PTT control through inverted GPIO pin */
                     };

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
                    bool supportGetVfo = false,
                    bool supportSetVfo = false,
                    bool supportGetRit_ = false,
                    bool supportSetRit_ = false,
                    bool supportGetRitState = false,
                    bool supportSetRitState = false,
                    bool supportGetRitMax = false,
                    bool supportSMeter_ = false,
                    RigCapConstants::PttPortType supportPttPortType_ = RigCapConstants::PttPortType::RIG_PTT_NONE,
                    bool supportGetPtt_ = false,
                    bool supportSetPtt_ = false,
                    bool supportGetVox_ = false,
                    bool supportSetVox_ = false,
                    bool supportVolume_ = false,
                    bool supportAntSw_ = false,
                    bool supportRigCtld_ = false,
                    bool supportVoiceMemory_ = false,
                    int startVoiceMemoryNumber_ = 0,
                    int endVoiceMemoryNumber_ = 0,
                    bool supportStopVoiceMemory_ = false,
                    bool supportCwMemory_ = false,
                    int startCwMemoryNumber_ = 0,
                    int endCwMemoryNumber_ = 0,
                    bool supportCwMemoryStop_ = false,
                    bool supportCwMemoryWait_ = false,
                    bool pollData_= RigCapConstants::pollDataOn);



    RigCapConstants::PortType portType;
    QString rigManufacturer;
    QString rigName;
    QString rigModelName;       // combined manufacturer and rig names
    int rigModelNumber;
    bool supportGetSupBands;
    bool supportGetVfo;
    bool supportSetVfo;
    bool supportGetRit;
    bool supportSetRit;
    bool supportGetRitState;
    bool supportSetRitState;
    bool supportGetRitMax;
    bool supportSMeter;
    RigCapConstants::PttPortType supportPttPortType = RigCapConstants::PttPortType::RIG_PTT_NONE;
    bool supportGetPtt;
    bool supportSetPtt;
    bool supportGetVox;
    bool supportSetVox;
    bool supportVolume;
    bool supportAntSw;
    bool supportRigCtld;
    bool supportVoiceMemory;
    int startVoiceMemoryNumber;
    int endVoiceMemoryNumber;
    bool supportStopVoiceMemory;
    bool supportCwMemory;
    int startCwMemoryNumber;
    int endCwMemoryNumber;
    bool supportCwMemoryStop;
    bool supportCwMemoryWait;
    bool pollData;



  RigCapabilities( const RigCapabilities &rigcap);
  RigCapabilities &operator= ( const RigCapabilities &rigcap);


};

#endif // RIGCAPABILITIES_H
