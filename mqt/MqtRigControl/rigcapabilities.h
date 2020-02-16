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
                    bool pollData_= RigCapConstants::pollDataOn);


    int modelNumber;
    RigCapConstants::PortType portType;
    QString rigManufacturer;
    QString rigModelName;
    bool supportGetRit;
    bool supportSetRit;
    bool supportRitOnOff;
    bool supportGetRitState;
    bool supportSmeter;
    bool supportPtt;
    bool supportVolume;
    bool supportAntennaSw;
    bool pollData;




};

#endif // RIGCAPABILITIES_H
