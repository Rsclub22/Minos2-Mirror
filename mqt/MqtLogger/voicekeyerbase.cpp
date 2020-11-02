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


#include "voicekeyerbase.h"


VoiceKeyerCommonParams::VoiceKeyerCommonParams()
{
    clear();
}

VoiceKeyerCommonParams::~VoiceKeyerCommonParams()
{

}

void VoiceKeyerCommonParams::operator = (const VoiceKeyerCommonParams& vkcp)
{
    numButtons = vkcp.numButtons;
    comport = vkcp.comport;
}

bool VoiceKeyerCommonParams::operator == (const VoiceKeyerCommonParams& vkcp)
{
    if (numButtons == vkcp.numButtons &&
            comport == vkcp.comport)
    {
        return true;
    }

    return false;
}

bool VoiceKeyerCommonParams::operator != (const VoiceKeyerCommonParams& vkcp)
{
    if (numButtons != vkcp.numButtons ||
            comport != vkcp.comport)
    {
        return true;
    }

    return false;
}
void VoiceKeyerCommonParams::clear()
{
    comport.clear();
}



VoiceKeyerParams::VoiceKeyerParams()
{
    clear();
}

void VoiceKeyerParams::clear()
{
    type.clear();
    vmName.clear();
    vmRepeatFlag = false;
    vmRepeatDur = 0;
    vmButtonNum = -1;   // None
}


void VoiceKeyerParams::operator = (const VoiceKeyerParams& vkp)
{
    type = vkp.type;
    vmName = vkp.vmName;
    vmRepeatFlag = vkp.vmRepeatFlag;
    vmRepeatDur = vkp.vmRepeatDur;
    vmButtonNum = vkp.vmButtonNum;
}


VoiceKeyerBase::VoiceKeyerBase(QObject *parent) : QObject(parent)
{

}
