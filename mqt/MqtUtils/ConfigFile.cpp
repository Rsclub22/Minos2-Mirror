#include "mqtUtils_pch.h"

#include <QTimer>
#include <QHostInfo>
#include <QSharedPointer>
#include "fileutils.h"
#include "ConfigFile.h"


//---------------------------------------------------------------------------

bool terminated = false;

QString RunLocal("RunLocal");
QString ConnectServer("ConnectServer");


/*static*/
MinosConfig *MinosConfig::thisDM = 0;
/*static*/
MinosConfig *MinosConfig::getMinosConfig()
{
   if ( !thisDM )
   {
      thisDM = new MinosConfig();
      thisDM->initialise();
   }
   return thisDM;
}
/*static*/
QString MinosConfig::getConfigIniName()
{
    return "./Configuration/MinosConfig.ini";
}

QString MinosConfig::getThisServerName()
{
    QString serverName;
    config.getPrivateProfileString( "Settings", "ServerName", QHostInfo::localHostName(), serverName );

    if ( serverName.size() == 0 )
    {
        QString h = QHostInfo::localHostName();
        serverName = h;
    }
    return serverName;
}
void MinosConfig::cleanElementsOnCancel()
{
    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = elelist.begin(); i != elelist.end(); i++ )
    {
        QSharedPointer<RunConfigElement> ele = (*i);
        if (ele->newElement)
        {
            ele->deleted = true;
        }
    }
}

//---------------------------------------------------------------------------
bool RunConfigElement::initialise(INIFile &config, QString sect )
{
    // config should refer to ./Configuration/MinosConfig.ini

    name = sect;

    config.getPrivateProfileString(sect, "Program", "", commandLine);
    config.getPrivateProfileString( sect, "Server", "localhost", server );
    config.getPrivateProfileString( sect, "Params", "", params );
    config.getPrivateProfileString( sect, "Directory", "", rundir );
    config.getPrivateProfileString( sect, "RemoteApp", "", remoteApp);
    showAdvanced = config.getPrivateProfileBool(sect, "ShowAdvanced", false);
    rEnabled = config.getPrivateProfileBool(sect, "Enabled", false);
    hideApp = config.getPrivateProfileBool(sect, "HideApp", false);
    config.getPrivateProfileString( sect, "RunType",  RunLocal, runType );
    config.getPrivateProfileString( sect, "AppType",  "", appType );

    AppConfigElement ace = MinosConfig::getMinosConfig()->getAppConfigElement(appType);
    requires = ace.requires;
    localOK = ace.localOK;
    remoteOK = ace.remoteOK;

    return true;
}
void RunConfigElement::save(INIFile &config)
{
    newElement = false;

    if (name.isEmpty())
    {
        name = appType;
    }

    if (!deleted)
    {
        config.writePrivateProfileString(name, "Program", commandLine);
        config.writePrivateProfileString(name, "Params", params);
        config.writePrivateProfileString(name, "Directory", rundir);
        config.writePrivateProfileString(name, "Server", server);
        config.writePrivateProfileString(name, "RemoteApp", remoteApp);
        config.writePrivateProfileString(name, "RunType", runType);
        config.writePrivateProfileString(name, "AppType", appType);
        config.writePrivateProfileBool(name, "ShowAdvanced", showAdvanced);
        config.writePrivateProfileBool(name, "Enabled", rEnabled);
        config.writePrivateProfileBool(name, "HideApp", hideApp);
    }
    else
    {
        // what was the old name?
        config.writePrivateProfileString(name, "", "");
    }
}
Connectable RunConfigElement::connectable()
{
    Connectable res;
    res.appName = name;
    res.appType = appType;
    res.runType = runType;
    if (runType == ConnectServer)
    {
        res.serverName = server;
        res.remoteAppName = remoteApp;
    }
    else
    {
        res.serverName = MinosConfig::getMinosConfig()->getThisServerName();
        res.remoteAppName = name;
    }
    return res;
}

void RunConfigElement::createProcess()
{
    if (deleted)
        return;
    if (rEnabled && runType == RunLocal && !runner)
    {
        runner = new QProcess(parent());

        QString program = commandLine;
        if (!FileExists(program))
        {
            trace(name + ":program doesn't exist:" + program);
        }

        program += " ";
        program += params;

        QString wdir = rundir;
        runner->setWorkingDirectory(wdir);

        if (appType != "None" )
        {
            QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
            env.insert("MQTRPCNAME", name); // Add an environment variable for the RPC name to use
            runner->setProcessEnvironment(env);
        }

        connect (runner, SIGNAL(started()), this, SLOT(on_started()));
        connect (runner, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(on_finished(int, QProcess::ExitStatus)));
        connect (runner, SIGNAL(error(QProcess::ProcessError)), this, SLOT(on_error(QProcess::ProcessError)));

        connect (runner, SIGNAL(readyReadStandardError()), this, SLOT(on_readyReadStandardError()));
        connect (runner, SIGNAL(readyReadStandardOutput()), this, SLOT(on_readyReadStandardOutput()));

        runner->start(program);

        if (hideApp)
            sendCommand("HideServers");
        else
            sendCommand("ShowServers");


    }
}
void RunConfigElement::stopProcess()
{
    if (runner)
    {
        stopping = true;
        runner->terminate();
    }
}

