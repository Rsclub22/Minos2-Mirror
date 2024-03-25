/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Voice Memory Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef RIGCONTROLVOICEMEMORYKEYER_H
#define RIGCONTROLVOICEMEMORYKEYER_H

#include <QObject>
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"

class RigControlVoiceMemoryKeyer : public VoiceKeyerBase
{
    Q_OBJECT
public:
    explicit RigControlVoiceMemoryKeyer(QObject *parent = nullptr);
    virtual ~RigControlVoiceMemoryKeyer() override;

    static void registerVoiceKeyer(VoiceKeyerFactory::VmKeyers*);

    void voiceKeyerInit(int &numButtons) override;
    void sendMsgNum(int buttonNum) override;
    void stopMsg(VoiceKeyerParams * vkParam) override;

    virtual void setRadioParams(int radioMaxNumButtons, QString selectedRadioName) override;


    void sendCwMsg(VoiceKeyerParams &vmParams) override {Q_UNUSED(vmParams)};
    void stopCwMsg() override {};
    void setCwMemType(int cwMemType) override {Q_UNUSED(cwMemType)};
    bool getSetCwModeAndRestoreFlag() override {return false;};

    virtual bool hasRecord() override {return false;}





    //int getKeyerState(int &state) override;
    void setPttOnOff(bool onOff) override;
    bool getUsePttForEomFlag() override;

    bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) override;
    void saveVmButtonParams(const VoiceKeyerParams &vmParams) override;

    virtual int setup(VoiceKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName) override;

    virtual int editButton(VoiceKeyerParams* vmData, QString title) override;



    void setUsePttForEom(bool usePttForEom_);
signals:



private:

    bool usePttForEom = false;

    bool radioKeyerAvail = false;

    int radioMaxNumButtons = 0;
    QString selectedRadioName;


    void getRadioCommonData(bool &usePttForEom, int &userNumberButtons, int radioMaxNumButtons);
};

#endif // RIGCONTROLVOICEMEMORYKEYER_H
