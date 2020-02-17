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



namespace RigCapConstants
{
    enum PortType {none, serial, network, usb};
    enum PollData {pollDataOn = true, pollDataOff = false};
}



class RigCapabilities
{
public:
    RigCapabilities(int modelNumber = 0,
                    RigCapConstants::PortType portType_ = RigCapConstants::none,
                    QString rigManufacturer_ = "",
                    QString rigModelName_ = "",
                    bool supportGetRit_ = false,
                    bool supportSetRit_ = false,
                    bool supportSMeter_ = false,
                    bool supportGetPtt_ = false,
                    bool supportSetPtt_ = false,
                    bool supportVolume_ = false,
                    bool supportAntSw_ = false,
                    bool supportRigCtld_ = false,
                    bool pollData_= RigCapConstants::pollDataOn);


    int modelNumber;
    RigCapConstants::PortType portType;
    QString rigManufacturer;
    QString rigModelName;
    int rigModelNumber;
    bool supportGetRit;
    bool supportSetRit;
    bool supportSMeter;
    bool supportGetPtt;
    bool supportPtt;
    bool supportSetPtt;
    bool supportVolume;
    bool supportAntSw;
    bool supportRigCtld;
    bool pollData;




};

#endif // RIGCAPABILITIES_H
