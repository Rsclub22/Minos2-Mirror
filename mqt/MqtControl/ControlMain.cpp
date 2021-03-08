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
    ui(new Ui::ControlMain),
    monitor(parent)
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


    connect(&stdinReader, &StdInReader::stdinLine, this, &ControlMain::onStdInRead);
    stdinReader.start();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());

    connect(rpc, SIGNAL(routerCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_routerCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(AnalysePubSubNotify ,QString)), this, SLOT(on_notify(AnalysePubSubNotify ,QString)));

    formShowTimer.setSingleShot(true);
    connect(&formShowTimer, SIGNAL(timeout()), this, SLOT(on_formShown()));
    formShowTimer.start(100);


}
ControlMain::~ControlMain()
{
    monitor.closeDown();
    controlMain = nullptr;
    delete ui;
}
void ControlMain::getRouterAppCatMap()
{
    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName());
    MinosConfig *config = MinosConfig::getMinosConfig();
    QVector<QSharedPointer<Connectable> > connectables;
    connectables = config->getConnectables();

    QMap<QString,QVector< QSharedPointer<Connectable> > > routerAppCatMap;

    for ( auto const &c: qAsConst(connectables) )
    {
        if (c->appType == "None")
        {
            // no action
        }
        else if (c->appType == "AppStarter")
        {
            // no action
        }
        else if (c->appType == "BandMap")
        {
            // no action
        }
        else if (c->appType == "Chat")
        {
            // no action - done in chat server
        }
        else if (c->appType == "Keyer")
        {
            // no action
        }
        else if (c->appType == "LineControl")
        {
            // no action except in keyer
        }
        else if (c->appType == "Logger")
        {
            // no action
        }
        else if (c->appType == "Monitor")
        {
            // no action
        }
         else if (c->appType == "Other")
        {
            // no action
        }
        else if (c->appType == "RigControl")
        {
            routerAppCatMap[rpcConstants::rigControlCategory].push_back(c);
            routerAppCatMap[rpcConstants::rigDetailsCategory].push_back(c);
            routerAppCatMap[rpcConstants::rigStateCategory].push_back(c);
        }
        else if (c->appType == "Rotator")
        {
            // no action
        }
        else if (c->appType == "Server")
        {
//            routerAppCatMap[rpcConstants::LocalStationCategory].push_back(c);
//            routerAppCatMap[rpcConstants::StationCategory].push_back(c);
        }
        else if (c->appType == "KSTClient")
        {

        }

    }

    rpc->setRouterAppCatMap(routerAppCatMap);
}
void ControlMain::subscribeApps()
{
    // This is called whenever the possible set of apps may have changed

    trace("subscribeApps");
    rigCache.invalidate();

    getRouterAppCatMap();

}

void ControlMain::onStdInRead(QString cmd)
{
    executeStdIn(cmd);
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
    bool show = getShowApp();
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
void ControlMain::on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
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
void ControlMain::on_notify( AnalysePubSubNotify an, const QString from )
{
    // PubSub notifications
    trace( "Notify callback from " + from + ( an.getOK() ? ":Error " : ":Normal " ) +  an.getPublisherProgram() + "@" + an.getPublisherRouter());

    if ( an.getOK())
    {
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
