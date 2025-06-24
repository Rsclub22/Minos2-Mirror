/////////////////////////////////////////////////////////////////////////////
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Tx Keyer Base
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2025
//
/////////////////////////////////////////////////////////////////////////////
#include "MTrace.h"
#include "txKeyerbase.h"



TxKeyerParams::TxKeyerParams()
{
    clear();
}

void TxKeyerParams::clear()
{
    type.clear();
    vmName.clear();
    vmCwMessage.clear();
    vmRepeatFlag = false;
    vmDuration = 0;
    vmRepeatPauseDur = 0;
    vmButtonNum = -1;   // None
    sAndPState = true;
    cwKeyerType = CW_KEYER_TYPE::KEYER_NONE;
}

TxKeyerParams& TxKeyerParams::operator = (const TxKeyerParams& vkp)
{
    type = vkp.type;
    selRadioName = vkp.selRadioName;
    vmName = vkp.vmName;

    vkBase = vkp.vkBase;
    

    vmRepeatFlag = vkp.vmRepeatFlag;
    vmDuration = vkp.vmDuration;
    vmCwMessage = vkp.vmCwMessage;
    vmRepeatPauseDur = vkp.vmRepeatPauseDur;
    vmButtonNum = vkp.vmButtonNum;
    sAndPState = vkp.sAndPState;
    cwKeyerType = vkp.cwKeyerType;
    return *this;
}

void TxKeyerParams::setVmDuration(const int vmDuration_)
{
    if (vmDuration_ > 0)
    {
        trace(QString("setVmDuration(%1)").arg(vmDuration_));
    }
    vmDuration = vmDuration_;
}


TxKeyerBase::TxKeyerBase(QObject *parent) : QObject(parent)
{

}

TxKeyerBase::~TxKeyerBase()
{

}




