#include <QHostInfo>
#include <QSettings>

#include "cutils.h"

#include "kstconfigure.h"
#include "airscoutlink.h"

#include "kstmainwindow.h"
#include "ui_kstmainwindow.h"

QStringList services =
{
"50/70 MHz",
"144/432 MHz",
"Microwave",
"EME/JT65",
};

KSTMainWindow *mainWindow = nullptr;
//==========================================================================================
KSTMainWindow::KSTMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::KSTMainWindow)
{
    ui->setupUi(this);

    mainWindow = this;
    connect(&stdinReader, SIGNAL(stdinLine(QString)), this, SLOT(onStdInRead(QString)));
    stdinReader.start();

    QSettings settings;

    serverName = settings.value("hostname", "www.on4kst.info").toString().trimmed();
    serverPort = settings.value("port", "23001").toString().trimmed();
    myCallsign = settings.value("username", "").toString().trimmed();
    password = settings.value("password", "").toString().trimmed();
    maxDistance = settings.value("maxDistance", 99999).toInt();

    ASActive = settings.value("ASActive", false).toBool();
    ASServerName = settings.value("ASServerName", "AS").toString().trimmed();
    ASMyName = settings.value("ASMyName", "Minos").toString().trimmed();
    ASActiveBand = static_cast<ASBand>(settings.value("ASActiveBand", 0).toInt());
    ASMinDistance = settings.value("ASMinDistance", 300).toInt();
    ASMaxDistance = settings.value("ASMaxDistance", 1000).toInt();
    ASPort = settings.value("ASPort", 9872).toInt();
    ASTimeout = settings.value("ASTimeout", 10).toInt();

    callVector =    QSharedPointer<QVector <QSharedPointer<KstUser> > >( new QVector<QSharedPointer<KstUser> > );
    messageVector = QSharedPointer<QVector <QSharedPointer<KstMessageLine> > >( new QVector<QSharedPointer<KstMessageLine> >);

    QString chatSelection = settings.value("service", "1").toString();
    QStringList selections = chatSelection.split(":");
    for (int i = 0; i < selections.count(); i++)
    {
        int s = selections[i].toInt();
        if (s <= 4 && s > 0)
        {
            kstChatSelection.append(s);
            switch(s)
            {
            case 1:
                ui->login1cb->setChecked(true);
                break;
            case 2:
                ui->login2cb->setChecked(true);
                break;
            case 3:
                ui->login3cb->setChecked(true);
                break;
            case 4:
                ui->login4cb->setChecked(true);
                break;
            }
        }
    }
    std::sort(kstChatSelection.begin(), kstChatSelection.end());

    activeChat = settings.value("active", "0").toInt();
    setActive(activeChat);

    ui->login1cb->setText(services[0]);
    ui->login2cb->setText(services[1]);
    ui->login3cb->setText(services[2]);
    ui->login4cb->setText(services[3]);

    connect(ui->login1cb, SIGNAL(stateChanged(int)), this, SLOT(logincb_stateChanged(int)));
    connect(ui->login2cb, SIGNAL(stateChanged(int)), this, SLOT(logincb_stateChanged(int)));
    connect(ui->login3cb, SIGNAL(stateChanged(int)), this, SLOT(logincb_stateChanged(int)));
    connect(ui->login4cb, SIGNAL(stateChanged(int)), this, SLOT(logincb_stateChanged(int)));

    ui->active1rb->setText(services[0]);
    ui->active2rb->setText(services[1]);
    ui->active3rb->setText(services[2]);
    ui->active4rb->setText(services[3]);

    connect(ui->active1rb, SIGNAL(clicked()), this, SLOT(activerb_clicked()));
    connect(ui->active2rb, SIGNAL(clicked()), this, SLOT(activerb_clicked()));
    connect(ui->active3rb, SIGNAL(clicked()), this, SLOT(activerb_clicked()));
    connect(ui->active4rb, SIGNAL(clicked()), this, SLOT(activerb_clicked()));

    checkActive();

    ui->CSChatFilter->addItem("");
    ui->CSChatFilter->addItems(services);
    ui->CSChatFilter->setCurrentIndex(0);

    ui->messageChatFilter->addItem("");
    ui->messageChatFilter->addItems(services);
    ui->messageChatFilter->setCurrentIndex(0);

    autoConnect = settings.value("autoConnect", false).toBool();
    myLoc = settings.value("locator", "").toString();

    QByteArray geometry = settings.value("geometry/Main").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QByteArray state;
    state = settings.value("kstSplitterState").toByteArray();
    ui->kstSplitter->restoreState(state);

    state = settings.value("msgSplitterState").toByteArray();
    ui->msgSplitter->restoreState(state);

    state = settings.value("callSplitterState").toByteArray();
    ui->callSplitter->restoreState(state);


    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    createCloseEvent();
    connect(&CloseTimer, SIGNAL(timeout()), this, SLOT(CloseTimerTimer()));
    CloseTimer.start(100);

    connect(&userCallTimer, SIGNAL(timeout()), this, SLOT(userCallTimerTimer()));
    userCallTimer.start(5000);

    kstMessageModel.setChatVector(messageVector);

    kstMessageFilterModel.setSourceModel(&kstMessageModel);

    ui->messageTable->setModel(&kstMessageFilterModel);
    ui->messageTable->horizontalHeader()->setStretchLastSection(true);

    kstMeepFilterModel.setSourceModel(&kstMessageModel);
    ui->meepTable->setModel(&kstMeepFilterModel);
    ui->meepTable->horizontalHeader()->setStretchLastSection(true);

    kstCallModel.setCallVector(callVector);

    kstCallFilterModel.setSourceModel(&kstCallModel);
    ui->CSTable->setModel(&kstCallFilterModel);

    kstPlanesFilterModel.setSourceModel(&kstPlanesModel);
    ui->planesView->setModel(&kstPlanesFilterModel);

    meepDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;
    messageDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;
    CSDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;
    PlanesDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;

    // these are used for sizing when adjust to content
    kstMessageModel.delegate = messageDelegate;
    kstCallModel.delegate = CSDelegate;

    ui->meepTable->setItemDelegate(meepDelegate.data());
    ui->messageTable->setItemDelegate(messageDelegate.data());
    ui->CSTable->setItemDelegate(CSDelegate.data());
    ui->planesView->setItemDelegate(PlanesDelegate.data());

    QHeaderView *verticalHeader = ui->meepTable->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);

    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

    verticalHeader = ui->messageTable->verticalHeader();
    verticalHeader->setVisible(false);

    QSize ms = messageDelegate->docSize("XX");
    verticalHeader->setDefaultSectionSize(ms.height() *4/5);
    verticalHeader->setMinimumSectionSize(10);
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);

    verticalHeader = ui->CSTable->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

    verticalHeader = ui->planesView->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->planesView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QVector<Aircraft> qva;
    kstPlanesModel.setPlanesVector(qva);

    state = settings.value("CSTable/state").toByteArray();
    ui->CSTable->horizontalHeader()->restoreState(state);

    state = settings.value("messageTable/state").toByteArray();
    ui->messageTable->horizontalHeader()->restoreState(state);

    state = settings.value("meepTable/state").toByteArray();
    ui->meepTable->horizontalHeader()->restoreState(state);

