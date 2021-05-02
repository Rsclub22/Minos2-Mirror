#ifndef INTERNALVOICEMEMORYKEYER_H
#define INTERNALVOICEMEMORYKEYER_H

#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"
#include "keyerBase.h"

class InternalVoiceMemoryKeyer : public VoiceKeyerBase
{
    Q_OBJECT
public:
    explicit InternalVoiceMemoryKeyer(QObject *parent =nullptr);
    virtual ~InternalVoiceMemoryKeyer() override;

    static void registerVoiceKeyer(VoiceKeyerFactory::VmKeyers*);

    // VoiceKeyerBase interface
    virtual void voiceKeyerInit(int numButtons) override;
    virtual void sendMsgNum(int msgNum) override;
    virtual void stopMsg() override;
    virtual bool hasRecord() override{return true;}
    virtual void doRecording(VoiceKeyerParams *vkParam) override;

    virtual bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) override;
    virtual void saveVmButtonParams(const VoiceKeyerParams &vmParams) override;
    virtual void setPttOnOff(bool onOff) override;

    virtual int setup(VoiceKeyerFactory *voiceKeyerFactory, VoiceKeyerCommonParams &vmCommonParams) override;
    virtual int editButton(VoiceKeyerParams* vmData, QString title) override;

private slots:
    void onDoPTT(bool onOff);
};

#endif // INTERNALVOICEMEMORYKEYER_H
