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


#include <QComboBox>
#include "voicekeyerfactory.h"
#include "rigcontrolvoicekeyer.h"

VoiceKeyerFactory::VoiceKeyerFactory(QObject *parent) : QObject(parent)
{
    RigControlVoiceKeyer::registerVoiceKeyer(&vmKeyersList);
}


VoiceKeyerFactory::~VoiceKeyerFactory()
{

}

VoiceKeyerFactory::VmKeyers* VoiceKeyerFactory::supportedVoiceKeyers()
{
    return &vmKeyersList;
}


VoiceKeyerBase* VoiceKeyerFactory::createVoiceKeyer(int vmKeyerId)
{
    if (vmKeyerId == VoiceKeyerId::RigControl)
    {
        return new RigControlVoiceKeyer(this);
    }

    return nullptr;
}

void VoiceKeyerFactory::populateComboRigList(QComboBox* comBox)
{

    comBox->clear();
    comBox->addItem("");
    for (auto r = supportedVoiceKeyers()->cbegin(); r != supportedVoiceKeyers()->cend(); ++r)
    {
        QString vmText = r.key();
        comBox->addItem(vmText);
    }
}