//    state = settings.value("planesView/state").toByteArray();
//    ui->planesView->horizontalHeader()->restoreState(state);

    ui->CSTable->horizontalHeader()->setStretchLastSection(true);
    ui->CSTable->horizontalHeader()->setSectionsMovable( true );

    connect( ui->CSTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);
    connect( ui->messageTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);
    connect( ui->meepTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);
    connect( ui->planesView->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);

    connect( ui->CSTable->horizontalHeader(), SIGNAL(sectionMoved(int, int , int)),
             this, SLOT( on_sectionMoved(int, int , int)));


    connect( ui->CSTable->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
             this, SLOT( on_sortIndicatorChanged(int, Qt::SortOrder)));


    kstclient = new QTcpSocket(this);

    connect(kstclient, SIGNAL(connected()), this, SLOT(connected()));
    connect(kstclient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(kstclient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(kstclient, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

    ui->CSFilter->installEventFilter(this);
    ui->messageFilter->installEventFilter(this);
    ui->callEdit->installEventFilter(this);
    ui->msgEdit->installEventFilter(this);
    ui->planesView->installEventFilter(this);

    installEventFilter(this);   // so we pick up return, and implement the default button

    ui->ASActivecb->setChecked(ASActive);
    ui->planesFrame->setVisible(ASActive);
    if(ASActive)
    {
        ui->CSTable->showColumn(ecscAirscout);
    }
    else
    {
        ui->CSTable->hideColumn(ecscAirscout);
    }
    asl = QSharedPointer<AirScoutLink>(new AirScoutLink());
    connect(asl.data(), SIGNAL(acChanged(QSharedPointer<KstUser>)), this, SLOT(acChanged(QSharedPointer<KstUser>)));

    for(int i = 0; i < asbMaxBand; i++)
    {
        ui->asBandCombo->addItem(AirScoutLink::tr(AirScoutLink::ASBandStrings[i]));
    }
    ui->asBandCombo->setCurrentIndex(ASActiveBand);

    ui->maxDistanceEdit->setText(QString::number(maxDistance));
    ui->maxDistanceEdit->setValidator(new QIntValidator(0, 0xffff, this));

    while (myLoc.isEmpty() || myCallsign.isEmpty())
    {
        if (!doConfiguration())
            break;
    }
    started = true;

    if (autoConnect)
        doLoginChanges();

    logincb_stateChanged(0);

    ui->genmsgButton->setDefault(true);

    ui->analyseButton->setVisible(false);
    ui->messageFilter->setFocus();
}

KSTMainWindow::~KSTMainWindow()
{
    delete ui;
}
void KSTMainWindow::onStdInRead(QString cmd)
{
    executeStdIn(cmd);
}
void KSTMainWindow::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry/Main", saveGeometry());
    QWidget::resizeEvent(event);
}
void KSTMainWindow::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry/Main", saveGeometry());
    QWidget::moveEvent(event);
}
void KSTMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry/Main", saveGeometry());
    }
}
void KSTMainWindow::closeEvent(QCloseEvent *event)
{
    trace("KSTMainWindow::closeEvent");

    userCallTimer.stop();

    // and tidy up all loose ends

    QSettings settings;
    settings.setValue("geometry/Main", saveGeometry());
    trace("KSTMainWindow Closing");
    QWidget::closeEvent(event);
}
void KSTMainWindow::CloseTimerTimer(  )
{
   static bool closed = false;
   if ( !closed )
   {
      if ( checkCloseEvent() )
      {
          trace("closing down due to close signalled");
         closed = true;
         close();
      }
   }
}

void KSTMainWindow::userCallTimerTimer()
{
    if (asl && getASActive() && callVectorChanged)
    {
        asl->usersChanged(callVector);
        callVectorChanged = false;
    }
}

void KSTMainWindow::connectToHost()
{
    kstLoggedIn.clear();
    while (myLoc.isEmpty() || myCallsign.isEmpty())
    {
        if (!doConfiguration())
            return;
    }
    if (kstChatSelection.count())
    {
        kstCallModel.locator = myLoc;
        if (kstclient->state() != QAbstractSocket::ConnectedState
           && kstclient->state() != QAbstractSocket::ConnectingState
           && kstclient->state() != QAbstractSocket::ClosingState
           && kstclient->state() != QAbstractSocket::HostLookupState)
        {
            kstclient->connectToHost(serverName, serverPort.toUShort());
        }
    }
}


void KSTMainWindow::connected()
{
    trace("connection to ON4KST established");
    ui->includeLabel->setText(tr("Including %1").arg(myCallsign));
    kstMeepFilterModel.setFilterString(myCallsign);
    kstMessageModel.setCacheSize();
    ui->connectButton->setText(tr("Disconnect"));
}


void KSTMainWindow::clearConnection()
{
    ui->includeLabel->clear();
    kstMeepFilterModel.setFilterString("");
    ui->connectButton->setText(tr("Connect"));
    kstconnected = false;
    kstLoggedIn.clear();
}

void KSTMainWindow::disconnected()
{
    trace("Disconnected from ON4KST");
    clearConnection();
}

