/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2021
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef VOICEKEYERBASE_H
#define VOICEKEYERBASE_H

#include <QObject>
#include <QSharedPointer>

class VoiceKeyerBase;
class VoiceKeyerFactory;

const int REPEAT_DUR_MIN = 0;
const int REPEAT_DUR_MAX = 180; // secs

class VoiceKeyerParams
{

public:

    explicit VoiceKeyerParams();

    ~VoiceKeyerParams(){}

    VoiceKeyerParams& operator = (const VoiceKeyerParams& vkp);


    QString getType() const {return type;}
    void setType(const QString type_){type = type_;}


    QString getVmName() const {return vmName;}
    void setVmName(const QString vmName_){vmName = vmName_.trimmed();}

    QString getVmCwMessage() const {return vmCwMessage;}
    void setVmCwMessage(const QString msg){vmCwMessage = msg;}


    bool getVmRepeatFlag() const {return vmRepeatFlag;}
    void setVmRepeatFlag(const bool vmRepeatFlag_){vmRepeatFlag = vmRepeatFlag_;}

    int getVmRepeatPauseDur() const {return vmRepeatPauseDur;}
    void setVmRepeatPauseDur(const int vmRepeatPauseDur_){vmRepeatPauseDur = vmRepeatPauseDur_;}


    int getVmDuration() const {return vmDuration;}
    void setVmDuration(const int vmDuration_){vmDuration = vmDuration_;}

    int getvmButtonNum() const {return vmButtonNum;}
    void setvmButtonNum(const int vmButtonNum_){vmButtonNum = vmButtonNum_;}

    QSharedPointer<VoiceKeyerBase> getVkBase() const {return vkBase;}
    void setVkBase(QSharedPointer<VoiceKeyerBase> value){vkBase = value;}

    void clear();

private:

    QString type;
    QSharedPointer<VoiceKeyerBase> vkBase;
    QString vmName;
    QString vmCwMessage;
    int vmDuration;
    bool vmRepeatFlag;
    int vmRepeatPauseDur;
    int vmButtonNum;
};

class VoiceKeyerBase  : public QObject
{
    Q_OBJECT

public:
    int numButtons = 0;

    explicit VoiceKeyerBase(QObject *parent = nullptr);
    virtual ~VoiceKeyerBase();

    virtual void voiceKeyerInit(int &numButtons) = 0;
    virtual void sendMsgNum(int msgNum) = 0;
    virtual void stopMsg() = 0;

    virtual void sendCwMsg(QString message) = 0;
    virtual void stopCwMsg() = 0;
    virtual void setCwMemType(int cwMemType) = 0;

    virtual bool hasRecord() = 0;
    virtual void doRecording(VoiceKeyerParams *){}

    virtual bool readVmButtonParams(int buttonNum, VoiceKeyerParams &vmParams) = 0;
    virtual void saveVmButtonParams(const VoiceKeyerParams &vmParams ) = 0;

    virtual void setPttOnOff(bool onOff) = 0;

    virtual int setup(VoiceKeyerFactory *voiceKeyerFactory, int &numButtons) = 0;
    virtual int editButton(VoiceKeyerParams* vmData, QString title) = 0;

    virtual bool hasPip() const
    {
        return false;
    }
    virtual bool hasSetup() const
    {
        return true;
    }
    virtual void setPip(bool){}
    virtual bool getPip() const {return false;}

    int getMaxNumButtons() const {return MAXNUM_BUTTONS;}

signals:

    void vmVoiceKeyPressed(int msgNum);
    void vmVoiceKeyStopPressed();

    void remoteConfigChanged();
    void remoteKeyerStopped();
    void remoteKeyerStarted(int key);

private:

    const int MAXNUM_BUTTONS = 8;
};

#endif // VOICEKEYERBASE_H
