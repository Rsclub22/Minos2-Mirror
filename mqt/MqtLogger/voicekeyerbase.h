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



class VoiceKeyerCommonParams
{

public:

    explicit VoiceKeyerCommonParams();
    ~VoiceKeyerCommonParams();

    void operator = (const VoiceKeyerCommonParams& vkcp);
    bool operator == (const VoiceKeyerCommonParams& vkcp);
    bool operator != (const VoiceKeyerCommonParams& vkcp);

    void clear();

    void setComport(const QString comport_){comport = comport_;}
    QString getComport(){return comport;}

    void setNumButtons(const int numButtons_){numButtons = numButtons_;}
    int getNumButtons(){return numButtons;}

private:

    QString comport;
    int numButtons;


};






class VoiceKeyerParams
{

public:

    explicit VoiceKeyerParams();

    ~VoiceKeyerParams(){}

    void operator = (const VoiceKeyerParams& vkp);


    QString getType(){return type;}
    void setType(const QString type_){type = type_;}


    QString getVmName(){return vmName;}
    void setVmName(const QString vmName_){vmName = vmName_.trimmed();}

    bool getVmRepeatFlag(){return vmRepeatFlag;}
    void setVmRepeatFlag(const bool vmRepeatFlag_){vmRepeatFlag = vmRepeatFlag_;}

    int getVmRepeatDur(){return vmRepeatDur;}
    void setVmRepeatDur(const int vmRepeatDur_){vmRepeatDur = vmRepeatDur_;}

    int getvmButtonNum(){return vmButtonNum;}
    void setvmButtonNum(const int vmButtonNum_){vmButtonNum = vmButtonNum_;}

    void clear();

private:

    QString type;
    QString vmName;
    bool vmRepeatFlag;
    int vmRepeatDur;
    int vmButtonNum;
};

class VoiceKeyerBase  : public QObject
{
    Q_OBJECT

public:
    explicit VoiceKeyerBase(QObject *parent = nullptr);

    virtual void voiceKeyerInit(int numButtons) = 0;
    virtual void sendMsgNum(int msgNum) = 0;
    virtual void stopMsg() = 0;

    virtual int getKeyerState(int &state) = 0;

    virtual bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) = 0;
    virtual void saveVmButtonParams(const VoiceKeyerParams &vmParams ) = 0;


    int getMaxNumButtons(){return MAXNUM_BUTTONS;}

signals:

    void vmVoiceKeyPressed(int msgNum);
    void vmVoiceKeyStopPressed();

private:

    const int MAXNUM_BUTTONS = 8;
};

#endif // VOICEKEYERBASE_H
