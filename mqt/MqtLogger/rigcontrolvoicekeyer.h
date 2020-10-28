/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef RIGCONTROLVOICEKEYER_H
#define RIGCONTROLVOICEKEYER_H

#include <QObject>
#include "voicekeyerbase.h"
#include "voicekeyerfactory.h"

class RigControlVoiceKeyer : public VoiceKeyerBase
{
    Q_OBJECT
public:
    explicit RigControlVoiceKeyer(QObject *parent = nullptr);
    virtual ~RigControlVoiceKeyer();

    static void registerVoiceKeyer(VoiceKeyerFactory::VmKeyers*);

    void voiceKeyerInit(int numButtons) override;
    void sendMsgNum(int msgNum) override;
    void stopMsg() override;

    int getKeyerState(int &state) override;

signals:

private:



};

#endif // RIGCONTROLVOICEKEYER_H
