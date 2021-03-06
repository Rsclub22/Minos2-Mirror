#include "mqtUtils_pch.h"

#include <QTimer>
#include <QHostInfo>
#include <QSharedPointer>
#include "fileutils.h"
#include "ConfigFile.h"


//---------------------------------------------------------------------------
MinosConfigEvents MinosConfigEvents::mce;
void MinosConfigEvents::sendAppStarted()
{
    emit mce.appStarted();
}
static bool terminated = false;

QString RunLocal("RunLocal");

QString ConnectRouter("ConnectServer");
const char * MinosConfig::appNone = QT_TR_NOOP("None");
const char * MinosConfig::appOther = QT_TR_NOOP("Other");

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

QString MinosConfig::getThisRouterName()
{
    QString routerName;
    config.getPrivateProfileString( "Settings", "ServerName", QHostInfo::localHostName(), routerName );

    if ( routerName.size() == 0 )
    {
        QString h = QHostInfo::localHostName();
        routerName = h;
    }
    return routerName;
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
    config.getPrivateProfileString( sect, "Server", "localhost", router );
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
    if (ace.appType == tr(MinosConfig::appOther))
    {
        showAdvanced = true;
    }

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
        config.writePrivateProfileString(name, "Server", router);
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
    if (runType == ConnectRouter)
    {
        res->routerName = router;
        res->remoteAppName = remoteApp;
    }
    else
    {
        res->routerName = MinosConfig::getMinosConfig()->getThisRouterName();
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

        connect (runner, &QProcess::started, this, &RunConfigElement::on_started);
        connect (runner, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &RunConfigElement::on_finished);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        connect (runner, &QProcess::errorOccurred, this, &RunConfigElement::on_error);
#else
        connect (runner, &QProcess::error, this, &RunConfigElement::on_error);
#endif

        connect (runner, &QProcess::readyReadStandardError, this, &RunConfigElement::on_readyReadStandardError);
        connect (runner, &QProcess::readyReadStandardOutput, this, &RunConfigElement::on_readyReadStandardOutput);

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        QStringList progArgs = runner->splitCommand(program);
        const QString prog = progArgs.takeFirst();

        runner->start(prog, progArgs);
#else
        runner->start(program);
#endif

        if (runner)
        {
            // and error will have removed runner!
            if (hideApp)
                sendCommand("HideServers");
            else
                sendCommand("ShowServers");

            QString fontCommand = "Font " + QApplication::font().toString();
            sendCommand(fontCommand);

             MinosConfigEvents::sendAppStarted();
        }
    }
}
void RunConfigElement::askStopProcess()
{
    if (runner)
    {
        trace( QString("Closing subProcess %1").arg(name) );
        stopping = true;
        sendCommand("Shutdown");
    }
}
void RunConfigElement::forceStopProcess()
{
    if (runner && !runner->waitForFinished(5000))
    {
        runner->terminate();
        trace( QString("subProcess %1 killed").arg(name) );
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
            trace(QString("Failed to write %1 to runner %2").arg(cmd).arg(name));
        }
        else
        {
            trace(QString("Wrote %1 to runner %2").arg(cmd).arg(name));
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
    runner->deleteLater();
    runner = nullptr;
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
      forceStop();

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
void MinosConfig::initialise()
{
    buildAppConfigList();
    config.startGroup();

    QStringList lsect = config.getSections();

    for ( auto const &s: qAsConst(lsect ))
    {
        QString sect = s.trimmed();
        if ( sect.compare("Settings", Qt::CaseInsensitive ) == 0)
        {
            config.getPrivateProfileString( "Settings", "ServerName", "", thisRouterName );

            if ( thisRouterName.size() == 0 )
            {
                QString h = QHostInfo::localHostName();
                thisRouterName = h;
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
    std::sort(newList.begin(), newList.end(), configSort);
    for ( auto const &i: newList )
    {
        i->save(config);
    }
    config.writePrivateProfileString("Settings", "ServerName", thisRouterName);
    config.writePrivateProfileBool( "Settings", "AutoStart", autoStart );

    config.writePrivateProfileString( "", "", "" );    // flush
    config.endGroup();

}
void MinosConfig::start()
{
   terminated = false;

   for ( auto const &i: qAsConst(elelist ))
   {
       i->createProcess();
   }
}

void MinosConfig::askStop()
{
   terminated = true;

   for ( auto const &i: qAsConst(elelist ))
   {
      if ( i )
      {
         i->askStopProcess();
      }
   }
}
void MinosConfig::forceStop()
{
   terminated = true;

   for ( auto const &i: qAsConst(elelist ))
   {
      if ( i )
      {
         i->forceStopProcess();
      }
   }
}

void MinosConfig::bounce()
{
    for ( auto const &i: qAsConst(elelist ))
    {
       if ( i )
       {
          logMessage( "Bouncing subProcess", "" );
          i->bounceProcess();
       }
    }
}
void MinosConfig::setThisRouterName( const QString &circle )
{
   thisRouterName = circle;
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
    for ( auto const &i: qAsConst(elelist ))
    {
        if (appName.compare(i->name, Qt::CaseInsensitive) == 0)
        {
            res = i->connectable();
            break;
        }
    }
    return res;
}

QStringList MinosConfig::getAppTypes()
{
    QStringList apps;
    for (auto const &a: qAsConst(appConfigList))
    {
        if (a.appType != tr(appNone) && a.appType != tr(appOther))
        {
            apps.append(a.appType);
        }
    }
    apps.sort();
    apps.prepend( tr(appNone));
    apps.append(tr(appOther));
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
    for (auto &a: apps)
    {
        if (a == appNone)
            a = tr(appNone);
        if (a == appOther)
            a = tr(appOther);

        bool otherApp = false;

        bool enabled = appConfig.getPrivateProfileBool(a, "Enabled", false);
        if (a == tr(appOther))
        {
            otherApp = true;
            enabled = true;
        }

        if (enabled)  // only include those elements we are allowed to as possibilities
        {
            AppConfigElement ac;

            ac.appType = a.trimmed();
            appConfig.getPrivateProfileString(a, "Path", "", ac.appPath);
#ifdef Q_OS_WIN
            if (!ac.appPath.isEmpty())
            {
                ac.appPath += ".exe";
            }
#endif
            ac.router = appConfig.getPrivateProfileBool(a, "Server", false);
            ac.defaultHide = appConfig.getPrivateProfileBool(a, "HideApp", false);

            QString whereString;
            appConfig.getPrivateProfileString(a, "Where", "Remote,Local", whereString);
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
            if (otherApp)
            {
                ac.router = true;
                ac.defaultHide = false;
                ac.localOK = true;
                ac.remoteOK = true;
            }


            // NB using comma in value give a string list! Single value will also go to list if desired
            QString reqs;
            appConfig.getPrivateProfileString(a, "Requires",  "", reqs);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            ac.requiresApps = reqs.split(',', Qt::SkipEmptyParts);
#else
            ac.requiresApps = reqs.split(',', QString::SkipEmptyParts);
#endif

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

    bool routerPresent = false;
    int eleListSize = 0;
    for ( auto const &ele: qAsConst(elelist ))
    {
        if (ele->deleted)
            continue;
        if (ele->rEnabled)
        {
            eleListSize++;
            if (ele->appType == "Server" && ele->runType == RunLocal )
            {
                if (routerPresent)
                {
                    reqErrs += tr("More than one router is defined and enabled");
                }
                routerPresent = true;
            }
        }
    }

    if (eleListSize && !routerPresent)
    {
        reqErrs += tr("A local router is required.\r\n\r\n") ;
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
    for ( auto const &ele: qAsConst(elelist ))
    {
        if (ele->deleted)
            continue;

        if (ele->rEnabled)
        {
            if ( ele->requiresApps.size() > 0 && ele->runType == RunLocal)
            {
                // "Requires" elements must be present
                for(auto const &req: qAsConst(ele->requiresApps))
                {
                    if (req.isEmpty())
                        continue;

                    if (req == "Server")
                        continue;


                    bool reqFound = false;
                    for ( auto const &j: qAsConst(elelist ))
                    {
                        if (j->deleted)
                            continue;
                        if (j->appType == req && j->rEnabled && j->runType == RunLocal)
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
    for ( auto const &i: qAsConst(elelist ))
    {
       if ( i && i->isRunning() )
       {
           return true;
       }
    }
    return false;
}
AppConfigElement MinosConfig::getAppConfigElement(QString appType)
{
    AppConfigElement ace;
    for ( auto const &a: qAsConst(appConfigList))
    {
        if (a.appType == appType)
        {
            ace = a;
            break;
        }
    }
    return ace;
}

QVector<QSharedPointer<Connectable> > MinosConfig::getConnectables()
{
    QVector<QSharedPointer<Connectable> >  connectables;

    for ( auto const &e: qAsConst(elelist ))
    {
        if (!e->deleted)
        {
            QSharedPointer<Connectable> res = e->connectable();
            connectables.push_back(res);
        }
    }
    return connectables;
}
void MinosConfig::checkAllStopped()
{
    if (!anyRunning())
    {
        emit allStopped();
    }
}