void KSTMainWindow::connectionError(QAbstractSocket::SocketError error)
{
    QString msg = QString("ON4KST Connection failed error %1").arg(error);
    trace(msg);
    clearConnection();
}
int KSTMainWindow::calcDistance(QString c)
{
    if (!c.isEmpty())
    {
        QSharedPointer<KstUser> test(new KstUser());
        test->call = c.toUpper();
        test->chat = activeChat;
        if (std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
        {
            int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());

            QSharedPointer<KstUser> user = callVector->at(row);

            return user->distance;
        }
    }
    return -1;
}
void KSTMainWindow::onReadyRead()
{
    QByteArray b = kstclient->readAll();
    QString msg = QString(b);

    QString traceMsg = msg.remove("\r");
    if (traceMsg.endsWith("\n"))
    {
        traceMsg.chop(1);
    }
    trace(QString("messageRx: %1").arg(traceMsg));

    // break into lines...
    msgbuf.append(msg);

    int p = msgbuf.indexOf("\n");
    while (p >= 0)
    {
        QString m = msgbuf.left(p + 1);
        msgbuf = msgbuf.mid(p + 1);
        p = msgbuf.indexOf("\n");

        analyseKstMessage(m);
    }
}
bool loadStringListFromFile (QStringList &list, const QString fname )
{
    QStringList stringsRead;
    QFile textFile( fname );
    if ( textFile.open( QIODevice::ReadOnly ) )
    {
        QTextStream textStream( &textFile );
        while ( true )
        {
            QString line = textStream.readLine();
            if ( line.isNull() )
                break;
            stringsRead.append( line.trimmed() );
        }
        list = stringsRead;
        return true;
    }
    return false;
}
void KSTMainWindow::on_analyseButton_clicked()
{
    filelines.clear();
    kstMessageModel.setCacheSize();

    QString InitialDir/* = GetCurrentDir()*/;

    QString Filter = tr("KST Chat Files (*.txt);Log Files (*.log);;"
                     "All Files (*.*)") ;

    QStringList KSTFileNames = QFileDialog::getOpenFileNames( this,
                       tr("Chat dumps from KST"),
                       InitialDir,                   // opendir
                       Filter );

    if (KSTFileNames.size() == 0)
    {
        return;
    }
    TWaitCursor fred(this);

    if (KSTFileNames.size())
    {
        for (int i = 0; i < KSTFileNames.size(); i++)
        {
            QString fname = KSTFileNames[i].trimmed();

            if (loadStringListFromFile ( filelines, fname ))
            {
                // list is in reverse time order, so reverse it; this will make things easier later

                // but .log is already i the right order
                //std::reverse(filelines.begin(), filelines.end());

                ui->includeLabel->setText(tr("Including %1").arg(myCallsign));
                kstMeepFilterModel.setFilterString(myCallsign);

                QTimer *timer = new QTimer(this);

                connect(timer, &QTimer::timeout, [=]()
                {
                    // NB a lambda function
                    if (curline < filelines.size())
                    {
                        if ( !filelines.at( curline ).isEmpty() )
                        {
                            QString atj = QString::fromLatin1(filelines.at( curline ).toLatin1());
                            int p = atj.indexOf("messageRx") ;
                            if (p >= 0)
                            {
                                atj = atj.mid( p + QString("messageRx: ").size());
                            }
                            analyseKstMessage(atj);
                        }
                        curline++;
                    }
                    else
                    {
                        timer->stop();
                        timer->deleteLater();
                        filelines.clear();
                    }
                }
                );

                timer->start(0);

             }
        }
    }
}
int KSTMainWindow::getMaxDistance() const
{
    return maxDistance;
}

bool KSTMainWindow::getASActive() const
{
    return ui->ASActivecb->isChecked();
}

ASBand KSTMainWindow::getASActiveBand() const
{
    ASBand b = static_cast<ASBand>(ui->asBandCombo->currentIndex());
    return b;
}

QString KSTMainWindow::getASServerName() const
{
    return ASServerName;
}

QString KSTMainWindow::getASMyName() const
{
    return ASMyName;
}

int KSTMainWindow::getASMinDistance() const
{
    return ASMinDistance;
}

int KSTMainWindow::getASMaxDistance() const
{
    return ASMaxDistance;
}

QString KSTMainWindow::getMyCallsign() const
{
    return myCallsign;
}

QString KSTMainWindow::getMyLoc() const
{
    return myLoc;
}

QSharedPointer<QVector<QSharedPointer<KstUser> > > KSTMainWindow::getCallVector() const
{
    return callVector;
}

int KSTMainWindow::getActiveChat() const
{
    return activeChat;
}

int KSTMainWindow::getASPort() const
{
    return ASPort;
}

int KSTMainWindow::getASTimeout() const
{
    return ASTimeout;
}

void KSTMainWindow::sendKST(QString msg)
{
    kstclient->write((msg + "\r\n").toLocal8Bit());
    trace("Send to KST: " + msg);
}
void KSTMainWindow::checkAwayButton()
{
    QSharedPointer<KstUser> test(new KstUser());
    test->call = myCallsign.toUpper();
    test->chat = activeChat;
    if (std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
    {
        int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());

        QSharedPointer<KstUser> user = callVector->at(row);

        if (user->away)
        {
            ui->awayButton->setText(tr("Set Back"));
        }
        else
        {
            ui->awayButton->setText(tr("Set Away"));
        }
    }
}