void RunConfigElement::sendCommand(const QString & cmd)
{
    if (runner && appType != "None" )
    {
        QByteArray command = (cmd + "\n").toUtf8();
        qint64 res = runner->write( command );
        if (res < 0)
        {
            trace("Failed to write " + cmd + " to runner");
        }
        else
        {
            trace("Wrote " + cmd + " to runner");
        }
    }
}

void RunConfigElement::on_started()
{
    trace(name + ":started");
}

void RunConfigElement::on_finished(int err, QProcess::ExitStatus exitStatus)
{
    trace(name + ":finished:" + QString::number(err) + ":" + QString::number(exitStatus));
    if (runner)
    {

        runner->closeWriteChannel();
        runner->deleteLater();
        runner = 0;
        if (stopping)
        {
            stopping = false;

        }
        else
        {
            createProcess();
//            runner->start();    // but we have to be careful when we close!
        }
    }
}

void RunConfigElement::on_error(QProcess::ProcessError error)
{
    trace(name + ":error:" + QString::number(error));
}

void RunConfigElement::on_readyReadStandardError()
{
    if (runner)
    {
        QString r = runner->readAllStandardError();
        trace(name + ":stdErr:" + r);
    }
}

void RunConfigElement::on_readyReadStandardOutput()
{
    if (runner)
    {
        QString r = runner->readAllStandardOutput();
        trace(name + ":stdOut:" + r);
    }
}

//---------------------------------------------------------------------------
MinosConfig::MinosConfig( )
    : QObject( 0 ), autoStart(false), config(getConfigIniName())
{
}
void MinosConfig::initialise()
{
    buildAppConfigList();
    config.startGroup();

    QStringList lsect = config.getSections();

    for ( int i = 0; i < lsect.count(); i++ )
    {
        QString sect = lsect[ i ].trimmed();
        if ( sect.compare("Settings", Qt::CaseInsensitive ) == 0)
        {
            config.getPrivateProfileString( "Settings", "ServerName", "", thisServerName );

            if ( thisServerName.size() == 0 )
            {
                QString h = QHostInfo::localHostName();
                thisServerName = h;
            }
            autoStart = config.getPrivateProfileBool( "Settings", "AutoStart", false );
        }
        else
        {
            QSharedPointer<RunConfigElement> tce = QSharedPointer<RunConfigElement>(new RunConfigElement());
            if ( tce->initialise( config, sect ) )
            {
                elelist.push_back( tce );
            }
        }
    }
    config.endGroup();
}

//---------------------------------------------------------------------------
MinosConfig::~MinosConfig()
{
   if ( !terminated )
      stop();

   elelist.clear();
}
//---------------------------------------------------------------------------
bool configSort( const QSharedPointer<RunConfigElement> c1, const QSharedPointer<RunConfigElement> c2)
{
    return c1->name < c2->name;
}
void MinosConfig::saveAll()
{
    config.startGroup();
    QVector <QSharedPointer<RunConfigElement> > newList = elelist;
    qSort(newList.begin(), newList.end(), configSort);
    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = newList.begin(); i != newList.end(); i++ )
    {
        (*i)->save(config);
    }
    config.writePrivateProfileString("Settings", "ServerName", thisServerName);
    config.writePrivateProfileBool( "Settings", "AutoStart", autoStart );

    config.writePrivateProfileString( "", "", "" );    // flush
    config.endGroup();

}
void MinosConfig::start()
{
   terminated = false;

   for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = elelist.begin(); i != elelist.end(); i++ )
   {
       (*i)->createProcess();
   }
}

void MinosConfig::stop()
{
   terminated = true;

   for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = elelist.begin(); i != elelist.end(); i++ )
   {
      if ( ( *i ) )
      {
         logMessage( "Killing subProcess", "" );
         ( *i ) ->stopProcess();
         logMessage( "subProcess killed", "" );
      }
   }
}
void MinosConfig::setThisServerName( const QString &circle )
{
   thisServerName = circle;
}

bool MinosConfig::getAutoStart()
{
   return autoStart;
}
void MinosConfig::setAutoStart(bool s)
{
    autoStart = s;
}
Connectable MinosConfig::getApp(QString appName)
{
    Connectable res;
    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = elelist.begin(); i != elelist.end(); i++ )
    {
        if (appName.compare((*i)->name, Qt::CaseInsensitive) == 0)
        {
            res = (*i)->connectable();
            break;
        }
    }
    return res;
}

