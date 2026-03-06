#include <QHostInfo>
#include <QSettings>
#include <QKeyEvent>
#include <QFileDialog>

#include "QtUtils.h"
#include "RPCCommandConstants.h"
#include "regsettings.h"
#include "AppStartup.h"
#include "MShowMessageDlg.h"
#include "MonitoredLog.h"
#include "cutils.h"
#include "callsign.h"
#include "fileutils.h"
#include "kstconfigure.h"
#include "airscoutlink.h"
#include "delayedaction.h"
#include "changename.h"
#include "LogEvents.h"
#include "kstmonitoredlogs.h"
#include "mults.h"
#include "MinosRPC.h"
#include "kstmainwindow.h"
#include "remotelogs.h"
#include "soundplayer.h"
#include "ui_kstmainwindow.h"

QStringList services =
{
"50/70 MHz",
"144/432 MHz",
"Microwave",
"EME/JT65",
};

KSTMainWindow *mainWindow = nullptr;

const char *traceStart = "*-*_*";
const char *traceEnd= "_**_";
const char traceEndnlChar= '*';
const char traceEndChar= '!';
//==========================================================================================
void KSTMainWindow::getSettings(QSettings &settings)
{
    KSTserverName = settings.value("hostname", "www.on4kst.info").toString().trimmed();
    KSTserverPort = settings.value("port", "23001").toString().trimmed();
    TNServerName = settings.value("tnhostname", "www.on4kst.info").toString();
    TNServerPort = settings.value("tnport", "23000").toString();
    myCallsign.setFullCall(settings.value("username", "").toString());
    password = settings.value("password", "").toString().trimmed();
    maxDistance = settings.value("maxDistance", 99999).toInt();
    firstName = settings.value("firstName", "").toString().trimmed();

    meepNotifyLogger = settings.value("meepNotifyLogger", false).toBool();
    meepPlaySound = settings.value("meepPlaySound", false).toBool();
    meepSoundFile = settings.value("meepSoundFile", "").toString();
    meepVolume = settings.value("meepVolume", 50).toInt();


    ASActive = settings.value("ASActive", false).toBool();
    ASServerName = settings.value("ASServerName", "AS").toString().trimmed();
    ASMyName = settings.value("ASMyName", "Minos").toString().trimmed();
    ASActiveBand = static_cast<ASBand>(settings.value("ASActiveBand", 0).toInt());
    ASMinDistance = settings.value("ASMinDistance", 300).toInt();
    ASMaxDistance = settings.value("ASMaxDistance", 1000).toInt();
    ASPort = settings.value("ASPort", 9872).toInt();
    ASTimeout = settings.value("ASTimeout", 10).toInt();
    chatSelection = settings.value("service", "1").toString();
    activeChat = settings.value("active", "0").toInt();
    autoConnect = settings.value("autoConnect", false).toBool();
    myLoc = settings.value("locator", "").toString();

}

KSTMainWindow::KSTMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::KSTMainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mainWindow = this;

    /*MinosRPC *rpc =*/ MinosRPC::getMinosRPC(getAppStartupName(), true);

    callVector =    QSharedPointer<QVector <QSharedPointer<KstUser> > >( new QVector<QSharedPointer<KstUser> > );
    messageVector = QSharedPointer<QVector <QSharedPointer<KstMessageLine> > >( new QVector<QSharedPointer<KstMessageLine> >);

    iniName = getDirectoryLocation(dlConfiguration) + "/" + getAppStartupName() + ".ini";

    RemoteLogs::setSettingsFile(iniName);
    bool needTransfer = !FileExists(iniName);

    if (needTransfer)
    {
        QSettings rsettings;    // NOT RegSettings
        getSettings(rsettings);
        doConfiguration(false); // transfer everything to INI file
    }
    else
    {
        QSettings isettings(iniName, QSettings::IniFormat);
        getSettings(isettings);
        if (firstName.length() > 16)
        {
            doConfiguration(true);
        }
    }

    QStringList selections = chatSelection.split(":");

    for (auto const &i: QASCONST(selections))
    {
        int s = i.toInt();
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

    setActive(activeChat);

    ui->login1cb->setText(services[0]);
    ui->login2cb->setText(services[1]);
    ui->login3cb->setText(services[2]);
    ui->login4cb->setText(services[3]);

    connect(ui->login1cb, &QCheckBox::stateChanged, this, &KSTMainWindow::logincb_stateChanged);
    connect(ui->login2cb, &QCheckBox::stateChanged, this, &KSTMainWindow::logincb_stateChanged);
    connect(ui->login3cb, &QCheckBox::stateChanged, this, &KSTMainWindow::logincb_stateChanged);
    connect(ui->login4cb, &QCheckBox::stateChanged, this, &KSTMainWindow::logincb_stateChanged);

    ui->active1rb->setText(services[0]);
    ui->active2rb->setText(services[1]);
    ui->active3rb->setText(services[2]);
    ui->active4rb->setText(services[3]);

    connect(ui->active1rb, &QRadioButton::clicked, this, &KSTMainWindow::activerb_clicked);
    connect(ui->active2rb, &QRadioButton::clicked, this, &KSTMainWindow::activerb_clicked);
    connect(ui->active3rb, &QRadioButton::clicked, this, &KSTMainWindow::activerb_clicked);
    connect(ui->active4rb, &QRadioButton::clicked, this, &KSTMainWindow::activerb_clicked);

    checkActive();

    ui->CSChatFilter->addItem(tr("Active"));
    ui->CSChatFilter->addItems(services);
    ui->CSChatFilter->setCurrentIndex(0);

    ui->messageChatFilter->addItem(tr("Active"));
    ui->messageChatFilter->addItems(services);
    ui->messageChatFilter->setCurrentIndex(0);

    RegSettings rsettings;

    QByteArray geometry = rsettings.getSettings().value("geometry/Main").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QByteArray state;
    state = rsettings.getSettings().value("kstSplitterState").toByteArray();
    ui->kstSplitter->restoreState(state);

    // Make sure the kstSplitter covers the maximum vertical space

//    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
//    sizePolicy.setHorizontalStretch(0);
//    sizePolicy.setVerticalStretch(0);
//    sizePolicy.setHeightForWidth(ui->kstSplitter->sizePolicy().hasHeightForWidth());
//    ui->kstSplitter->setSizePolicy(sizePolicy);

    state = rsettings.getSettings().value("msgSplitterState").toByteArray();
    ui->msgSplitter->restoreState(state);

    state = rsettings.getSettings().value("callSplitterState").toByteArray();
    ui->callSplitter->restoreState(state);

    createCloseEvent();
    connect(&CloseTimer, &QTimer::timeout, this, &KSTMainWindow::CloseTimerTimer);
    CloseTimer.start(100);

    connect(&userCallTimer, &QTimer::timeout, this, &KSTMainWindow::userCallTimerTimer);
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

    meepDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate("meepDelegate", 1.0, 1.0)) ;
    messageDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate("messageDelegate", 1.0, 1.0)) ;
    CSDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate("CSDelegate", 1.0, 1.0)) ;
    PlanesDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate("PlanesDelegate", 1.0, 1.0)) ;

    // these are used for sizing when adjust to content
    kstMessageModel.delegate = messageDelegate;
    kstCallModel.delegate = CSDelegate;

    ui->meepTable->setItemDelegate(meepDelegate.data());
    ui->messageTable->setItemDelegate(messageDelegate.data());
    ui->CSTable->setItemDelegate(CSDelegate.data());
    ui->planesView->setItemDelegate(PlanesDelegate.data());

    QHeaderView *verticalHeader = ui->meepTable->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setMinimumSectionSize(10);
    verticalHeader->setDefaultSectionSize(10);
    //verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);

    verticalHeader = ui->messageTable->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setMinimumSectionSize(10);
    verticalHeader->setDefaultSectionSize(10);
