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
    RigControl,
    SerialControl
};


class VoiceKeyerCapabilities
{
public:

    VoiceKeyerCapabilities(){};

    QString getKeyerName(){return keyerName;}
    void setKeyerName(QString keyerName_){keyerName = keyerName_;}

    int getNumVoiceKeys(){return numVoiceKeys;}
    void setNumVoiceKeys(int numVoiceKeys_){numVoiceKeys = numVoiceKeys_;}

    int getVmIdNum(){return vmIdNum;}
    void setVmIdNum(int vmIdNum_){vmIdNum = vmIdNum_;}

    bool getSupportSerial(){return supportSerial;}
    void setSupportSerial(bool supportSerial_){supportSerial = supportSerial_;}

    bool getSupportRepeatMsg(){return supportRepeatMsg;}
    void setSupportRepeatMsg(bool supportRepeatMsg_){supportRepeatMsg = supportRepeatMsg_;}

    bool getSetupButton(){return setupButton;}
    void setSetupButton(bool setupButton_){setupButton = setupButton_;}


    QString getComPort(){return comPort;}
    void setComPort(QString comPort_){comPort_ = comPort;}

    QString getComSpeed(){return comSpeed;}
    void setComSpeed(QString comSpeed_){comSpeed_ = comSpeed;}
private:

   int vmIdNum;
    QString keyerName;
   int numVoiceKeys;
   QString comPort;
   QString comSpeed;
   bool supportSerial;
   bool supportRepeatMsg;
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
