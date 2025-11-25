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
#include "txKeyerbase.h"
#include "txkeyerfactory.h"

class LoggerContestLog;
class BaseContestLog;

class PcCWMessageKeyer : public TxKeyerBase
{
    Q_OBJECT

public:

    explicit PcCWMessageKeyer(QObject *parent = nullptr);
    virtual ~PcCWMessageKeyer() override;

    static void registerTxKeyer(TxKeyerFactory::TxKeyers*);

    virtual void txKeyerInit(int &numButtons) override;
    virtual void sendMsgNum(TxKeyerParams &vkParam) override {Q_UNUSED(vkParam)};

    virtual void stopMsg(TxKeyerParams &vkParams) override {Q_UNUSED(vkParams)};

    virtual void sendCwMsg(TxKeyerParams &vmParams) override;
    virtual void sendCwFreeTextMsg(QString message) override;
    virtual void stopCwMsg() override;
    virtual void setCwMemType(int cwMemType) override;
    bool getSetCwModeAndRestoreFlag() override;

    virtual bool hasRecord() override {return false;}

    virtual void setPttOnOff(bool onOff) override;
    virtual int getSelectedEomType() override;
    virtual void setSelectedEomType(int eomType) override;

    //virtual bool readVmButtonParams(int buttonNum, TxKeyerParams &vmParams) override;
    //virtual void saveVmButtonParams(const TxKeyerParams &vmParams) override;

    virtual int setup(TxKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName) override;
    virtual void setRadioParams(int radioMaxNumButtons, QString selectedRadioName, serialCommonData::MINOS_PTT_TYPES pttType_, bool pttEnabled_) override;

    //virtual int editButton(TxKeyerParams* vmData, QString title) override;


    virtual void setContest(BaseContestLog *c) override;


    void setUseCATPttForEom(bool usePttForEom_);
    void saveFixedRadioCommonData();



private:

    LoggerContestLog *ct = nullptr;

    int cwMemType;
    int selectedEomType = TxKeyerCommon::KeyerEomTypes::Eom_None;
    serialCommonData::MINOS_PTT_TYPES pttType = serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE;
    bool pttEnabled = false;
    bool setCwModeAndRestoreCurrentMode = true;


    bool dtrKeyerAvail = false;

    int radioMaxNumButtons = 0;

    QString selectedRadioName;



    void getRadioCommonData(int &selectedEomType, int &userNumberButtons, int radioMaxNumButtons);

    QString parseMacrosInMessage(TSingleLogFrame *tslf, QString mess);



    void logMessage(QString msg);
};

#endif // PCCWMESSAGEKEYER_H
