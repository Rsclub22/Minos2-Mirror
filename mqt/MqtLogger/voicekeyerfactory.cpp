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



#include "voicekeyerfactory.h"

VoiceKeyerFactory::VoiceKeyerFactory(QObject *parent) : QObject(parent)
{

}


VoiceKeyerFactory::~VoiceKeyerFactory()
{

}


VoiceKeyerBase* VoiceKeyerFactory::createVoiceKeyer(int vmKeyerId)
{

}

void VoiceKeyerFactory::populateComboRigList(QComboBox* comBox)
{

}
