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
                                 bool pollData_ )
    : modelNumber (modelNumber_),
      portType (portType_),
      rigManufacturer (rigManufacturer_),
      rigModelName (rigModelName_),
      pollData (pollData_)
{

}
