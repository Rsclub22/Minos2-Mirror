#include "MinosRPC.h"
#include "MinosLoggerEvents.h"
#include "contest.h"
#include "ChatServer.h"

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
    connect(&SyncTimer, &QTimer::timeout, this, &ChatServer::SyncTimerTimer);
    SyncTimer.start(100);

    MinosRPC *rpc = MinosRPC::getMinosRPC();

    QStringList chatCats = {
        rpcConstants::ChatCategory
    };

    rpc->findProviders(rpcConstants::ChatServer, chatCats);

    connect(rpc, &MinosRPC::routerCall, this, &ChatServer::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &ChatServer::on_notify);
    connect(rpc, &MinosRPC::provider, this, &ChatServer::on_provider);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::RigFreqChanged, this, &ChatServer::onRigFreqChanged);

    QString a = MinosRPC::getMinosRPC()->getAppName();
    QString s = MinosConfig::getMinosConfig()->getThisRouterName();

    RPCPubSub::publish(rpcConstants::ChatServer,  a + "@" + s, "", psPublished);

}

ChatServer::~ChatServer()
{
}
void ChatServer::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{
    if ( an.getOK() )
    {
        if ( an.getCategory() == rpcConstants::ChatCategory )
        {
            //trace( QString("ChatServer::on_notify ") + QString(stateIndicator[an.getState()]) + " " + an.getCategory() + " " + an.getKey() + " " + an.getValue() );
            if (an.getKey() == rpcConstants::ChatServerFrequency)
            {
                Provider p(an);
                Frequency f = Frequency(an.getValue());
                if (chatServerList[p].freq != f)
                {
                    //trace("stat freq set");
                    chatServerList[p].freq = f;
                    syncstat = true;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void ChatServer::on_provider(Provider provider)
{
    ChatServerApp s;
    chatServerList[provider] = s;
    QString mess = tr("%1/%2 changed state to %3").arg(provider.routerName, provider.app, tr(stateIndicator[provider.state]));
    addChat( mess );
    syncstat = true;
}
//---------------------------------------------------------------------------
void ChatServer::on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
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

        emit ChatServerList(chatServerList);
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
    for(QMap<Provider, ChatServerApp>::iterator i = chatServerList.begin(); i != chatServerList.end(); i++)
    {
        RPCGeneralClient rpc(rpcConstants::chatMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember( mess, rpcConstants::SendChatMessage );
        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( i.key().psn() );
    }
}
void ChatServer::onRigFreqChanged(Frequency f, BaseContestLog * /*c*/)
{
    RPCPubSub::publish(rpcConstants::ChatCategory, rpcConstants::ChatServerFrequency, f.str(), psPublished);
}