void KSTMainWindow::analyseKstMessage(QString atj)
{
//    18:58:18.640 messageRx: 1858Z ES4RM Sergei> (OH3DP) i am on 1558
//    18:58:44.037 messageRx: 1858Z OH3DP Hannu 2m, 70, 23> 1000


    atj = atj.trimmed();

    if (!kstconnected && atj.contains(" login "))
    {
//        11:28:01.769 Client read : LOGINC|G0GJV|62rosehill|2|KST2Me 1.2.0.0
//        11:28:01.769 Client read : |20|20|1|0|0|

        QString loginMessage = "LOGINC|"
                + myCallsign
                + "|" + password
                + "|" + QString::number(kstChatSelection[0])
                + "|" + "Minos 0.0.0.999"   // client software version
                + "|20" // past messages
                + "|0"  // past DX/map messages
                + "|1"  // users list/update flags - If the users list/update flags = 0, no Uxx frames will be sent (even after the login)
                + "|0"   // last Unix timestamp for messages
                + "|0"   // last Unix timestamp for dx/map
                + "|";

        sendKST(loginMessage);

        kstLoggedIn.append(kstChatSelection[0]);

        //Optional allowed frames between LOGINC and SDONE are SDXQ, SMAQ, RDXQ and RMAQ.
        // we don't need them - yet


        return;
    }

    QStringList sl;
    sl = atj.split("|");

    if (sl[0] == "LOGSTAT")
    {
        // user config
//        LOGSTATS if LOGINC:
//        LOGSTAT|100|2|20040703a|239E038F12E685FB75C6C03A79A1DE8A|11|Alain/telnet|Stiévenart|JO20HI|on4kst@skynet.be|
//        LOGSTAT|100|chat id|client software version|session key|config|first name|last name|locator|email|
        if (sl[1] == "100")
        {
            kstconnected = true;
            QString sdone = "SDONE|" + QString::number(kstChatSelection[0]) +"|";
            sendKST(sdone);
        }
        else
        {
            //messageRx: LOGSTAT|101|Unknown user "XX0GJV".|
            //messageRx: LOGSTAT|114|Wrong password!|
            kstclient->disconnectFromHost();

//            clearConnection();
            mShowMessage(sl[2], this);
            doConfiguration();
        }

    }
    else if (sl[0] == "CR")
    {
        // message frame at login
        // CR|chat id|Unix time|callsign|firstname|destination|msg|highlight|

        QSharedPointer<KstMessageLine> kst(new KstMessageLine());

        kst->sequence = messageSequence++;

        kst->chat = sl[1].toInt();
        kst->fullLine = atj;

        QString unixTime = sl[2];
        kst->dtg = QDateTime::fromMSecsSinceEpoch(unixTime.toLongLong() * 1000);

        kst->call = sl[3];
        kst->distance = -2;
        kst->name = sl[4];
        QString destination = sl[5];
        kst->message = sl[6];
        kst->otherCall = sl[7];
        if (kst->otherCall == "0")
        {
            kst->otherCall.clear();
            if (destination != "0")
            {
                kst->otherCall = destination;
            }
        }
        kst->otherDistance = -2;
        bool found = false;
        for(QVector<QSharedPointer<KstMessageLine> >::iterator i = messageVector->begin(); i != messageVector->end(); i++)
        {
            QSharedPointer<KstMessageLine> msg = (*i);
            if (kst->fullLine == msg->fullLine)
            {
                found = true;
                break;
            }
        }
        if (!found)
            kstMessageModel.appendLastRow(kst);

    }
    else if (sl[0] == "CE")
    {
        // end of CR frames
        QModelIndex mesIndex = kstMessageFilterModel.index(kstMessageFilterModel.rowCount() - 1, 0);
        ui->messageTable->scrollTo(mesIndex);

        QModelIndex meepIndex = kstMeepFilterModel.index(kstMeepFilterModel.rowCount() - 1, 0);
        ui->meepTable->scrollTo(meepIndex);
    }
    else if (sl[0] == "CH")
    {
        // message frame after login
        // CH|chat id|date|callsign|firstname|destination|msg|highlight|

        QSharedPointer<KstMessageLine> kst(new KstMessageLine());

        kst->sequence = messageSequence++;

        kst->chat = sl[1].toInt();
        kst->fullLine = atj;

        QString unixTime = sl[2];
        kst->dtg = QDateTime::fromMSecsSinceEpoch(unixTime.toLongLong() * 1000);

        kst->call = sl[3];
        kst->distance = calcDistance(kst->call);
        kst->name = sl[4];
        QString destination = sl[5];
        kst->message = sl[6];
        kst->otherCall = sl[7];
        if (kst->otherCall == "0")
        {
            kst->otherCall.clear();
            if (destination != "0")
            {
                kst->otherCall = destination;
            }
        }
        kst->otherDistance = calcDistance(kst->otherCall);

        kstMessageModel.appendLastRow(kst);

        QModelIndex mesIndex = kstMessageFilterModel.index(kstMessageFilterModel.rowCount() - 1, 0);
        ui->messageTable->scrollTo(mesIndex);

        QModelIndex meepIndex = kstMeepFilterModel.index(kstMeepFilterModel.rowCount() - 1, 0);
        ui->meepTable->scrollTo(meepIndex);

        if (kst->otherCall == myCallsign)
        {
            QApplication::alert(this, 10000);   // 10 sec alert
        }

    }
    else if (sl[0] == "DL")
    {
        // DX frames for the DX window
    }
    else if (sl[0] == "DE")
    {
        // end of DX frames
    }
    else if (sl[0] == "ML")
    {
        // DX frames for the map window
    }
    else if (sl[0] == "ME")
    {
        // end of ML frames
    }
    else if (sl[0] == "DM")
    {
        // DX frames for the DX and the MAP windows
    }
    else if (sl[0] == "DF")
    {
        // end of DM frames
    }
    else if (sl[0] == "LOC")
    {
        //Locator update
        // LOC|Unix time|callsign|locator|
    }

    else if (sl[0] == "UA0")
    {
        //    User frame at login
//    UA0|chat id|callsign|firstname|locator|state|
//    UA0|2|DK5EW|Erwin MMMonVHF|JN47NX|0|
//    UA0|2|DK5OX|Boris 6/2m|JN59MO|2|
//    UA0|2|DL3JIN|Peter|JO60LX|1|
//    User state:
//        bit 0: the user is away -displayed "(callsign)" -
//        bit 1: logged within the last 5 minutes
//        bit 2: sysop
//        bit 3: it would be not used (user with privileges)

        QSharedPointer<KstUser> test(new KstUser());
        test->chat = sl[1].toInt();
        test->call = sl[2];
        test->name = sl[3];
        test->loc = sl[4];
        QString state = sl[5];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        if (!std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
        {
            Callsign cs(test->call);
            cs.validate();
            QSharedPointer<CountrySynonym> syn = MultLists::getMultLists()->searchCountrySynonym ( cs.locCtryPrefix );
            if ( syn )
            {
                test->prefix = syn->country->basePrefix;
                test->country = syn->country->realName;
                test->baseCall = cs.realCall;
                test->distance = -2;
            }


            int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());
            callVector->insert(row, test);
            callVectorChanged = true;
        }
    }

    else if (sl[0] == "UE")
    {
//    Users statistics/end of users frames
//    UE|chat id|nb registered users|
//    UE|2|4777|

        kstCallModel.setCallVector(callVector);
        kstMessageModel.setChatVector(messageVector);

        kstCallFilterModel.invalidate();
        kstMessageFilterModel.invalidate();

        QModelIndex mesIndex = kstMessageFilterModel.index(kstMessageFilterModel.rowCount() - 1, 0);
        ui->messageTable->scrollTo(mesIndex);

        QModelIndex meepIndex = kstMeepFilterModel.index(kstMeepFilterModel.rowCount() - 1, 0);
        ui->meepTable->scrollTo(meepIndex);

    }

    else if (sl[0] == "US4")
    {
//    User state (here/not here/more than 5 min logged)
//    US4|chat id|callsign|state|
//    US4|2|OH2JXA|state|

        QSharedPointer<KstUser> test(new KstUser());
        test->chat = sl[1].toInt();
        test->call = sl[2];
        QString state = sl[3];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        QVector<QSharedPointer<KstUser> >::iterator l = std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare);
        if (l != callVector->end() && l->data()->call == test->call && l->data()->chat == test->chat)
        {
            // as it should be...
            l->data()->away = test->away;
            l->data()->recent = test->recent;
            int row = l - callVector->begin();
            emit kstCallModel.dataChanged(kstCallModel.index(row, 0), kstCallModel.index(row, kstCallModel.columnCount() - 1));
        }
    }

    else if (sl[0] == "UM3")
    {
//    User already logged
//    UM3|chat id|callsign|firstname|locator|state|
//    UM3|2|OZ2M|Bo|JO65FR|2|

        QSharedPointer<KstUser> test(new KstUser());
        test->chat = sl[1].toInt();
        test->call = sl[2];
        test->name = sl[3];
        test->loc = sl[4];
        QString state = sl[5];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        QVector<QSharedPointer<KstUser> >::iterator l = std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare);
        if (l != callVector->end() && l->data()->call == test->call && l->data()->chat == test->chat)
        {
            // as it should be...
            l->data()->name = test->name;
            l->data()->loc = test->loc;
            l->data()->away = test->away;
            l->data()->recent = test->recent;
            int row = l - callVector->begin();
            emit kstCallModel.dataChanged(kstCallModel.index(row, 0), kstCallModel.index(row, kstCallModel.columnCount() - 1));

        }

    }

    else if (sl[0] == "UR6")
    {
//    User disconnected (to remove)
//    UR6|chat id|callsign|
//    UR6|2|RA3MR/3|
        QSharedPointer<KstUser> test(new KstUser());
        test->chat = sl[1].toInt();
        test->call = sl[2];

        QVector<QSharedPointer<KstUser> >::iterator l = std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare);
        if (l != callVector->end() && l->data()->call == test->call && l->data()->chat == test->chat)
        {
            // as it should be...

            // if we remove the last row then the call model
            // is one short as we have already removed it from the vector
            int row = l - callVector->begin();

            kstCallModel.removeRow(row);
            callVectorChanged = true;
        }
    }

    else if (sl[0] == "UA5")
    {
//    UA5 user connected (to add)
//    UA5|chat id|callsign|firstname|locator|state|
//    UA5|2|PA0GUS|GUUS|JO23TA|2|

        QSharedPointer<KstUser> test(new KstUser());
        test->chat = sl[1].toInt();
        test->call = sl[2];
        test->name = sl[3];
        test->loc = sl[4];
        QString state = sl[5];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        if (!std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
        {
            Callsign cs(test->call);
            cs.validate();
            QSharedPointer<CountrySynonym> syn = MultLists::getMultLists()->searchCountrySynonym ( cs.locCtryPrefix );
            if ( syn )
            {
                test->prefix = syn->country->basePrefix;
                test->country = syn->country->realName;
                test->baseCall = cs.realCall;
            }
            int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());
            kstCallModel.insertRow(row, test);
            callVectorChanged = true;
        }

    }
    else if (sl[0] == "CK")
    {
        doLoginChanges();
        // link check
        sendKST("\r\n");
        std::sort(messageVector->begin(), messageVector->end(), &compMessages);
        emit kstMessageModel.dataChanged(kstMessageModel.index(0, 0), kstMessageModel.index(kstMessageModel.rowCount() - 1, kstMessageModel.columnCount() - 1));

    }

    checkAwayButton();


}
void KSTMainWindow::on_connectButton_clicked()
{
    if (kstconnected)
    {
        // MSG|chat id|destination|command|0|
        QString quitMsg = "MSG|" + QString::number(kstChatSelection[0]) + "|0|/QUIT|0|";
        sendKST(quitMsg);
        kstclient->waitForBytesWritten(1000);
        kstclient->disconnectFromHost();
        kstLoggedIn.clear();
    }
    else
    {
        connectToHost();
    }
}
void KSTMainWindow::on_closeButton_clicked()
{
    close();
}


