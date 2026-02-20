/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      TX Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2026
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef TXKEYERFACTORY_H
#define TXKEYERFACTORY_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QModelIndex>
#include "txKeyerbase.h"
#include "txkeyerCommonConstants.h"





class QComboBox;


class TxKeyerCapabilities
{
public:

    TxKeyerCapabilities(){clear();};

    QString getKeyerName(){return keyerName;}
    void setKeyerName(const QString keyerName_){keyerName = keyerName_;}

    int getNumVoiceKeys(){return numVoiceKeys;}
    void setNumVoiceKeys(const int numVoiceKeys_){numVoiceKeys = numVoiceKeys_;}


    TxKeyerCommon::TxKeyerId getTxKeyerIdNum(){return txKeyerId;}
    void setTxKeyerId(const TxKeyerCommon::TxKeyerId txKeyerId_){txKeyerId = txKeyerId_;}

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

    void clear()
    {
        txKeyerId = TxKeyerCommon::TxKeyerId::None;
        keyerName.clear();
        numVoiceKeys = 0;
        supportRepeatMsg = false;
        supportSerial = false;
        useCatPTTForEom = false;
        enableCwMode = false;
        setupButton = false;
        hasPip = false;
        hasTxStatus = false;
        hasAvailStatus = false;
        hasMessageRepeat = false;

    }


private:

   TxKeyerCommon::TxKeyerId txKeyerId;
   QString keyerName;
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


class TxKeyerFactory : public QObject
{
    Q_OBJECT
public:

    typedef QMap<QString, TxKeyerCapabilities> TxKeyers;


    explicit TxKeyerFactory(QObject *parent = nullptr);
    ~TxKeyerFactory();

    TxKeyerBase* createTxKeyer(int vmKeyerId);


    void populateComboKeyerList(QComboBox *comBox, QString txKeyerName);

    TxKeyerFactory::TxKeyers *supportedTxKeyers();

signals:

private:
    TxKeyers txKeyersList;


    QModelIndex extInd;

};

#endif // TXKEYERFACTORY_H
