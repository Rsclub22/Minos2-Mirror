
#include "qrzdisplayframe.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "ui_qrzdisplayframe.h"
#include "minosqlabel.h"

#include <QSizePolicy>
#include <QDebug>

const int PINGTIMER_DURATION = 10000;





QrzDisplayFrame::QrzDisplayFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::QrzDisplayFrame),
    receivedServerPing(false )
{
    ui->setupUi(this);
    clear();

    ui->searchQrzLineEdit->setValidator(&ucValidator);

    connect (QrzDisplayServerRpc::getQrzDisplayServerRpc(), &QrzDisplayServerRpc::loggerQrzReply, this, &QrzDisplayFrame::onLoggerQrzReply);
    connect (QrzDisplayServerRpc::getQrzDisplayServerRpc(), &QrzDisplayServerRpc::qrzServerLoggedState, this, &QrzDisplayFrame::onQrzServerLoggedState);
    onQrzServerLoggedState(false, "");

    connect (ui->searchQrzLineEdit, &QLineEdit::returnPressed, this, &QrzDisplayFrame::onSearchQrzReturnPressed);
    connect (ui->callsignText, &MinosQLabel::mouseDoubleClicked, this, &QrzDisplayFrame::onCallsignTextMouseDoubleClicked);
    connect (ui->qraText, &MinosQLabel::mouseDoubleClicked, this, &QrzDisplayFrame::onQraTextMouseDoubleClicked);
    connect (ui->nameText, &MinosQLabel::mouseDoubleClicked, this, &QrzDisplayFrame::onNameTextMouseDoubleClicked);
    connect (ui->bearingText, &MinosQLabel::mouseDoubleClicked, this, &QrzDisplayFrame::onBearingMouseDoubleClicked);

    setLogonPushButtonLabelText(false);

    ui->callsignText->setToolTip(tr("Double Click to transfer Callsign, Locator and Name to log"));
    ui->qraText->setToolTip(tr("Double Click to transfer locator to log"));
    ui->bearingText->setToolTip(tr("Double Click to transfer Bearing to rotator control"));
    ui->nameText->setToolTip(tr("Double Click to transfer Name to comments in log"));

    setQrzMessageText(tr("Note! Qrz XML subscription required to get QRA"));

    serverPingTimer = new QTimer(this);
    connect (serverPingTimer, &QTimer::timeout, this, [=](){onServerPingTimerTimeout();});
    serverPingTimer->start(PINGTIMER_DURATION);
}

QrzDisplayFrame::~QrzDisplayFrame()
{
    delete ui;
}


void QrzDisplayFrame::onSearchQrzReturnPressed()
{
    if (!ui->searchQrzLineEdit->text().isEmpty())
    {
        Callsign cs;
        QString callsign = ui->searchQrzLineEdit->text().trimmed();
        cs.setFullCall(callsign);
        if (cs.getValRes() == CS_OK)
        {
            trace(QString("QrzDisplayFrame - Search for callsign = %1").arg(callsign));
            getQrzDetailsForLogger(cs.getFullCall());
        }
        else
        {
            setQrzMessageText(tr("Search callsign invalid"));
        }



    }


}


void QrzDisplayFrame::onCallsignTextMouseDoubleClicked()
{


    if (!ui->callsignText->text().isEmpty())
    {

        MinosLoggerEvents::sendQRZInfoToLog(ui->callsignText->text(), ui->qraText->text(), ui->nameText->text());
    }

}


void QrzDisplayFrame::onQraTextMouseDoubleClicked()
{
    if (!ui->qraText->text().isEmpty())
    {
        MinosLoggerEvents::sendQRZInfoToLog("" , ui->qraText->text(), "");
    }
}

void QrzDisplayFrame::onBearingMouseDoubleClicked()
{
    if (!ui->bearingText->text().isEmpty())
    {
        MinosLoggerEvents::SendSpotBrgStrToRot(ui->bearingText->text());
    }
}

void QrzDisplayFrame::onNameTextMouseDoubleClicked()
{
    if (!ui->nameText->text().isEmpty())
    {
        MinosLoggerEvents::sendQRZInfoToLog("" , "", ui->nameText->text());
    }
}


void QrzDisplayFrame::onQrzServerLoggedState(bool state, QString stateMessage)
{
    receivedServerPing = true;

    if (state)
    {
        ui->logOnStatusPb->setStyleSheet(QRZ_BUTTON_ON_STYLE);
        setLogonPushButtonLabelText(true);
        setQrzMessageText("");

    }
    else
    {
        ui->logOnStatusPb->setStyleSheet(QRZ_BUTTON_OFF_STYLE);
        setLogonPushButtonLabelText(false);
    }



    if (!stateMessage.isEmpty())
    {
        setQrzMessageText(stateMessage);

    }

}

