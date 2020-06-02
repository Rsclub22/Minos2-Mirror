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

#include <QSettings>
#include <QFileInfo>
#include "MTrace.h"
#include "rigfactory.h"
#include "hamlibrigcontrol.h"
#if defined (WIN32)
#include "omnirigcontrol.h"
#endif
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
#if defined (WIN32)
    if (rigId == OmniRigOneId)
    {
        return new OmnirigControl(OmnirigControl::One, this);
    }
    else if (rigId == OmniRigTwoId)
    {
        return new OmnirigControl(OmnirigControl::Two, this);
    }
    else
#endif
    if (rigId <= HamlibRigCtld)
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
    QString omnirigFilePathx86 = config.value("omnirigPathx86", "C:/Program Files (x86)/Afreet/OmniRig/").toString();
    QString omnirigFilePath = config.value("omnirigPath", "C:/Program Files/Afreet/OmniRig/").toString();

    QString omnirigFileName = config.value("omnirigExe", "OmniRig.exe").toString();
    config.endGroup();

    QString fileNamex86 = omnirigFilePathx86 + omnirigFileName;
    trace(QString("looking for Omnirig here - %1").arg(fileNamex86));

    bool fileExistsx86 = QFileInfo::exists(fileNamex86) && QFileInfo(fileNamex86).isFile();
    trace(QString("Omnirig found here %1 = %2").arg(fileNamex86).arg(fileExistsx86 ? "Yes" : "No"));

    bool fileExists = false;
    if (!fileExistsx86)
    {
        fileName = omnirigFilePath + omnirigFileName;
        trace(QString("looking for Omnirig here - %1").arg(fileName));
        fileExists = QFileInfo::exists(fileName) && QFileInfo(fileName).isFile();
        trace(QString("Omnirig found here %1 = %2").arg(fileName).arg(fileExists ? "Yes" : "No"));
    }

    if (fileExists || fileExistsx86)
    {
        return true;
    }
    return false;
}
