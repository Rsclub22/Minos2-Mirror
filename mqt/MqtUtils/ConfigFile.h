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
    QStringList requiresApps;
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
    QProcess *runner = nullptr;
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

    QStringList requiresApps;

    bool showAdvanced = false;
    bool rEnabled = false;
    bool hideApp = false;

    bool stopping = false;

    RunConfigElement();
    ~RunConfigElement();
    bool initialise( INIFile &, QString sect );

    void save(INIFile &);

    QSharedPointer<Connectable> connectable();

    void createProcess();
    void stopProcess();
    void bounceProcess();
    void sendCommand(const QString & cmd);
    bool isRunning()
    {
        return runner != nullptr;
    }
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

    void reset();

    QVector <QSharedPointer<RunConfigElement> > elelist;

    QString getThisServerName();

    QStringList getAppTypes();
    QSharedPointer<Connectable> getApp(QString appName);
    AppConfigElement getAppConfigElement(QString appType);

//    void cleanElementsOnCancel();
    void saveAll();

    void setThisServerName( const QString &circle );

    bool getAutoStart();
    void setAutoStart(bool);

    QString checkConfig();
    bool anyRunning();

    void checkAllStopped();
    void bounce();
    void start();
    void stop();

signals:
    void    stdOutLine(QString);
    void    allStopped();
};
//---------------------------------------------------------------------------

#endif // CONFIGFILE_H