void KSTMainWindow::on_messageFilter_textChanged(const QString &arg1)
{
    kstMessageFilterModel.setFilterString(arg1);
    QModelIndex mesIndex = kstMessageFilterModel.index(kstMessageFilterModel.rowCount() - 1, 0);
    ui->messageTable->scrollTo(mesIndex);
}

void KSTMainWindow::on_CSFilter_textChanged(const QString &arg1)
{
    kstCallFilterModel.setFilterString(arg1);
}

void KSTMainWindow::on_kstSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QSettings settings;
    QByteArray state = ui->kstSplitter->saveState();
    settings.setValue("kstSplitterState" , state);
}

void KSTMainWindow::on_msgSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QSettings settings;
    QByteArray state = ui->msgSplitter->saveState();
    settings.setValue("msgSplitterState" , state);
}

void KSTMainWindow::on_callSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QSettings settings;
    QByteArray state = ui->callSplitter->saveState();
    settings.setValue("callSplitterState" , state);
}

void KSTMainWindow::on_sectionResized(int, int, int)
{
    QSettings settings;
    QByteArray state;

    state = ui->CSTable->horizontalHeader()->saveState();
    settings.setValue("CSTable/state", state);

    state = ui->messageTable->horizontalHeader()->saveState();
    settings.setValue("messageTable/state", state);

    state = ui->meepTable->horizontalHeader()->saveState();
    settings.setValue("meepTable/state", state);
}
void KSTMainWindow::on_sectionMoved(int, int, int)
{
    on_sectionResized(0, 0, 0);
}

