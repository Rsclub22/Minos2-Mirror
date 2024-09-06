/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2024
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef VOICEKEYERBASE_H
#define VOICEKEYERBASE_H

#include <QObject>
#include <QSharedPointer>
#include "serialCommonData.h"

class VoiceKeyerBase;
class VoiceKeyerFactory;
class LoggerContestLog;
class BaseContestLog;

const int REPEAT_DUR_MIN = 0;
const int REPEAT_DUR_MAX = 180; // secs


class VoiceKeyerParams
{

public:

    explicit VoiceKeyerParams();

    ~VoiceKeyerParams(){}

    VoiceKeyerParams& operator = (const VoiceKeyerParams& vkp);

    VoiceKeyerParams(const VoiceKeyerParams& vkp)
    {
        *this = vkp;
    }

    QString getType() const {return type;}
    void setType(const QString type_){type = type_;}

    QString getSelRadioName(){return selRadioName;}
    void setSelRadioName(const QString selRadioName_){selRadioName = selRadioName_;}


    QString getVmName() const {return vmName;}
    void setVmName(const QString vmName_){vmName = vmName_.trimmed();}

    QString getVmCwMessage() const {return vmCwMessage;}
    void setVmCwMessage(const QString msg){vmCwMessage = msg;}



    bool getVmRepeatFlag() const {return vmRepeatFlag;}
    void setVmRepeatFlag(const bool vmRepeatFlag_){vmRepeatFlag = vmRepeatFlag_;}

    int getVmRepeatPauseDur() const {return vmRepeatPauseDur;}
    void setVmRepeatPauseDur(const int vmRepeatPauseDur_){vmRepeatPauseDur = vmRepeatPauseDur_;}

    int getVmDuration() const {return vmDuration;}
    void setVmDuration(const int vmDuration_);

    int getvmButtonNum() const {return vmButtonNum;}
    void setvmButtonNum(const int vmButtonNum_){vmButtonNum = vmButtonNum_;}

    void setRigModel(const QString rigModel_) {rigModel = rigModel_;}
    QString getRigModel() const {return rigModel;}

    void setSAndPState(const bool sAndPState_) {sAndPState = sAndPState_;}
    bool getSAndPState() const {return sAndPState;}

    QSharedPointer<VoiceKeyerBase> getVkBase() const {return vkBase;}
    void setVkBase(QSharedPointer<VoiceKeyerBase> value){vkBase = value;}



    void clear();

private:

    QString type;
    QSharedPointer<VoiceKeyerBase> vkBase;
    QString selRadioName;
    QString rigModel;
    QString vmName;
    QString vmCwMessage;
    int vmDuration = 0;
    bool vmRepeatFlag = false;
    int vmRepeatPauseDur = -1;
    int vmButtonNum = -1;
    bool sAndPState = true; // S&P = true, Run = false
};

class VoiceKeyerBase  : public QObject
{
    Q_OBJECT

public:
    int numButtons = 0;
    int maxNumberButtons = 8;       // set with max buttons from radio or 8


    explicit VoiceKeyerBase(QObject *parent = nullptr);
    virtual ~VoiceKeyerBase();

    virtual  void setContest(BaseContestLog *c) = 0;

    virtual void voiceKeyerInit(int &numButtons) = 0;
    virtual void sendMsgNum(int msgNum) = 0;
    virtual void stopMsg(VoiceKeyerParams * vkParam) = 0;

    virtual bool doRepeatFromLogger(){return true;}

    virtual void sendCwMsg(VoiceKeyerParams &vmParams) = 0;
    virtual void stopCwMsg() = 0;
    virtual void setCwMemType(int cwMemType) = 0;
    virtual bool getSetCwModeAndRestoreFlag() = 0;

    virtual bool hasRecord() = 0;
    virtual void doRecording(VoiceKeyerParams *){}

    virtual bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) = 0;
    virtual void saveVmButtonParams(const VoiceKeyerParams &vmParams ) = 0;

    virtual void setPttOnOff(bool onOff) = 0;
    virtual int getSelectedEomType() = 0;

    virtual int setup(VoiceKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName) = 0;
    virtual void setRadioParams(int radioMaxNumButtons, QString selectedRadioName, serialCommonData::MINOS_PTT_TYPES pttType_, bool pttEnabled_) = 0;
    virtual int editButton(VoiceKeyerParams* vmData, QString title) = 0;



    virtual bool hasPip() const
    {
        return false;
    }
    //virtual bool hasSetup() const
    //{
    //    return true;
    //}
    //virtual bool hasTxStatus() const
    //{
    //    return true;
    //}
    virtual void setPip(bool){}
    virtual bool getPip() const {return false;}

    int getMaxNumButtons() const {return MAXNUM_BUTTONS;}

signals:

    void vmVoiceKeyPressed(int msgNum);
    void vmVoiceKeyStopPressed();

    void internalVoiceMemoryKeyerPlayState(bool onOff);

    void remoteConfigChanged();
    void remoteKeyerStopped();
    void remoteKeyerStarted(int key);

private:

    const int MAXNUM_BUTTONS = 8;
};

#endif // VOICEKEYERBASE_H
