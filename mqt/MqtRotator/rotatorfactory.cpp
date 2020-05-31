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


#if defined (WIN32)
#include "pstRotControl.h"
#endif

RotatorFactory::RotatorFactory(bool tracecommFlag, QObject *parent) : QObject(parent)
{

    HamlibRotControl::setTraceCommsFlag(tracecommFlag);
    HamlibRotControl::register_rotators(&rotatorsList);


#if defined (WIN32)
    // PSTRotator is only on Windows
    PstRotControl::register_rotators(&rotatorsList, RotId::PSTRotatorId);
#endif

}

RotatorFactory::~RotatorFactory()
{

}


RotatorFactory::Rotators* RotatorFactory::supported_rotators()
{
    return &rotatorsList;
}


RotatorBase* RotatorFactory::createRotator(int rotatorId)
{
#if defined (WIN32)

    if (rotatorId == PSTRotatorId)
    {
        return new PstRotControl();
    }
    else
#endif
    {
        return new HamlibRotControl();
    }
}
