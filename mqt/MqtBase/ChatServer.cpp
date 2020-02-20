#include "MinosRPC.h"
#include "MinosLoggerEvents.h"

#include "ChatServer.h"

static bool syncstat = false;
static QVector<QString> chatQueue;
const char * ChatServer::stateIndicator[] =
{
    QT_TR_NOOP("Available"),
    QT_TR_NOOP("Not Available"),
    QT_TR_NOOP("No Contact")
};


ChatServer *ChatServer::chatServer = nullptr;


ChatServer *ChatServer::getChatServer()
{
    if (!chatServer)
    {
        chatServer = new ChatServer();
    }
    return chatServer;
}
ChatServer::ChatServer()
{
    connect(&SyncTimer, SIGNAL(timeout()), this, SLOT(SyncTimerTimer()));
    SyncTimer.start(100);

    MinosRPC *rpc = MinosRPC::getMinosRPC();

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(&MinosLoggerEvents::mle, SIGNAL(RigFreqChanged(QString,BaseContestLog*)), this, SLOT(onRigFreqChanged(QString,BaseContestLog*)));
}

ChatServer::~ChatServer()
{
}
void ChatServer::on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &/*from*/ )
{
    AnalysePubSubNotify an( err, mro );

    if ( an.getOK() )
    {
        if ( an.getCategory() == rpcConstants::LocalStationCategory)
        {
            QString server = an.getKey();
            QVector<Server>::iterator stat;
            bool pubNeeded = true;
            QString a = MinosRPC::getMinosRPC()->getAppName();
            for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
            {
                if ((*stat).app == a + "@" + server)
                {
                    pubNeeded = false;
                    break;
                }
            }
            if (pubNeeded)
            {
                RPCPubSub::publish(rpcConstants::ChatServer,  a + "@" + server, "", psPublished);
            }
        }
        if (an.getCategory() == rpcConstants::StationCategory)
        {
            QString server = an.getKey();
            QVector<Server>::iterator stat;
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
                RPCPubSub::subscribeRemote(server, rpcConstants::ChatCategory);
                RPCPubSub::subscribeRemote(server, rpcConstants::ChatServer);
            }
        }

        if ( an.getCategory() == rpcConstants::ChatServer )
        {
            trace( QString(stateIndicator[an.getState()]) + " " + an.getCategory() + " " + an.getKey() );
            QVector<Server>::iterator stat;
            bool chatFound = false;
            for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
            {
                if ((*stat).app == an.getKey())
                {
                    if ((*stat).state != an.getState())
                    {
                        (*stat).state = an.getState();
                        QString mess = tr("%1 changed state to %2").arg(an.getKey()).arg(tr(stateIndicator[an.getState()]));
                        addChat( mess );
                        syncstat = true;
                    }
                    chatFound = true;
                    break;
                }
            }
            if ( !chatFound )
            {
                // We have received notification from a previously unknown station - so report on it
                Server s;
                s.serverName = an.getPublisherServer();
                s.state = an.getState();
                s.app = an.getKey();
                serverList.push_back( s );
                QString mess = tr("%1 changed state to %2").arg(an.getKey()).arg(tr(stateIndicator[an.getState()]));
                addChat( mess );
                syncstat = true;
            }
        }
        if ( an.getCategory() == rpcConstants::ChatCategory )
        {
            trace( QString(stateIndicator[an.getState()]) + " " + an.getCategory() + " " + an.getKey() + " " + an.getValue() );
            if (an.getKey() == rpcConstants::ChatServerFrequency)
            {
                QVector<Server>::iterator stat;
                for ( stat = serverList.begin(); stat != serverList.end(); stat++ )
                {
                    if ((*stat).serverName == an.getPublisherServer())
                    {
                        if ((*stat).freq != an.getValue())
                        {
                            (*stat).freq = an.getValue();
                            syncstat = true;
                        }
                        break;
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void ChatServer::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{

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
void ChatServer::SyncTimerTimer(  )
{
    syncStations();
    syncChat();
}

//---------------------------------------------------------------------------
void ChatServer::syncStations()
{
    if ( syncstat )
    {
        syncstat = false;

        emit ChatServerList(serverList);
    }
}
void ChatServer::addChat(const QString &mess)
{
    QDateTime dt = QDateTime::currentDateTime();
    QString sdt = dt.toString( "HH:mm:ss " ) + mess;
    chatQueue.push_back(sdt);
}
void ChatServer::syncChat()
{
    if (chatQueue.count())
    {
        emit ChatMessages(chatQueue);
        chatQueue.clear();
    }
}
//---------------------------------------------------------------------------
void ChatServer::sendMessage(QString mess)
{
    // We need to send the message to all connected stations
    for ( QVector<Server>::iterator i = serverList.begin(); i != serverList.end(); i++ )
    {
        RPCGeneralClient rpc(rpcConstants::chatMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember( mess, rpcConstants::SendChatMessage );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( (*i).app );
    }
}
void ChatServer::onRigFreqChanged(QString f, BaseContestLog * /*c*/)
{
    RPCPubSub::publish(rpcConstants::ChatCategory, rpcConstants::ChatServerFrequency, f, psPublished);
}
