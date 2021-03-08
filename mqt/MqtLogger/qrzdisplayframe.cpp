
#include "qrzdisplayframe.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "ui_qrzdisplayframe.h"








QrzDisplayFrame::QrzDisplayFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::QrzDisplayFrame)
{
    ui->setupUi(this);
    clear();


    connect (QrzDisplayServerRpc::getQrzDisplayServerRpc(), SIGNAL(clusterQrzMsg(QrzServerMessage)), this, SLOT(onLoggerQrzMessage(QrzServerMessage)));
    connect (QrzDisplayServerRpc::getQrzDisplayServerRpc(), SIGNAL(loggerQrzReply(QrzCallsignData, QString, QString)), this, SLOT(onLoggerQrzReply(QrzCallsignData, QString, QString)));


}

QrzDisplayFrame::~QrzDisplayFrame()
{
    delete ui;
}


void QrzDisplayFrame::onLoggerQrzMessage(QrzServerMessage qrzRequest)
{

    qrzRequestQueue += qrzRequest;

}

void QrzDisplayFrame::onLoggerQrzReply(QrzCallsignData cd, QString qrzReplyState, QString uuid)
{
    clear();
    if (qrzReplyState.isEmpty())
    {
        ui->callsignText->setText(cd.getCallsign());
        ui->nameText->setText(cd.getFirstName());
        ui->townText->setText(cd.getQth());
        ui->qraText->setText(cd.getQra());
        if (!cd.getQra().isEmpty())
        {
            distance = 0;
            bearing = 0;
            calcSpotDistanceBearing(cd.getQra(), &distance, &bearing);

            ui->distanceText->setText(QString::number(distance));
            if (bearing >= 0 && bearing <= 360)
            {
                ui->bearingText->setText(QString::number(bearing));
            }

        }
        ui->countryText->setText(cd.getCountry());
        ui->cqZoneText->setText(cd.getCqZone());
        ui->ituZoneText->setText(cd.getItuZone());
    }
    else
    {
        ui->callsignText->setText(cd.getCallsign());
        ui->qrzMessageText->setText(qrzReplyState);
    }


}



void QrzDisplayFrame::calcSpotDistanceBearing(const QString& _locator, double* distance, int* bearing)
{
    bool locValid = true;
    QString locator = _locator;
    double latitude;
    double longitude;
    double dist;
    int brg = 0;

    if (ct && !locator.isEmpty())
    {
        if (locator.count() == 4)
        {
            locator.append("MM");
        }

        int locValres = lonlat( locator, longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() );
        if ( ( locValres ) != LOC_OK )
        {
            locValid = false;
        }
        if (locValid)
        {
            ct->disbeara(longitude, latitude, dist, brg);
            *distance = dist;
            *bearing = brg;
        }
    }
}


void QrzDisplayFrame::clear()
{
    ui->callsignText->clear();
    ui->nameText->clear();
    ui->townText->clear();
    ui->qraText->clear();
    ui->distanceText->clear();
    ui->bearingText->clear();
    ui->cqZoneText->clear();
    ui->ituZoneText->clear();
    ui->qrzMessageText->clear();

}


void QrzDisplayFrame::getQrzDetailsForLogger(QString callsign)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    QString frame_uuid = tslf->getContest()->uuid;
    trace(QString("qrzDisplayFrame: sending callsign %1, from frame uuid %2 to QrzServer").arg(callsign, frame_uuid));
    QrzDisplayServerRpc::getQrzDisplayServerRpc()->sendCallsignFromLoggerToQrzServer(callsign, frame_uuid);


}


void QrzDisplayFrame::setContest(BaseContestLog( *c))
{
    ct = c;
    if (ct == nullptr)
    {
        return;
    }
}



// -------- QRZ Display RPC -----------------------------------------------------



static bool syncstat = false;
static QVector<QrzServerMessage> qrzRequestsQueue;

