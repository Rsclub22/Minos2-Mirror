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

    void voiceKeyerInit(int &numButtons) override;
    void sendMsgNum(int buttonNum) override {Q_UNUSED(buttonNum)};

    void stopMsg(VoiceKeyerParams *vkParams) override {Q_UNUSED(vkParams)};

    void sendCwMsg(const QString message) override;
    void stopCwMsg() override;
    void setCwMemType(int cwMemType) override;
    bool getSetCwModeAndRestoreFlag() override;

    virtual bool hasRecord() override {return false;}

    void setPttOnOff(bool onOff) override;
    virtual bool getUsePttForEomFlag() override;

    bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) override;
    void saveVmButtonParams(const VoiceKeyerParams &vmParams) override;

    virtual int setup(VoiceKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName) override;
    virtual int editButton(VoiceKeyerParams* vmData, QString title) override;



private:

    int cwMemType;
    bool usePttForEom = false;
    bool setCwModeAndRestoreCurrentMode = true;


    QString getCwRadioManufacturer(int cwMemType);
    QString getRadioModel(QString selectedRadio);
};



#endif // RIGCONTROLCWMESSAGEKEYER_H