//    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
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

    state = rsettings.getSettings().value("CSTable/state").toByteArray();
    ui->CSTable->horizontalHeader()->restoreState(state);

    state = rsettings.getSettings().value("messageTable/state").toByteArray();
    ui->messageTable->horizontalHeader()->restoreState(state);

    state = rsettings.getSettings().value("meepTable/state").toByteArray();
    ui->meepTable->horizontalHeader()->restoreState(state);

    ui->CSTable->horizontalHeader()->setStretchLastSection(true);
    ui->CSTable->horizontalHeader()->setSectionsMovable( true );

    connect( ui->CSTable->horizontalHeader(), &QHeaderView::sectionResized,
             this, &KSTMainWindow:: on_sectionResized, Qt::UniqueConnection);
    connect( ui->messageTable->horizontalHeader(), &QHeaderView::sectionResized,
             this, &KSTMainWindow::on_sectionResized, Qt::UniqueConnection);
    connect( ui->meepTable->horizontalHeader(), &QHeaderView::sectionResized,
             this, &KSTMainWindow::on_sectionResized, Qt::UniqueConnection);
    connect( ui->planesView->horizontalHeader(), &QHeaderView::sectionResized,
             this, &KSTMainWindow::on_sectionResized, Qt::UniqueConnection);

    connect( ui->CSTable->horizontalHeader(), &QHeaderView::sectionMoved,
             this, &KSTMainWindow::on_sectionMoved);


    connect( ui->CSTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
             this, &KSTMainWindow::on_sortIndicatorChanged);

    connect(ui->CSTable->selectionModel(),&QItemSelectionModel::selectionChanged,
            this, &KSTMainWindow::onCSTableSelectionChanged);

    kstclient = new QTcpSocket(this);

    connect(kstclient, &QTcpSocket::connected, this, &KSTMainWindow::connected);
    connect(kstclient, &QTcpSocket::disconnected, this, &KSTMainWindow::disconnected);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(kstclient, &QTcpSocket::errorOccurred, this, &KSTMainWindow::connectionError);
#else
    connect(kstclient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
#endif
    connect(kstclient, &QTcpSocket::readyRead, this, &KSTMainWindow::onReadyRead);

    ui->CSFilter->installEventFilter(this);
    ui->messageFilter->installEventFilter(this);
    ui->callEdit->installEventFilter(this);
    ui->msgEdit->installEventFilter(this);
    ui->planesView->installEventFilter(this);
    ui->messageTable->installEventFilter(this);
    ui->toMeFilter->installEventFilter(this);

    installEventFilter(this);   // so we pick up return, and implement the default button

    ui->callEdit->setValidator(&ucValidator);

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
    connect(asl.data(), &AirScoutLink::acChanged, this, &KSTMainWindow::acChanged);

    for(auto const &s: QASCONST(AirScoutLink::ASBandStrings))
    {
        ui->asBandCombo->addItem(AirScoutLink::tr(s));
    }
    ui->asBandCombo->setCurrentIndex(ASActiveBand);

    ui->maxDistanceEdit->setText(QString::number(maxDistance));
    ui->maxDistanceEdit->setValidator(new QIntValidator(0, 0xffff, this));

    while ( myCallsign.getValRes() != CS_OK)
    {
        if (!doConfiguration(true))
            break;
    }
    started = true;


    if (autoConnect)
        doLoginChanges();

    logincb_stateChanged(0);

    ui->genmsgButton->setDefault(true);

    ui->messageFilter->setFocus();

    ui->stringRb->setChecked(true);

    ml = new KSTMonitoredLogs();
    connect(RemoteLogs::getRemoteLogs(), &RemoteLogs::newMonitoredLog, this, &KSTMainWindow::onNewLog);
    connect(RemoteLogs::getRemoteLogs(), &RemoteLogs::currentLogChanged, this, &KSTMainWindow::onLogChanged);

    connect(ml, &KSTMonitoredLogs::logStarted, this, &KSTMainWindow::onLogStarted);
    connect(ml, &KSTMonitoredLogs::logClosed, this, &KSTMainWindow::onLogClosed);

    ui->kstFrame->layout()->setContentsMargins(0, 0, 0, 0);
    ui->centralwidget->layout()->setContentsMargins(0, 0, 0, 0);

    ui->callSplitter->setMinimumWidth(10);
    ui->msgSplitter->setMinimumWidth(10);
    ui->callsFrame->setMinimumHeight(10);
    ui->planesFrame->setMinimumHeight(10);
    ui->msgFrame->setMinimumHeight(10);
    ui->tomeFrame->setMinimumHeight(10);

    on_FontChanged();
}

KSTMainWindow::~KSTMainWindow()
{
    asl.reset();
    delete ui;
}
void KSTMainWindow::on_FontChanged()
{
    int ls = 10;
    if (messageDelegate)
    {
        QString s = "Memxx";
        QSize r = messageDelegate->docSize(s);
        ls = r.height() *5/4;
    }

    QHeaderView *verticalHeader = ui->meepTable->verticalHeader();
    verticalHeader->setDefaultSectionSize(ls);

    verticalHeader = ui->messageTable->verticalHeader();
    verticalHeader->setDefaultSectionSize(ls);

}
void KSTMainWindow::resizeEvent(QResizeEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue("geometry/Main", saveGeometry());
    QWidget::resizeEvent(event);
}
void KSTMainWindow::moveEvent(QMoveEvent * event)
{
    RegSettings settings;
    settings.getSettings().setValue("geometry/Main", saveGeometry());
    QWidget::moveEvent(event);
}
void KSTMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        RegSettings settings;
        settings.getSettings().setValue("geometry/Main", saveGeometry());
    }
}
void KSTMainWindow::closeEvent(QCloseEvent *event)
{
    trace("KSTMainWindow::closeEvent");

    userCallTimer.stop();

    // and tidy up all loose ends

    if (kstconnected)
    {
        on_connectButton_clicked();
    }

    RegSettings settings;
    settings.getSettings().setValue("geometry/Main", saveGeometry());
    trace("KSTMainWindow Closing");

    delete ml;
    ml =nullptr;

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
   RemoteLogs::getRemoteLogs()->testAutoStart();
}

