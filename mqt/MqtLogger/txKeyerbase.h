/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Tx Keyer Base
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2025
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef TXKEYERBASE_H
#define TXKEYERBASE_H

#include <QObject>
#include <QSharedPointer>
#include "serialCommonData.h"

class TxKeyerBase;
class TxKeyerFactory;
class LoggerContestLog;
class BaseContestLog;

const int REPEAT_DUR_MIN = 0;
const int REPEAT_DUR_MAX = 180; // secs

enum CW_KEYER_TYPE {KEYER_NONE, RIG, PC_CwKeyer, WINKEYER};

class TxKeyerParams
{

public:

    explicit TxKeyerParams();

    ~TxKeyerParams(){}

    TxKeyerParams& operator = (const TxKeyerParams& vkp);

    TxKeyerParams(const TxKeyerParams& vkp)
    {
        *this = vkp;
    }

    QString getType() const {return type;}
    void setType(const QString type_){type = type_;}

    QString getSelRadioName(){return selRadioName;}
    void setSelRadioName(const QString selRadioName_){selRadioName = selRadioName_;}


    QString getKeyerName() const {return keyerName;}
    void setKeyerName(const QString keyerName_){keyerName = keyerName_.trimmed();}

    QString getKeyerCwMessage() const {return keyerCwMessage;}
    void setKeyerCwMessage(const QString msg){keyerCwMessage = msg;}

    int getRigVoiceMemNum() const {return rigVoiceMemNum;}
    void setRigVoiceMemNum(const int rigVoiceMemNum_){rigVoiceMemNum = rigVoiceMemNum_;}

    bool getKeyerRepeatFlag() const {return keyerRepeatFlag;}
    void setKeyerRepeatFlag(const bool keyerRepeatFlag_){keyerRepeatFlag = keyerRepeatFlag_;}

    int getKeyerRepeatPauseDur() const {return keyerRepeatPauseDur;}
    void setKeyerRepeatPauseDur(const int keyerRepeatPauseDur_){keyerRepeatPauseDur = keyerRepeatPauseDur_;}

    int getKeyerDuration() const {return keyerDuration;}
    void setKeyerDuration(const int keyerDuration_);

    int getKeyerButtonNum() const {return keyerButtonNum;}
    void setKeyerButtonNum(const int keyerButtonNum_){keyerButtonNum = keyerButtonNum_;}

    void setRigModel(const QString rigModel_) {rigModel = rigModel_;}
    QString getRigModel() const {return rigModel;}

    void setSAndPState(const bool sAndPState_) {sAndPState = sAndPState_;}
    bool getSAndPState() const {return sAndPState;}

    void setCwKeyerType(CW_KEYER_TYPE type){cwKeyerType = type;}
    CW_KEYER_TYPE getCwKeyerType(){return cwKeyerType;}

    QSharedPointer<TxKeyerBase> getVkBase() const {return vkBase;}
    void setVkBase(QSharedPointer<TxKeyerBase> value){vkBase = value;}



    void clear();

private:

    QString type;
    QSharedPointer<TxKeyerBase> vkBase;
    QString selRadioName;
    QString rigModel;
    QString keyerName;
    QString keyerCwMessage;
    int rigVoiceMemNum = 0;
    int keyerDuration = 0;
    bool keyerRepeatFlag = false;
    int keyerRepeatPauseDur = -1;
    int keyerButtonNum = -1;
    CW_KEYER_TYPE cwKeyerType = CW_KEYER_TYPE::KEYER_NONE;

    bool sAndPState = true; // S&P = true, Run = false
};

class TxKeyerBase  : public QObject
{
    Q_OBJECT

public:
    int numButtons = 0;
    int maxNumberButtons = 8;       // set with max buttons from radio or 8


    explicit TxKeyerBase(QObject *parent = nullptr);
    virtual ~TxKeyerBase();

    virtual  void setContest(BaseContestLog *c) = 0;

    virtual void txKeyerInit(int &numButtons) = 0;
    virtual void sendMsgNum(TxKeyerParams &vkParam) = 0;
    virtual void stopMsg(TxKeyerParams &vkParam) = 0;

    virtual bool doRepeatFromLogger(){return true;}

    virtual void sendCwMsg(TxKeyerParams &vmParams) = 0;
    virtual void stopCwMsg() = 0;
    virtual void sendCwFreeTextMsg(QString message) = 0;
    virtual void setCwMemType(int cwMemType) = 0;
    virtual bool getSetCwModeAndRestoreFlag() = 0;

    virtual bool hasRecord() = 0;
    virtual void doRecording(TxKeyerParams *){}

    //virtual bool readVmButtonParams(int buttonNum, TxKeyerParams &vmParams) = 0;
    //virtual void saveVmButtonParams(const TxKeyerParams &vmParams ) = 0;

    virtual void setPttOnOff(bool onOff) = 0;
    virtual int getSelectedEomType() = 0;
    virtual void setSelectedEomType(int eomType) = 0;

    virtual int setup(TxKeyerFactory *txKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName) = 0;
    virtual void setRadioParams(int radioMaxNumButtons, QString selectedRadioName, serialCommonData::MINOS_PTT_TYPES pttType_, bool pttEnabled_) = 0;
    //virtual int editButton(TxKeyerParams* vmData, QString title) = 0;



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

    void cwMacroExpandedText(const QString &text);

    void internalVoiceMemoryKeyerPlayState(bool onOff);

    void remoteConfigChanged();
    void remoteKeyerStopped();
    void remoteKeyerStarted(int key);

private:

    const int MAXNUM_BUTTONS = 8;
};

#endif // VOICEKEYERBASE_H
