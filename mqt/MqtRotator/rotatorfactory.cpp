/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
//
//
/////////////////////////////////////////////////////////////////////////////

#include "rotatorfactory.h"
#include "hamlibRotcontrol.h"


#if defined (WIN32)
#include "pstRotControl.h"
#endif

RotatorFactory::RotatorFactory(bool tracecommFlag, QObject *parent) : QObject(parent)
{

    trace(QString("RotatorFactory: adding hamlib rotators"));
    HamlibRotControl::setTraceCommsFlag(tracecommFlag);
    HamlibRotControl::register_rotators(&rotatorsList);


#if defined (WIN32)
    // PSTRotator is only on Windows
    trace(QString("RotatorFactory: adding PSTRotator"));
    if (checkPstRotatorInstalled())
    {
       trace("PSTRotator installed - add to rotator list");
        PstRotControl::register_rotators(&rotatorsList, RotId::PSTRotatorId);
    }
    else
    {
        trace(QString("PSTRotator is not installed"));
    }

#endif

}

RotatorFactory::~RotatorFactory()
{

}


RotatorFactory::Rotators* RotatorFactory::supported_rotators()
{
    return &rotatorsList;
}


RotatorBase* RotatorFactory::createRotator(int rotatorId)
{
#if defined (WIN32)

    if (rotatorId == PSTRotatorId)
    {
        return new PstRotControl();
    }
    else
#endif
    {
        return new HamlibRotControl();
    }
}


bool RotatorFactory::checkPstRotatorInstalled()
{

    QString fileName = RADIO_PATH_LOGGER + MINOS_ROTATOR_CONFIG_FILE;
    QSettings  config(fileName, QSettings::IniFormat);

    config.beginGroup("PSTRotator");
    QString pstRotatorFilePathx86 = config.value("pstRotatorPathx86", "C:/Program Files (x86)/PstRotator/").toString();
    QString pstRotatorFilePath = config.value("pstRotatorPath", "C:/Program Files/PstRotator/").toString();

    QString pstRotatorFileName = config.value("pstRotatorExe", "PstRotator.exe").toString();

    QString pstRotatorAzFilePathx86 = config.value("pstRotatorAzPathx86", "C:/Program Files (x86)/PstRotatorAz/").toString();
    QString pstRotatorAzFilePath = config.value("pstRotatorAzPath", "C:/Program Files/PstRotatorAz/").toString();


    QString pstRotatorAzFileName = config.value("pstRotatorAzExe", "PstRotatorAz.exe").toString();
    config.endGroup();

    QString pstFileNamex86 = pstRotatorFilePathx86 + pstRotatorFileName;
    bool pstRotatorfileExistsx86;

    checkPstRotatorExists(pstFileNamex86, pstRotatorfileExistsx86);

    QString pstFileName = pstRotatorFilePath + pstRotatorFileName;
    bool pstRotatorfileExists;

    checkPstRotatorExists(pstFileName, pstRotatorfileExists);

    QString pstAzFileNamex86 = pstRotatorAzFilePathx86 + pstRotatorAzFileName;
    bool pstRotatorAzfileExistsx86;

    checkPstRotatorExists(pstAzFileNamex86, pstRotatorAzfileExistsx86);

    QString pstAzFileName = pstRotatorAzFilePath + pstRotatorAzFileName;
    bool pstRotatorAzfileExists;

    checkPstRotatorExists(pstAzFileName, pstRotatorAzfileExists);


    if (pstRotatorfileExistsx86 || pstRotatorfileExists || pstRotatorAzfileExistsx86 || pstRotatorAzfileExists)
    {
        return true;
    }
    return false;
}


void RotatorFactory::checkPstRotatorExists(QString fileName, bool &fileExists)
{

    trace(QString("looking for PstRotator here %1").arg(fileName));
    fileExists = QFileInfo::exists(fileName) && QFileInfo(fileName).isFile();
    trace(QString("PstRotator found here %1 = %2").arg(fileName).arg(fileExists ? "Yes" : "No"));

}
