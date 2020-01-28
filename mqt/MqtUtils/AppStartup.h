#ifndef APPSTARTUP_H
#define APPSTARTUP_H

#include <QApplication>

class Translation
{
public:
    QString code;
    QString dispName;
};

extern void appStartup(const QString &appName);
extern QString getAppStartupName();
extern QVector<Translation> getLanguages();
extern void switchTranslation(QString loc);
extern QString getCurrentLanguage();
extern void setAppFont();
extern void setAppClosing();

#endif // APPSTARTUP_H
