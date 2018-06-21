#include "MinosRPC.h"

#include "ChatFrame.h"
#include "ui_ChatFrame.h"

static bool syncstat = false;
static QVector<QString> chatQueue;
static QString stateIndicator[] =
{
   "Available",
   "NotAvailable",
   "NoContact"
};
static QString stateList[] =
{
   "Available",
   "Not Available",
   "No Contact"
};

ChatFrame::ChatFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ChatFrame)
{
    ui->setupUi(this);
    connect(&SyncTimer, SIGNAL(timeout()), this, SLOT(SyncTimerTimer()));
    SyncTimer.start(100);

    MinosRPC *rpc = MinosRPC::getMinosRPC(rpcConstants::chatApp, false);    // DO NOT use the environment variable - use "Chat" everywhere

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));

    rpc->subscribe(rpcConstants::LocalStationCategory);
}

ChatFrame::~ChatFrame()
{
    delete ui;
}
void ChatFrame::on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &/*from*/ )
{
    AnalysePubSubNotify an( err, mro );

    if ( an.getOK() )
    {
      if ( an.getCategory() == rpcConstants::LocalStationCategory)
      {
          QString s = an.getKey();
          QString a = MinosRPC::getMinosRPC()->getAppName();
          RPCPubSub::publish(rpcConstants::ChatCategory, rpcConstants::ChatServer, a + "@" + s, psPublished);
          RPCPubSub::subscribe(rpcConstants::StationCategory);
      }
      if (an.getCategory() == rpcConstants::StationCategory)
      {
          QString key = an.getKey();
          RPCPubSub::subscribeRemote(key, rpcConstants::ChatCategory);
      }

      if ( an.getCategory() == rpcConstants::ChatCategory )
      {
         trace( QString(stateIndicator[an.getState()]) + " " + an.getKey() + " " + an.getValue() );
         QVector<Server>::iterator stat;
         for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
         {
            if ((*stat).name == an.getPublisherServer())
            {
               if ((*stat).state != an.getState())
               {
                  (*stat).state = an.getState();
                  QString mess = an.getPublisherServer() + " changed state to " + stateList[an.getState()];
                  addChat( mess );
                  syncstat = true;
               }
               break;
            }
         }

         if ( stat == serverList.end() )
         {
            // We have received notification from a previously unknown station - so report on it
            Server s;
            s.name = an.getPublisherServer();
            s.ip = an.getValue();
            s.state = an.getState();
            s.app = an.getValue();
            serverList.push_back( s );
            QString mess = an.getPublisherServer() + " changed state to " + stateList[an.getState()];
            addChat( mess );
            syncstat = true;
         }
      }
   }
}
//---------------------------------------------------------------------------
void ChatFrame::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
   trace( "chat callback from " + from + ( err ? ":Error" : ":Normal" ) );

   // Should we use QMap to give a list of name/value pairs?
   // BUT the value isn't always the same type - should it be?
   // We could use QVariant, of course...

   if ( !err )
   {
      RPCArgs *args = mro->getCallArgs();

      if (args)
      {
            QSharedPointer<RPCParam> psMess;
            if (args->getStructArgMember(0, rpcConstants::SendChatMessage, psMess))
            {
                QString pmess;
                if (psMess->getString(pmess))
                {
                   // add to chat window
                   QString mess = from + " : " + pmess;
                   addChat( mess );
                }
            }
      }
   }
}
void ChatFrame::SyncTimerTimer(  )
{
   syncStations();
   syncChat();

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

//---------------------------------------------------------------------------
void ChatFrame::syncStations()
{
   if ( syncstat )
   {
      syncstat = false;
      ui->StationList->clear();
      for ( QVector<Server>::iterator i = serverList.begin(); i != serverList.end(); i++ )
      {
         // should we link an object with the state, and owner draw
          QString state = stateIndicator[(*i).state] + " " + (*i).name;
         ui->StationList->addItem( state );
         trace("syncStations " + state);
      }
   }
}
void ChatFrame::addChat(const QString &mess)
{
   QDateTime dt = QDateTime::currentDateTime();
   QString sdt = dt.toString( "HH:mm:ss " ) + mess;
   chatQueue.push_back(sdt);
}
void ChatFrame::syncChat()
{
   for ( QVector<QString>::iterator i = chatQueue.begin(); i != chatQueue.end(); i++ )
   {
      ui->ChatMemo->append( (*i) );
      trace("syncChat " + (*i));
   }
   chatQueue.clear();
}
//---------------------------------------------------------------------------

void ChatFrame::on_SendButton_clicked()
{
    // We need to send the message to all connected stations
    for ( QVector<Server>::iterator i = serverList.begin(); i != serverList.end(); i++ )
    {
        RPCGeneralClient rpc(rpcConstants::chatMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember( ui->ChatEdit->text(), rpcConstants::SendChatMessage );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( (*i).app );
    }
    ui->ChatEdit->clear(); // otherwise it is a pain!
    ui->ChatEdit->setFocus();
}
void ChatFrame::keyPressEvent( QKeyEvent* event )
{
    int Key = event->key();

    if (Key == Qt::Key_Return || Key == Qt::Key_Enter)
    {
        ui->SendButton->clicked();
    }
    if (Key == Qt::Key_Escape)
    {
        ui->ChatEdit->clear();
    }
}