void KSTMainWindow::acChanged(QSharedPointer<KstUser> user)
{
    int row = callVector->indexOf(user);
    emit kstCallModel.dataChanged(kstCallModel.index(row, ecscAirscout), kstCallModel.index(row, ecscAirscout));

    if (user == planeActive)
    {
        showPlanes(user);
    }
}
void KSTMainWindow::showPlanes(QSharedPointer<KstUser> user)
{
    planeActive = user;

    if (user->lastCalcTime.isEmpty())
    {
        ui->planeslabel->setText(tr(""));
    }
    else
    {
        QString l = QString("%1\n%2 at %3\nto %4 at %5")
                .arg(user->lastCalcTime)
                .arg(user->fromCall).arg(user->fromLoc).arg(user->toCall)
                .arg(user->toLoc);

        ui->planeslabel->setText(l);
    }
    kstPlanesModel.setPlanesVector(user->planes);

//    ui->planesText->clear();
//    ui->planesText->append(QString("%1 %2 at %3 to %4 at %5").arg(user->lastCalcTime).arg(user->fromCall).arg(user->fromLoc).arg(user->toCall).arg(user->toLoc) );
//    ui->planesText->append(QString());
//    foreach(const Aircraft &ac, user->planes)
//    {
//        ui->planesText->append(ac.getAircraft() );
//    }
//    ui->planesText->moveCursor (QTextCursor::Start) ;
//    ui->planesText->ensureCursorVisible() ;
}

void KSTMainWindow::on_CSTable_clicked(const QModelIndex &index)
{
    QModelIndex sourceIndex = kstCallFilterModel.mapToSource(index);
    int row = sourceIndex.row();
    if (row >= callVector->size())
        return;

    QSharedPointer<KstUser> user = callVector->at(row);

    if (!ui->noSetCallcb->isChecked())
    {
        // messages
        QString call = user->call;

        setNameFromCall(call);

        ui->messageFilter->setText(call);
        ui->callEdit->setText(call);
        ui->msgEdit->setFocus();
        setActive(user->chat);
        ui->messageChatFilter->setCurrentIndex(user->chat);
    }
    // Planes
    showPlanes(user);
}

bool KSTMainWindow::doConfiguration()
{
    KSTConfigure conf;

    conf.hostname = serverName;
    conf.port = serverPort;
    conf.username = myCallsign;
    conf.password = password;
    conf.autoConnect = autoConnect;
    conf.locator = myLoc;
    conf.maxDistance = maxDistance;

    conf.ASActive = ASActive;
    conf.ASActiveBand = ASActiveBand;
    conf.ASServerName = ASServerName;
    conf.ASMyName = ASMyName;
    conf.ASMinDistance = ASMinDistance;
    conf.ASMaxDistance = ASMaxDistance;
    conf.ASPort = ASPort;
    conf.ASTimeout = ASTimeout;

    int ret = conf.exec();
    if (ret == QDialog::Accepted)
    {
        serverName = conf.hostname.trimmed();
        serverPort = conf.port.trimmed();
        myCallsign = conf.username.trimmed();
        password = conf.password.trimmed();
        autoConnect = conf.autoConnect;
        myLoc = conf.locator.trimmed();
        maxDistance = conf.maxDistance;
        ASActive = conf.ASActive;
        ASActiveBand = conf.ASActiveBand;
        ASServerName = conf.ASServerName.trimmed();
        ASMyName = conf.ASMyName.trimmed();
        ASMinDistance = conf.ASMinDistance;
        ASMaxDistance = conf.ASMaxDistance;
        ASPort = conf.ASPort;
        ASTimeout = conf.ASTimeout;

        QSettings settings;

        settings.setValue("hostname", serverName);
        settings.setValue("port", serverPort);
        settings.setValue("username", myCallsign);
        settings.setValue("password", password);
        settings.setValue("autoConnect", autoConnect);
        settings.setValue("locator", myLoc);
        settings.setValue("maxDistance", maxDistance);

        settings.setValue("ASActive", ASActive);
        settings.setValue("ASServerName", ASServerName);
        settings.setValue("ASMyName", ASMyName);
        settings.setValue("ASActiveBand", ASActiveBand);
        settings.setValue("ASMinDistance", ASMinDistance);
        settings.setValue("ASMaxDistance", ASMaxDistance);
        settings.setValue("ASPort", ASPort);
        settings.setValue("ASTimeout", ASTimeout);

        kstCallFilterModel.invalidate();
        kstMessageFilterModel.invalidate();

        if (getASActive())
        {
            asl.reset();

            asl = QSharedPointer<AirScoutLink>(new AirScoutLink());
            connect(asl.data(), SIGNAL(acChanged(QSharedPointer<KstUser>)), this, SLOT(acChanged(QSharedPointer<KstUser>)));
        }
        if  (kstconnected)
        {
            reconnect();
        }
        return true;
    }
    return false;
}

void KSTMainWindow::on_configureButton_clicked()
{
    doConfiguration();
}
void KSTMainWindow::reconnect()
{
    if (kstconnected)
    {
        // MSG|chat id|destination|command|0|
        QString quitMsg = "MSG|" + QString::number(kstChatSelection[0]) + "|0|/QUIT|0|";
        sendKST(quitMsg);

        kstclient->waitForBytesWritten(1000);
        kstclient->disconnectFromHost();
    }

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, [=]()
    {
        // NB a lambda function
        connectToHost();
        timer->deleteLater();
    }
    );

    timer->start(100);
}
void KSTMainWindow::on_genmsgButton_clicked()
{
    QString msg = ui->msgEdit->text();
    if (!msg.isEmpty())
    {
        QString msg2 = "MSG|" + QString::number(activeChat) + "|0|" + msg + "|0|";
        sendKST(msg2);
    }
    ui->msgEdit->clear();
}

void KSTMainWindow::on_meepButton_clicked()
{
    QString msg = ui->msgEdit->text();
    QString call = ui->callEdit->text();
    if (!msg.isEmpty() && !call.isEmpty())
    {
        QString msg = ui->msgEdit->text();
        if (!msg.isEmpty())
        {
            QString msg2 = "MSG|" + QString::number(activeChat) + "|0|/CQ " + call + " " + msg + "|0|";
            sendKST(msg2);
        }
        ui->msgEdit->clear();
    }
}

void KSTMainWindow::setNameFromCall(QString call)
{
    QSharedPointer<KstUser> test(new KstUser());
    test->call = call;
    if (std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
    {
        int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());

        QSharedPointer<KstUser> user = callVector->at(row);

        QStringList name = user->name.split(' ');

        ui->msgEdit->setText("Hi " + name[0] + " ");
    }
    else
    {
        ui->msgEdit->clear();
    }
}


