#ifndef EXTERNALMQTKEYER_H
#define EXTERNALMQTKEYER_H

#include "KeyerJson.h"
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"

class ExternalMqtKeyer:public VoiceKeyerBase
{
    Q_OBJECT
    KeyerJson remoteConfig;
public:
    ExternalMqtKeyer(QObject *parent =nullptr);
    virtual ~ExternalMqtKeyer() override;

    static QString keyerName;


    static void registerVoiceKeyer(VoiceKeyerFactory::VmKeyers*);

    // VoiceKeyerBase interface
    virtual void voiceKeyerInit(int &numButtons) override;
    virtual void sendMsgNum(int msgNum) override;
    virtual void stopMsg(VoiceKeyerParams * vkParam) override;

    void sendCwMsg(QString message) override {Q_UNUSED(message)};
    void stopCwMsg() override {};
    void setCwMemType(int cwMemType) override {Q_UNUSED(cwMemType)};
    bool getSetCwModeAndRestoreFlag() override {return false;};

    virtual bool hasRecord() override{return true;}
    virtual void doRecording(VoiceKeyerParams *vkParam) override;

    virtual bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) override;
    virtual void saveVmButtonParams(const VoiceKeyerParams &vmParams) override;
    virtual void setPttOnOff(bool onOff) override;
    virtual bool getUsePttForEomFlag() override;

    virtual int setup(VoiceKeyerFactory *voiceKeyerFactory, int &numButtons) override;
    virtual int editButton(VoiceKeyerParams* vmData, QString title) override;

    //virtual bool hasPip() const override
    //{
    //    return true;
    //}
    //virtual bool hasSetup() const override
    //{
    //    return false;
    //}
    //virtual bool hasTxStatus() const override
    //{
    //    return false;
    //}

    virtual void setPip(bool) override;

    virtual bool getPip() const override {return remoteConfig.pipEnable;}

private slots:
    void onKeyerConfig(QString key, QString val);
    void onKeyerReport(QString val);


private:

    bool usePttForEom = false;

};

#endif // EXTERNALMQTKEYER_H
