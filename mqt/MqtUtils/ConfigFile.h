#ifndef CONFIGFILE_H
#define CONFIGFILE_H
#include <QWidget>
#include <QProcess>
#include "INIFile.h"
//---------------------------------------------------------------------------
extern QString RunLocal;
extern QString ConnectServer;

class AppConfigElement
{
public:
    AppConfigElement(){}

    QString appType;
    QString appPath;
    QStringList requires;
    bool server = false;
    bool localOK = false;
    bool remoteOK = false;
    bool defaultHide = false;

};

class Connectable
{
public:
    Connectable(){}

    QString serverName;
    QString remoteAppName;
    QString appName;
    QString runType;
    QString appType;
};

class RunConfigElement: public QObject
{
    Q_OBJECT
private:  	// User declarations
    QProcess *runner = 0;
public:  		// User declarations
    bool newElement = false;
    bool deleted = false;
    QString name;
    QString rundir;
    QString commandLine;
    QString params;
    QString server;
    QString remoteApp;
    bool localOK = false;
    bool remoteOK = false;

    QString runType;
    QString appType;

    QStringList requires;

    bool showAdvanced = false;
    bool rEnabled = false;
    bool hideApp = false;

    bool stopping = false;

    RunConfigElement(){}
    bool initialise( INIFile &, QString sect );

    void save(INIFile &);

    Connectable connectable();

    void createProcess();
    void stopProcess();
    void sendCommand(const QString & cmd);
private slots:
    void	on_started();
    void    on_finished(int, QProcess::ExitStatus exitStatus);
    void	on_error(QProcess::ProcessError error);

    void	on_readyReadStandardError();
    void	on_readyReadStandardOutput();
};
class MinosConfig : public QObject
{
    Q_OBJECT
private:  	// User declarations
    static MinosConfig *thisDM;
    static QString getConfigIniName();
    MinosConfig();

    INIFile config;
    void initialise();

    QVector<AppConfigElement> appConfigList;

    void buildAppConfigList();

    QString thisServerName;
    bool autoStart;

public:  		// User declarations
    static MinosConfig *getMinosConfig( );

    ~MinosConfig();

    QVector <QSharedPointer<RunConfigElement> > elelist;

    QString getThisServerName();

    QStringList getAppTypes();
    Connectable getApp(QString appName);
    AppConfigElement getAppConfigElement(QString appType);

    void cleanElementsOnCancel();
    void saveAll();

    void start();
    void stop();
    void setThisServerName( const QString &circle );

    bool getAutoStart();
    void setAutoStart(bool);

    QString checkConfig();

};
//---------------------------------------------------------------------------

#endif // CONFIGFILE_H
