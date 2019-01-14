#include "ConfigFile.h"
#include "ContestApp.h"

#include "WsjtxFrame.h"
#include "ui_WsjtxFrame.h"
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

WsjtxFrame::WsjtxFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::WsjtxFrame)
{
    ui->setupUi(this);

    MinosRPC *rpc = MinosRPC::getMinosRPC();

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));

}

WsjtxFrame::~WsjtxFrame()
{
    delete ui;
}

void WsjtxFrame::setContest(BaseContestLog *c)
{
    ct = c;
}

void WsjtxFrame::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from)
{
    if (ct && ct == TContestApp::getContestApp() ->getCurrentContest())
    {

    }
}

void WsjtxFrame::on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from)
{
    AnalysePubSubNotify an( err, mro );
    trace( "Notify callback from " + from + ( err ? ":Error " : ":Normal " ) +  an.getPublisherProgram() + "@" + an.getPublisherServer());
    if ( an.getOK() )
    {
        MinosRPC *rpc = MinosRPC::getMinosRPC();

        if ( an.getCategory() == rpcConstants::wsjtConnector)
        {
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
                trace(QString("***" + an.getKey() + " changed state to " + wsjtStateList[an.getState()] + " and added"));
                QString mess = an.getKey() + " changed state to " + wsjtStateList[an.getState()] + " and added";

                QString a = rpc->getAppName();
                QString station = MinosConfig::getMinosConfig()->getThisServerName();
                RPCPubSub::publish(rpcConstants::wsjtClientServer,  a + "@" + station, "", psPublished);
            }

            // and save on server list for sending commands
        }
    }
}
