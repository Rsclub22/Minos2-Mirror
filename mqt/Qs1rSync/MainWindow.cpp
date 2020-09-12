#include "base_pch.h"

#include <QHostAddress>

#include "MinosRPC.h"
#include "ConfigFile.h"
#include "rigutils.h"

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "qs_defines.h"
/*

Bands and limits etc

160m LSB    1840-2000
160m CW     1810-1850
160m DG
160m FT

80m LSB     3,600 - 3,775
80m CW      3,510 - 3,570
80m DG      PSK63:	3580-3590 kHz
            RTTY:	3590-3620 kHz
80m FT

40m LSB     7,060 - 7,200
40m CW      7,000 - 7,040
40m DG
40m FT

20m USB     14,125-14,300
20m CW      14,000-14,060
20m DG
20m FT

15m USB 	21,151-21,450
15m CW  	21,000-21,070
15m DG
15m FT

10m USB 	28,320-29,000
10m CW  	28,000-28,070
10m DG
10m FT

6m USB  	50.100-50.300
6m CW       50.000-50.100
6m DG
6m FT

4m USB      70.100-70.250
4m CW       70.100-70.250
4m DG
4m FT

2m USB      144.150-144.400, 144.500-144.794
2m CW       144.000-144.150
2m DG
2m FT

70cm USB    432.1000-432.4000
70cm CW     432.0000-432.1000
70cm DG
70cm FT

23cm USB    1296.150-1296.800
23cm CW     1296.000-1296.150
23cm DG
23cm FT
*/
/*
DisplayFreqOffset Gets/Sets the display frequency
offset value.
Valid values are any number that
can be represented with type
double

(centre frequency)
Freq Gets/Sets the tuned frequency in
Hz.
fHz Gets/Sets the tuned frequency in
Hz.
fkHz Gets/Sets the tuned frequency in
kHz.
fMHz Gets/Sets the tuned frequency in
MHz.

(offset from centre frequency)
ToneFrequency Gets/Sets the local oscillator
frequency.
tf Gets/Sets the local oscillator
frequency

Hide Hides the server window. This is a single word command.
Show Shows the server window. This is a single word command.

Mode Gets/Sets the current
demodulation mode.
See QSDEMODMODE in
qs_defines.h below.

SampleRate Gets/Sets the current sample
rate.
SupportedSampleRates Gets the supported sample rates









*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    n1mmLink(parent)
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

    lastF = "OK\r\n"
            "fHz=28123456\r\n"
            "tf=-123456\r\n";

    int fOffset = lastF.indexOf("fHz=");
    int tfOffset = lastF.indexOf("tf=");
    if (fOffset >= 0 && tfOffset >= 0)
    {
        QString temp = lastF.mid(fOffset + 4, tfOffset - fOffset - 4);
        int l = temp.length();
        while (l > 0 && ((temp[l - 1] == '\r') || (temp[l - 1] == '\n')))
        {
            temp = temp.right(l - 1);
            l = temp.length();
        }
        fCentre = temp.toInt();
        temp = lastF.mid(tfOffset + 3, 100);
        l = temp.length();
        while (l > 0 && ((temp[l - 1] == '\r') || (temp[l - 1] == '\n')))
        {
            temp = temp.right( l - 1);
            l = temp.length();
        }
        ftf = temp.toInt();
    }
    double f = (fCentre + ftf);
    lastF = "fCentre " + QString::number(fCentre) + "\r\n tf " + QString::number(ftf) + " freq " + QLocale::system().toString(f, 'f', 0);

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
    connect(rpc, SIGNAL(notify(bool,QSharedPointer<MinosRPCObj>,QString)), this, SLOT(on_notify(bool,QSharedPointer<MinosRPCObj>,QString)));

    MinosConfig *config = MinosConfig::getMinosConfig();

    QStringList servers;
    for ( QVector <QSharedPointer<RunConfigElement> >::iterator i = config->elelist.begin(); i != config->elelist.end(); i++ )
    {
        QSharedPointer<Connectable> res = (*i)->connectable();
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
        ui->QS1RFLabel->setText(lastF);
    }
    else
    {
        ui->QS1RFLabel->setText("Not connected");
    }

    if (n1mmLink.isConnected())
    {
        freq = convertStrToFreq(n1mmLink.getFrequency());
        ui->Rig1Label->setText(n1mmLink.getRadioName());
    }
    ui->QF1Label->setText(convertFreqToStr(freq));
}


void MainWindow::timer2Timeout()
{
    // Poll the QS1R
    if (qs1rConnected)
    {
        QString mess = ">UpdateRxFreq\n?fHz\n?tf\n";
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

            lastF =  sockbuffer;
            int fOffset = lastF.indexOf("fHz=");
            int tfOffset = lastF.indexOf("tf=");
            if (fOffset >= 0 && tfOffset >= 0)
            {
                QString temp = lastF.mid(fOffset + 4, tfOffset - fOffset - 4);
                int l = temp.length();
                while ((temp[l] == '\r') || (temp[l] == '\n'))
                {
                    temp = temp.right(l - 1);
                    l = temp.length();
                }
                fCentre = temp.toInt();
                temp = lastF.mid(tfOffset + 3, 100);
                l = temp.length();
                while ((temp[l] == '\r') || (temp[l] == '\n'))
                {
                    temp = temp.right( l - 1);
                    l = temp.length();
                }
                ftf = temp.toInt();
            }
            double f = (fCentre + ftf);
            lastF = "fCentre " + QString::number(fCentre) + "\r\n tf " + QString::number(ftf) + " freq " + QLocale::system().toString(f, 'f', 0);
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
    long lFreq = static_cast<long>(freq - transvertOffset);

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

            st->addMember( convertFreqToStr(lFreq + transvertOffset), rpcConstants::rigControlLogFreq );
            rpc.getCallArgs() ->addParam( st );

            rpc.queueCall( rigSelected);
        }
    }
}

void MainWindow::on_notify( bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
    // PubSub notifications
    trace( "Notify callback from " + from + ( err ? ":Error" : ":Normal" ) );
    AnalysePubSubNotify an( err, mro );

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

            if (selState.isDirty())
            {


                mode = selState.radioMode().getValue();
                freq = selState.radioFreq().getValue();
                //                   status = selState.status();
                selState.clearDirty();
                ui->QF1Label->setText(convertFreqToStr(freq));

                if (ui->trackRig->isChecked())
                {
                    on_transfer12Button_clicked();
                }

            }
            if (selDetail.isDirty())
            {
                //bandlist = selDetail.bandList();
                transvertState = selDetail.transverterStatus().getValue();
                transvertOffset = selDetail.transverterOffset().getValue();
                if (!transvertState)
                    transvertOffset = 0.0;
                selDetail.clearDirty();

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
void MainWindow::on_serverCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from )
{
    trace( "server callback from " + from + ( err ? ":Error" : ":Normal" ) );
    trace("method is " + mro->getMethodName());

}
//---------------------------------------------------------------------------

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
