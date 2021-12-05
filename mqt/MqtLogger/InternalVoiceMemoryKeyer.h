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
    virtual void voiceKeyerInit(int &numButtons) override;
    virtual void sendMsgNum(int msgNum) override;
    virtual void stopMsg() override;

    void sendCwMsg(QString message) override {Q_UNUSED(message)};
    void stopCwMsg() override {};
    void setCwMemType(int cwMemType) override {Q_UNUSED(cwMemType)};

    virtual bool hasRecord() override{return true;}
    virtual void doRecording(VoiceKeyerParams *vkParam) override;

    virtual bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) override;
    virtual void saveVmButtonParams(const VoiceKeyerParams &vmParams) override;
    virtual void setPttOnOff(bool onOff) override;
    virtual bool getUsePttForEomFlag() override;

    virtual int setup(VoiceKeyerFactory *voiceKeyerFactory, int &numButtons) override;
    virtual int editButton(VoiceKeyerParams* vmData, QString title) override;

private slots:
    void onDoPTT(bool onOff);

private:

    bool usePttForEom = false;
};

#endif // INTERNALVOICEMEMORYKEYER_H