void KSTMainWindow::onNewLog(QSharedPointer<MonitoredLog> ml)
{
    connect(ml.data(), &MonitoredLog::newStanzas, this, &KSTMainWindow::onNewStanzas, Qt::QueuedConnection);
}
void KSTMainWindow::onLogChanged(QSharedPointer<MonitoredLog> /*ml*/)
{
    kstMessageFilterModel.invalidate();
    kstMeepFilterModel.invalidate();
    kstCallFilterModel.invalidate();
    kstPlanesFilterModel.invalidate();
}
void KSTMainWindow::onNewStanzas()
{
    kstMessageFilterModel.invalidate();
    kstMeepFilterModel.invalidate();
    kstCallFilterModel.invalidate();
    kstPlanesFilterModel.invalidate();
}
void KSTMainWindow::onLogStarted(QSharedPointer<MonitoredLog> /*ml*/)
{
    kstMessageFilterModel.invalidate();
    kstMeepFilterModel.invalidate();
    kstCallFilterModel.invalidate();
    kstPlanesFilterModel.invalidate();
}
void KSTMainWindow::onLogClosed(QSharedPointer<MonitoredLog> /*ml*/)
{
    kstMessageFilterModel.invalidate();
    kstMeepFilterModel.invalidate();
    kstCallFilterModel.invalidate();
    kstPlanesFilterModel.invalidate();
}
void KSTMainWindow::userCallTimerTimer()
{
    if (asl && getASActive() && callVectorChanged && callVector)
    {
        asl->usersChanged(callVector);
        callVectorChanged = false;
    }
}

void KSTMainWindow::connectToHost()
{
    kstLoggedIn.clear();
    while (myCallsign.getValRes() != CS_OK)
    {
        if (!doConfiguration(true))
            return;
    }
    if (kstChatSelection.count())
    {
        if (kstclient->state() != QAbstractSocket::ConnectedState
           && kstclient->state() != QAbstractSocket::ConnectingState
           && kstclient->state() != QAbstractSocket::ClosingState
           && kstclient->state() != QAbstractSocket::HostLookupState)
        {
            kstclient->connectToHost(KSTserverName, KSTserverPort.toUShort());
        }
    }
}

void KSTMainWindow::connected()
{
    trace("connection to ON4KST established");
    ui->includeMeCb->setText(tr("Including %1").arg(myCallsign.getFullCall()));
    ui->includeMeCb->setChecked(true);

    kstMeepFilterModel.setMyCsFilterString(myCallsign.getFullCall());
    ui->toMeFilter->clear();

    ui->connectButton->setText(tr("Disconnect"));

    setMeepFilters();
}


