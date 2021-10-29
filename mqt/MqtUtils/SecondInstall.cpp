#include "SecondInstall.h"

SecondInstall::SecondInstall()
{

}

QString SecondInstall::getSecondInstallText()
{
#if defined SECONDINSTALL
    return "Second Installation";
#else
    return QString();
#endif
}

QString SecondInstall::getZConfName()
{
#if defined SECONDINSTALL
    return "minosBetaServer";
#else
    return "minosServer";
#endif
}
QString SecondInstall::getOrgName()
{
#if defined SECONDINSTALL
    return "Minos2QtBeta";
#else
    return "Minos2Qt";
#endif
}
QString SecondInstall::getSingleAppLoggerName()
{
#if defined SECONDINSTALL
    return "MinosLoggerBeta";
#else
    return "MinosLogger";
#endif
}
QString SecondInstall::getSingleAppRouterName()
{
#if defined SECONDINSTALL
    return "MinosServerBeta";
#else
    return "MinosServer";
#endif
}
QString SecondInstall::getRouterEventName()
{
#if defined SECONDINSTALL
    return "MinosQtServerBeta";
#else
    return "MinosQtServer";
#endif
}

QString SecondInstall::getSingleAppLoggerDescription()
{
#if defined SECONDINSTALL
    return "Minos Logger (Beta)";
#else
    return "Minos Logger";
#endif
}
QString SecondInstall::getSingleAppRouterDescription()
{
#if defined SECONDINSTALL
    return "Minos Server (Beta)";
#else
    return "Minos Server";
#endif
}

qint16 SecondInstall::getClientPort()
{
#if defined SECONDINSTALL
    return 7775;
#else
    return 7777;
#endif
}
qint16 SecondInstall::getRouterPort()
{
#if defined SECONDINSTALL
    return 7776;
#else
    return 7778;
#endif
}
QString SecondInstall::getLineMapFileName()
{
#if defined SECONDINSTALL
    return "LineMapBetaFile";
#else
    return "LineMapFile";
#endif
}
