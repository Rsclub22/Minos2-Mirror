#include "minos_pch.h"

#include "MinosLink.h"
#include "clientThread.h"
#include "serverThread.h"
#include "MServerZConf.h"
#include "MServer.h"
#include "MServerPubSub.h"

#include "servermain.h"
#include "ui_servermain.h"

ServerMain *MinosMainForm = nullptr;

bool closeApp = false;

ServerMain::ServerMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(&stdinReader, &StdInReader::stdinLine, this, &ServerMain::onStdInRead);
    stdinReader.start();

    createCloseEvent();
    QSettings settings;
    QByteArray geometry = settings.value("geometry/Main").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    connect(&LogTimer, SIGNAL(timeout()), this, SLOT(LogTimerTimer()));

    QString sname = ThisMinosRouter::getThisMinosRouter()->getRouterName();

    clientListener = QSharedPointer<MinosClientListener>(new MinosClientListener);
    clientListener ->initialise( "Client", MinosClientPort );

    routerListener = QSharedPointer<MinosRouterListener>(new MinosRouterListener);
    routerListener ->initialise( "Server", MinosRouterPort );

    ZConf = QSharedPointer<TZConf>(new TZConf);

    PubSubMain = QSharedPointer<TPubSubMain>(new TPubSubMain);
    ZConf->startZConf (sname );

    LogTimer.start(100);
    ScanTimer.start(20000);

    makeRouterEvent( true );
}

ServerMain::~ServerMain()
{
    delete ui;
}
void ServerMain::onStdInRead(QString cmd)
{
    if (cmd.indexOf("Shutdown", 0, Qt::CaseInsensitive) >= 0)
    {
        closeApp = true;
    }
    executeStdIn(cmd);
}

void ServerMain::LogTimerTimer( )
{

   static int lastServerCount = 0;
   static int lastClientCount = 0;
   static int lastSubCount = 0;
   static int lastPubCount = 0;

   MinosRouterListener *msl = MinosRouterListener::getListener();
   MinosClientListener *mcl = MinosClientListener::getListener();
   int serverCount = msl ? msl->getConnectionCount() : 0;
   int clientCount = mcl ? mcl->getConnectionCount() : 0;
   int pubCount = GetPublishedCount();
   int subCount = GetSubscribedCount();

   if ( lastServerCount != serverCount )
   {
      lastServerCount = serverCount;
      ui->ServerLabel->setText(QString::number(serverCount));
   }
   if ( lastClientCount != clientCount )
   {
      lastClientCount = clientCount;
      ui->ClientLabel->setText(QString::number(clientCount));
   }

   if ( lastSubCount != subCount )
   {
      lastSubCount = subCount;
      ui->SubLabel->setText(QString::number(subCount));
   }
   if ( lastPubCount != pubCount )
   {
      lastPubCount = pubCount;
      ui->PubLabel->setText(QString::number(pubCount));
   }

   bool show = getShowApp();
   if ( !isVisible() && show )
   {
      setVisible(true);
   }
   if ( isVisible() && !show )
   {
      setVisible(false);
   }

   if ( closeApp )
      close();

   static bool closed = false;
   if ( !closed )
   {
      if ( checkCloseEvent() )
      {
         closeApp = true;
         closed = true;
         on_CloseButton_clicked( );
      }
   }
}

void ServerMain::on_CloseButton_clicked()
{
    trace("Server close requested");
    closeApp = true;
}
void ServerMain::closeEvent(QCloseEvent *event)
{
    static bool closeSeen = false;
    if (!closeSeen)
    {
        closeSeen = true;
        trace("Server close event seen");
        closeApp = true;
        PubSubMain->closeDown();
        ZConf->closeDown();

        clientListener ->closeDown();
        routerListener ->closeDown();

        LogTimerTimer( );

        QWidget::closeEvent(event);
    }
}
void ServerMain::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry/Main", saveGeometry());
    QWidget::resizeEvent(event);
}
void ServerMain::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry/Main", saveGeometry());
    QWidget::moveEvent(event);
}
void ServerMain::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry/Main", saveGeometry());
    }
}


void ServerMain::on_showButton_clicked()
{
    // show all the details
    if (!sd)
    {
        sd = QSharedPointer<ServerDetails>(new ServerDetails(this));
    }
    sd->show();

}
