#ifndef INTERNALVOICEMEMORYKEYER_H
#define INTERNALVOICEMEMORYKEYER_H

#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"
#include "serialCommonData.h"

class InternalVoiceMemoryKeyer : public VoiceKeyerBase
{
    Q_OBJECT
public:
    explicit InternalVoiceMemoryKeyer(QObject *parent =nullptr);
    virtual ~InternalVoiceMemoryKeyer() override;

    static void registerVoiceKeyer(VoiceKeyerFactory::VmKeyers*);

    // VoiceKeyerBase interface
    virtual void voiceKeyerInit(int &numButtons) override;
    virtual void sendMsgNum(int msgNum) override;
    virtual void stopMsg(VoiceKeyerParams *vkParam) override;

    void sendCwMsg(VoiceKeyerParams &vmParams) override {Q_UNUSED(vmParams)};
    virtual void stopCwMsg() override {};
    virtual void setCwMemType(int cwMemType) override {Q_UNUSED(cwMemType)};
    virtual bool getSetCwModeAndRestoreFlag() override {return false;};

    virtual bool hasRecord() override{return true;}
    virtual void doRecording(VoiceKeyerParams *vkParam) override;


    //virtual bool hasTxStatus() const override
    //{
    //    return false;
    //}

    virtual bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) override;
    virtual void saveVmButtonParams(const VoiceKeyerParams &vmParams) override;
    virtual void setPttOnOff(bool onOff) override;
    virtual int getSelectedEomType() override {return voiceKeyerCommon::VoiceCwKeyerEomTypes::InternalSoundCardVoiceKeyer;};


    virtual int setup(VoiceKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName) override;
    virtual void setRadioParams(int radioMaxNumButtons, QString selectedRadioName, int pttType_, bool pttEnabled_) override{Q_UNUSED(radioMaxNumButtons) Q_UNUSED(selectedRadioName) Q_UNUSED(pttType_) Q_UNUSED(pttEnabled_)};
    virtual int editButton(VoiceKeyerParams* vmData, QString title) override;




private slots:
    void onDoPTT(bool onOff);

private:

    int selectedEomType = voiceKeyerCommon::VoiceCwKeyerEomTypes::Eom_None;
    int pttType = serialCommonData::PTT_METHOD_NONE;
    bool pttEnabled = false;
};

#endif // INTERNALVOICEMEMORYKEYER_H
