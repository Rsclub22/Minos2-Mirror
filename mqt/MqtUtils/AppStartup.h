#ifndef APPSTARTUP_H
#define APPSTARTUP_H

#include <QApplication>
#include <QFont>

class Translation
{
public:
    QString code;
    QString dispName;
};

extern void appStartup(const QString &appName);
extern QString getAppExecutable();
extern QString getAppExecutableName();
extern QString getAppStartupName();
extern QVector<Translation> getLanguages();
extern void switchTranslation(QString loc);
extern QString getCurrentLanguage();
extern void setAppFont();
void setAppFont(QString fs);
extern void setAppClosing();
extern bool cpDir(const QString &srcPath, const QString &dstPath);
extern void setAppLanguage(QString loc);

enum DirectoryLocation {dlBinaries
                         ,dlTranslations
                         ,dlConfiguration
                         ,dlLists
                         ,dlLogs
                         ,dlDocs
                         ,dlTraceLog
                         ,dlDB
};

extern QString getDirectoryLocation(DirectoryLocation, QString runDir = ".");
extern void setDefLogDir(QString l);
extern void setDefListDir(QString l);

class AppStart:public QObject
{
    Q_OBJECT
public:
    AppStart(){}
    void emitFontChanged();

    void emitListCompressionChanged(qreal hmult);
signals:
    void fontChanged();
    void listCompressionChanged(qreal hmult);
};

extern AppStart appStart;
#endif // APPSTARTUP_H