QrzDisplayServerRpc *QrzDisplayServerRpc::qrzDisplayServerRpc = nullptr;

const char * QrzDisplayServerRpc::stateIndicator[] =
{
    QT_TR_NOOP("Available"),
    QT_TR_NOOP("Not Available"),
    QT_TR_NOOP("No Contact")
};


QrzDisplayServerRpc *QrzDisplayServerRpc::getQrzDisplayServerRpc()
{
    if (!qrzDisplayServerRpc)
    {
        qrzDisplayServerRpc = new QrzDisplayServerRpc();
    }
    return qrzDisplayServerRpc;
}



QrzDisplayServerRpc::QrzDisplayServerRpc()
{
    //connect(&SyncTimer, SIGNAL(timeout()), this, SLOT(SyncTimerTimer()));
    //SyncTimer.start(100);

    MinosRPC *rpc = MinosRPC::getMinosRPC();

    QStringList sv{
        rpcConstants::qrzServerApp
    };

    rpc->initialiseRouters(sv);

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(AnalysePubSubNotify ,QString)), this, SLOT(on_notify(AnalysePubSubNotify ,QString)));

    QString a = rpc->getAppName();
    QString station = MinosConfig::getMinosConfig()->getThisRouterName();
    RPCPubSub::publish(rpcConstants::qrzDisplayApp,  a + "@" + station, "", psPublished);


}

QrzDisplayServerRpc::~QrzDisplayServerRpc()
{
}





void QrzDisplayServerRpc::sendCallsignFromLoggerToQrzServer(QString callsign, QString frameid)
{
    for (auto const &s: qAsConst(serverList))
    {
        trace(QString("Send Qrz Response to Cluster Server = %1").arg(s.app));
        RPCGeneralClient rpc(rpcConstants::qrzLogger );
        QSharedPointer<RPCParam>st(new RPCParamStruct);
        st->addMember(rpcConstants::qrzLogger, rpcConstants::qrzLogger);
        st->addMember( callsign, rpcConstants::qrzDxCallsign );
        st->addMember(frameid, rpcConstants::qrzLogFrameId);

        rpc.getCallArgs() ->addParam( st );
        rpc.queueCall( s.app );

    }
}


