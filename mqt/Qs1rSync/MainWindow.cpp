#include "base_pch.h"

#include <QHostAddress>
#include <QThread>
#include "MinosRPC.h"
#include "ConfigFile.h"
#include "rigutils.h"
#include "BandList.h"
#include "delayedaction.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "qs_defines.h"

/*
 Valid sample rates
[VAL] = 25000, 50000, 125000,
250000, 500000, 625000,
1250000, 1562500, 2500000
*/
/*
SO use bandlist.xml to determine band edges (unit, flow, fhigh)
We then need to add mode/contest segments (??bandmaplimits.ini - incomplete)
Then we can see bandwidth requirement (and so sample rate) and centre frequency

*/
/*

These are some of the useful SERVER commands
There are a few we'd like for the display as well
such as zoom level

DisplayFreqOffset Gets/Sets the display frequency
offset value. Valid values are any number that
can be represented with type double

(centre frequency)
Freq Gets/Sets the tuned frequency in Hz.
fHz Gets/Sets the tuned frequency in Hz.
fkHz Gets/Sets the tuned frequency in kHz.
fMHz Gets/Sets the tuned frequency in MHz.

(offset from centre frequency)
ToneFrequency Gets/Sets the local oscillator frequency.
tf Gets/Sets the local oscillator frequency

Hide Hides the server window. This is a single word command.
Show Shows the server window. This is a single word command.

Mode Gets/Sets the current demodulation mode.
See QSDEMODMODE in qs_defines.h below.

SampleRate Gets/Sets the current sample
rate.
 Valid sample rates
[VAL] = 25000(20kHz), 50000(40kHz), 125000(100kHz),
250000(200kHz), 500000(400kHz), 625000(500kHz),
1250000(1MHz), 1562500(1.25MHz), 2500000(2MHz)

?? maximum display bandwidth is half the sample rate

SupportedSampleRates Gets the supported sample rates
*/

class BandWidth
{
public:
    int bandWidth;
    int sampleRate;
};

