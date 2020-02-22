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

#include "rigfactory.h"
#include "hamlibrigcontrol.h"

RigFactory::RigFactory(bool tracecommFlag, QObject *parent) : QObject(parent)
{
    HamlibRigControl::setTraceCommsFlag(tracecommFlag);
    HamlibRigControl::register_rigs(&rigsList);
}


RigFactory::~RigFactory()
{

}

RigFactory::Rigs* RigFactory::supported_rigs()
{
    return &rigsList;
}

RigBase* RigFactory::createRigs(int rigId)
{
    if (rigId == OmniRigOneId)
    {
    }
    else if (rigId == OmniRigTwoId)
    {
    }

    return new HamlibRigControl();
}


bool RigFactory::checkForBands(int rigNumber, Frequency freq)
{
    if (rigNumber == OmniRigOneId)
    {

    }
    else if (rigNumber == OmniRigTwoId)
    {

    }

    return HamlibRigControl::checkFreqRange(rigNumber, freq);
}


void RigFactory::populateComboRigList(QComboBox* comBox )
{
    comBox->clear();
    comBox->addItem("");
    for (auto r = supported_rigs()->cbegin(); r != supported_rigs()->cend(); ++r)
    {
        QString rigText = r.key();
        comBox->addItem(rigText);
    }
}
