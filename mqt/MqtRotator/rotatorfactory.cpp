/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "rotatorfactory.h"
#include "hamlibRotcontrol.h"
#include "pstRotControl.h"

#if defined (WIN32)
#include "pstRotControl.h"
#endif

RotatorFactory::RotatorFactory(QObject *parent) : QObject(parent)
{
    HamlibRotControl::register_rotators(this);

#if defined (WIN32)
    // PSTRotator is only on Windows
    PstRotControl::register_rotators(this, RotId::PSTRotatorId);
#endif

}

RotatorFactory::~RotatorFactory()
{

}


Rotators* RotatorFactory::supported_rotators()
{
    return &rotatorsList;
}


RotatorBase* RotatorFactory::createRotator(int rotatorId)
{
    if (rotatorId == PSTRotatorId)
    {
        return new PstRotControl();
    }
    else
    {
        return new HamlibRotControl();
    }
}
