/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"
#include "rotatorcommon.h"

RotPresetData::RotPresetData(int _number, QString _name, QString _bearing)
{
    number = _number;
    name = _name;
    bearing = _bearing;
}


RotPresetData::RotPresetData()
{

}