void KSTMainWindow::doLoginChanges()
{
    bool detached = false;
    int j = 0;
    if (kstLoggedIn.count())
    {
        if (kstChatSelection.count())
        {
            for (int i = 0; i < 4; i++)
            {
                bool loggedin = kstLoggedIn.contains(i+1);
                bool loginWanted = kstChatSelection.contains(i+1);
                if (!loggedin && loginWanted)
                {
                    QTimer *timer = new QTimer(this);
                    timer->setSingleShot(true);

                    connect(timer, &QTimer::timeout, [=]()
                    {
                        // NB a lambda function
                        // add chat
                        QString attachMessage = QString("ACHAT")
                                + "|" + QString::number(i + 1)
                                + "|20"  // past messages
                                + "|0"  // past DX/map messages
                                + "|1"   // users list/update flags - If the users list/update flags = 0, no Uxx frames will be sent (even after the login)
                                + "|0"   // last Unix timestamp for messages
                                + "|0"   // last Unix timestamp for dx/map
                                + "|";
                        sendKST(attachMessage);
                        timer->deleteLater();
                    }
                    );

                    timer->start(1000 * j);
                    j++;

                }
                if (loggedin && !loginWanted)
                {
                    QTimer *timer = new QTimer(this);
                    timer->setSingleShot(true);

                    detached = true;
                    connect(timer, &QTimer::timeout, [=]()
                    {
                        // NB a lambda function
                        // detach chat
                        QString detachMessage = QString("DCHAT")
                                + "|" + QString::number(i + 1)
                                + "|";
                        sendKST(detachMessage);
                        timer->deleteLater();
                    }
                    );

                    timer->start(1000 * j);
                    j++;
                }
            }
            kstLoggedIn = kstChatSelection;
        }
        else
        {
            kstclient->disconnectFromHost();
            kstLoggedIn.clear();
            kstCallModel.reset();
            callVector->clear();

        }
    }
    else
    {
        if (kstChatSelection.count())
        {
            kstCallModel.locator = myLoc;
            if (autoConnect)
            {
                if (kstclient->state() != QAbstractSocket::ConnectedState
                   && kstclient->state() != QAbstractSocket::ConnectingState
                   && kstclient->state() != QAbstractSocket::ClosingState
                   && kstclient->state() != QAbstractSocket::HostLookupState)
                {
                    kstclient->connectToHost(serverName, serverPort.toUShort());
                }
            }
        }

    }
    if (detached)
    {
        QSharedPointer<QVector<QSharedPointer<KstUser> > > newCallVector(new QVector<QSharedPointer<KstUser> >);
        for(QVector<QSharedPointer<KstUser> >::iterator i = callVector->begin(); i != callVector->end(); i++)
        {
            int c = (*i)->chat;
            if (kstLoggedIn.contains(c))
            {
                newCallVector->push_back((*i));
            }
        }
        kstCallModel.setCallVector(newCallVector);
        callVector = newCallVector;
    }

    checkActive();
}

void KSTMainWindow::on_messageTable_clicked(const QModelIndex &index)
{
    QModelIndex sourceIndex = kstMessageFilterModel.mapToSource(index);
    int row = sourceIndex.row();
    if (row >= messageVector->size())
        return;
    QSharedPointer<KstMessageLine> line = messageVector->at(row);
    QString call = line->call;
    if (call.compare(myCallsign, Qt::CaseInsensitive) == 0)
    {
        call = line->otherCall;
    }

    setNameFromCall(call);

    ui->callEdit->setText(call);
    ui->msgEdit->setFocus();
    setActive(line->chat);
}

void KSTMainWindow::setActive(int chat)
{
    if (kstChatSelection.contains(chat))
    {
        switch(chat)
        {
        case 1:
            ui->active1rb->setChecked(true);
            break;
        case 2:
            ui->active2rb->setChecked(true);
            break;
        case 3:
            ui->active3rb->setChecked(true);
            break;
        case 4:
            ui->active4rb->setChecked(true);
            break;
        }
        activeChat = chat;
    }
    checkAwayButton();
}
void KSTMainWindow::checkActive()
{
    if (kstChatSelection.count() > 0 && !kstChatSelection.contains( activeChat))
    {
        int a = kstChatSelection[0];
        setActive(a);
    }
}
void KSTMainWindow::on_meepTable_clicked(const QModelIndex &index)
{
    QModelIndex sourceIndex = kstMeepFilterModel.mapToSource(index);
    QSharedPointer<KstMessageLine> line = messageVector->at(sourceIndex.row());
    QString call = line->call;
    if (call.compare(myCallsign, Qt::CaseInsensitive) == 0)
    {
        call = line->otherCall;
    }
    setNameFromCall(call);
    ui->callEdit->setText(call);
    ui->msgEdit->setFocus();
    setActive(line->chat);

}

void KSTMainWindow::on_clearButton_clicked()
{
    kstMessageModel.reset();
}
bool KSTMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress )
    {
        QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            if (obj == ui->messageFilter)
            {
                ui->messageFilter->clear();
            }
            else if (obj == ui->CSFilter)
            {
                ui->CSFilter->clear();
            }
            else if (obj == ui->callEdit)
            {
                ui->callEdit->clear();
            }
            else if (obj == ui->msgEdit)
            {
                ui->msgEdit->clear();
            }
        }
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
        {
            if (ui->meepButton->isDefault())
            {
                ui->meepButton->click();
            }
            else if (ui->genmsgButton->isDefault())
            {
                ui->genmsgButton->click();
            }
        }
    }

   return false;    // pass the event on
}
void KSTMainWindow::on_sortIndicatorChanged(int /*logicalIndex*/, Qt::SortOrder /*order*/)
{
    on_sectionResized(0, 0, 0);
}

void KSTMainWindow::on_callEdit_textChanged(const QString &/*arg1*/)
{
    if (ui->callEdit->text().isEmpty())
    {
        ui->meepButton->setDefault(false);
        ui->genmsgButton->setDefault(true);
    }
    else
    {
        ui->genmsgButton->setDefault(false);
        ui->meepButton->setDefault(true);
    }
}

void KSTMainWindow::on_clearMessageButton_clicked()
{
    ui->callEdit->clear();
    ui->msgEdit->clear();
    ui->messageFilter->setFocus();
}

