#include "mqtUtils_pch.h"

#include <QTimer>
#include <QHostInfo>
#include <QSharedPointer>
#include "fileutils.h"
#include "ConfigFile.h"


//---------------------------------------------------------------------------

static bool terminated = false;

QString RunLocal("RunLocal");
QString ConnectServer("ConnectServer");
const char * MinosConfig::appNone = QT_TR_NOOP("None");

/*static*/
MinosConfig *MinosConfig::thisDM = nullptr;
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


//---------------------------------------------------------------------------
RunConfigElement::RunConfigElement()
{}

RunConfigElement::~RunConfigElement()
{
    delete runner;
}

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
    requiresApps = ace.requiresApps;
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
QSharedPointer<Connectable> RunConfigElement::connectable()
{
    QSharedPointer<Connectable> res(new Connectable);
    res->appName = name;
    res->appType = appType;
    res->runType = runType;
    if (runType == ConnectServer)
    {
        res->serverName = server;
        res->remoteAppName = remoteApp;
    }
    else
    {
        res->serverName = MinosConfig::getMinosConfig()->getThisServerName();
        res->remoteAppName = name;
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
            trace(name + tr(":program doesn't exist:") + program);
        }

        program += " ";

        QString locale = getCurrentLanguage();
        if (!locale.isEmpty())
        {
            program += "--lang " + locale + " ";
        }

        program += params;

        QString wdir = rundir;
        runner->setWorkingDirectory(wdir);

        if (appType != MinosConfig::tr(MinosConfig::appNone) )
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

        QString fontCommand = "Font " + QApplication::font().toString();
        sendCommand(fontCommand);
    }
}
void RunConfigElement::stopProcess()
{
    if (runner)
    {
        logMessage( "Killing subProcess", "" );
        stopping = true;
        sendCommand("Shutdown");
        if (!runner->waitForFinished(10000))
        {
            runner->terminate();
        }
        logMessage( "subProcess killed", "" );
    }
}
void RunConfigElement::bounceProcess()
{
    if (runner)
    {
        stopping = false;
        sendCommand("Shutdown");
    }
}
void RunConfigElement::sendCommand(const QString & cmd)
{
    if (runner && appType != MinosConfig::tr(MinosConfig::appNone) )
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
        runner = nullptr;
        if (stopping)
        {
            stopping = false;
            MinosConfig::getMinosConfig()->checkAllStopped();
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
        QString line = runner->readLine();
        trace(name + ":stdOut:" + line);
        emit MinosConfig::getMinosConfig()->stdOutLine(line);
    }
}

//---------------------------------------------------------------------------
MinosConfig::MinosConfig( )
    : QObject( nullptr )
    , config(getConfigIniName())
    , autoStart(false)
{
}
MinosConfig::~MinosConfig()
{
   if ( !terminated )
      stop();

   elelist.clear();
}

void MinosConfig::reset()
{
    // get rid of current config, reload from disc
    appConfigList.clear();
    delete thisDM;
    thisDM = nullptr;
    getMinosConfig();
}
/*
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
*/
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
bool configSort( const QSharedPointer<RunConfigElement> c1, const QSharedPointer<RunConfigElement> c2)
{
    return c1->name < c2->name;
}
void MinosConfig::saveAll()
{
    config.startGroup();
    config.clear();
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
         ( *i ) ->stopProcess();
      }
   }
}
void MinosConfig::bounce()
{
    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = elelist.begin(); i != elelist.end(); i++ )
    {
       if ( ( *i ) )
       {
          logMessage( "Bouncing subProcess", "" );
          ( *i ) ->bounceProcess();
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
QSharedPointer<Connectable> MinosConfig::getApp(QString appName)
{
    QSharedPointer<Connectable> res;
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
    apps.insert(0, tr(appNone));
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
        if (apps[i] == appNone)
            apps[i] = tr(appNone);

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
            ac.requiresApps = reqs.split(',', QString::SkipEmptyParts);

            for(auto& str : ac.requiresApps)    // trim all elements of leading and trailing spaces
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
                if (serverPresent)
                {
                    reqErrs += tr("More than one server is defined and enabled");
                }
                serverPresent = true;
            }
        }
    }

    if (eleListSize && !serverPresent)
    {
        reqErrs += tr("A local server is required.\r\n\r\n") ;
    }

    //Check that the name is not blank, and only has allowed characters
    //Check that the names aren't duplicates
    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = elelist.begin(); i != elelist.end(); i++ )
    {
        QSharedPointer<RunConfigElement> elei = (*i);
        if (elei->deleted)
        {
            continue;
        }

        if (elei->name.contains('[') || elei->name.contains(']'))
        {
            reqErrs += elei->name + tr(" contains bad characters [ and/or ]");
        }
        for ( QVector <QSharedPointer<RunConfigElement> >::iterator j = i; j != elelist.end(); j++ )
        {
            if (j == i)
                continue;

            QSharedPointer<RunConfigElement> elej = (*j);
            if (elej->deleted)
            {
                continue;
            }

            if (elei->name.compare(elej->name, Qt::CaseInsensitive) == 0)
            {
                reqErrs += elei->name + tr(" appears more than once (names are not case sensitive)");
                break;
            }
        }
    }

    // Go through the configured elements, and check that their requirements are also present
    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = elelist.begin(); i != elelist.end(); i++ )
    {
        QSharedPointer<RunConfigElement> ele = (*i);

        if (ele->deleted)
            continue;

        if (ele->rEnabled)
        {
            if ( ele->requiresApps.size() > 0 && ele->runType == RunLocal)
            {
                // "Requires" elements must be present
                foreach(QString req, ele->requiresApps)
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
                        reqErrs += tr("%1 requires a local %2\n\n").arg(ele->appType).arg(req);
                    }
                }
            }


            if (ele->runType == RunLocal)
            {
                if (!FileExists(ele->commandLine))
                {
                    reqErrs += ele->appType + tr(" Executable path does not exist\n\n");
                }
                if (ele->appType != tr(appNone) && !FileExists(ele->rundir + "/Configuration/MinosConfig.ini"))
                {
                    reqErrs += ele->appType + tr(" Working directory is not valid - no Configuration/MinosConfig.ini\n\n");
                }
            }
        }
    }
    return reqErrs;
}

bool MinosConfig::anyRunning()
{
    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = elelist.begin(); i != elelist.end(); i++ )
    {
       if ( ( *i ) && (*i)->isRunning() )
       {
           return true;
       }
    }
    return false;
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
void MinosConfig::checkAllStopped()
{
    if (!anyRunning())
    {
        emit allStopped();
    }
}
