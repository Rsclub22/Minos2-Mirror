#include "ControlMain.h"
#include "ui_ControlMain.h"

#include "portconf.h"
#include "MinosLines.h"

ControlMain *controlMain = nullptr;
void ControlMain::logMessage( QString s )
{
   trace( s );
}
//---------------------------------------------------------------------------
void LineLog( const QString &msg )
{
   trace( msg );
}
ControlMain::ControlMain(QWidget *parent) :
    QMainWindow(parent),
    monitor(parent),
    ui(new Ui::ControlMain)
{
    LineSet *ls = LineSet::GetLineSet();
    ls->lsLog = LineLog;
    connect(ls, SIGNAL(linesChanged()), this, SLOT(linesChangedEvent()));

    controlMain = this;
    ui->setupUi(this);

    createCloseEvent();

    QSettings settings;
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));

    formShowTimer.setSingleShot(true);
    connect(&formShowTimer, SIGNAL(timeout()), this, SLOT(on_formShown()));
    formShowTimer.start(100);


}
void ControlMain::onStdInRead(QString cmd)
{
    trace("Command read from stdin: " + cmd);
    if (cmd.indexOf("ShowServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(true);
    if (cmd.indexOf("HideServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(false);

}
void ControlMain::on_formShown( )
{
    static bool shown = false;
    if (!shown)
    {
        shown = true;
        configurePorts( monitor );

        monitor.initialise();

        connect(&LogTimer, SIGNAL(timeout()), this, SLOT(LogTimerTimer()));
        LogTimer.start(100);

        subscribeApps();
    }
}

ControlMain::~ControlMain()
{
    monitor.closeDown();
    controlMain = nullptr;
    delete ui;
}
void ControlMain::closeEvent(QCloseEvent * event)
{
    LogTimer.stop();
    monitor.closeDown();
    event->accept();
    //QWidget::closeEvent(event);
}
void ControlMain::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void ControlMain::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void ControlMain::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}
void ControlMain::LogTimerTimer( )
{
    bool show = getShowServers();
    if ( !isVisible() && show )
    {
       setVisible(true);
    }
    if ( isVisible() && !show )
    {
       setVisible(false);
    }

   static bool closed = false;
   if ( !closed )
   {
      if ( checkCloseEvent() )
      {
         closed = true;
         close();
      }
   }
}
void ControlMain::linesChangedEvent( )
{
    LineSet * ls = LineSet::GetLineSet();

    ls->readLines();
    // check on the input lines - PTTOut and Key
    commonLineControl *l = monitor.findLine( "PTTOut", false );   // output line
    if ( l )
    {
        l->setState( ls->getState( "PTTOut" ) );
    }
    l = monitor.findLine( "T1", false );   // output line
    if ( l )
    {
        l->setState( ls->getState( "T1" ) );
    }
    l = monitor.findLine( "T2", false );   // output line
    if ( l )
    {
        l->setState( ls->getState( "T2" ) );
    }
}
//---------------------------------------------------------------------------
void ControlMain::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
   trace( "control callback from " + from + ( err ? ":Error" : ":Normal" ) );

   if ( !err )
   {
      QSharedPointer<RPCParam> psName;
      QSharedPointer<RPCParam>psLine;
      RPCArgs *args = mro->getCallArgs();
      if ( args->getStructArgMember( 0, rpcConstants::controlParamName, psName ) && args->getStructArgMember( 0, rpcConstants::controlParamLine, psLine ) )
      {
         QString Name;
         QString Line;

         if ( psName->getString( Name ) && psLine->getString( Line ) )
         {
            QSharedPointer<RPCParam>st(new RPCParamStruct);

            if ( Name == rpcConstants::controlGetLine )
            {
               // action the message
               // can be set line, get line
               // also want to allow integer valued commands and responses
               // to allow a group of lines to be set (e.g. a BCD control output)

               // get commands need a response, set commands are true/false
               // to describe if the line was recognised or not.

               // may be good to allow bundled set/get as well - especially if
               // we can pass the bundle down further

               // Do we also want to have subscribe option, so that changes are
               // forced out? Probably... But we would need to mark the
               // lines(s) as subscription lines in the configuration

               commonLineControl *l = monitor.findLine( Line, true );   // input line
               if ( l )
               {
                  st->addMember( l->getState(),rpcConstants::controlLineState );

               }
               else
               {
                  st->addMember( false, rpcConstants::controlResult );
               }

            }
            else
               if ( Name == rpcConstants::controlSetLine )
               {
                  QSharedPointer<RPCParam> psState;
                  if ( args->getStructArgMember( 0, rpcConstants::controlState, psState ) )
                  {
                     bool state;
                     psState->getBoolean( state );
                     // action the message
                     // can be set line, get line
                     // also want to allow integer valued commands and responses
                     // to allow a group of lines to be set (e.g. a BCD control output)

                     // get commands need a response, set commands are true/false
                     // to describe if the line was recognised or not.

                     // may be good to allow bundled set/get as well - especially if
                     // we can pass the bundle down further

                     // Do we also want to have subscribe option, so that changes are
                     // forced out? Probably... But we would need to mark the
                     // lines(s) as subscription lines in the configuration

                     commonLineControl *l = monitor.findLine( Line, false );   // output line
                     if ( l )
                     {
                        l->setState( state );
                        //st->addMember( true, rpcConstants::controlResult );

                     }
                     else
                     {
                        //st->addMember( false, rpcConstants::controlResult );
                     }
                  }

               }
               else if (Name == rpcConstants::controlSetTransverter)
               {
                   int t = Line.toInt();
                   commonLineControl *t1 = monitor.findLine( "T1", false );   // output line
                   commonLineControl *t2 = monitor.findLine( "T2", false );   // output line

                   // would be better to do this all in one...
                   t1->setState(t & 1);
                   t2->setState(t & 2);
               }
         }
      }
   }
}
void ControlMain::setPTTIn(bool s)
{
    ui->PTTInCheckBox->setChecked(s);
}
void ControlMain::setPTTOut(bool s)
{
    ui->PTTOutCheckBox->setChecked(s);
}
void ControlMain::setL1(bool s)
{
    ui->L1CheckBox->setChecked(s);
}
void ControlMain::setL2(bool s)
{
    ui->L2CheckBox->setChecked(s);
}
void ControlMain::setL3(bool s)
{
    ui->L3CheckBox->setChecked(s);
}
void ControlMain::setL4(bool s)
{
    ui->L4CheckBox->setChecked(s);
}
void ControlMain::setL5(bool s)
{
    ui->L5CheckBox->setChecked(s);
}
void ControlMain::setL6(bool s)
{
    ui->L6CheckBox->setChecked(s);
}
void ControlMain::setT1(bool s)
{
    ui->T1CheckBox->setChecked(s);
}
void ControlMain::setT2(bool s)
{
    ui->T2CheckBox->setChecked(s);
}

void setLines(bool PTTOut, bool PTTIn, bool L1, bool L2, bool L3, bool L4, bool L5, bool L6 , bool T1, bool T2)
{
   // This ought to be synchronised...
   controlMain->setPTTOut(PTTOut);
   controlMain->setPTTIn(PTTIn);
   controlMain->setL1(L1);
   controlMain->setL2(L2);
   controlMain->setL3(L3);
   controlMain->setL4(L4);
   controlMain->setL5(L5);
   controlMain->setL6(L6);
   controlMain->setT1(T1);
   controlMain->setT2(T2);
}
void ControlMain::on_notify( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
    // PubSub notifications
    AnalysePubSubNotify an( err, mro );
    trace( "Notify callback from " + from + ( err ? ":Error " : ":Normal " ) +  an.getPublisherProgram() + "@" + an.getPublisherServer());

    // Need to check that the server/app is in the category map; if not, don't pass it on
    if ( an.getOK())
    {
        QString category = an.getCategory();
        if (category != rpcConstants::LocalStationCategory && category != rpcConstants::StationCategory)
        {
            bool notificationOK = false;
            for ( QVector <QSharedPointer<Connectable> >::iterator j = catMap[category].begin(); j != catMap[category].end(); j++ )
            {
                if ((*j)->runType == RunLocal)
                {
                    if (an.getPublisherServer() != (*j)->serverName)
                    {
                        //trace("RunLocal server " + an.getPublisherServer() + " " + (*j)->serverName);
                        continue;
                    }
                    if (an.getPublisherProgram() != (*j)->appName)
                    {
                        //trace("RunLocal appName " + an.getPublisherProgram() + " " + (*j)->appName);
                        continue;
                    }

                    notificationOK = true;
                    break;
                }
                else if ((*j)->runType == ConnectServer)
                {
                    if ((*j)->serverName.isEmpty())
                    {
                        notificationOK = true;
                        break;
                    }
                    else if (an.getPublisherServer() != (*j)->serverName)
                    {
                        //trace("ConnectServer server " + an.getPublisherServer() + " " + (*j)->serverName);
                        continue;
                    }
                    if ((*j)->remoteAppName.isEmpty())
                    {
                        notificationOK = true;
                        break;
                    }
                    else if (an.getPublisherProgram() != (*j)->remoteAppName)
                    {
                        //trace("ConnectServer appName " + an.getPublisherProgram() + " " + (*j)->appName);
                        continue;
                    }

                    notificationOK = true;
                    break;
                }
            }

            if (!notificationOK)
                return;

        }
        if ( an.getState() == psPublished)
        {
            trace(QString("SendRPC category %1 key %2").arg(an.getCategory()).arg(an.getKey()));
            if ( an.getCategory() == rpcConstants::rigStateCategory)
            {
                rigCache.setStateString(an);
            }
            if ( an.getCategory() == rpcConstants::rigDetailsCategory)
            {
                rigCache.setDetailsString(an);
            }
            else if ( an.getCategory() == rpcConstants::rigControlCategory && an.getKey() == rpcConstants::rigControlRadioList )
            {
                rigCache.addRigList(an.getValue());
            }
            else if ( an.getCategory() == rpcConstants::StationCategory)
            {
                QString server = an.getKey();
                if (!servers.contains(server))
                {
                    servers.append(server);
                    for ( QMap<QString,QVector< QSharedPointer<Connectable> > >::iterator i = catMap.begin(); i != catMap.end(); i++)
                    {
                        for ( QVector <QSharedPointer<Connectable> >::iterator j = (*i).begin(); j != (*i).end(); j++ )
                        {
                            if ((*j)->runType == ConnectServer && (*j)->serverName.isEmpty())
                            {
                                RPCPubSub::subscribeRemote(server, i.key());
                            }
                        }
                    }
                }
            }
            else
                return;

            rigSelected = rigCache.getSelected("");
            if (!rigSelected.isEmpty())
            {
                RigDetails &selDetail = rigCache.getDetails(rigSelected);
                int transverterSwNum = selDetail.transverterSwitch().getValue();
                commonLineControl *t1 = monitor.findLine( "T1", false );   // output line
                commonLineControl *t2 = monitor.findLine( "T2", false );   // output line

                // would be better to do this all in one...
                t1->setState(transverterSwNum & 1);
                t2->setState(transverterSwNum & 2);
            }

        }
    }
}
void ControlMain::subscribeApps()
{
    /*
        for each type of interest (i.e. not chat or monitor)

        We need to subscribe to all server names - cf chatserver

    rpc->subscribe(rpcConstants::LocalStationCategory);

        look for all config entries

        If local, then we subscribe to it

        if remote and server is empty, then we want all servers as they become available
        if remote and a named server, then subscribe to that server only

        We need to save all this, and restrict on the app name as well

        So, we need some structures

        ?? key a list by category subscribed - each entry a chain of entries?

        type of app
        server name
        app name
        state

        When we get a LocalStationCategory notification, we need to look down the list
        and if this servername or server name is blank, then subcribe to the relevant
        category on this server. Extra subscriptions are harmless(I am pretty certain -
        maybe they will force a set of notifications).

        When we get an "other category" notification we need to find the relevant entries
        and check the app name before responding to it.

    */
    trace("subscribeApps");
    rigCache.invalidate();

    catMap.clear();
    connectables.clear();
    servers.clear();

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    MinosConfig *config = MinosConfig::getMinosConfig();

    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = config->elelist.begin(); i != config->elelist.end(); i++ )
    {
        if (!(*i)->deleted)
        {
            QSharedPointer<Connectable> res = (*i)->connectable();
            connectables.push_back(res);
        }
    }

    for ( QVector <QSharedPointer<Connectable> >::iterator i = connectables.begin(); i != connectables.end(); i++ )
    {
        if ((*i)->appType == "None")
        {
            // no action
        }
        else if ((*i)->appType == "AppStarter")
        {
            // no action
        }
        else if ((*i)->appType == "BandMap")
        {
            // no action
        }
        else if ((*i)->appType == "Chat")
        {
            // no action - done in chat server
        }
        else if ((*i)->appType == "Keyer")
        {
            // no action
        }
        else if ((*i)->appType == "LineControl")
        {
            // no action except in keyer
        }
        else if ((*i)->appType == "Logger")
        {
            // no action
        }
        else if ((*i)->appType == "Monitor")
        {
            // no action
        }
         else if ((*i)->appType == "Other")
        {
            // no action
        }
        else if ((*i)->appType == "RigControl")
        {
            catMap[rpcConstants::rigControlCategory].push_back((*i));
            catMap[rpcConstants::rigDetailsCategory].push_back((*i));
            catMap[rpcConstants::rigStateCategory].push_back((*i));
        }
        else if ((*i)->appType == "Rotator")
        {
            // no action
        }
        else if ((*i)->appType == "Server")
        {
            catMap[rpcConstants::LocalStationCategory].push_back((*i));
            catMap[rpcConstants::StationCategory].push_back((*i));
        }
    }

    if (!servers.contains(config->getThisServerName()))
    {
        servers.append(config->getThisServerName());
        for ( QMap<QString,QVector< QSharedPointer<Connectable> > >::iterator i = catMap.begin(); i != catMap.end(); i++)
        {
            for ( QVector <QSharedPointer<Connectable> >::iterator j = (*i).begin(); j != (*i).end(); j++ )
            {
                if ((*j)->runType == RunLocal)
                {
                    rpc->subscribeRemote(config->getThisServerName(), i.key());
                }
                else if ((*j)->runType == ConnectServer && !(*j)->serverName.isEmpty())
                {
                    rpc->subscribeRemote((*j)->serverName, i.key());
                }
            }
        }
    }
}
