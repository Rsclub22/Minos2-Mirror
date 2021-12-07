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


const QString STOPCODE = "0";


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

    void sendCwMsg(QString message) override {Q_UNUSED(message)};
    void stopCwMsg() override {};
    void setCwMemType(int cwMemType) override {Q_UNUSED(cwMemType)};

    virtual bool hasRecord() override {return false;}




    //int getKeyerState(int &state) override;
    void setPttOnOff(bool onOff) override;
    bool getUsePttForEomFlag() override;

    bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) override;
    void saveVmButtonParams(const VoiceKeyerParams &vmParams) override;

    virtual int setup(VoiceKeyerFactory *voiceKeyerFactory, int &numButtons) override;
    virtual int editButton(VoiceKeyerParams* vmData, QString title) override;


signals:



private:

    bool usePttForEom = false;




};

#endif // RIGCONTROLVOICEMEMORYKEYER_H