void QrzDisplayFrame::onServerPingTimerTimeout()
{
    if (receivedServerPing)
    {
       // setLogonPushButtonLabelText(true);;
        receivedServerPing = false;
    }
    else
    {
        setLogonPushButtonLabelText(false);
    }
}

void QrzDisplayFrame::onLoggerQrzReply(QrzCallsignData cd, QString qrzReplyState, QString /*uuid*/)
{
    if (!ct->isAgeProtected())
    {
        clear();
        if (qrzReplyState.isEmpty())
        {
            ui->callsignText->setText(cd.getCallsign());
            ui->nameText->setText(cd.getFirstName().left(20));
            ui->addr1Text->setText(cd.getAddr1().left(20));
            ui->addr2Text->setText(cd.getAddr2().left(20));
            ui->qraText->setText(cd.getQra());
            if (!cd.getQra().isEmpty())
            {
                distance = 0;
                bearing = 0;
                calcSpotDistanceBearing(cd.getQra(), &distance, &bearing);

                ui->distanceText->setText(QString::number(distance));
                if (bearing >= 0 && bearing <= 360)
                {
                    if (bearing == 360)
                    {
                        bearing = 0;
                    }
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
            setQrzMessageText(qrzReplyState);

        }
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
    ui->qraText->clear();
    ui->nameText->clear();
    ui->addr1Text->clear();
    ui->addr2Text->clear();
    ui->distanceText->clear();
    ui->bearingText->clear();
    ui->countryText->clear();
    ui->cqZoneText->clear();
    ui->ituZoneText->clear();
    ui->qrzMessageText->clear();

}


void QrzDisplayFrame::getQrzDetailsForLogger(QString callsign)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    QString frame_uuid = tslf->getContest()->uuid;
    trace(QString("[qrzDisplayFrame] sending callsign %1, from frame uuid %2 to QrzServer").arg(callsign, frame_uuid));
    QrzDisplayServerRpc::getQrzDisplayServerRpc()->sendCallsignFromLoggerToQrzServer(callsign, frame_uuid);


}


void QrzDisplayFrame::setQrzMessageText(QString msg)
{
    ui->qrzMessageText->clear();
    ui->qrzMessageText->setText(msg);
}

void QrzDisplayFrame::setLogonPushButtonLabelText(bool loggedOn)
{
    //qDebug() << "state = " << (loggedOn ? "true" : "false");
    if (loggedOn)
    {
        ui->logOnStatusPb->setText(tr("Connected"));
    }
    else
    {
        ui->logOnStatusPb->setText(tr("Disconnected"));
    }


}

void QrzDisplayFrame::setContest(BaseContestLog( *c))
{
    ct = c;

}



// -------- QRZ Display RPC -----------------------------------------------------



//static bool syncstat = false;
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
    MinosRPC *rpc = MinosRPC::getMinosRPC();

    QStringList sv{  };

    rpc->findProviders(rpcConstants::qrzServerApp, sv);
    rpc->findProviders(rpcConstants::clusterApp, sv);

    connect(rpc, &MinosRPC::routerCall, this, &QrzDisplayServerRpc::on_routerCall);
    connect(rpc, &MinosRPC::notify, this, &QrzDisplayServerRpc::on_notify);
    connect(rpc, &MinosRPC::provider, this, &QrzDisplayServerRpc::on_provider);

    QString a = rpc->getAppName();
    QString station = MinosConfig::getMinosConfig()->getThisRouterName();
    trace(QString("[QrzDisplayServer]Publish %1 %2@%3").arg(rpcConstants::qrzDisplayApp, a, station));
    RPCPubSub::publish(rpcConstants::qrzDisplayApp,  a + "@" + station, "", psPublished);


}

QrzDisplayServerRpc::~QrzDisplayServerRpc()
{
}





void QrzDisplayServerRpc::sendCallsignFromLoggerToQrzServer(QString callsign, QString frameid)
{
    // We need to send the message to all connected cluster clients, except the spot server
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    for ( auto p = rpc->getProviders().begin(); p != rpc->getProviders().end(); p++ )
    {
        QString cat = p.key();
        if (cat == rpcConstants::qrzServerApp || cat == rpcConstants::clusterApp)
        {
            for (auto const &s: (*p))
            {
                trace(QString("[QrzDisplayServer]  Send Qrz Response to Cluster Server = %1").arg(s.app));
                RPCGeneralClient rpc(rpcConstants::qrzLogger );
                QSharedPointer<RPCParam>st(new RPCParamStruct);
                st->addMember(rpcConstants::qrzLogger, rpcConstants::qrzLogger);
                st->addMember( callsign, rpcConstants::qrzDxCallsign );
                st->addMember(frameid, rpcConstants::qrzLogFrameId);

                rpc.getCallArgs() ->addParam( st );
                rpc.queueCall( s.psn() );
            }
        }
    }
}


void QrzDisplayServerRpc::on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
{
    trace(QString("[QrzDisplayServer]  on_serverCall - Message from %1").arg(from));
    if ( !err )
    {

        QString mName = mro->getMethodName();


        if (mName == rpcConstants::qrzMethod)
        {
            RPCArgs *args = mro->getCallArgs();
            QSharedPointer<RPCParam> psName;

            if (args->getStructArgMember(0, rpcConstants::paramName, psName))
            {
                QString paraName;
                psName->getString(paraName);

                if (paraName == rpcConstants::qrzLoggerResponse)
                {
                    trace(QString("Cluster RPC: callback from %1 paraName = %2").arg(mName, paraName));

                    QSharedPointer<RPCParam> msgQrzLoggerResponse;
                    QSharedPointer<RPCParam> msgDxCall;
                    QSharedPointer<RPCParam> msgQrzFirstName;
                    QSharedPointer<RPCParam> msgQrzName;
                    QSharedPointer<RPCParam> msgQrzCounty;
                    QSharedPointer<RPCParam> msgQrzAddr1;
                    QSharedPointer<RPCParam> msgQrzAddr2;
                    QSharedPointer<RPCParam> msgQrzCountry;
                    QSharedPointer<RPCParam> msgQrzLat;
                    QSharedPointer<RPCParam> msgQrzLon;
                    QSharedPointer<RPCParam> msgQrzDxGrid;
                    QSharedPointer<RPCParam> msgQrzCQZone;
                    QSharedPointer<RPCParam> msgQrzITUZone;
                    QSharedPointer<RPCParam> msgQrzDxReplyState;
                    QSharedPointer<RPCParam> msgLogFrameId;

                    if (args->getStructArgMember(0, rpcConstants::qrzDxCallsign, msgDxCall)
                            && args->getStructArgMember(0, rpcConstants::qrzFirstName, msgQrzFirstName)
                            && args->getStructArgMember(0, rpcConstants::qrzName, msgQrzName)
                            && args->getStructArgMember(0, rpcConstants::qrzCounty, msgQrzCounty)
                            && args->getStructArgMember(0, rpcConstants::qrzAddr1, msgQrzAddr1)
                            && args->getStructArgMember(0, rpcConstants::qrzAddr2, msgQrzAddr2)
                            && args->getStructArgMember(0, rpcConstants::qrzCountry, msgQrzCountry)
                            && args->getStructArgMember(0, rpcConstants::qrzLat, msgQrzLat)
                            && args->getStructArgMember(0, rpcConstants::qrzLon, msgQrzLon)
                            && args->getStructArgMember(0, rpcConstants::qrzDxGrid, msgQrzDxGrid)
                            && args->getStructArgMember(0, rpcConstants::qrzCqZone, msgQrzCQZone)
                            && args->getStructArgMember(0, rpcConstants::qrzItuZone, msgQrzITUZone)
                            && args->getStructArgMember(0, rpcConstants::qrzDxReplyState, msgQrzDxReplyState)
                            && args->getStructArgMember(0, rpcConstants::qrzLogFrameId, msgLogFrameId))
                    {

                        QrzCallsignData cd;

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

                        QString addr1;
                        msgQrzAddr1->getString(addr1);
                        cd.setAddr1(addr1);

                        QString addr2;
                        msgQrzAddr2->getString(addr2);
                        cd.setAddr2(addr2);

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
                if (paraName == rpcConstants::qrzServerState)
                {
                    trace(QString("Cluster RPC: callback from %1 paraName = %2").arg(mName, paraName));

                    QSharedPointer<RPCParam> msgQrzLogonState;
                    QSharedPointer<RPCParam> msgQrzServerMessage;
                    bool loggedState = false;


                    if (args->getStructArgMember(0, rpcConstants::qrzServerLogonState, msgQrzLogonState)
                        && args->getStructArgMember(0, rpcConstants::qrzServerStateMessage, msgQrzServerMessage))

                    {
                        QString logStateStr;
                        QString stateMessage;

                        msgQrzLogonState->getString(logStateStr);
                        if (logStateStr == rpcConstants::qrzServerLoggedIn)
                        {
                            loggedState = true;
                        }

                        if (loggedState)
                        {
                            trace(QString("QRZDisplayFrame - Logged on to QRZ"));
                        }
                        else
                        {
                            trace(QString("QRZDisplayFrame - Logged off from QRZ"));
                        }

                        msgQrzServerMessage->getString(stateMessage);

                        emit qrzServerLoggedState(loggedState, stateMessage);
                    }



                }


            }

        }
    }
}


void QrzDisplayServerRpc::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{
    trace(QString("[QrzDisplayServer]   on_notify - routerName = %1, publisherProgram = %2, app = %3").arg(an.getPublisherRouter(), an.getPublisherProgram(), an.getKey()));
}

void QrzDisplayServerRpc::on_provider(Provider, QString /*cat*/)
{
//    syncstat = true;
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