void QrzDisplayServerRpc::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
{
    trace(QString("QrzServer: on_serverCall - Message from %1").arg(from));
    if ( !err )
    {
        RPCArgs *args = mro->getCallArgs();


        if (args)
        {
            QSharedPointer<RPCParam> msgQrzLoggerResponse;
            QSharedPointer<RPCParam> msgDxCall;
            QSharedPointer<RPCParam> msgQrzFirstName;
            QSharedPointer<RPCParam> msgQrzName;
            QSharedPointer<RPCParam> msgQrzCounty;
            QSharedPointer<RPCParam> msgQrzQth;
            QSharedPointer<RPCParam> msgQrzCountry;
            QSharedPointer<RPCParam> msgQrzLat;
            QSharedPointer<RPCParam> msgQrzLon;
            QSharedPointer<RPCParam> msgQrzDxGrid;
            QSharedPointer<RPCParam> msgQrzCQZone;
            QSharedPointer<RPCParam> msgQrzITUZone;
            QSharedPointer<RPCParam> msgQrzDxReplyState;

            QSharedPointer<RPCParam> msgLogFrameId;

            QrzCallsignData cd;


            if (args->getStructArgMember(0, rpcConstants::qrzLoggerResponse, msgQrzLoggerResponse)
                    && args->getStructArgMember(0, rpcConstants::qrzDxCallsign, msgDxCall)
                    && args->getStructArgMember(0, rpcConstants::qrzFirstName, msgQrzFirstName)
                    && args->getStructArgMember(0, rpcConstants::qrzName, msgQrzName)
                    && args->getStructArgMember(0, rpcConstants::qrzCounty, msgQrzCounty)
                    && args->getStructArgMember(0, rpcConstants::qrzQth, msgQrzQth)
                    && args->getStructArgMember(0, rpcConstants::qrzCountry, msgQrzCountry)
                    && args->getStructArgMember(0, rpcConstants::qrzLat, msgQrzLat)
                    && args->getStructArgMember(0, rpcConstants::qrzLon, msgQrzLon)
                    && args->getStructArgMember(0, rpcConstants::qrzDxGrid, msgQrzDxGrid)
                    && args->getStructArgMember(0, rpcConstants::qrzCqZone, msgQrzCQZone)
                    && args->getStructArgMember(0, rpcConstants::qrzItuZone, msgQrzITUZone)
                    && args->getStructArgMember(0, rpcConstants::qrzDxReplyState, msgQrzDxReplyState)
                    && args->getStructArgMember(0, rpcConstants::qrzLogFrameId, msgLogFrameId))
            {
                QString callsign;
                msgDxCall->getString(callsign);
                cd.setCallsign(callsign);

                QString firstName;
                msgQrzFirstName->getString(firstName);
                cd.setFirstName(firstName);

                QString name;
                msgQrzName->getString(name);
                cd.setName(name);

                QString county;
                msgQrzCounty->getString(county);
                cd.setCounty(county);

                QString qth;
                msgQrzQth->getString(qth);
                cd.setQth(qth);

                QString country;
                msgQrzCountry->getString(country);
                cd.setCountry(country);

                QString lat;
                msgQrzLat->getString(lat);
                cd.setLat(lat);

                QString lon;
                msgQrzLon->getString(lon);
                cd.setLon(lon);

                QString dxGrid;
                msgQrzDxGrid->getString(dxGrid);
                cd.setQra(dxGrid);

                QString cqZone;
                msgQrzCQZone->getString(cqZone);
                cd.setCqZone(cqZone);

                QString ituZone;
                msgQrzITUZone->getString(ituZone);
                cd.setItuZone(ituZone);

                QString dxReplyState;
                msgQrzDxReplyState->getString(dxReplyState);

                QString uuid;
                msgLogFrameId->getString(uuid);


                emit loggerQrzReply(cd, dxReplyState, uuid);

            }

        }
    }
}


void QrzDisplayServerRpc::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{


    trace("qrzDisplayServer: on_notify");
    if ( an.getOK() )
    {

        if ( an.getCategory() == rpcConstants::ChatServer )
        {
            trace( QString(stateIndicator[an.getState()]) + " " + an.getCategory() + " " + an.getKey() );
            bool stationFound = false;
            for ( auto &stat: serverList )
            {
                if (stat.app == an.getKey())
                {
                    if (stat.state != an.getState())
                    {
                        stat.state = an.getState();
                        QString mess = tr("%1 changed state to %2").arg(an.getKey()).arg(tr(stateIndicator[an.getState()]));
                        //addChat( mess );
                        syncstat = true;
                    }
                    stationFound = true;
                    break;
                }
            }
            if ( !stationFound )
            {
                // We have received notification from a previously unknown station - so report on it
                QrzServer s;
                s.routerName = an.getPublisherRouter();
                s.state = an.getState();
                s.publisherProgram = an.getPublisherProgram();
                s.app = an.getKey();
                serverList.push_back( s );
                trace(QString("qrzDisplayServerRpc: on_notify - server found %1, publisher program %2, key %3").arg(s.routerName, s.publisherProgram, s.app));
                //QString mess = tr("%1 changed state to %2").arg(an.getKey()).arg(tr(stateIndicator[an.getState()]));

                syncstat = true;
            }
        }

    }

}

/*
void QrzServerRpc::SyncTimerTimer(  )
{
    if (qrzRequestsQueue.count())
    {
        //emit qrzRequestQueue(qrzRequestsQueue);
        //qrzRequestsQueue.clear();
    }
}

*/
