#ifndef INTERNALVOICEMEMORYKEYER_H
#define INTERNALVOICEMEMORYKEYER_H

#include "txKeyerbase.h"
#include "txKeyerfactory.h"
#include "serialCommonData.h"

class InternalVoiceMemoryKeyer : public TxKeyerBase
{
    Q_OBJECT
public:
    explicit InternalVoiceMemoryKeyer(QObject *parent =nullptr);
    virtual ~InternalVoiceMemoryKeyer() override;

    static void registerTxKeyer(TxKeyerFactory::TxKeyers*);

    // VoiceKeyerBase interface
    virtual void txKeyerInit(int &numButtons) override;
    virtual void sendMsgNum(int msgNum) override;
    virtual void stopMsg(TxKeyerParams *vkParam) override;

    void sendCwMsg(TxKeyerParams &vmParams) override {Q_UNUSED(vmParams)};
    void sendCwFreeTextMsg(QString message) override {Q_UNUSED(message)};
    virtual void stopCwMsg() override {};
    virtual void setCwMemType(int cwMemType) override {Q_UNUSED(cwMemType)};
    virtual bool getSetCwModeAndRestoreFlag() override {return false;};

    virtual bool hasRecord() override{return true;}
    virtual void doRecording(TxKeyerParams *vkParam) override;


    //virtual bool hasTxStatus() const override
    //{
    //    return false;
    //}

    virtual bool readVmButtonParams(int buttonNum, TxKeyerParams &vmParams) override;
    virtual void saveVmButtonParams(const TxKeyerParams &vmParams) override;
    virtual void setPttOnOff(bool onOff) override;
    virtual int getSelectedEomType() override {return TxKeyerCommon::KeyerEomTypes::InternalSoundCardVoiceKeyer;};
    virtual void setSelectedEomType(int eomType)override{Q_UNUSED(eomType)};

    virtual int setup(TxKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName) override;
    virtual void setRadioParams(int radioMaxNumButtons, QString selectedRadioName, serialCommonData::MINOS_PTT_TYPES pttType_, bool pttEnabled_) override{Q_UNUSED(radioMaxNumButtons) Q_UNUSED(selectedRadioName) Q_UNUSED(pttType_) Q_UNUSED(pttEnabled_)};
    virtual int editButton(TxKeyerParams* vmData, QString title) override;

    virtual void setContest(BaseContestLog *c) override {Q_UNUSED(c)};



private slots:
    void onDoPTT(bool onOff);

private:

    int selectedEomType = TxKeyerCommon::KeyerEomTypes::Eom_None;
    serialCommonData::MINOS_PTT_TYPES pttType = serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE;
    bool pttEnabled = false;

};

#endif // INTERNALVOICEMEMORYKEYER_H
