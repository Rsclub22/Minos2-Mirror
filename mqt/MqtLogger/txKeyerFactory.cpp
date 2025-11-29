/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Tx Keyer Factory
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2025
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include <QComboBox>
#include <QStandardItemModel>
#include <QListView>
#include "AppStartup.h"
#include "txKeyerFactory.h"
#include "txkeyerCommonConstants.h"
#include "rigcontrolvoicetxkeyer.h"
#include "rigcontrolcwtxkeyer.h"
#include "pccwmessagekeyer.h"
#include "InternalVoiceTxKeyer.h"
#include "ExternalMqtKeyer.h"
#include "winkeyertxkeyer.h"
#include "tlogcontainer.h"
#include "SendRPCDM.h"

using namespace TxKeyerCommon;

TxKeyerFactory::TxKeyerFactory(QObject *parent) : QObject(parent)
{
    RigControlVoiceTxKeyer::registerTxKeyer(&txKeyersList);
    RigControlCwTxKeyer::registerTxKeyer(&txKeyersList);
    PcCWMessageKeyer::registerTxKeyer(&txKeyersList);
    InternalVoiceTxKeyer::registerTxKeyer(&txKeyersList);
    ExternalMqtKeyer::registerTxKeyer(&txKeyersList);
    WinKeyerTxKeyer::registerTxKeyer(&txKeyersList);
}


TxKeyerFactory::~TxKeyerFactory()
{

}

TxKeyerFactory::TxKeyers* TxKeyerFactory::supportedTxKeyers()
{
    return &txKeyersList;
}


TxKeyerBase* TxKeyerFactory::createTxKeyer(int txKeyerId)
{
    if (txKeyerId == TxKeyerId::RigControl)
    {        
        return new RigControlVoiceTxKeyer(this);
    }
    else if (txKeyerId == TxKeyerId::CW_RigControl)
    {
        return new RigControlCwTxKeyer(this);
    }
    else if (txKeyerId == TxKeyerId::PcCwKeyer)
    {
        //if (LogContainer->sendDM->isPcCWkeyerLoaded())
       // {
       return new PcCWMessageKeyer(this);
       // }
    }
    else if (txKeyerId == TxKeyerId::InternalVoiceKeyer)
    {
        return new InternalVoiceTxKeyer(this);
    }
    else if (txKeyerId == TxKeyerId::ExternalMqtKeyer)
    {
        /*if (LogContainer->sendDM->isKeyerLoaded())
        {
            return new ExternalMqtKeyer(this);
        }*/
    }
    else if (txKeyerId == TxKeyerId::WinKeyer)
    {
        return new WinKeyerTxKeyer(this);
    }

    return nullptr;
}

void TxKeyerFactory::populateComboKeyerList(QComboBox* comBox, QString txKeyerName)
{
    if (comBox->count())
    {
        int row = -1;
        int i = 1;
        for (auto r = supportedTxKeyers()->cbegin(); r != supportedTxKeyers()->cend(); ++r)
        {
            if (r.key() == ExternalMqtKeyer::keyerName)
            {
                row = i;
            }
            i++;
        }
        extInd = comBox->model()->index(row, 0);
        qobject_cast<QListView *>(comBox->view())->setRowHidden(extInd.row(), !LogContainer->sendDM->isKeyerLoaded());
    }
    else
    {
        comBox->clear();
        comBox->addItem("");
        int row = -1;
        int i = 1;
        for (auto r = supportedTxKeyers()->cbegin(); r != supportedTxKeyers()->cend(); ++r)
        {
            if (r.key() == ExternalMqtKeyer::keyerName)
            {
                row = i;
            }
            QString vmText = r.key();
            comBox->addItem(vmText);
            i++;
        }
        extInd = comBox->model()->index(row, 0);
        bool enableExt = !(LogContainer->sendDM->isKeyerLoaded() || txKeyerName == ExternalMqtKeyer::keyerName);
        qobject_cast<QListView *>(comBox->view())->setRowHidden(extInd.row(), enableExt);

        comBox->setCurrentText(txKeyerName);
    }
}