void KSTMainWindow::clearConnection()
{
    ui->includeMeCb->setChecked(false);
    ui->includeMeCb->setText(QString());
    kstMeepFilterModel.setMyCsFilterString("");
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
int KSTMainWindow::calcDistance(const Callsign &c)
{
    if (!c.getFullCall().isEmpty())
    {
        QSharedPointer<KstUser> user = getUser(KstUser(c, activeChat));
        if (user)
        {
            return user->distance;
        }
    }
    return -1;
}
void KSTMainWindow::onReadyRead()
{
    QByteArray b = kstclient->readAll();
    QString msg = QString(b);

    QChar addTraceChar = traceEndChar;
    QString traceMsg = msg.remove("\r");
    if (traceMsg.endsWith("\n"))
    {
        traceMsg.chop(1);
        addTraceChar = traceEndnlChar;
    }
    trace(QString("KSTMainWindow::messageRx: %1").arg(traceStart + traceMsg + traceEnd + addTraceChar));

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

int KSTMainWindow::getMaxDistance() const
{
    return maxDistance;
}

bool KSTMainWindow::getASActive() const
{
    bool ret = ui->ASActivecb->isChecked();
    return ret;
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

Callsign KSTMainWindow::getMyCallsign() const
{
    return myCallsign;
}

QString KSTMainWindow::getMyLoc() const
{
    if (myLoc.isEmpty())
        return recLoc;
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
    if (!KSTImportFile.isOpen())
    {
        kstclient->write((msg + "\r\n").toLocal8Bit());
        trace("Send to KST: " + msg);
    }
}
void KSTMainWindow::checkAwayButton()
{
    QSharedPointer<KstUser> user = getUser(KstUser(myCallsign, activeChat));
    if (user)
    {
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

void KSTMainWindow::addMessage(QSharedPointer<KstMessageLine> kst)
{
    // Add to the counts of messages per user
    kstMessageModel.appendLastRow(kst);
    Callsign userName = kst->call;
    QSharedPointer<KstUser> user = getUser(KstUser(userName, kst->chat));
    if (user)
    {
        user->messageCount++;
    }
    else
    {
        QString user = userName.getFullCall();
        if (!user.isEmpty() && user != "SERVER")
        {
            // NB we can get messages before we see the UA5 announcing them
            trace(QString("User %1 not found in chat %2").arg(user).arg(kst->chat));
        }
    }
}
void KSTMainWindow::checkUserMessages(QSharedPointer<KstUser> user)
{
    // scan messages for to/from this user
    int ucount = 0;
    QString userName = user->call.getFullCall();
    for (const auto &m: QASCONST(*messageVector))
    {
        int chat = user->chat;
        if (chat == m->chat)
        {
            QString to = m->call.getFullCall();
            //QString other = m->otherCall.getFullCall();
            if (to == userName /*|| other == userName*/)
            {
                ucount++;
            }
        }
    }
    user->messageCount = ucount;
}

void KSTMainWindow::playMeepSound()
{
    if (meepPlaySound && FileExists(meepSoundFile))
    {
        SoundPlayer::playSound(meepSoundFile, meepVolume);

    }
    else
    {
        trace(QString("%1 doesn't exist").arg(meepSoundFile));

    }
}
void KSTMainWindow::analyseKstMessage(QString atj) {
    //    18:58:18.640 messageRx: 1858Z ES4RM Sergei> (OH3DP) i am on 1558
    //    18:58:44.037 messageRx: 1858Z OH3DP Hannu 2m, 70, 23> 1000

    atj = atj.trimmed();

    if (!kstconnected && atj.contains(" login ")) {
        //        11:28:01.769 Client read :
        //        LOGINC|G0GJV|62rosehill|2|KST2Me 1.2.0.0 11:28:01.769 Client read
        //        : |20|20|1|0|0|

        QString loginMessage =
            "LOGINC|" + myCallsign.getFullCall() + "|" + password + "|" +
                               QString::number(kstChatSelection[0]) + "|" +
                               "Minos 0.0.0.999" // client software version
                               + "|20"           // past messages
                               + "|0"            // past DX/map messages
                               + "|1" // users list/update flags - If the users list/update flags = 0,
                               // no Uxx frames will be sent (even after the login)
                               + "|0" // last Unix timestamp for messages
                               + "|0" // last Unix timestamp for dx/map
                               + "|";

        sendKST(loginMessage);

        kstLoggedIn.append(kstChatSelection[0]);

        // Optional allowed frames between LOGINC and SDONE are SDXQ, SMAQ, RDXQ and
        // RMAQ.
        //  we don't need them - yet

        return;
    }

    QStringList sl;
    sl = atj.split("|");

    if (sl[0] == "LOGSTAT") {
        // user config
        //        LOGSTATS if LOGINC:
        //        LOGSTAT|100|2|20040703a|239E038F12E685FB75C6C03A79A1DE8A|11|Alain/telnet|Stiévenart|JO20HI|on4kst@skynet.be|
        //        LOGSTAT|100|chat id|client software version|session
        //        key|config|first name|last name|locator|email|
        if (sl[1] == "100") {
            kstconnected = true;
            QString sdone = "SDONE|" + QString::number(kstChatSelection[0]) + "|";
            sendKST(sdone);
            recName = sl[6];
            recLoc = sl[8];
            kstCallModel.locator = recLoc;  //set from LOGSTAT after LOGINC
        } else {
            // messageRx: LOGSTAT|101|Unknown user "XX0GJV".|
            // messageRx: LOGSTAT|114|Wrong password!|
            kstclient->disconnectFromHost();

            //            clearConnection();
            mShowMessage(sl[2], this);
            doConfiguration(true);
        }

    } else if (sl[0] == "CR") {
        // message frame at login
        // CR|chat id|Unix time|callsign|firstname|destination|msg|highlight|

        QSharedPointer<KstMessageLine> kst(new KstMessageLine());

        kst->sequence = messageSequence++;

        kst->chat = sl[1].toInt();
        kst->fullLine = atj;

        QString unixTime = sl[2];
        kst->dtg = QDateTime::fromMSecsSinceEpoch(unixTime.toLongLong() * 1000);

        kst->call.setFullCall(sl[3]);
        kst->distance = -2;
        kst->name = sl[4];
        QString destination = sl[5];
        kst->message = sl[6];
        kst->otherCall.setFullCall(sl[7]);
        if (sl[7] == "0") {
            kst->otherCall = Callsign();
            if (destination != "0") {
                kst->otherCall.setFullCall(destination);
            }
        }
        kst->otherDistance = -2;
        bool found = false;
        for (auto const &msg : QASCONST(*messageVector)) {
            if (kst->fullLine == msg->fullLine) {
                found = true;
                break;
            }
        }
        if (!found) {
            addMessage(kst);
        }

    } else if (sl[0] == "CE") {
        // end of CR frames
        scrollMesToBottom();

        scrollMeepToBotton();
    } else if (sl[0] == "CH") {
        // message frame after login
        // CH|chat id|date|callsign|firstname|destination|msg|highlight|

        QSharedPointer<KstMessageLine> kst(new KstMessageLine());

        kst->sequence = messageSequence++;

        kst->chat = sl[1].toInt();
        kst->fullLine = atj;

        QString unixTime = sl[2];
        kst->dtg = QDateTime::fromMSecsSinceEpoch(unixTime.toLongLong() * 1000);

        kst->call.setFullCall(sl[3]);
        kst->distance = calcDistance(kst->call);
        kst->name = sl[4];
        QString destination = sl[5];
        kst->message = sl[6];
        kst->otherCall.setFullCall(sl[7]);
        if (sl[7] == "0") {
            kst->otherCall = Callsign();
            if (destination != "0") {
                kst->otherCall.setFullCall(destination);
            }
        }
        kst->otherDistance = calcDistance(kst->otherCall);

        addMessage(kst);

        scrollMesToBottom();
        scrollMeepToBotton();

        if (kst->otherCall == myCallsign) {
            QApplication::alert(this, 10000); // 10 sec alert
            if (meepNotifyLogger) {
                QStringList rList = routerList();
                for (const auto &router : QASCONST(rList)) {

                    RPCGeneralClient rpc(rpcConstants::KSTTransferMeep);
                    QSharedPointer<RPCParam> st(new RPCParamStruct);
                    st->addMember(kst->call.getFullCall(),
                                  rpcConstants::KSTTransferCall);
                    st->addMember(kst->message, rpcConstants::KSTTransferMeep);
                    rpc.getCallArgs()->addParam(st);
                    rpc.queueCall(router);
                }
            }
            playMeepSound();
        }

    } else if (sl[0] == "DL") {
        // DX frames for the DX window
    } else if (sl[0] == "DE") {
        // end of DX frames
    } else if (sl[0] == "ML") {
        // DX frames for the map window
    } else if (sl[0] == "ME") {
        // end of ML frames
    } else if (sl[0] == "DM") {
        // DX frames for the DX and the MAP windows
    } else if (sl[0] == "DF") {
        // end of DM frames
    } else if (sl[0] == "LOC") {
        // Locator update
        //  LOC|Unix time|callsign|locator|

        int row = 0;
        for (auto const &l : QASCONST(*callVector)) {
            if (l->call.getFullCall() == sl[2]) {
                l->loc = sl[3];
                l->distance = -1; // force recalc
                emit kstCallModel.dataChanged(
                    kstCallModel.index(row, 0),
                    kstCallModel.index(row, kstCallModel.columnCount() - 1));
            }
            row++;
        }
    }

    else if (sl[0] == "UA0") {
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
        test->call.setFullCall(sl[2], true); // allow e.g. G0GJV-7
        test->name = sl[3];
        test->loc = sl[4];

        QString state = sl[5];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        if (!callMap.contains(*test.data())) {
            QSharedPointer<CountrySynonym> syn =
                MultLists::getMultLists()->searchCountrySynonym(
                test->call.locCtryPrefix);
            if (syn) {
                test->prefix = syn->getBasePrefix();
                test->country = syn->getRealName();
                test->dxcc = syn->getCountry()->getBasePrefix();
                test->distance = -1; // force recalc
            }

            int row = (std::lower_bound(callVector->begin(), callVector->end(), test,
                                        KstUserCompare) -
                       callVector->begin());
            callVector->insert(row, test);
            callVectorChanged = true;
            callMap[*test.data()] = test;
            QSharedPointer<KstUser> user = getUser(*test.data());
            checkUserMessages(user);
        }
    }

    else if (sl[0] == "UE") {
        //    Users statistics/end of users frames
        //    UE|chat id|nb registered users|
        //    UE|2|4777|

        kstCallModel.setCallVector(callVector);
        kstMessageModel.setChatVector(messageVector);

        kstCallFilterModel.invalidate();
        kstMessageFilterModel.invalidate();

        scrollMesToBottom();

        scrollMeepToBotton();

        if (!firstName.isEmpty() && recName != firstName) {
            // setnam isn't valid, and not likely to be
            //            QString msg = "MSG|" + QString::number(activeChat) +
            //            "|0|/SETNAM " + firstName + "|0|"; sendKST(msg);

            kstclient->disconnectFromHost();
            kstLoggedIn.clear();

            kstCallModel.reset();
            callVector->clear();
            callMap.clear();
            kstMessageModel.reset();
            messageVector->clear();

            ChangeName cn;
            cn.newName = firstName;
            cn.myCallsign = myCallsign;
            cn.password = password;
            cn.kstChatSelection = kstChatSelection[0];

            cn.exec();

            reconnect();
        }
        if (!myLoc.isEmpty() && myLoc != recLoc) {
            // /SETLOC locator    To set his own locator.
            QString msg =
                "MSG|" + QString::number(activeChat) + "|0|/SETLOC " + myLoc + "|0|";
            sendKST(msg);
            recLoc = myLoc;
            kstCallModel.locator = myLoc;   // change loc with /SETLOC
            for (auto const &l : QASCONST(*callVector)) {
                l->distance = -1;
            }
            kstCallFilterModel.invalidate();
        }

    }

    else if (sl[0] == "US4") {
        //    User state (here/not here/more than 5 min logged)
        //    US4|chat id|callsign|state|
        //    US4|2|OH2JXA|state|

        QSharedPointer<KstUser> test(new KstUser());
        test->chat = sl[1].toInt();
        test->call.setFullCall(sl[2]);
        QString state = sl[3];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        QVector<QSharedPointer<KstUser>>::const_iterator l = std::lower_bound(
            callVector->constBegin(), callVector->constEnd(), test, KstUserCompare);
        if (l != callVector->constEnd() && l->data()->call == test->call &&
            l->data()->chat == test->chat) {
            // as it should be...
            l->data()->away = test->away;
            l->data()->recent = test->recent;
            int row = l - callVector->constBegin();
            emit kstCallModel.dataChanged(
                kstCallModel.index(row, 0),
                kstCallModel.index(row, kstCallModel.columnCount() - 1));
        }
    }

    else if (sl[0] == "UM3") {
        //    User already logged
        //    UM3|chat id|callsign|firstname|locator|state|
        //    UM3|2|OZ2M|Bo|JO65FR|2|

        QSharedPointer<KstUser> test(new KstUser());
        test->chat = sl[1].toInt();
        test->call.setFullCall(sl[2]);
        test->name = sl[3];
        test->loc = sl[4];
        QString state = sl[5];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        QVector<QSharedPointer<KstUser>>::const_iterator l = std::lower_bound(
            callVector->constBegin(), callVector->constEnd(), test, KstUserCompare);
        if (l != callVector->constEnd() && l->data()->call == test->call &&
            l->data()->chat == test->chat) {
            // as it should be...
            l->data()->name = test->name;
            l->data()->loc = test->loc;
            l->data()->away = test->away;
            l->data()->recent = test->recent;
            l->data()->distance = -1; // force recalc
            int row = l - callVector->constBegin();
            emit kstCallModel.dataChanged(
                kstCallModel.index(row, 0),
                kstCallModel.index(row, kstCallModel.columnCount() - 1));
        }

    }

    else if (sl[0] == "UR6") {
        //    User disconnected (to remove)
        //    UR6|chat id|callsign|
        //    UR6|2|RA3MR/3|
        QSharedPointer<KstUser> test(new KstUser());
        test->chat = sl[1].toInt();
        test->call.setFullCall(sl[2]);

        QVector<QSharedPointer<KstUser>>::const_iterator l = std::lower_bound(
            callVector->constBegin(), callVector->constEnd(), test, KstUserCompare);
        if (l != callVector->constEnd() && l->data()->call == test->call &&
            l->data()->chat == test->chat) {
            // as it should be...

            // if we remove the last row then the call model
            // is one short as we have already removed it from the vector
            int row = l - callVector->constBegin();

            kstCallModel.removeRow(row);
            callVectorChanged = true;
            callMap.remove(*test.data());
        }
    }

    else if (sl[0] == "UA5") {
        //    UA5 user connected (to add)
        //    UA5|chat id|callsign|firstname|locator|state|
        //    UA5|2|PA0GUS|GUUS|JO23TA|2|

        QSharedPointer<KstUser> test(new KstUser());
        test->chat = sl[1].toInt();
        test->call.setFullCall(sl[2]);
        test->name = sl[3];
        test->loc = sl[4];
        test->distance = -1; // force recalc
        QString state = sl[5];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        if (!callMap.contains(*test.data())) {
            QSharedPointer<CountrySynonym> syn =
                MultLists::getMultLists()->searchCountrySynonym(
                test->call.locCtryPrefix);
            if (syn) {
                test->prefix = syn->getBasePrefix();
                test->country = syn->getRealName();
                test->dxcc = test->call.locCtryPrefix;
            }
            int row = (std::lower_bound(callVector->begin(), callVector->end(), test,
                                        KstUserCompare) -
                       callVector->begin());
            kstCallModel.insertRow(row, test);
            callVectorChanged = true;

            // NB callMap and callVector are different!
            callMap[*test.data()] = test; // should already have happened
            QSharedPointer<KstUser> user = getUser(*test.data());
            checkUserMessages(user);
        }

    } else if (sl[0] == "CK") {
        doLoginChanges();
        // link check
        sendKST("\r\n");
        std::sort(messageVector->begin(), messageVector->end(), &compMessages);
        emit kstMessageModel.dataChanged(
            kstMessageModel.index(0, 0),
            kstMessageModel.index(kstMessageModel.rowCount() - 1,
                                  kstMessageModel.columnCount() - 1));
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
    kstMessageFilterModel.setFilterString(arg1.toUpper());
    scrollMesToBottom();
}

void KSTMainWindow::on_CSFilter_textChanged(const QString &arg1)
{
    kstCallFilterModel.setFilterString(arg1.toUpper());
}

void KSTMainWindow::on_kstSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    RegSettings settings;
    QByteArray state = ui->kstSplitter->saveState();
    settings.getSettings().setValue("kstSplitterState" , state);
}

void KSTMainWindow::on_msgSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    RegSettings settings;
    QByteArray state = ui->msgSplitter->saveState();
    settings.getSettings().setValue("msgSplitterState" , state);
}

void KSTMainWindow::on_callSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    RegSettings settings;
    QByteArray state = ui->callSplitter->saveState();
    settings.getSettings().setValue("callSplitterState" , state);
}

void KSTMainWindow::on_sectionResized(int, int, int)
{
    RegSettings settings;
    QByteArray state;

    state = ui->CSTable->horizontalHeader()->saveState();
    settings.getSettings().setValue("CSTable/state", state);

    state = ui->messageTable->horizontalHeader()->saveState();
    settings.getSettings().setValue("messageTable/state", state);

    state = ui->meepTable->horizontalHeader()->saveState();
    settings.getSettings().setValue("meepTable/state", state);
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
        QString l = QString("%1\n%2 at %3\n")
                        .arg(user->lastCalcTime, user->fromCall, user->fromLoc)
                    + QString("to %1 at %2")
                        .arg(user->toCall, user->toLoc);

        ui->planeslabel->setText(l);
    }
    kstPlanesModel.setPlanesVector(user->planes);

}
void KSTMainWindow::onCSTableSelectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    QModelIndexList mil = ui->CSTable->selectionModel()->selectedRows();

    QString mselstring;
    for(auto &mi: mil)
    {
        QModelIndex m = kstCallFilterModel.mapToSource(mi);
        int r = m.row();
        if (r >= 0 && r < callVector->size())
        {
            QSharedPointer<KstUser> user = callVector->at(r);
            if (!mselstring.isEmpty())
            {
                mselstring += " ";
            }
            mselstring += user->call.getFullCall();
        }
    }
    ui->messageFilter->setText(mselstring);
    if (mil.count() == 1)
    {
        QModelIndex m = kstCallFilterModel.mapToSource(mil[0]);
        int r = m.row();
        if (r >= 0 && r < callVector->size())
        {
            QSharedPointer<KstUser> user = callVector->at(r);

            if (!ui->noSetCallcb->isChecked())
            {
                // messages

                setNameFromCall(user->call);

                ui->callEdit->setText(user->call.getFullCall());
                ui->msgEdit->setFocus();
                setActive(user->chat);
                ui->messageChatFilter->setCurrentIndex(user->chat);
            }
            // Planes
            showPlanes(user);
            setDefaultButton(ui->loggerXferButton);
        }
    }
    else if (mil.count() == 0)
    {
        on_clearMessageFilter_clicked();
    }
}
bool KSTMainWindow::doConfiguration(bool showForm)
{
    KSTConfigure conf;

    conf.hostname = KSTserverName;
    conf.port = KSTserverPort;
    conf.username = myCallsign.getFullCall();
    conf.password = password;
    conf.autoConnect = autoConnect;
    conf.locator = myLoc;
    conf.maxDistance = maxDistance;
    conf.firstName = firstName;
    conf.meepNotifyLogger = meepNotifyLogger;
    conf.meepPlaySound = meepPlaySound;
    conf.meepSoundFile = meepSoundFile;
    conf.meepVolume = meepVolume;

    conf.ASActive = ASActive;
    conf.ASActiveBand = ASActiveBand;
    conf.ASServerName = ASServerName;
    conf.ASMyName = ASMyName;
    conf.ASMinDistance = ASMinDistance;
    conf.ASMaxDistance = ASMaxDistance;
    conf.ASPort = ASPort;
    conf.ASTimeout = ASTimeout;

    conf.tnservername = TNServerName;
    conf.tnserverport = TNServerPort;


    int ret = QDialog::Accepted;
    if (showForm)
    {
        ret = conf.exec();
    }
    if (ret == QDialog::Accepted)
    {
        KSTserverName = conf.hostname.trimmed();
        KSTserverPort = conf.port.trimmed();
        TNServerName = conf.tnservername.trimmed();
        TNServerPort = conf.tnserverport.trimmed();
        myCallsign.setFullCall(conf.username);
        password = conf.password.trimmed();
        autoConnect = conf.autoConnect;
        meepNotifyLogger = conf.meepNotifyLogger;
        meepPlaySound = conf.meepPlaySound;
        meepSoundFile = conf.meepSoundFile;
        meepVolume = conf.meepVolume;

        myLoc = conf.locator.trimmed();
        maxDistance = conf.maxDistance;
        firstName = conf.firstName.trimmed();
        ASActive = conf.ASActive;
        ASActiveBand = conf.ASActiveBand;
        ASServerName = conf.ASServerName.trimmed();
        ASMyName = conf.ASMyName.trimmed();
        ASMinDistance = conf.ASMinDistance;
        ASMaxDistance = conf.ASMaxDistance;
        ASPort = conf.ASPort;
        ASTimeout = conf.ASTimeout;

        QSettings settings(iniName, QSettings::IniFormat);

        settings.setValue("hostname", KSTserverName);
        settings.setValue("port", KSTserverPort);
        settings.setValue("tnservername", TNServerName);
        settings.setValue("tnserverport", TNServerPort);
        settings.setValue("username", myCallsign.getFullCall());
        settings.setValue("password", password);
        settings.setValue("autoConnect", autoConnect);
        settings.setValue("locator", myLoc);
        settings.setValue("maxDistance", maxDistance);
        settings.setValue("firstName", firstName);

        settings.setValue("meepNotifyLogger", meepNotifyLogger);
        settings.setValue("meepPlaySound", meepPlaySound);
        settings.setValue("meepSoundFile", meepSoundFile);
        settings.setValue("meepVolume", meepVolume);

        settings.setValue("ASActive", ASActive);
        settings.setValue("ASServerName", ASServerName);
        settings.setValue("ASMyName", ASMyName);
        settings.setValue("ASActiveBand", ASActiveBand);
        settings.setValue("ASMinDistance", ASMinDistance);
        settings.setValue("ASMaxDistance", ASMaxDistance);
        settings.setValue("ASPort", ASPort);
        settings.setValue("ASTimeout", ASTimeout);

        if (showForm)
        {
            for (auto const &l: QASCONST(*callVector))
            {
                l->distance = -1;
            }
            kstCallFilterModel.invalidate();
            kstMessageFilterModel.invalidate();

            if (getASActive())
            {
                asl.reset();

                asl = QSharedPointer<AirScoutLink>(new AirScoutLink());
                connect(asl.data(), &AirScoutLink::acChanged, this, &KSTMainWindow::acChanged);
            }
            if  (kstconnected)
            {
                reconnect();
            }
        }
        return true;
    }
    return false;
}

void KSTMainWindow::on_configureButton_clicked()
{
    doConfiguration(true);
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

    delayedAction(this, [=]()
    {
        // NB a lambda function
        connectToHost();
    }
    );
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

void KSTMainWindow::setNameFromCall(const Callsign &call)
{
    QSharedPointer<KstUser> user = getUser(KstUser(call, activeChat));

    if (user)
    {
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
                    delayedAction(this, [=]()
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
                    }
                    , 1000 * j
                    );
                    j++;

                }
                if (loggedin && !loginWanted)
                {
                    detached = true;
                    delayedAction(this, [=]()
                    {
                        // NB a lambda function
                        // detach chat
                        QString detachMessage = QString("DCHAT")
                                + "|" + QString::number(i + 1)
                                + "|";
                        sendKST(detachMessage);
                    }
                    , 1000 * j
                    );
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
            callMap.clear();
        }
    }
    else
    {
        if (kstChatSelection.count())
        {
            kstCallModel.locator = myLoc;   // set from config
            if (autoConnect)
            {
                if (kstclient->state() != QAbstractSocket::ConnectedState
                   && kstclient->state() != QAbstractSocket::ConnectingState
                   && kstclient->state() != QAbstractSocket::ClosingState
                   && kstclient->state() != QAbstractSocket::HostLookupState)
                {
                    kstclient->connectToHost(KSTserverName, KSTserverPort.toUShort());
                }
            }
        }

    }
    if (detached)
    {
        QSharedPointer<QVector<QSharedPointer<KstUser> > > newCallVector(new QVector<QSharedPointer<KstUser> >);
        for(auto const &i: QASCONST(*callVector))
        {
            int c = i->chat;
            if (kstLoggedIn.contains(c))
            {
                newCallVector->push_back(i);
            }
        }
        kstCallModel.setCallVector(newCallVector);
        callVector = newCallVector;
        callMap.clear();
        for(auto const &i: QASCONST(*callVector))
        {
            callMap[*i.data()] = i;
        }
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
    Callsign call = line->call;
    if (myCallsign == call)
    {
        call = line->otherCall;
    }

    setNameFromCall(call);

    ui->callEdit->setText(call.getFullCall());
    ui->msgEdit->setFocus();
    setActive(line->chat);

    QString t = line->message;
    ui->bodyLabel->setText(t);
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
    if (index.isValid())
    {
        QModelIndex sourceIndex = kstMeepFilterModel.mapToSource(index);
        QSharedPointer<KstMessageLine> line = messageVector->at(sourceIndex.row());
        Callsign call = line->call;
        if (call == myCallsign)
        {
            call = line->otherCall;
        }
        setNameFromCall(call);
        ui->callEdit->setText(call.getFullCall());
        ui->msgEdit->setFocus();
        setActive(line->chat);
    }
}

void KSTMainWindow::on_clearButton_clicked()
{
    kstMessageModel.reset();
    for (auto const &l: QASCONST(*callVector))
    {
        l->messageCount = 0;
    }
    kstCallFilterModel.invalidate();
}
bool KSTMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->messageTable)
    {
       if (event->type() == QEvent::Enter)
       {
           QModelIndex mesIndex = kstMessageFilterModel.index(kstMessageModel.rowCount() - 1, 0);
           mouseInMessages = true;
           kstMessageFilterModel.setMousePausePoint(mesIndex.row());
           ui->messageTable->update();
           ui->pauseLabel->setText(HtmlFontColour(Qt::red) + tr("Message updates paused"));
       }
       else if (event->type() == QEvent::Leave)
       {
           mouseInMessages = false;
           kstMessageFilterModel.setMousePausePoint(-1);
           ui->messageTable->update();
           ui->pauseLabel->clear();
           scrollMesToBottom();
       }
    }
    else
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
                else if (obj == ui->toMeFilter)
                {
                    ui->toMeFilter->clear();
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
                else if (ui->loggerXferButton->isDefault())
                {
                    ui->loggerXferButton->click();
                }
            }
        }
    }

   return false;    // pass the event on
}
void KSTMainWindow::on_sortIndicatorChanged(int /*logicalIndex*/, Qt::SortOrder /*order*/)
{
    on_sectionResized(0, 0, 0);
}

