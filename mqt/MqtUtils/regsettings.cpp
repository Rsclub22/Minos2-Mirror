#include <QSettings>
#include "AppStartup.h"
#include "SecondInstall.h"
#include "fileutils.h"
#include "regsettings.h"

QSettings *RegSettings::s = nullptr;

RegSettings::RegSettings()
{
    if (!s)
    {
        QString oname = SecondInstall::getOrgName();
        QString appName = getAppStartupName();
        QString fileName = getDirectoryLocation(dlConfiguration) + "/" + oname + "_" + appName + "_reg.ini";
        s = new QSettings(fileName, QSettings::IniFormat);
    }
    s->sync();
}

RegSettings::~RegSettings()
{
    s->sync();
}

QSettings &RegSettings::getSettings()
{
    return *s;
}
