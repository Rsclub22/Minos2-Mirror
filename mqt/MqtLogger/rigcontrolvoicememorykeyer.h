/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Voice Memory Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2025
//
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef RIGCONTROLVOICEMEMORYKEYER_H
#define RIGCONTROLVOICEMEMORYKEYER_H

#include <QObject>
#include "txKeyerbase.h"
#include "txKeyerfactory.h"
#include "serialCommonData.h"

class RigControlVoiceMemoryKeyer : public TxKeyerBase
{
    Q_OBJECT
public:
    explicit RigControlVoiceMemoryKeyer(QObject *parent = nullptr);
    virtual ~RigControlVoiceMemoryKeyer() override;

    static void registerVoiceKeyer(TxKeyerFactory::TxKeyers*);

    void txKeyerInit(int &numButtons) override;
    void sendMsgNum(int buttonNum) override;
    void stopMsg(TxKeyerParams * vkParam) override;

    virtual void setRadioParams(int radioMaxNumButtons, QString selectedRadioName, serialCommonData::MINOS_PTT_TYPES pttType_, bool pttEnabled_) override;


    void sendCwMsg(TxKeyerParams &vmParams) override {Q_UNUSED(vmParams)};
    void stopCwMsg() override {};

    void setCwMemType(int cwMemType) override {Q_UNUSED(cwMemType)};
    void sendCwFreeTextMsg(QString message) override {Q_UNUSED(message)};
    bool getSetCwModeAndRestoreFlag() override {return false;};

    virtual bool hasRecord() override {return false;}





    //int getKeyerState(int &state) override;
    void setPttOnOff(bool onOff) override;
    int getSelectedEomType() override;
    virtual void setSelectedEomType(int eomType)override{Q_UNUSED(eomType)};

    bool readVmButtonParams(int buttonNum, TxKeyerParams &vmParams) override;
    void saveVmButtonParams(const TxKeyerParams &vmParams) override;

    virtual int setup(TxKeyerFactory *voiceKeyerFactory, int &maxNumButtons, int &numButtons, QString selectedRadioName) override;

    virtual int editButton(TxKeyerParams* vmData, QString title) override;




    virtual void setContest(BaseContestLog *c) override {Q_UNUSED(c)};

signals:



private:

    int selectedEomType = TxKeyerCommon::KeyerEomTypes::Eom_None;
    serialCommonData::MINOS_PTT_TYPES pttType = serialCommonData::MINOS_PTT_TYPES::PTT_TYPE_NONE;
    bool pttEnabled = false;

    bool radioKeyerAvail = false;



    int radioMaxNumButtons = 0;
    QString selectedRadioName;


    void getRadioCommonData(int &usePttForEom, int &userNumberButtons, int radioMaxNumButtons);
};

#endif // RIGCONTROLVOICEMEMORYKEYER_H
