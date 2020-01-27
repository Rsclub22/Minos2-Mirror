#ifndef APPSTARTUP_H
#define APPSTARTUP_H

#include <QApplication>

extern void appStartup(const QString &appName);
extern QString getAppStartupName();
extern QStringList getLanguages();
extern void switchTranslation(QString loc);
extern void setAppFont();
extern void setAppClosing();

#endif // APPSTARTUP_H
