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

#include "rigfactory.h"
#include "hamlibrigcontrol.h"
#include "omnirigcontrol.h"

RigFactory::RigFactory(bool tracecommFlag, QObject *parent) : QObject(parent)
{
    HamlibRigControl::setTraceCommsFlag(tracecommFlag);
    HamlibRigControl::register_rigs(&rigsList);


#if defined (WIN32)
    // Omnirig is only on Windows
    if (checkOmniRigInstalled())
    {
        trace("OmniRig installed - add to riglist");
        OmnirigControl::register_rigs(&rigsList, RigId::OmniRigOneId, RigId::OmniRigTwoId);
    }
    else
    {
        trace(QString("OmniRig is not installed"));
    }
#endif

}


RigFactory::~RigFactory()
{

}

RigFactory::Rigs* RigFactory::supported_rigs()
{
    return &rigsList;
}

RigBase* RigFactory::createRigs(int rigId)
{
    if (rigId == OmniRigOneId)
    {
        return new OmnirigControl(OmnirigControl::One, this);
    }
    else if (rigId == OmniRigTwoId)
    {
        return new OmnirigControl(OmnirigControl::Two, this);
    }
    else if (rigId <= HamlibRigCtld)
    {
        return new HamlibRigControl(this);
    }

    return nullptr;



}


bool RigFactory::checkForBands(int rigNumber, Frequency freq)
{
    if (rigNumber == OmniRigOneId)
    {

    }
    else if (rigNumber == OmniRigTwoId)
    {

    }

    return HamlibRigControl::checkFreqRange(rigNumber, freq);
}


void RigFactory::populateComboRigList(QComboBox* comBox )
{
    comBox->clear();
    comBox->addItem("");
    for (auto r = supported_rigs()->cbegin(); r != supported_rigs()->cend(); ++r)
    {
        QString rigText = r.key();
        if (!rigText.contains("rigctl"))
        {
            comBox->addItem(rigText);
        }

    }
}


bool RigFactory::checkOmniRigInstalled()
{

    QString fileName = RADIO_PATH_LOGGER + MINOS_RADIO_CONFIG_FILE;
    QSettings  config(fileName, QSettings::IniFormat);

    config.beginGroup("Omnirig");
    QString omnirigFilePath = config.value("omnirigPath", "C:/Program Files (x86)/Afreet/OmniRig/").toString();
    QString omnirigFileName = config.value("omnirigExe", "OmniRig.exe").toString();
    config.endGroup();

    fileName = omnirigFilePath + omnirigFileName;
    trace(QString("looking for Omnirig here - %1").arg(fileName));

    bool fileExists = QFileInfo::exists(fileName) && QFileInfo(fileName).isFile();
    trace(QString("Omnirig found = %1").arg(fileExists ? "true":"false"));
    return fileExists;
}
