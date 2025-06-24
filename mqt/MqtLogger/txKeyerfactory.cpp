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
#include "txKeyerfactory.h"
#include "rigcontrolvoicememorykeyer.h"
#include "rigcontrolcwmessagekeyer.h"
#include "pccwmessagekeyer.h"
#include "InternalVoiceMemoryKeyer.h"
#include "ExternalMqtKeyer.h"
#include "tlogcontainer.h"
#include "SendRPCDM.h"

using namespace TxKeyerCommon;

TxKeyerFactory::TxKeyerFactory(QObject *parent) : QObject(parent)
{
    RigControlVoiceMemoryKeyer::registerVoiceKeyer(&txKeyersList);
    RigControlCwMessageKeyer::registerVoiceKeyer(&txKeyersList);
    PcCWMessageKeyer::registerVoiceKeyer(&txKeyersList);
    InternalVoiceMemoryKeyer::registerVoiceKeyer(&txKeyersList);
    ExternalMqtKeyer::registerVoiceKeyer(&txKeyersList);
}


TxKeyerFactory::~TxKeyerFactory()
{

}

TxKeyerFactory::TxKeyers* TxKeyerFactory::supportedTxKeyers()
{
    return &txKeyersList;
}


TxKeyerFactory* TxKeyerFactory::createTxKeyer(int txKeyerId)
{
    if (txKeyerId == TxKeyerId::RigControl)
    {        
        return new RigControlVoiceMemoryKeyer(this);
    }
    else if (txKeyerId == TxKeyerId::CW_RigControl)
    {
        return new RigControlCwMessageKeyer(this);
    }
    else if (txKeyerId == TxKeyerId::PcCwKeyer)
    {
        return new PcCWMessageKeyer(this);
    }
    else if (txKeyerId == TxKeyerId::InternalVoiceKeyer)
    {
        return new InternalVoiceMemoryKeyer(this);
    }
    else if (txKeyerId == TxKeyerId::ExternalVoiceKeyer)
    {
        if (LogContainer->sendDM->isKeyerLoaded())
        {
            return new ExternalMqtKeyer(this);
        }
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





