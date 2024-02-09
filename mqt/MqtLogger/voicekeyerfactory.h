/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Voice Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef VOICEKEYERFACTORY_H
#define VOICEKEYERFACTORY_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QModelIndex>
#include "voicekeyerbase.h"
#include "voicekeyerCommonConstants.h"


class QComboBox;

<<<<<<< HEAD
enum VoiceKeyerId
{
    None = 0,
    RigControl,
    CW_RigControl,    // Rig sends message by CW
    SerialControl,    // Sends user serial message to control external voice keyer
    InternalVoiceKeyer,
    ExternalVoiceKeyer
};

const QStringList keyerTypes = {"", "Voice RigControl", "CW RigControl", "SerialControl", "Internal", "mqtKeyer"};

QString VOICE_KEYER_PATH();
const QString VOICE_KEYER_BASE_FILE_NAME = "txVoiceMemory";
QString VOICEKEYER_COMMON_PARAMS_PATH();
const QString VOICEKEYER_COMMON_PARAMS_FILENAME = "txVoiceKeyCommonParams.ini";
const QString VOICEKEYER_COMMON_PARAMS_GROUPNAME = "commonParams";

const QString CW_MESSAGE_BASE_FILE_NAME = "txCwMemory";
=======
>>>>>>> 8fkh_newDev


class VoiceKeyerCapabilities
{
public:

    VoiceKeyerCapabilities(){};

    QString getKeyerName(){return keyerName;}
    void setKeyerName(const QString keyerName_){keyerName = keyerName_;}

    QString getKeyerType(){return keyerType;}
    void setKeyerType(const QString keyerType_){keyerType = keyerType_;}

    int getNumVoiceKeys(){return numVoiceKeys;}
    void setNumVoiceKeys(const int numVoiceKeys_){numVoiceKeys = numVoiceKeys_;}


    int getVmIdNum(){return vmIdNum;}
    void setVmIdNum(const int vmIdNum_){vmIdNum = vmIdNum_;}

    bool getSupportRepeatMsg(){return supportRepeatMsg;}
    void setSupportRepeatMsg(const bool supportRepeatMsg_){supportRepeatMsg = supportRepeatMsg_;}

    bool getSupportSerial(){return supportSerial;}
    void setsupportSerial(const bool supportSerial_){supportSerial = supportSerial_;}

    bool getUseCatPTTForEom() const {return useCatPTTForEom;}
    void setUseCatPTTForEom(const bool useCatPTTForEom_){useCatPTTForEom = useCatPTTForEom_;}

    bool getEnableCwMode() const {return enableCwMode;}
    void setEnableCwMode(const bool enableCwMode_){enableCwMode = enableCwMode_;}


    bool getHasPip(){return hasPip;}
    void setHasPip(const bool hasPip_){hasPip = hasPip_;}

    bool getHasTxStatus(){return hasTxStatus;}
    void setHasTxStatus(const bool hasTxStatus_){hasTxStatus = hasTxStatus_;}

    bool getSetupButton(){return setupButton;}
    void setSetupButton(const bool setupButton_){setupButton = setupButton_;}

    bool getHasAvailStatus(){return hasAvailStatus;}
    void setHasAvailStatus(const bool hasAvailStatus_){hasAvailStatus = hasAvailStatus_;}

    bool getHasMessageRepeat(){return hasMessageRepeat;}
    void setHasMessageRepeat(const bool hasMessageRepeat_){hasMessageRepeat = hasMessageRepeat_;}


private:

   int vmIdNum;
   QString keyerName;
   QString keyerType;
   int numVoiceKeys;
   bool supportRepeatMsg =false;
   bool supportSerial = false;
   bool useCatPTTForEom = false;
   bool enableCwMode = false;
   bool setupButton = false;
   bool hasPip = false;
   bool hasTxStatus = false;
   bool hasAvailStatus = false;
   bool hasMessageRepeat = false;

};


class VoiceKeyerFactory : public QObject
{
    Q_OBJECT
public:

    typedef QMap<QString, VoiceKeyerCapabilities> VmKeyers;


    explicit VoiceKeyerFactory(QObject *parent = nullptr);
    ~VoiceKeyerFactory();

    VoiceKeyerBase* createVoiceKeyer(int vmKeyerId);


    void populateComboKeyerList(QComboBox *comBox, QString voiceKeyerName);

    VoiceKeyerFactory::VmKeyers *supportedVoiceKeyers();

signals:

private:
    VmKeyers vmKeyersList;

    QModelIndex extInd;

};

#endif // VOICEKEYERFACTORY_H
