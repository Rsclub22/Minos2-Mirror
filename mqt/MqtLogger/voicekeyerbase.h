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


#ifndef VOICEKEYERBASE_H
#define VOICEKEYERBASE_H

#include <QObject>

class VoiceKeyerBase  : public QObject
{
    Q_OBJECT

public:
    explicit VoiceKeyerBase(QObject *parent = nullptr);

    virtual void voiceKeyerInit(int numButtons) = 0;
    virtual void sendMsgNum(int msgNum) = 0;
    virtual void stopMsg() = 0;

    virtual int getKeyerState(int &state) = 0;

    int getMaxNumButtons(){return MAXNUM_BUTTONS;}

signals:

    void vmVoiceKeyPressed(int msgNum);
    void vmVoiceKeyStopPressed();

private:

    const int MAXNUM_BUTTONS = 8;
};

#endif // VOICEKEYERBASE_H
