#include "ConfigFile.h"

#include "WConMain.h"
#include "ui_WConMain.h"
static QString wsjtStateIndicator[] =
{
    "Available",
    "NotAvailable",
    "NoContact"
};
static QString wsjtStateList[] =
{
   "Available",
   "Not Available",
   "No Contact"
};


WConMain::WConMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WConMain)
{
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
    connect(&LogTimer, SIGNAL(timeout()), this, SLOT(LogTimerTimer()));

    LogTimer.start(100);

    //RPCPubSub::subscribe(rpcConstants::LocalStationCategory);
    RPCPubSub::subscribe(rpcConstants::StationCategory);

    QString a = rpc->getAppName();
    QString station = MinosConfig::getMinosConfig()->getThisServerName();
    RPCPubSub::publish(rpcConstants::wsjtConnector,  a + "@" + station, "", psPublished);
}

WConMain::~WConMain()
{
    delete ui;
}
void WConMain::onStdInRead(QString cmd)
{
    trace("Command read from stdin: " + cmd);
    if (cmd.indexOf("ShowServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(true);
    if (cmd.indexOf("HideServers", 0, Qt::CaseInsensitive) >= 0)
        setShowServers(false);

}
void WConMain::on_pushButton_clicked()
{
    close();
}
void WConMain::closeEvent(QCloseEvent * event)
{
    LogTimer.stop();
    event->accept();
    //QWidget::closeEvent(event);
}
void WConMain::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void WConMain::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void WConMain::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}
void WConMain::LogTimerTimer( )
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
void WConMain::on_notify( bool err, QSharedPointer<MinosRPCObj>mro, const QString &from )
{
    AnalysePubSubNotify an( err, mro );
    trace( "Notify callback from " + from + ( err ? ":Error " : ":Normal " ) +  an.getPublisherProgram() + "@" + an.getPublisherServer());
    if ( an.getOK() )
    {
        MinosRPC *rpc = MinosRPC::getMinosRPC();
        if (an.getCategory() == rpcConstants::StationCategory)
        {
            QString server = an.getKey();
            QVector<WsjtServer>::iterator stat;
            bool subNeeded = true;
            for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
            {
                if ((*stat).serverName == server)
                {
                    subNeeded = false;
                    break;
                }
            }
            if (subNeeded)
            {
                RPCPubSub::subscribeRemote(server, rpcConstants::wsjtClientServer);
            }
        }

        if ( an.getCategory() == rpcConstants::wsjtClientServer )
        {
            //trace( QString("***" + clusterStateIndicator[an.getState()]) + " " + an.getCategory() + " " + an.getKey() );
            QVector<WsjtServer>::iterator stat;
            bool clusterFound = false;
            for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
            {
                if ((*stat).app == an.getKey())
                {
                    if ((*stat).state != an.getState())
                    {
                        (*stat).state = an.getState();
                        QString mess = an.getKey() + " changed state to " + wsjtStateList[an.getState()];
                        trace(QString("On notify: %1").arg(mess));
                    }
                    clusterFound = true;
                    break;
                }
            }
            if ( !clusterFound )
            {
                // We have received notification from a previously unknown station - so report on it
                WsjtServer s;
                s.serverName = an.getPublisherServer();
                s.state = an.getState();
                s.app = an.getKey();
                serverList.push_back( s );

                QString mess = an.getKey() + " changed state to " + wsjtStateList[an.getState()] + " and added";
                trace(QString("***" + mess));

                //mShowMessage(mess, this);

            }
        }
    }
}
void WConMain::on_serverCall(bool err, QSharedPointer<MinosRPCObj> /*mro*/, const QString &from )
{
   trace( "control callback from " + from + ( err ? ":Error" : ":Normal" ) );

   if ( !err )
   {

   }
}
