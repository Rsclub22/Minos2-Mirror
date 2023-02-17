#include "SecondInstall.h"

int SecondInstall::installation = -1;
void SecondInstall::parseSecondInstall(int argc, char *argv[])
{
    installation = -1;
    for (int i = 0; i < argc; i++)
    {
        QString arg(argv[i]);
        if (arg.compare("-1") == 0 || arg.compare("/1") == 0)
        {
            installation = 1;
            break;
        }
        if (arg.compare( "-2") == 0 || arg.compare("/2") == 0)
        {
            installation = 2;
            break;
        }
    }
}

QString SecondInstall::getSecondInstallSwitch()
{
    if (installation <= 0)
    {
        return QString();
    }
    return QString("-%1").arg(installation);
}
SecondInstall::SecondInstall()
{

}

QString SecondInstall::getSecondInstallText()
{
    switch(installation)
    {
    case 1:
        return QString();
        break;
    case 2:
        return "Second Installation";
        break;
    default:
        break;
    }

#if defined SECONDINSTALL
    return "Second Installation";
#else
    return QString();
#endif
}

QString SecondInstall::getZConfName()
{
    switch(installation)
    {
    case 1:
        return "minosServer";
        break;
    case 2:
        return "minosBetaServer";
        break;
    default:
        break;
    }
#if defined SECONDINSTALL
    return "minosBetaServer";
#else
    return "minosServer";
#endif
}
QString SecondInstall::getOrgName()
{
    switch(installation)
    {
    case 1:
        return "Minos2Qt";
        break;
    case 2:
        return "Minos2QtBeta";
        break;
    default:
        break;
    }
#if defined SECONDINSTALL
    return "Minos2QtBeta";
#else
    return "Minos2Qt";
#endif
}
QString SecondInstall::getSingleAppLoggerName()
{
    switch(installation)
    {
    case 1:
        return "MinosLogger";
        break;
    case 2:
        return "MinosLoggerBeta";
        break;
    default:
        break;
    }

#if defined SECONDINSTALL
    return "MinosLoggerBeta";
#else
    return "MinosLogger";
#endif
}
QString SecondInstall::getSingleAppRouterName()
{
    switch(installation)
    {
    case 1:
        return "MinosServer";
        break;
    case 2:
        return "MinosServerBeta";
        break;
    default:
        break;
    }
#if defined SECONDINSTALL
    return "MinosServerBeta";
#else
    return "MinosServer";
#endif
}
QString SecondInstall::getRouterEventName()
{
    switch(installation)
    {
    case 1:
        return "MinosQtServer";
        break;
    case 2:
        return "MinosQtServerBeta";
        break;
    default:
        break;
    }
#if defined SECONDINSTALL
    return "MinosQtServerBeta";
#else
    return "MinosQtServer";
#endif
}
QString SecondInstall::getSingleAppStarterName()
{
    switch(installation)
    {
    case 1:
        return "MinosAppStarter";
        break;
    case 2:
        return "MinosAppStarterBeta";
        break;
    default:
        break;
    }
#if defined SECONDINSTALL
    return "MinosAppStarterBeta";
#else
    return "MinosAppStarter";
#endif
}

QString SecondInstall::getSingleAppLoggerDescription()
{
    switch(installation)
    {
    case 1:
        return "Minos Logger";
        break;
    case 2:
        return "Minos Logger (Beta)";
        break;
    default:
        break;
    }

#if defined SECONDINSTALL
    return "Minos Logger (Beta)";
#else
    return "Minos Logger";
#endif
}
QString SecondInstall::getSingleAppRouterDescription()
{
    switch(installation)
    {
    case 1:
        return "Minos Server";
        break;
    case 2:
        return "Minos Server (Beta)";
        break;
    default:
        break;
    }
#if defined SECONDINSTALL
    return "Minos Server (Beta)";
#else
    return "Minos Server";
#endif
}
QString SecondInstall::getSingleAppStarterDescription()
{
    switch(installation)
    {
    case 1:
        return "Minos App Starter";
        break;
    case 2:
        return "Minos App Starter (Beta)";
        break;
    default:
        break;
    }
#if defined SECONDINSTALL
    return "Minos App Starter (Beta)";
#else
    return "Minos App Starter";
#endif
}

qint16 SecondInstall::getClientPort()
{
    switch(installation)
    {
    case 1:
        return 7777;
        break;
    case 2:
        return 7775;
        break;
    default:
        break;
    }
#if defined SECONDINSTALL
    return 7775;
#else
    return 7777;
#endif
}
qint16 SecondInstall::getRouterPort()
{
    switch(installation)
    {
    case 1:
        return 7778;
        break;
    case 2:
        return 7776;
        break;
    default:
        break;
    }
#if defined SECONDINSTALL
    return 7776;
#else
    return 7778;
#endif
}
QString SecondInstall::getLineMapFileName()
{
    switch(installation)
    {
    case 1:
        return "LineMapFile";
        break;
    case 2:
        return "LineMapBetaFile";
        break;
    default:
        break;
    }
#if defined SECONDINSTALL
    return "LineMapBetaFile";
#else
    return "LineMapFile";
#endif
}
