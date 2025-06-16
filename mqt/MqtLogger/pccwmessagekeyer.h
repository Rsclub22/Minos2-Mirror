/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Logger PC DTR CW Message Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2025
//
//
//
//
////////////////////////////////////////////////////////////////////////////
///
///
#ifndef PCCWMESSAGEKEYER_H
#define PCCWMESSAGEKEYER_H

#include "tsinglelogframe.h"
#include "serialCommonData.h"
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"

class LoggerContestLog;
class BaseContestLog;

class PcCWMessageKeyer : public VoiceKeyerBase
{
    Q_OBJECT

public:

    explicit PcCWMessageKeyer(QObject *parent = nullptr);
    virtual ~PcCWMessageKeyer() override;

    static void registerVoiceKeyer(VoiceKeyerFactory::VmKeyers*);

    virtual void voiceKeyerInit(int &numButtons) override;
    virtual void sendMsgNum(int buttonNum) override {Q_UNUSED(buttonNum)};

    virtual void stopMsg(VoiceKeyerParams *vkParams) override {Q_UNUSED(vkParams)};

    virtual void sendCwMsg(VoiceKeyerParams &vmParams) override;
    virtual void sendCwFreeTextMsg(QString message) override;
    virtual void stopCwMsg() override;
    virtual void setCwMemType(int cwMemType) override;
    bool getSetCwModeAndRestoreFlag() override;

    virtual bool hasRecord() override {return false;}

    virtual void setPttOnOff(bool onOff) override;
    virtual int getSelectedEomType() override;

    virtual bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) override;
    virtual void saveVmButtonParams(const VoiceKeyerParams &vmParams) override;

    virtual int setup(VoiceKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName) override;
    virtual void setRadioParams(int radioMaxNumButtons, QString selectedRadioName, serialCommonData::MINOS_PTT_TYPES pttType_, bool pttEnabled_) override;

    virtual int editButton(VoiceKeyerParams* vmData, QString title) override;


    virtual void setContest(BaseContestLog *c) override;


    void setUseCATPttForEom(bool usePttForEom_);
    void setSelectedEomType(int selectedEomType_);



private:

    LoggerContestLog *ct = nullptr;

    int cwMemType;
    int selectedEomType = voiceKeyerCommon::VoiceCwKeyerEomTypes::Eom_None;
    serialCommonData::MINOS_PTT_TYPES pttType = serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE;
    bool pttEnabled = false;
    bool setCwModeAndRestoreCurrentMode = true;


    bool dtrKeyerAvail = false;

    int radioMaxNumButtons = 0;

    QString selectedRadioName;



    void getRadioCommonData(int &selectedEomType, int &userNumberButtons, int radioMaxNumButtons);
    QString parseMacrosInMessage(TSingleLogFrame *tslf, QString mess);


};

#endif // PCCWMESSAGEKEYER_H