QStringList MinosConfig::getAppTypes()
{
    QStringList apps;
    for (int i = 0; i < appConfigList.size(); i++)
    {
        apps.append(appConfigList[i].appType);
    }
    apps.sort();
    apps.insert(0, "None");
    apps.removeDuplicates();
    return apps;
}
void MinosConfig::buildAppConfigList()
{
    INIFile appConfig("./Configuration/AppConfig.ini");
    /*
[BandMap]
Path=./mqtBandMap
Enabled=false
Requires=Server
Server=false

   */
    appConfig.startGroup();

    QStringList apps = appConfig.getSections();
    for (int i = 0; i < apps.size(); i++)
    {
        if (appConfig.getPrivateProfileBool(apps[i], "Enabled", false))  // only include those elements we are allowed to as possibilities
        {
            AppConfigElement ac;

            ac.appType = apps[i].trimmed();
            appConfig.getPrivateProfileString(apps[i], "Path", "", ac.appPath);
#ifdef Q_OS_WIN
            ac.appPath += ".exe";
#endif
            ac.server = appConfig.getPrivateProfileBool(apps[i], "Server", false);
            ac.defaultHide = appConfig.getPrivateProfileBool(apps[i], "HideApp", false);

            QString whereString;
            appConfig.getPrivateProfileString(apps[i], "Where", "Remote,Local", whereString);
            if (whereString.contains("local", Qt::CaseInsensitive))
            {
                ac.localOK = true;
            }
            else
            {
                ac.localOK = false;
            }
            if (whereString.contains("remote", Qt::CaseInsensitive))
            {
                ac.remoteOK = true;
            }
            else
            {
                ac.remoteOK = false;
            }


            // NB using comma in value give a string list! Single value will also go to list if desired
            QString reqs;
            appConfig.getPrivateProfileString(apps[i], "Requires",  "", reqs);
            ac.requires = reqs.split(',', QString::SkipEmptyParts);

            for(auto& str : ac.requires)    // trim all elements of leading and trailing spaces
                str = str.trimmed();

            appConfigList.append(ac);
        }
    }
    appConfig.endGroup();
}
QString MinosConfig::checkConfig()
{
    QString reqErrs;

    bool serverPresent = false;
    int eleListSize = 0;
    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = elelist.begin(); i != elelist.end(); i++ )
    {
        QSharedPointer<RunConfigElement> ele = (*i);
        if (ele->deleted)
            continue;
        if (ele->rEnabled)
        {
            eleListSize++;
            if (ele->appType == "Server" && ele->runType == RunLocal )
            {
                serverPresent = true;
                break;
            }
        }
    }

    if (eleListSize && !serverPresent)
    {
        reqErrs += "A local server is required.\r\n\r\n" ;
    }

    //Check that the name is not blank, and only has allowed characters
    //Check that the names aren't duplicates

    // Go through the configured elements, and check that their requirements are also present
    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = elelist.begin(); i != elelist.end(); i++ )
    {
        QSharedPointer<RunConfigElement> ele = (*i);

        if (ele->deleted)
            continue;

        if (ele->rEnabled)
        {
            if ( ele->requires.size() > 0 && ele->runType == RunLocal)
            {
                // "Requires" elements must be present
                foreach(QString req, ele->requires)
                {
                    if (req.isEmpty())
                        continue;

                    if (req == "Server")
                        continue;


                    bool reqFound = false;
                    for ( QVector <QSharedPointer<RunConfigElement> >::iterator j = elelist.begin(); j != elelist.end(); j++ )
                    {
                        if ((*j)->deleted)
                            continue;
                        if ((*j)->appType == req && (*j)->rEnabled && (*j)->runType == RunLocal)
                        {
                            reqFound = true;
                            continue;
                        }
                    }
                    if (!reqFound)
                    {
                        reqErrs += ele->appType + " requires a local " + req + "\n\n";
                    }
                }
            }


            if (ele->runType == RunLocal)
            {
                if (!FileExists(ele->commandLine))
                {
                    reqErrs += ele->appType + " Executable path does not exist\n\n";
                }
                if (ele->appType != "None" && !FileExists(ele->rundir + "/Configuration/MinosConfig.ini"))
                {
                    reqErrs += ele->appType + " Working directory is not valid - no Configuration/MinosConfig.ini\n\n";
                }
            }
        }
    }
    return reqErrs;
}
AppConfigElement MinosConfig::getAppConfigElement(QString appType)
{
    AppConfigElement ace;
    for ( QVector <AppConfigElement >::iterator j = appConfigList.begin(); j != appConfigList.end(); j++ )
    {
        if ((*j).appType == appType)
        {
            ace = (*j);
            break;
        }
    }
    return ace;
}
