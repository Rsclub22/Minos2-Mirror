#ifndef SECONDINSTALL_H
#define SECONDINSTALL_H

#include <QString>

class SecondInstall
{
public:
    SecondInstall();

    static QString getSecondInstallText();

    static QString getOrgName();
    static QString getSingleAppLoggerName();
    static QString getSingleAppRouterName();
    static QString getSingleAppLoggerDescription();
    static QString getSingleAppRouterDescription();
    static qint16 getClientPort();
    static qint16 getRouterPort();
    static QString getZConfName();

    static QString getRouterEventName();
    static QString getLineMapFileName();

};

#endif // SECONDINSTALL_H
