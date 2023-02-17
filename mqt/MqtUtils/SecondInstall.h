#ifndef SECONDINSTALL_H
#define SECONDINSTALL_H

#include <QString>

class SecondInstall
{
    static int installation;
public:
    static void parseSecondInstall(int argc, char *argv[]);
    static QString getSecondInstallSwitch();

    SecondInstall();

    static QString getSecondInstallText();

    static QString getOrgName();
    static QString getSingleAppLoggerName();
    static QString getSingleAppRouterName();
    static QString getSingleAppStarterName();
    static QString getSingleAppLoggerDescription();
    static QString getSingleAppRouterDescription();
    static QString getSingleAppStarterDescription();
    static qint16 getClientPort();
    static qint16 getRouterPort();
    static QString getZConfName();

    static QString getRouterEventName();
    static QString getLineMapFileName();

};

#endif // SECONDINSTALL_H