void KSTMainWindow::setDefaultButton(QPushButton *d)
{
    ui->loggerXferButton->setDefault(false);
    if (d)
    {
        ui->meepButton->setDefault(false);
        ui->genmsgButton->setDefault(false);

        d->setDefault(true);
    }
    else
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
void KSTMainWindow::on_callEdit_textChanged(const QString & /*arg1*/)
{
    setDefaultButton(nullptr);
}
void KSTMainWindow::on_msgEdit_textChanged(const QString &/*arg1*/)
{
    setDefaultButton(nullptr);
}

void KSTMainWindow::on_clearMessageButton_clicked()
{
    ui->callEdit->clear();
    ui->msgEdit->clear();
    ui->messageFilter->setFocus();
}

void KSTMainWindow::on_awayButton_clicked()
{
    QSharedPointer<KstUser> user = getUser(KstUser(myCallsign, activeChat));
    if (user)
    {
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
    QSettings settings(iniName, QSettings::IniFormat);
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
    QSettings settings(iniName, QSettings::IniFormat);
    settings.setValue("active", QString::number(activeChat));
    checkAwayButton();
}
void KSTMainWindow::on_messageChatFilter_currentIndexChanged(int index)
{
    if (started)
    {
        messageChatFilter = index;

        kstMessageFilterModel.setChatFilter(messageChatFilter);

        QSettings settings(iniName, QSettings::IniFormat);
        settings.setValue("messageChatFilter", QString::number(messageChatFilter));
    }
}
void KSTMainWindow::on_CSChatFilter_currentIndexChanged(int index)
{
    if (started)
    {
        CSChatFilter = index;

        kstCallFilterModel.setChatFilter(CSChatFilter);

        QSettings settings(iniName, QSettings::IniFormat);
        settings.setValue("CSChatFilter", QString::number(CSChatFilter));
    }
}

void KSTMainWindow::on_clearMessageFilter_clicked()
{
    ui->messageChatFilter->setCurrentIndex(0);
    ui->messageFilter->clear();

    kstMeepFilterModel.invalidate();    // try to get rid of the colouring in the meep table

    scrollMesToBottom();
}

void KSTMainWindow::on_clearUserFilter_clicked()
{
    ui->CSChatFilter->setCurrentIndex(0);
    ui->CSFilter->clear();
    ui->CSTable->clearSelection();
    on_clearMessageFilter_clicked();
    ui->CSFilter->setFocus();
    on_clearMessageButton_clicked();
}

void KSTMainWindow::on_asBandCombo_currentIndexChanged(int band)
{
    if (started)
    {
        if (asl && getASActive())
        {
            for (auto const &kstuser: QASCONST(*callVector))
            {
                kstuser->planes.clear();
                kstuser->planeResponseSeen = false;
            }
            callVectorChanged = true;
            emit kstCallModel.dataChanged(kstCallModel.index(0, ecscAirscout), kstCallModel.index(callVector->size(), ecscAirscout));

            userCallTimerTimer();
        }
        QSettings settings(iniName, QSettings::IniFormat);

        settings.setValue("ASActiveBand", band);
    }
}

void KSTMainWindow::on_ASActivecb_stateChanged(int state)
{
    if (started)
    {
        if (asl && getASActive())
        {
            for (auto const &kstuser: QASCONST(*callVector))
            {
                kstuser->planes.clear();
                kstuser->planeResponseSeen = false;
            }
            callVectorChanged = true;
            emit kstCallModel.dataChanged(kstCallModel.index(0, ecscAirscout), kstCallModel.index(callVector->size(), ecscAirscout));

            asl->clearWatchList();
            userCallTimerTimer();
        }

        QSettings settings(iniName, QSettings::IniFormat);

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
QSharedPointer<KstUser> KSTMainWindow::getUser(const KstUser &test)
{
    if (callMap.contains(test))
    {
        return callMap[test];
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

    QSharedPointer<KstUser> user = getUser(KstUser(line->call, activeChat));
    QSharedPointer<KstUser> other = getUser(KstUser(line->otherCall, activeChat));

    if (user && other)
    {
        asl->asShowPath(user, other);
    }
}

void KSTMainWindow::on_maxDistanceEdit_editingFinished()
{
    maxDistance = ui->maxDistanceEdit->text().toInt();
    QSettings settings(iniName, QSettings::IniFormat);
    settings.setValue("maxDistance", maxDistance);

    kstCallFilterModel.invalidate();
    kstMessageFilterModel.invalidate();
}

void KSTMainWindow::on_showReadcb_stateChanged(int /*arg1*/)
{
    kstMeepFilterModel.setShowRead(ui->showReadcb->isChecked());
}

void KSTMainWindow::on_stringRb_clicked()
{
    kstCallFilterModel.setStringDXCC(ui->countryRb->isChecked());
    kstCallFilterModel.invalidate();
}


void KSTMainWindow::on_countryRb_clicked()
{
    kstCallFilterModel.setStringDXCC(ui->countryRb->isChecked());
    kstCallFilterModel.invalidate();
}


void KSTMainWindow::on_clearSelectedMessage_clicked()
{
    ui->bodyLabel->clear();
}

void KSTMainWindow::scrollMesToBottom()
{
    if (!mouseInMessages)
    {
        delayedAction(this, [=]()
        {
            QModelIndex mesIndex = kstMessageFilterModel.index(kstMessageFilterModel.rowCount() - 1, 0);
            if (mesIndex.isValid())
            {
                ui->messageTable->scrollTo(mesIndex, QAbstractItemView::PositionAtBottom);
            }
        });
    }
}

void KSTMainWindow::scrollMeepToBotton()
{
    delayedAction(this, [=]()
    {
        QModelIndex meepIndex = kstMeepFilterModel.index(kstMeepFilterModel.rowCount() - 1, 0);
        if (meepIndex.isValid())
        {
            ui->meepTable->scrollTo(meepIndex, QAbstractItemView::PositionAtBottom);
        }
    });
}

void KSTMainWindow::setMeepFilters()
{
    if (ui->includeMeCb->isChecked())
    {
        kstMeepFilterModel.setMyCsFilterString(myCallsign.getFullCall());
    }
    else
    {
        kstMeepFilterModel.setMyCsFilterString(QString());
    }
    kstMeepFilterModel.setFilterString(ui->toMeFilter->text().trimmed());

    scrollMeepToBotton();
}

void KSTMainWindow::on_includeMeCb_stateChanged(int /*arg1*/)
{
    setMeepFilters();
}


void KSTMainWindow::on_toMeFilter_textChanged(const QString &/*arg1*/)
{
    setMeepFilters();
}


void KSTMainWindow::on_clearMeepFiltersButton_clicked()
{
    ui->toMeFilter->clear();
    ui->includeMeCb->setChecked(true);
    setMeepFilters();
}


void KSTMainWindow::on_logsButton_clicked()
{
    ml->show();
}


QStringList KSTMainWindow::routerList()
{
    QStringList routerList;

    for ( auto s = RemoteLogs::getRemoteLogs()->stationList.begin();
         s != RemoteLogs::getRemoteLogs()->stationList.end();
         s++ )
    {
        routerList.append((*s)->name);
    }
    routerList.sort();
    routerList.removeDuplicates();

    return routerList;
}

void KSTMainWindow::on_loggerXferButton_clicked()
{
    QModelIndexList mil = ui->CSTable->selectionModel()->selectedRows();

    if (mil.size() == 1)
    {

        auto &mi = mil[0];
        QModelIndex m = kstCallFilterModel.mapToSource(mi);
        int r = m.row();
        QSharedPointer<KstUser> user = callVector->at(r);
        QString call = user->call.getFullCall();
        QString loc = user->loc;

        int hyphen = call.indexOf("-");
        if (hyphen > 0)
        {
            call = call.left(hyphen);
        }

        QStringList rList = routerList();
        for(const auto &router: QASCONST(rList))
        {
            RPCGeneralClient rpc(rpcConstants::KSTTransfer);
            QSharedPointer<RPCParam>st(new RPCParamStruct);
            st->addMember( call, rpcConstants::KSTTransferCall );
            st->addMember( loc, rpcConstants::KSTTransferLocator );
            rpc.getCallArgs() ->addParam( st );
            rpc.queueCall( router );
        }
    }
}


void KSTMainWindow::on_awayCallscb_stateChanged(int)
{
    if (started)
    {
        kstCallFilterModel.setAwayFilter(ui->awayCallscb->isChecked());
    }
}


void KSTMainWindow::on_inactiveCallscb_stateChanged(int)
{
    if (started)
    {
        kstCallFilterModel.setInactiveFilter(ui->inactiveCallscb->isChecked());
    }
}



void KSTMainWindow::on_KSTTestButton_clicked()
{
    if (KSTexpFile)
    {
        KSTexpFile->close();
    }

    if (!KSTImportFile.isOpen())
    {
        QString dpath = "mqtKSTClient_*";

        QString InitialDir = GetCurrentDir() + "/TraceLog/" + dpath;

        QString Filter = tr("KST Test data Files") + " (*.txt);;" +
                         tr("All Files") + " (*.*)" ;

        QString baseFileName = QFileDialog::getOpenFileName( this,
                                                            tr("KST Client log files Files"),
                                                            InitialDir,                   // opendir
                                                            Filter );

        if ( !baseFileName.isEmpty() )
        {
            KSTImportFile.setFileName(baseFileName);
            if (!KSTImportFile.open(QIODevice::ReadOnly))
                return;

            if (KSTImportFile.isOpen())
            {
                KSTImportStream.setDevice(&KSTImportFile);

                connect(&KSTTestTimer, &QTimer::timeout, this, &KSTMainWindow::testTimeout, Qt::UniqueConnection);

                msgbuf.clear();
                inTestMsg = false;

                KSTTestTimer.start(10);
            }
        }
    }

}

void KSTMainWindow::testTimeout()
{
    if ( KSTImportFile.isOpen())
    {
        if (KSTImportStream.atEnd())
        {
            KSTImportFile.close();
            return;
        }
        else
        {
            QString kline;

            kline = KSTImportStream.readLine();

            if (kline.isEmpty())
            {
                return;
            }

            //look for ***** in message, this message start
            // Then look for **** or ***!
            // If **** then add \n

            int sline = kline.indexOf(traceStart);
            if (sline >= 0)
            {
                kline = kline.mid(sline + 5);
                inTestMsg = true;
            }

            int eline = kline.indexOf(traceEnd + traceEndnlChar);
            int eline2 = kline.indexOf(traceEnd + traceEndChar);
            if (eline >= 0)
            {
                kline = kline.left(eline) + "\n";
                msgbuf.append(kline);
                inTestMsg = false;
            }
            else if (eline2 >= 0)
            {
                kline = kline.left(eline2);
                msgbuf.append(kline);
                inTestMsg = false;
            }
            if (inTestMsg)
            {
                msgbuf.append(kline + "\n");
            }


            // break into lines...

            int p = msgbuf.indexOf("\n");
            while (p >= 0)
            {
                QString m = msgbuf.left(p + 1);
                msgbuf = msgbuf.mid(p + 1);
                p = msgbuf.indexOf("\n");

                trace(m);
                analyseKstMessage(m);
            }
        }
    }
}

void KSTMainWindow::on_meepTable_doubleClicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        QModelIndex sourceIndex = kstMeepFilterModel.mapToSource(index);
        int row = sourceIndex.row();
        QSharedPointer<KstMessageLine> line = messageVector->at(row);
        Callsign call = line->call;
        if (call == myCallsign)
        {
            call = line->otherCall;
        }
        ui->CSFilter->setText(call.getFullCall());
        ui->CSTable->selectRow(0);
    }
}


void KSTMainWindow::on_messageTable_doubleClicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        QModelIndex sourceIndex = kstMessageFilterModel.mapToSource(index);
        int row = sourceIndex.row();
        QSharedPointer<KstMessageLine> line = messageVector->at(row);
        Callsign call = line->call;
        if (call == myCallsign)
        {
            call = line->otherCall;
        }
        ui->CSFilter->setText(call.getFullCall());
        ui->CSTable->selectRow(0);
    }
}

