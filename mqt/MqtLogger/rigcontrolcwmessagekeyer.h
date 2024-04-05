/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      CW Message Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2021
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef RIGCONTROLCWMESSAGEKEYER_H
#define RIGCONTROLCWMESSAGEKEYER_H

#include "serialCommonData.h"
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"


#include <QObject>
#include <QValidator>

class RigControlCwMessageKeyer : public VoiceKeyerBase
{
    Q_OBJECT

public:
    explicit RigControlCwMessageKeyer(QObject *parent = nullptr);
    virtual ~RigControlCwMessageKeyer() override;

    static void registerVoiceKeyer(VoiceKeyerFactory::VmKeyers*);

    virtual void voiceKeyerInit(int &numButtons) override;
    virtual void sendMsgNum(int buttonNum) override {Q_UNUSED(buttonNum)};

    virtual void stopMsg(VoiceKeyerParams *vkParams) override {Q_UNUSED(vkParams)};

    virtual void sendCwMsg(VoiceKeyerParams &vmParams) override;
    virtual void stopCwMsg() override;
    virtual void setCwMemType(int cwMemType) override;
    bool getSetCwModeAndRestoreFlag() override;

    virtual bool hasRecord() override {return false;}

    virtual void setPttOnOff(bool onOff) override;
    virtual int getSelectedEomType() override;

    virtual bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) override;
    virtual void saveVmButtonParams(const VoiceKeyerParams &vmParams) override;

    virtual int setup(VoiceKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName) override;
    virtual void setRadioParams(int radioMaxNumButtons, QString selectedRadioName, int pttType_, bool pttEnabled_) override;

    virtual int editButton(VoiceKeyerParams* vmData, QString title) override;



    void setUseCATPttForEom(bool usePttForEom_);
    void setSelectedEomType(int selectedEomType_);

private:

    int cwMemType;
    int selectedEomType = voiceKeyerCommon::VoiceCwKeyerEomTypes::Eom_None;
    int pttType = serialCommonData::PTT_METHOD_NONE;
    bool pttEnabled = false;
    bool setCwModeAndRestoreCurrentMode = true;

    bool radioKeyerAvail = false;
    int radioMaxNumButtons = 0;
    QString selectedRadioName;



    void getRadioCommonData(int &selectedEomType, int &userNumberButtons, int radioMaxNumButtons);
};



#endif // RIGCONTROLCWMESSAGEKEYER_H