QVector<BandWidth> bws =
{
    {20000, 25000},
    {40000, 50000},
    {100000, 125000},
    {200000, 250000},
    {400000, 500000},
    {500000, 625000},
    {1000000, 1250000},
    {1250000, 1562500},
    {2000000, 2500000}
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    n1mmLink(parent),
    wsjtxLink(parent)
{
    ui->setupUi(this);

    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    createCloseEvent();

    QSettings settings;
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    bool trackBand = settings.value("trackBand", false).toBool();
    ui->trackBandcb->setChecked(trackBand);

    lastQS1RRx = "OK\r\n"
            "fHz=28123456\r\n"
            "tf=-123456\r\n";

    int fOffset = lastQS1RRx.indexOf("fHz=");
    int tfOffset = lastQS1RRx.indexOf("tf=");
    if (fOffset >= 0 && tfOffset >= 0)
    {
        QString temp = lastQS1RRx.mid(fOffset + 4, tfOffset - fOffset - 4);
        int l = temp.length();
        while (l > 0 && ((temp[l - 1] == '\r') || (temp[l - 1] == '\n')))
        {
            temp = temp.right(l - 1);
            l = temp.length();
        }
        fCentre = temp.toInt();
        temp = lastQS1RRx.mid(tfOffset + 3, 100);
        l = temp.length();
        while (l > 0 && ((temp[l - 1] == '\r') || (temp[l - 1] == '\n')))
        {
            temp = temp.right( l - 1);
            l = temp.length();
        }
        ftf = temp.toInt();
    }
    double f = (fCentre + ftf);
    lastQS1RRx = "fCentre " + QString::number(fCentre) + "\r\n tf " + QString::number(ftf) + " freq " + QLocale::system().toString(f, 'f', 0);

    //    ui->Rig1Label->setText(omni_rig->Rig1()->RigType());

    connect(&timer2, SIGNAL(timeout()), this, SLOT(timer2Timeout()));
    timer2.start(1000);

    connect(&ClientSocket1, SIGNAL(connected()), this, SLOT(onSocketConnect()));
    connect(&ClientSocket1, SIGNAL(disconnected()), this, SLOT(onSocketDisconnect()));
    connect(&ClientSocket1, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(&ClientSocket1, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));

    connect(&SyncTimer, SIGNAL(timeout()), this, SLOT(SyncTimerTimer()));
    SyncTimer.start(100);

    MinosRPC *rpc = MinosRPC::getMinosRPC(getAppStartupName(), false);

    connect(rpc, SIGNAL(serverCall(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_serverCall(bool,QSharedPointer<MinosRPCObj>,QString)));
    connect(rpc, SIGNAL(notify(AnalysePubSubNotify ,QString)), this, SLOT(on_notify(AnalysePubSubNotify ,QString)));

    MinosConfig *config = MinosConfig::getMinosConfig();

    QVector<QSharedPointer<Connectable> >connectables = config->getConnectables();

    QStringList servers;
    for ( auto const &res: connectables )
    {
        servers.append(res->serverName);
    }
    servers.sort();
    servers.removeDuplicates();

    for (int i = 0; i < servers.size(); i++)
    {
        // this only works for local servers - so OK for me, but...
        rpc->subscribeRemote( servers[i], rpcConstants::rigControlCategory );
        rpc->subscribeRemote( servers[i], rpcConstants::rigDetailsCategory );
        rpc->subscribeRemote( servers[i], rpcConstants::rigStateCategory );
    }

    n1mmLink.initialise();
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::onStdInRead(QString cmd)
{
    trace(QString("MainWindow::onStdInRead %1").arg(cmd));
    executeStdIn(cmd);
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    // and tidy up all loose ends

    SyncTimerTimer( );

    QWidget::closeEvent(event);
}
void MainWindow::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void MainWindow::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void MainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}
void MainWindow::SyncTimerTimer(  )
{
    static bool closed = false;
    if ( !closed )
    {
        if ( checkCloseEvent() )
        {
            closed = true;
            close();
        }
    }
    if (qs1rConnected)
    {
        ui->QS1RFLabel->setText(lastQS1RRx);
    }
    else
    {
        ui->QS1RFLabel->setText("Not connected");
    }

    if (n1mmLink.isConnected())
    {
        Frequency f = n1mmLink.getFrequency();
        if (f.isOK() )
        {
            mainRigFreq = n1mmLink.getFrequency();
            mainRigMode = n1mmLink.getMode();
            ui->Rig1Label->setText(n1mmLink.getRadioName());
            if (ui->trackRig->isChecked())
            {
                on_transfer12Button_clicked();
            }
            trackBand();
        }
    }
    ui->QF1Label->setText(mainRigFreq.convertFreqStrDisp());
}


void MainWindow::timer2Timeout()
{
    // Poll the QS1R
    if (qs1rConnected)
    {
        QString mess = ">UpdateRxFreq\n?fHz\n?tf\n";
        mess += "?SampleRate\n";
        ClientSocket1.write( mess.toLatin1().data(), mess.length() );
    }
    else if (!ClientSocket1.isOpen())
    {
        ClientSocket1.connectToHost(QHostAddress::LocalHost, RX1_CMD_SERV_TCP_PORT + 2);
    }

}

void MainWindow::onSocketConnect()
{
    qs1rConnected = true;
}

void MainWindow::onSocketDisconnect()
{
    qs1rConnected = false;
}
void MainWindow::onError(QAbstractSocket::SocketError /*err*/)
{
    ClientSocket1.close();
    qs1rConnected = false;
}

void MainWindow::onReadyRead()
{
    static char sockbuffer[ 4096 ];
    //Hz=5.02e+07
    //tf=-121700

    while(ClientSocket1.bytesAvailable())
    {
        qint64 retlen = ClientSocket1.read( sockbuffer, 4095 );
        if ( retlen > 0 )
        {
            sockbuffer[ retlen ] = 0;
            lastQS1RRx =  sockbuffer;

            if (lastQS1RRx.indexOf("NAK") >= 0 || lastQS1RRx.indexOf("?") >= 0)
            {
                trace(lastQS1RRx.remove('\r'));
            }

            int fOffset = lastQS1RRx.indexOf("fHz=");
            int tfOffset = lastQS1RRx.indexOf("tf=");
            int srOffset = lastQS1RRx.indexOf("SampleRate=");
            if (fOffset >= 0 && tfOffset >= 0 && srOffset >= 0)
            {
                QString temp = lastQS1RRx.mid(fOffset + 4, tfOffset - fOffset - 4);
                int l = temp.length();
                while (l > 0 && ((temp[l-1] == '\r') || (temp[l-1] == '\n')))
                {
                    temp = temp.left(l - 1);
                    l = temp.length();
                }
                fCentre = temp.toInt();

                temp = lastQS1RRx.mid(tfOffset + 3, srOffset - tfOffset - 4);
                l = temp.length();
                while (l > 0 && ((temp[l-1] == '\r') || (temp[l-1] == '\n')))
                {
                    temp = temp.left( l - 1);
                    l = temp.length();
                }
                ftf = temp.toInt();

                temp = lastQS1RRx.mid(srOffset + 11, 100);
                l = temp.length();
                while (l > 0 && ((temp[l-1] == '\r') || (temp[l-1] == '\n')))
                {
                    temp = temp.left( l - 1);
                    l = temp.length();
                }
                sampleRate = static_cast<int>(temp.toDouble());
            }
            Frequency f(fCentre + ftf);
            lastQS1RRx = "fCentre " + QString::number(fCentre) + " Sample Rate "+ QString::number(sampleRate/1000) + "Ksps\r\n"
                    "tf " + QString::number(ftf) + " freq " + f.convertFreqStrDisp();
            if (ui->trackQS1R->isChecked())
            {
                on_transfer21Button_clicked();
            }
        }
    }
}

void MainWindow::on_closeButton_clicked()
{
    close();
}

void MainWindow::on_transfer12Button_clicked()
{
    long lFreq = qint64(mainRigFreq) - qint64(transvertOffset);

    QString mess = ">fHz " + QString::number(fCentre) + "\n";
    mess += ">tf " + QString::number(lFreq - fCentre) + "\n";
    ClientSocket1.write( mess.toLatin1().data(), mess.length() );
}

void MainWindow::on_transfer21Button_clicked()
{
    long lFreq = fCentre + ftf;

    if (n1mmLink.isConnected())
    {
        n1mmLink.sendFrequencyRequest(lFreq);
    }
    else
    {
        RPCGeneralClient rpc(rpcConstants::rigControlMethod);
        QSharedPointer<RPCParam>st(new RPCParamStruct);

        QStringList qsl = rigCache.getSelectedLoggers(rigSelected);
        if (qsl.count())
        {
            QString loggerUuid = qsl[0];

            QSharedPointer<RPCParam>logger(new RPCStringParam(loggerUuid ));
            st->addMember( logger, rpcConstants::loggerUuid );

            QString selc = rigCache.getSelectedContest(rigSelected, loggerUuid);

            QSharedPointer<RPCParam>select(new RPCStringParam(selc ));
            st->addMember( select, rpcConstants::selected );

            st->addMember( QString::number(lFreq + qint64(transvertOffset)), rpcConstants::rigControlLogFreq );
            rpc.getCallArgs() ->addParam( st );

            rpc.queueCall( rigSelected);
        }
    }
}

void MainWindow::on_notify( AnalysePubSubNotify an, const QString from )
{
    // PubSub notifications
    trace( "Notify callback from " + from + ( an.getOK() ? ":Error" : ":Normal" ) );

    if ( an.getOK() )    // won't be true now
    {
        if ( an.getState() == psPublished)
        {
            if ( an.getCategory() == rpcConstants::rigStateCategory)
            {
                rigCache.setStateString(an);
            }
            else if ( an.getCategory() == rpcConstants::rigDetailsCategory)
            {
                rigCache.setDetailsString(an);
            }
            else if ( an.getCategory() == rpcConstants::rigControlCategory && an.getKey() == rpcConstants::rigControlRadioList )
            {
                rigCache.addRigList(an.getValue());
            }
            else
                return;
        }
        rigSelected = rigCache.getSelected("");
        if (!rigSelected.isEmpty())
        {
            RigState &selState = rigCache.getState(rigSelected);
            RigDetails &selDetail = rigCache.getDetails(rigSelected);
            ui->Rig1Label->setText(rigSelected.toString());

            if (selDetail.isDirty())
            {
                //bandlist = selDetail.bandList();  // bandlist is the bands supported
                transvertState = selDetail.transverterStatus().getValue();
                transvertOffset = selDetail.transverterOffset().getValue();
                if (!transvertState)
                    transvertOffset = 0.0;
                selDetail.clearDirty();

            }
            if (selState.isDirty())
            {
                mainRigMode = selState.radioMode().getValue().remove(":");
                mainRigFreq = selState.radioFreq().getValue();

                selState.clearDirty();
                ui->QF1Label->setText(mainRigFreq.convertFreqStrDisp());

                if (ui->trackRig->isChecked())
                {
                    on_transfer12Button_clicked();
                }
                delayedAction(this, [=]{
                    trackBand();
                }, 50);
            }
        }
        else
        {
            ui->Rig1Label->setText("");
            ui->QF1Label->setText("");
        }
    }
}
//---------------------------------------------------------------------------
void MainWindow::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
{
    trace( "server callback from " + from + ( err ? ":Error" : ":Normal" ) );
    trace("method is " + mro->getMethodName());
}
//---------------------------------------------------------------------------
void MainWindow::QS1RCentre(const Frequency &fLow, const Frequency &fHigh)
{
    if (qs1rConnected)
    {
        trace(QString("%1 %2").arg(fLow.traceStr()).arg(fHigh.traceStr()));
        qint64 bandWidth = qint64(fHigh) - qint64(fLow);
        qint64 centre = qint64(fLow) + bandWidth/2;
        centre += 12500;
        centre /= 25000;
        centre *= 25000;

        // search for nearest matching bandwidth on QS1R

        qint64 sampleRate = 0;

        for(int i = 0; i < bws.size(); i++)
        {
            if (bws[i].bandWidth >= bandWidth)
            {
                sampleRate = bws[i].sampleRate;
                break;
            }
        }
        if (sampleRate > 0)
        {
            qint64 lFreq = qint64(mainRigFreq) - qint64(transvertOffset);
            fCentre = centre - qint64(transvertOffset);

            QString mess;

            mess = ">SampleRate " + QString::number(sampleRate) + "\n";
            trace(mess);
            ClientSocket1.write( mess.toLatin1().data(), mess.length() );
            ClientSocket1.waitForBytesWritten(250);
            QThread::msleep(500);

            mess = ">fHz " + QString::number(fCentre) + "\n";
            mess += ">tf " + QString::number(lFreq - fCentre) + "\n";

            trace(mess);
            ClientSocket1.write( mess.toLatin1().data(), mess.length() );
        }
    }
}
void MainWindow::trackBand()
{
    if (mainRigFreq == lastMainRigFreq && transvertOffset == lastTransverterOffset && mainRigMode == lastMainRigMode)
    {
        return; // nothing to do
    }
    lastMainRigFreq = mainRigFreq;
    lastTransverterOffset = transvertOffset;
    lastMainRigMode = mainRigMode;

    trace(QString("rig %1 tv %2 mode %3").arg(lastMainRigFreq.traceStr()).arg(lastTransverterOffset.traceStr()).arg(lastMainRigMode));

    // mainRigFreq is absolute, i.e. on air frequency
    // so we use it to find the band
    // then we offset the band by the transverter offset before telling the QS1R

    BandList &blist = BandList::getBandList();
    QSharedPointer<BandInfo>  bi;
    bool bandOK = blist.findBand(mainRigFreq, bi);
    if (!bandOK)
    {
        return;
    }

    if (!ui->trackBandcb->isChecked())
    {
        return;
    }
    int modePart = -1;
    QSharedPointer<ModeInfo> mi = bi->findMode(mainRigMode, mainRigFreq, modePart);
    if (mi == lastBandMode && modePart == lastModePart && bi == lastBand)
    {
        trace("band/mode unchanged");
        return;
    }
    if (!mi)
    {
        trace("band/mode not found");
        QS1RCentre(bi->fLow, bi->fHigh);
    }
    else
    {
        trace("mode found OK");
        if (modePart == 1)
        {
            QS1RCentre(mi->fcLow1, mi->fcHigh1);
        }
        else if (modePart == 2)
        {
            QS1RCentre(mi->fcLow2, mi->fcHigh2);
        }
        else
        {
            QS1RCentre(mi->fLow, mi->fHigh);
        }
    }
    lastBand = bi;
    lastBandMode = mi;
    lastModePart = modePart;
}

void MainWindow::on_noTrack_clicked()
{
    // do nothing
}

void MainWindow::on_trackRig_clicked()
{
    // set QS1R to rig
    on_transfer12Button_clicked();
}

void MainWindow::on_trackQS1R_clicked()
{
    // set rig to QS1R
    on_transfer21Button_clicked();
}

void MainWindow::on_trackBandcb_stateChanged(int /*arg1*/)
{
    QSettings settings;
    settings.setValue("trackBand", ui->trackBandcb->isChecked());
}

void MainWindow::on_wsjtxCb_stateChanged(int /*arg1*/)
{
    if (ui->wsjtxCb->isChecked())
    {
        wsjtxLink.initialise();
    }
    else
    {
        wsjtxLink.disconnect();
    }
}
