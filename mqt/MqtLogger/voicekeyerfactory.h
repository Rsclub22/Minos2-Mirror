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

#ifndef VOICEKEYERFACTORY_H
#define VOICEKEYERFACTORY_H

#include <QObject>
#include "voicekeyerbase.h"

class QComboBox;

enum VoiceKeyerId
{
    None = 0,
    RigControl,
    SerialControl
};

const QStringList keyerTypes = {"None", "RigControl", "SerialControl"};

const QString VOICE_KEYER_PATH = "./Configuration/VoiceKeyer/";
const QString VOICE_KEYER_BASE_FILE_NAME = "txVoiceMemory";
const QString VOICEKEYER_COMMON_PARAMS_PATH = VOICE_KEYER_PATH + "CommonParams/";
const QString VOICEKEYER_COMMON_PARAMS_FILENAME = "txVoiceKeyCommonParams.ini";
const QString VOICEKEYER_COMMON_PARAMS_GROUPNAME = "commonParams";

const int VOICEKEYER_MAX_NUMBUTTONS = 8;

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

    bool getSetupButton(){return setupButton;}
    void setSetupButton(const bool setupButton_){setupButton = setupButton_;}


private:

   int vmIdNum;
   QString keyerName;
   QString keyerType;
   int numVoiceKeys;
   QString comPort;
   bool supportRepeatMsg;
   bool supportSerial;
   bool setupButton;

};


class VoiceKeyerFactory : public QObject
{
    Q_OBJECT
public:

    typedef QMap<QString, VoiceKeyerCapabilities> VmKeyers;

    explicit VoiceKeyerFactory(QObject *parent = nullptr);
    ~VoiceKeyerFactory();

    VoiceKeyerBase* createVoiceKeyer(int vmKeyerId);


    void populateComboKeyerList(QComboBox *comBox);

    VoiceKeyerFactory::VmKeyers *supportedVoiceKeyers();

signals:

private:
    VmKeyers vmKeyersList;

};

#endif // VOICEKEYERFACTORY_H
