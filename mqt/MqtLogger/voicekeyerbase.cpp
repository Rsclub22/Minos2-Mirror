/////////////////////////////////////////////////////////////////////////////
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
/////////////////////////////////////////////////////////////////////////////

#include "voicekeyerbase.h"

VoiceKeyerParams::VoiceKeyerParams()
{
    clear();
}

void VoiceKeyerParams::clear()
{
    type.clear();
    vmName.clear();
    vmRepeatFlag = false;
    vmDuration = 0;
    vmRepeatPauseDur = 0;
    vmButtonNum = -1;   // None
}

VoiceKeyerParams& VoiceKeyerParams::operator = (const VoiceKeyerParams& vkp)
{
    type = vkp.type;
    vmName = vkp.vmName;
    
    vmRepeatFlag = vkp.vmRepeatFlag;
    vmDuration = vkp.vmDuration;
    vmRepeatPauseDur = vkp.vmRepeatPauseDur;
    vmButtonNum = vkp.vmButtonNum;
    return *this;
}


VoiceKeyerBase::VoiceKeyerBase(QObject *parent) : QObject(parent)
{

}

VoiceKeyerBase::~VoiceKeyerBase()
{

}
