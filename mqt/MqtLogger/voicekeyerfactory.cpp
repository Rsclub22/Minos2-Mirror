/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Voice Keyer
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2021
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include <QComboBox>
#include <QStandardItemModel>
#include <QListView>
#include "AppStartup.h"
#include "voicekeyerfactory.h"
#include "rigcontrolvoicememorykeyer.h"
#include "rigcontrolcwmessagekeyer.h"
#include "InternalVoiceMemoryKeyer.h"
#include "ExternalMqtKeyer.h"
#include "tlogcontainer.h"
#include "SendRPCDM.h"

VoiceKeyerFactory::VoiceKeyerFactory(QObject *parent) : QObject(parent)
{
    RigControlVoiceMemoryKeyer::registerVoiceKeyer(&vmKeyersList);
    RigControlCwMessageKeyer::registerVoiceKeyer(&vmKeyersList);
    InternalVoiceMemoryKeyer::registerVoiceKeyer(&vmKeyersList);
    ExternalMqtKeyer::registerVoiceKeyer(&vmKeyersList);
}


VoiceKeyerFactory::~VoiceKeyerFactory()
{

}

VoiceKeyerFactory::VmKeyers* VoiceKeyerFactory::supportedVoiceKeyers()
{
    return &vmKeyersList;
}


VoiceKeyerBase* VoiceKeyerFactory::createVoiceKeyer(int vmKeyerId)
{
    if (vmKeyerId == VoiceKeyerId::RigControl)
    {        
        return new RigControlVoiceMemoryKeyer(this);
    }
    else if (vmKeyerId == VoiceKeyerId::CW_RigControl)
    {
        return new RigControlCwMessageKeyer(this);
    }
    else if (vmKeyerId == VoiceKeyerId::InternalVoiceKeyer)
    {
        return new InternalVoiceMemoryKeyer(this);
    }
    else if (vmKeyerId == VoiceKeyerId::ExternalVoiceKeyer)
    {
        if (LogContainer->sendDM->isKeyerLoaded())
        {
            return new ExternalMqtKeyer(this);
        }
    }

    return nullptr;
}

void VoiceKeyerFactory::populateComboKeyerList(QComboBox* comBox, QString voiceKeyerName)
{
    if (comBox->count())
    {
        int row = -1;
        int i = 1;
        for (auto r = supportedVoiceKeyers()->cbegin(); r != supportedVoiceKeyers()->cend(); ++r)
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
        for (auto r = supportedVoiceKeyers()->cbegin(); r != supportedVoiceKeyers()->cend(); ++r)
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
        bool enableExt = !(LogContainer->sendDM->isKeyerLoaded() || voiceKeyerName == ExternalMqtKeyer::keyerName);
        qobject_cast<QListView *>(comBox->view())->setRowHidden(extInd.row(), enableExt);

        comBox->setCurrentText(voiceKeyerName);
    }
}

QString VOICE_KEYER_PATH()
{
    return getDirectoryLocation(dlConfiguration) + "/VoiceKeyer/";
}

QString VOICEKEYER_COMMON_PARAMS_PATH()
{
    return VOICE_KEYER_PATH() + "CommonParams/";
}
