#ifndef EXTERNALMQTKEYER_H
#define EXTERNALMQTKEYER_H

#include "KeyerJson.h"
#include "txKeyerbase.h"
#include "txKeyerFactory.h"
#include "txkeyerCommonConstants.h"

using namespace TxKeyerCommon;

class ExternalMqtKeyer:public TxKeyerBase
{
    Q_OBJECT
    KeyerJson remoteConfig;
public:
    ExternalMqtKeyer(QObject *parent =nullptr);
    virtual ~ExternalMqtKeyer() override;

    static QString keyerName;


    static void registerTxKeyer(TxKeyerFactory::TxKeyers*);

    // VoiceKeyerBase interface
    virtual void txKeyerInit(int &numButtons) override;
    virtual void sendMsgNum(TxKeyerParams &vkParam) override;
    virtual void stopMsg(TxKeyerParams &vkParam) override;

    virtual bool doRepeatFromLogger() override
    {
        return false;
    }

    void sendCwMsg(TxKeyerParams &vmParams) override {Q_UNUSED(vmParams)};
    virtual void stopCwMsg() override {};
     void sendCwFreeTextMsg(QString message) override {Q_UNUSED(message)};
    virtual void setCwMemType(int cwMemType) override {Q_UNUSED(cwMemType)};
    virtual bool getSetCwModeAndRestoreFlag() override {return false;};

    virtual bool hasRecord() override{return true;}
    virtual void doRecording(TxKeyerParams *vkParam) override;

    //virtual bool readVmButtonParams(int buttonNum, TxKeyerParams &vmParams) override;
    //virtual void saveVmButtonParams(const TxKeyerParams &vmParams) override;
    virtual void setPttOnOff(bool onOff) override;

    virtual int getSelectedEomType() override {return TxKeyerCommon::KeyerEomTypes::Eom_None;};
    virtual void setSelectedEomType(int eomType)override{Q_UNUSED(eomType)};

    virtual int setup(TxKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadio) override;
    virtual void setRadioParams(int radioMaxNumButtons, QString selectedRadioName, serialCommonData::MINOS_PTT_TYPES pttType_, bool pttEnabled_) override{Q_UNUSED(radioMaxNumButtons) Q_UNUSED(selectedRadioName) Q_UNUSED(pttType_) Q_UNUSED(pttEnabled_)};
    //virtual int editButton(TxKeyerParams* vmData, QString title) override;


    virtual void setContest(BaseContestLog *c) override {Q_UNUSED(c)};

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




};

#endif // EXTERNALMQTKEYER_H