void KSTMainWindow::on_awayButton_clicked()
{
    QSharedPointer<KstUser> test(new KstUser());
    test->call = myCallsign.toUpper();
    test->chat = activeChat;
    if (std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
    {
        int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());

        QSharedPointer<KstUser> user = callVector->at(row);

        if (user->away)
        {
            QString msg = "MSG|" + QString::number(activeChat) + "|0|/BACK|0|";
            sendKST(msg);

        }
        else
        {
            QString msg = "MSG|" + QString::number(activeChat) + "|0|/AWAY|0|";
            sendKST(msg);
        }
    }
}
void KSTMainWindow::resetVectors(QCheckBox *cb, QRadioButton *rb, int c, QStringList &s, QVector<int> &v, QVector<int> &a)
{
    if (!kstChatSelection.contains(c) && cb->isChecked())
    {
        // not selected -> selected
        {
            s.append(QString::number(c));
            v.append(c);
            rb->setVisible(true);
            setActive(c);
            a.append(c);
        }
    }
    else if (kstChatSelection.contains(c) && !cb->isChecked())
    {
        // selected -> not selected
        rb->setVisible(false);
    }
    else if (kstChatSelection.contains(c))
    {
        s.append(QString::number(c));
        v.append(c);
    }
    else if (!kstChatSelection.contains(c) && !cb->isChecked())
    {
        rb->setVisible(false);
    }
    checkAwayButton();
}

void KSTMainWindow::logincb_stateChanged(int /*arg1*/)
{
    QStringList s;
    QVector<int> v;
    QVector<int> a;

    resetVectors(ui->login1cb, ui->active1rb, 1, s, v, a);
    resetVectors(ui->login2cb, ui->active2rb, 2, s, v, a);
    resetVectors(ui->login3cb, ui->active3rb, 3, s, v, a);
    resetVectors(ui->login4cb, ui->active4rb, 4, s, v, a);

    kstChatSelection = v;
    if (a.count())
        setActive(a[0]);
    doLoginChanges();
    QSettings settings;
    settings.setValue("service", s.join(":"));
}
void KSTMainWindow::activerb_clicked()
{
    if (ui->active1rb->isChecked())
    {
        activeChat = 1;
    }
    else if (ui->active2rb->isChecked())
    {
        activeChat = 2;
    }
    else if (ui->active3rb->isChecked())
    {
        activeChat = 3;
    }
    else if (ui->active4rb->isChecked())
    {
        activeChat = 4;
    }
    QSettings settings;
    settings.setValue("active", QString::number(activeChat));
    checkAwayButton();
}
void KSTMainWindow::on_messageChatFilter_currentIndexChanged(int index)
{
    if (started)
    {
        messageChatFilter = index;

        kstMessageFilterModel.setChatFilter(messageChatFilter);

        QSettings settings;
        settings.setValue("messageChatFilter", QString::number(messageChatFilter));
    }
}
void KSTMainWindow::on_CSChatFilter_currentIndexChanged(int index)
{
    if (started)
    {
        CSChatFilter = index;

        kstCallFilterModel.setChatFilter(CSChatFilter);

        QSettings settings;
        settings.setValue("CSChatFilter", QString::number(CSChatFilter));
    }
}


void KSTMainWindow::on_clearMessageFilter_clicked()
{
    ui->messageChatFilter->setCurrentIndex(0);
    ui->messageFilter->clear();

    kstMeepFilterModel.invalidate();    // try to get rid of the colouring in the meep table
}

void KSTMainWindow::on_clearUserFilter_clicked()
{
    ui->CSChatFilter->setCurrentIndex(0);
    ui->CSFilter->clear();
}

void KSTMainWindow::on_asBandCombo_currentIndexChanged(int band)
{
    if (started)
    {
        if (asl && getASActive())
        {
            for (int i = 0; i < callVector->size(); i++)
            {
                QSharedPointer<KstUser> kstuser = callVector->at(i);
                kstuser->planes.clear();
                kstuser->planeResponseSeen = false;
            }
            callVectorChanged = true;
            emit kstCallModel.dataChanged(kstCallModel.index(0, ecscAirscout), kstCallModel.index(callVector->size(), ecscAirscout));

            userCallTimerTimer();
        }
        QSettings settings;

        settings.setValue("ASActiveBand", band);
    }
}

void KSTMainWindow::on_ASActivecb_stateChanged(int state)
{
    if (started)
    {
        if (asl && getASActive())
        {
            for (int i = 0; i < callVector->size(); i++)
            {
                QSharedPointer<KstUser> kstuser = callVector->at(i);
                kstuser->planes.clear();
                kstuser->planeResponseSeen = false;
            }
            callVectorChanged = true;
            emit kstCallModel.dataChanged(kstCallModel.index(0, ecscAirscout), kstCallModel.index(callVector->size(), ecscAirscout));

            asl->clearWatchList();
            userCallTimerTimer();
        }

        QSettings settings;

        settings.setValue("ASActive", state != 0);

        ui->planesFrame->setVisible(state != 0);
        if(state != 0)
        {
            ui->CSTable->showColumn(ecscAirscout);
        }
        else
        {
            ui->CSTable->hideColumn(ecscAirscout);
        }
    }
}

void KSTMainWindow::on_showInAS_clicked()
{
    asl->asSelected(planeActive);
}
QSharedPointer<KstUser> KSTMainWindow::getUser(QString call)
{
    QSharedPointer<KstUser> test(new KstUser());
    test->call = call.toUpper();
    test->chat = activeChat;
    if (std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
    {
        int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());

        QSharedPointer<KstUser> user = callVector->at(row);

        return user;
    }
    return QSharedPointer<KstUser>();
}
void KSTMainWindow::on_showMPath_clicked()
{
    QModelIndex index = ui->messageTable->currentIndex();
    QModelIndex sourceIndex = kstMessageFilterModel.mapToSource(index);
    int row = sourceIndex.row();
    if (row < 0 || row >= messageVector->size())
        return;
    QSharedPointer<KstMessageLine> line = messageVector->at(row);
    QString call = line->call;
    QString otherCall = line->otherCall;

    QSharedPointer<KstUser> user = getUser(call);
    QSharedPointer<KstUser> other = getUser(otherCall);

    if (user && other)
    {
        asl->asShowPath(user, other);
    }
}

void KSTMainWindow::on_maxDistanceEdit_editingFinished()
{
    maxDistance = ui->maxDistanceEdit->text().toInt();
    QSettings settings;
    settings.setValue("maxDistance", maxDistance);

    kstCallFilterModel.invalidate();
    kstMessageFilterModel.invalidate();
}
