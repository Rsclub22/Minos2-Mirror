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
    keyerName.clear();
    keyerCwMessage.clear();
    keyerRepeatFlag = false;
    keyerDuration = 0;
    keyerRepeatPauseDur = 0;
    keyerButtonNum = -1;   // None
    sAndPState = true;
    cwKeyerType = CW_KEYER_TYPE::KEYER_NONE;
}

TxKeyerParams& TxKeyerParams::operator = (const TxKeyerParams& vkp)
{
    type = vkp.type;
    selRadioName = vkp.selRadioName;
    keyerName = vkp.keyerName;

    vkBase = vkp.vkBase;
    

    keyerRepeatFlag = vkp.keyerRepeatFlag;
    keyerDuration = vkp.keyerDuration;
    keyerCwMessage = vkp.keyerCwMessage;
    keyerRepeatPauseDur = vkp.keyerRepeatPauseDur;
    keyerButtonNum = vkp.keyerButtonNum;
    sAndPState = vkp.sAndPState;
    cwKeyerType = vkp.cwKeyerType;
    return *this;
}

void TxKeyerParams::setKeyerDuration(const int keyerDuration_)
{
    if (keyerDuration_ > 0)
    {
        trace(QString("setVmDuration(%1)").arg(keyerDuration_));
    }
    keyerDuration = keyerDuration_;
}


TxKeyerBase::TxKeyerBase(QObject *parent) : QObject(parent)
{

}

TxKeyerBase::~TxKeyerBase()
{

}




