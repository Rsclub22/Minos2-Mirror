#include <QHostInfo>
#include <QSettings>
#include <QKeyEvent>
#include <QFileDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QPushButton>

#include "QtUtils.h"
#include "SecondInstall.h"
#include "CommandReader.h"
#include "RPCCommandConstants.h"
#include "ScreenConfigManager.h"
#include "WindowsAppId.h"
#include "kstasactiveframe.h"
#include "kstpageframe.h"
#include "kstsendmeepframe.h"
#include "minossplitter.h"
#include "regsettings.h"
#include "AppStartup.h"
#include "MShowMessageDlg.h"
#include "MonitoredLog.h"
#include "callsign.h"
#include "fileutils.h"
#include "delayedaction.h"
#include "mults.h"
#include "MinosRPC.h"
#include "remotelogs.h"
#include "soundplayer.h"
#include "LogEvents.h"

#include "kstbuttonsframe.h"
#include "kstcallsframe.h"
#include "kstloginframe.h"
#include "kstmsgframe.h"
#include "kstplanesframe.h"
#include "ksttomeframe.h"
#include "kstconfigure.h"
#include "airscoutlink.h"
#include "changename.h"
#include "kstmonitoredlogs.h"

#include "kstmainwindow.h"
#include "ui_kstmainwindow.h"

QStringList services =
{
"50/70 MHz",
"144/432 MHz",
"Microwave",
"EME/JT65",
};

KSTMainWindow *mainWindow = nullptr;;
bool KSTMainWindow::inApplyScreenLayout = false;

const char *traceStart = "*-*_*";
const char *traceEnd= "_**_";
const char traceEndnlChar= '*';
const char traceEndChar= '!';
//==========================================================================================
void KSTMainWindow::getSettings(QSettings &settings)
{
    curScreenLayout = settings.value("screenLayout", "default").toString().trimmed();
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
    setActiveChat(settings.value("active", "0").toInt());
    autoConnect = settings.value("autoConnect", false).toBool();
    myLoc = settings.value("locator", "").toString();

    splitIcons = settings.value("splitIcons", false).toBool();
    lcf = settings.value("lcf", 100).toInt();

}

KSTMainWindow::KSTMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::KSTMainWindow)
{
    inStartup = true;

    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mainWindow = this;
    setWinAppId(this, SecondInstall::getOrgName() + QString(".MqtKstClient.SubScreen%1").arg(0) );

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
    {
        RegSettings rsettings;

        QByteArray geometry = rsettings.getSettings().value("geometry/Main").toByteArray();
        if (geometry.size() > 0)
            restoreGeometry(geometry);
    }

    configureAction = newAction(QT_TR_NOOP("Configure"), &kstPopup, &KSTMainWindow::do_configureButton_clicked);
    layoutAction = newAction(QT_TR_NOOP("Manage Layout"), &kstPopup, &KSTMainWindow::do_layoutButton_clicked);
    logsAction = newAction(QT_TR_NOOP("Show Logs"), &kstPopup, &KSTMainWindow::do_logsButton_clicked);
    clearMessagesAction = newAction(QT_TR_NOOP("Clear Messages"), &kstPopup, &KSTMainWindow::do_clearLogsButton_clicked);
    awayAction = newAction(QT_TR_NOOP("(set away)"), &kstPopup, &KSTMainWindow::do_awayButton_clicked);
    connectAction = newAction(QT_TR_NOOP("Connect"), &kstPopup, &KSTMainWindow::do_connectButton_clicked);

    testAction = newAction(QT_TR_NOOP("Test"), &kstPopup, &KSTMainWindow::do_KSTTestButton_clicked);
#ifdef Q_OS_WIN
    splitIconsAction = newCheckableAction(QT_TR_NOOP("Split Icons"), &kstPopup, &KSTMainWindow::do_splitIcons);

    splitIconsAction->setChecked(splitIcons);
#endif

    kstPopup.addSeparator();

    kstLoginAction = newAction(QT_TR_NOOP("Chats Login"), &kstPopup, &KSTMainWindow::do_dialog_clicked);
    kstLoginAction->setData(sctkLogins);

    kstCallsAction = newAction(QT_TR_NOOP("Active Calls"), &kstPopup, &KSTMainWindow::do_dialog_clicked);
    kstCallsAction->setData(sctkCallList);

    kstMsgAction = newAction(QT_TR_NOOP("Messages"), &kstPopup, &KSTMainWindow::do_dialog_clicked);
    kstMsgAction->setData(sctkMessageList);

    kstTomeAction = newAction(QT_TR_NOOP("Messages To Me"), &kstPopup, &KSTMainWindow::do_dialog_clicked);
    kstTomeAction->setData(sctkMeepList);

    kstSendMeepAction = newAction(QT_TR_NOOP("Send Meep"), &kstPopup, &KSTMainWindow::do_dialog_clicked);
    kstSendMeepAction->setData(sctkSendMeep);

    kstButtonsAction = newAction(QT_TR_NOOP("Buttons"), &kstPopup, &KSTMainWindow::do_dialog_clicked);
    kstButtonsAction->setData(sctkButtons);

    kstASActiveAction = newAction(QT_TR_NOOP("AirScout Activation"), &kstPopup, &KSTMainWindow::do_dialog_clicked);
    kstASActiveAction->setData(sctkASActive);

    kstPlanesAction = newAction(QT_TR_NOOP("AirScout"), &kstPopup, &KSTMainWindow::do_dialog_clicked);
    kstPlanesAction->setData(sctkAirScout);

    kstPopup.addSeparator();

    closeAction = newAction(QT_TR_NOOP("Close"), &kstPopup, &KSTMainWindow::do_closeButton_clicked);

    createScreenComponents();

    // this has to wait until the components are created
    asl = QSharedPointer<AirScoutLink>(new AirScoutLink());
    connect(asl.data(), &AirScoutLink::acChanged, kstPlanesFrame, &KSTPlanesFrame::acChanged);

    selectLayout(curScreenLayout);

    QStringList selections = chatSelection.split(":");

    kstLoginFrame->setLogins(selections);

    for (auto const &i: QASCONST(selections))
    {
        int s = i.toInt();
        if (s <= 4 && s > 0)
        {
            kstChatSelection.append(s);
        }
    }
    std::sort(kstChatSelection.begin(), kstChatSelection.end());

    kstLoginFrame->setActive(getActiveChat());

    kstLoginFrame->setLoginTexts(services);

    kstLoginFrame->checkActive();

    kstCallsFrame->setServices(services);

    kstMsgFrame->setServices(services);

    createCloseEvent();
    connect(&CloseTimer, &QTimer::timeout, this, &KSTMainWindow::CloseTimerTimer);
    CloseTimer.start(100);

    connect(&userCallTimer, &QTimer::timeout, this, &KSTMainWindow::userCallTimerTimer);
    userCallTimer.start(5000);

    kstMsgFrame->kstMessageModel.setChatVector(messageVector);
    kstMsgFrame->kstMessageFilterModel.setSourceModel(&kstMsgFrame->kstMessageModel);


    kstclient = new QTcpSocket(this);

    connect(kstclient, &QTcpSocket::connected, this, &KSTMainWindow::connected);
    connect(kstclient, &QTcpSocket::disconnected, this, &KSTMainWindow::disconnected);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(kstclient, &QTcpSocket::errorOccurred, this, &KSTMainWindow::connectionError);
#else
    connect(kstclient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
#endif
    connect(kstclient, &QTcpSocket::readyRead, this, &KSTMainWindow::onReadyRead);

    kstPlanesFrame->setVisible(ASActive);

    while ( myCallsign.getValRes() != CS_OK)
    {
        if (!doConfiguration(true))
            break;
    }
    started = true;

    if (autoConnect)
        doLoginChanges();

    kstLoginFrame->do_logincb_stateChanged();

    monitoredLogs = new KSTMonitoredLogs();
    connect(RemoteLogs::getRemoteLogs(), &RemoteLogs::newMonitoredLog, this, &KSTMainWindow::onNewLog);
    connect(RemoteLogs::getRemoteLogs(), &RemoteLogs::currentLogChanged, this, &KSTMainWindow::onLogChanged);

    connect(monitoredLogs, &KSTMonitoredLogs::logStarted, this, &KSTMainWindow::onLogStarted);
    connect(monitoredLogs, &KSTMonitoredLogs::logClosed, this, &KSTMainWindow::onLogClosed);

    on_FontChanged();
    connect(commandReader.data(), &CommandReader::fontChanged, this, &KSTMainWindow::on_FontChanged);
    inStartup = false;
}

KSTMainWindow::~KSTMainWindow()
{
    asl.reset();
    delete ui;
}
void KSTMainWindow::on_FontChanged()
{
    kstASActiveFrame->on_FontChanged();
    kstButtonsFrame->on_FontChanged();
    kstCallsFrame->on_FontChanged();
    kstLoginFrame->on_FontChanged();
    kstMsgFrame->on_FontChanged();
    kstPlanesFrame->on_FontChanged();
    kstSendMeepFrame->on_FontChanged();
    kstTomeFrame->on_FontChanged();
}
void KSTMainWindow::resizeEvent(QResizeEvent * event)
{
    if (!inStartup && !inClosedown)
    {
        RegSettings settings;
        settings.getSettings().setValue("geometry/Main", saveGeometry());
    }
    QWidget::resizeEvent(event);
}
void KSTMainWindow::moveEvent(QMoveEvent * event)
{
    if (!inStartup && !inClosedown)
    {
        RegSettings settings;
        settings.getSettings().setValue("geometry/Main", saveGeometry());
    }
    QWidget::moveEvent(event);
}
void KSTMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        if (!inStartup && !inClosedown)
        {
            RegSettings settings;
            settings.getSettings().setValue("geometry/Main", saveGeometry());
        }
    }
}


int KSTMainWindow::getLcf() const
{
    return lcf;
}

void KSTMainWindow::setLcf(int newLcf)
{
    lcf = newLcf;
}

void KSTMainWindow::closeEvent(QCloseEvent *event)
{
    inClosedown = true;
    trace("KSTMainWindow::closeEvent");

    userCallTimer.stop();

    // and tidy up all loose ends

    if (kstconnected)
    {
        do_connectButton_clicked();
    }

    trace("KSTMainWindow Closing");

    delete monitoredLogs;
    monitoredLogs =nullptr;

    clearScreenLayout();

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
void KSTMainWindow::createScreenComponents()
{
    // create component frames, parentless

    trace("createScreenComponents start");

    kstASActiveFrame= new KSTASActiveFrame(this);
    kstASActiveFrame->setObjectName(QStringLiteral("KSTASActiveFrame"));
    kstASActiveFrame->setFrameShape(QFrame::StyledPanel);
    kstASActiveFrame->setFrameShadow(QFrame::Raised);
    kstASActiveFrame->setVisible(false);

    kstButtonsFrame = new KSTButtonsFrame(this);
    kstButtonsFrame->setObjectName(QStringLiteral("KSTButtonsFrame"));
    kstButtonsFrame->setFrameShape(QFrame::StyledPanel);
    kstButtonsFrame->setFrameShadow(QFrame::Raised);
    kstButtonsFrame->setVisible(false);

    kstCallsFrame = new KSTCallsFrame(this);
    kstCallsFrame->setObjectName(QStringLiteral("KSTCallsFrame"));
    kstCallsFrame->setFrameShape(QFrame::StyledPanel);
    kstCallsFrame->setFrameShadow(QFrame::Raised);
    kstCallsFrame->setVisible(false);

    kstLoginFrame = new KSTLoginFrame(this);
    kstLoginFrame->setObjectName(QStringLiteral("KSTLoginFrame"));
    kstLoginFrame->setFrameShape(QFrame::StyledPanel);
    kstLoginFrame->setFrameShadow(QFrame::Raised);
    kstLoginFrame->setVisible(false);

    kstMsgFrame = new KSTMsgFrame(this);
    kstMsgFrame->setObjectName(QStringLiteral("KSTMsgFrame"));
    kstMsgFrame->setFrameShape(QFrame::StyledPanel);
    kstMsgFrame->setFrameShadow(QFrame::Raised);
    kstMsgFrame->setVisible(false);

    kstPlanesFrame = new KSTPlanesFrame(this);
    kstPlanesFrame->setObjectName(QStringLiteral("KSTPlanesFrame"));
    kstPlanesFrame->setFrameShape(QFrame::StyledPanel);
    kstPlanesFrame->setFrameShadow(QFrame::Raised);
    kstPlanesFrame->setVisible(false);

    kstSendMeepFrame = new KSTSendMeepFrame(this);
    kstSendMeepFrame->setObjectName(QStringLiteral("KSTSendMeepFrame"));
    kstSendMeepFrame->setFrameShape(QFrame::StyledPanel);
    kstSendMeepFrame->setFrameShadow(QFrame::Raised);
    kstSendMeepFrame->setVisible(false);

    kstTomeFrame = new KSTTomeFrame(this);
    kstTomeFrame->setObjectName(QStringLiteral("KSTTomeFrame"));
    kstTomeFrame->setFrameShape(QFrame::StyledPanel);
    kstTomeFrame->setFrameShadow(QFrame::Raised);
    kstTomeFrame->setVisible(false);

    trace("createScreenComponents end");
}
void KSTMainWindow::clearScreenLayout()
{
    // clear down the screen elements

    suppressSaveHeaders = true; // stop cutils saving headers

    trace("clearScreenLayout starts ");

    kstASActiveFrame->setParent(this);
    kstASActiveFrame->hide();
    kstButtonsFrame->setParent(this);
    kstButtonsFrame->hide();
    kstCallsFrame->setParent(this);
    kstCallsFrame->hide();
    kstLoginFrame->setParent(this);
    kstLoginFrame->hide();

    kstMsgFrame->setParent(this);
    kstMsgFrame->hide();
    kstPlanesFrame->setParent(this);
    kstPlanesFrame->hide();
    kstSendMeepFrame->setParent(this);
    kstSendMeepFrame->hide();
    kstTomeFrame->setParent(this);
    kstTomeFrame->hide();

    for (auto cpc = pages.begin(); cpc != pages.end(); cpc++)
    {
        if (*cpc == nullptr)
        {
            continue;
        }
        delete *cpc;
        *cpc = nullptr;
    }
    pages.clear();

    suppressSaveHeaders = false;
    trace("clearScreenLayout complete");
}
void KSTMainWindow::applyScreenLayout()
{
    inApplyScreenLayout = true;

    trace("applyScreenLayout start");

    clearScreenLayout();
    buildScreenLayout();
    mainWindow->setContentsMargins(0, 0, 0, 0);

    delayedAction(this,  [=](){
        inApplyScreenLayout = false;
    }
     );
}

QString KSTMainWindow::getCurScreenLayout() const
{
    return curScreenLayout;
}

void KSTMainWindow::setCurScreenLayout(const QString &value)
{
    trace(QString("setCurScreenLayout %1").arg(value));
    curScreenLayout = value;
}

void KSTMainWindow::buildRow(KSTPageFrame *cp, SCRow &scrow, MinosSplitter *splitterParent)
{
    // This builds the dependant ContestPage (including the one we derive from)
    if (scrow.elements.count())
    {
        // insert horizontal splitter in splitterParent
        MinosSplitter *hs = new MinosSplitter();
        hs->setObjectName("row" + QString::number(cp->rowSplitters.size()) + "splitter");
        hs->setOrientation(Qt::Horizontal);
        hs->setChildrenCollapsible(false);

        // we want this in the contest page...
        cp->rowSplitters.push_back(hs);

        for (auto &scele: scrow.elements)
        {
            //SCElement scele = scrow.elements[srele];
            SCType type = scele.type;
            if (type == sctNone)
                continue;

            QScrollArea *elementScrollArea = nullptr;
            if (type == sctkASActive ||
                type == sctkLogins ||
                type == sctkSendMeep ||
                type == sctkButtons
                )
            {
                elementScrollArea = new QScrollArea();
                elementScrollArea->setWidgetResizable(true);
                elementScrollArea->setFocusPolicy(Qt::NoFocus);
                elementScrollArea->setFrameStyle(QStyleOptionFrame::None);
                elementScrollArea->setFrameShadow(QFrame::Plain);

                hs->addWidget(elementScrollArea);
            }

            // insert correct widget type in horizontal splitter

            switch (type)
            {
            case sctNone:
            case sctMainScreen:
            case sctScreen:
            {
                break;
            }

            case sctkASActive:
                elementScrollArea->setWidget(kstASActiveFrame);
                kstASActiveFrame->fontsize = scele.fontSize;
                break;

            case sctkCallList:
                hs->addWidget(kstCallsFrame);
                kstCallsFrame->fontsize = scele.fontSize;
                kstCallsFrame->setVisible(true);
                break;

            case sctkAirScout:
                hs->addWidget(kstPlanesFrame);
                kstPlanesFrame->fontsize = scele.fontSize;
                kstPlanesFrame->setVisible(true);
                break;

            case sctkMessageList:
                hs->addWidget(kstMsgFrame);
                kstMsgFrame->fontsize = scele.fontSize;
                kstMsgFrame->setVisible(true);
                break;

            case sctkMeepList:
                hs->addWidget(kstTomeFrame);
                kstTomeFrame->fontsize = scele.fontSize;
                kstTomeFrame->setVisible(true);
                break;

            case sctkLogins:
                elementScrollArea->setWidget(kstLoginFrame);
                kstLoginFrame->fontsize = scele.fontSize;
                break;

            case sctkSendMeep:
                elementScrollArea->setWidget(kstSendMeepFrame);
                kstSendMeepFrame->fontsize = scele.fontSize;
                break;

            case sctkButtons:
                elementScrollArea->setWidget(kstButtonsFrame);
                kstButtonsFrame->fontsize = scele.fontSize;
                break;

            case sctSplit:
            {
                MinosSplitter *vs = new MinosSplitter();
                vs->setObjectName("splitRow" + QString::number(cp->rowSplitters.size()) + "splitter");
                vs->setOrientation(Qt::Vertical);
                vs->setChildrenCollapsible(false);
                cp->rowSplitters.push_back(vs);

                for (auto &srow: scele.rows)
                {
                    buildRow(cp, srow, vs);
                }

                hs->addWidget(vs);
                break;
            }
            default:
                break;
            }
        }
        splitterParent->addWidget(hs);
    }

}
void KSTMainWindow::buildScreen(SCScreen &s, int t)
{
    // we need to add this contest page to the relevant contestPageControl
    // as a new tab

    // How do we make sure that ALL contests are in ALL page controls, even when
    // they have no such screen?

    KSTPageFrame *cp = new KSTPageFrame(nullptr, t);
    QString n = QString("kstpage%1").arg(t);
    cp->setObjectName(n);
    pages.push_back(cp);
    if (t == 0)
    {
        ui->centralwidget->layout()->addWidget(cp);
    }
    else
    {
        cp->setWindowFlags( Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint);
        cp->setAttribute(Qt::WA_ShowWithoutActivating);
        cp->show();
    }

    cp->pageNo = t;
    cp->buildScreen(s);

}
void KSTMainWindow::buildScreenLayout()
{
    suppressSaveHeaders = true; // stop cutils saving headers

    ScreenConfigFile &scf = ScreenConfigFile::getScreenConfigFile(this);

    if (curScreenLayout.isEmpty() || !scf.configs.contains(curScreenLayout))
    {
        curScreenLayout = defaultLayoutName();
    }
    trace("buildScreenLayout to layout " + curScreenLayout);
    setCurScreenLayout(curScreenLayout);

    SC sc = scf.configs[curScreenLayout];

    // build the pages
    int t = 0;
    for (auto &s: sc.baseElement->screens)
    {
        buildScreen(s, t++);
    }

    suppressSaveHeaders = false;
}
void KSTMainWindow::do_dialog_clicked()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action && action->data() != QVariant())
    {
        int tag = action->data().toInt();
        QFrame *actionFrame = nullptr;
        switch(tag)
        {
        case sctkLogins:
            actionFrame = kstLoginFrame;
            break;
        case sctkCallList:
            actionFrame = kstCallsFrame;
            break;
        case sctkAirScout:
            actionFrame = kstPlanesFrame;
            break;
        case sctkMessageList:
            actionFrame = kstMsgFrame;
            break;
        case sctkMeepList:
            actionFrame = kstTomeFrame;
            break;
        case sctkASActive:
            actionFrame = kstASActiveFrame;
            break;
        case sctkSendMeep:
            actionFrame = kstSendMeepFrame;
            break;
        case sctkButtons:
            actionFrame = kstButtonsFrame;
            break;
        }

        if (actionFrame)
        {
            // create a dialog containing the desired frame
            // making it temporarily usable
            QDialog *actDialog = new QDialog(this);
            QVBoxLayout *vb = new QVBoxLayout(actDialog);
            vb->addWidget( actionFrame);

            QFrame *line = new QFrame(actDialog);
            line->setObjectName("line");
            line->setFrameShape(QFrame::Shape::HLine);
            line->setFrameShadow(QFrame::Shadow::Sunken);
            vb->addWidget(line);

            QHBoxLayout *hb = new QHBoxLayout(actDialog);
            QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

            hb->addItem(horizontalSpacer);
            QPushButton *closeButton = new QPushButton(tr("Exit"), actDialog);
            hb->addWidget(closeButton);
            vb->addItem(hb);
            actionFrame->setVisible(true);

            connect(closeButton, &QPushButton::clicked, [=]()
                    {actDialog->close();});

            actDialog->exec();
            actionFrame->setParent(this);
            actionFrame->setVisible(false);
            monitoredLogs->hide();
        }
    }
}
void KSTMainWindow::onNewLog(QSharedPointer<MonitoredLog> ml)
{
    connect(ml.data(), &MonitoredLog::newStanzas, this, &KSTMainWindow::onNewStanzas, Qt::QueuedConnection);
}
void KSTMainWindow::onLogChanged(QSharedPointer<MonitoredLog> /*ml*/)
{
    kstMsgFrame->kstMessageFilterModel.invalidate();
    kstTomeFrame->kstMeepFilterModel.invalidate();
    kstCallsFrame->kstCallFilterModel.invalidate();
    kstPlanesFrame->kstPlanesFilterModel.invalidate();
}
void KSTMainWindow::onNewStanzas()
{
    kstMsgFrame->kstMessageFilterModel.invalidate();
    kstTomeFrame->kstMeepFilterModel.invalidate();
    kstCallsFrame->kstCallFilterModel.invalidate();
    kstPlanesFrame->kstPlanesFilterModel.invalidate();
}
void KSTMainWindow::onLogStarted(QSharedPointer<MonitoredLog> /*ml*/)
{
    kstMsgFrame->kstMessageFilterModel.invalidate();
    kstTomeFrame->kstMeepFilterModel.invalidate();
    kstCallsFrame->kstCallFilterModel.invalidate();
    kstPlanesFrame->kstPlanesFilterModel.invalidate();
}
void KSTMainWindow::onLogClosed(QSharedPointer<MonitoredLog> /*ml*/)
{
    kstMsgFrame->kstMessageFilterModel.invalidate();
    kstTomeFrame->kstMeepFilterModel.invalidate();
    kstCallsFrame->kstCallFilterModel.invalidate();
    kstPlanesFrame->kstPlanesFilterModel.invalidate();
}
void KSTMainWindow::userCallTimerTimer()
{
    if (asl && kstASActiveFrame->getASActive() && callVectorChanged && callVector)
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

    kstTomeFrame->setConnected(true);

    kstButtonsFrame->setConnected(true);
    kstTomeFrame->setMeepFilters();
    connectAction->setText(tr("Disconnect"));
}


void KSTMainWindow::clearConnection()
{
    kstTomeFrame->setConnected(false);
    kstButtonsFrame->setConnected(false);
    kstconnected = false;
    kstLoggedIn.clear();
    connectAction->setText(tr("Connect"));
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
        QSharedPointer<KstUser> user = getUser(KstUser(c, getActiveChat()));
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
void KSTMainWindow::setMaxDistance(int m)
{
    maxDistance = m;
}
int KSTMainWindow::getMaxDistance() const
{
    return maxDistance;
}

int KSTMainWindow::getASActive() const
{
    return ASActive;
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

void KSTMainWindow::setActiveChat(int c)
{
    activeChat = c;
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


void KSTMainWindow::addMessage(QSharedPointer<KstMessageLine> kst)
{
    // Add to the counts of messages per user
    kstMsgFrame->kstMessageModel.appendLastRow(kst);
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
            kstCallsFrame->kstCallModel.locator = recLoc;  //set from LOGSTAT after LOGINC
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
        kstMsgFrame->scrollMesToBottom();

        kstTomeFrame->scrollMeepToBottom();
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

        kstMsgFrame->scrollMesToBottom();
        kstTomeFrame->scrollMeepToBottom();

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
                emit kstCallsFrame->kstCallModel.dataChanged(
                    kstCallsFrame->kstCallModel.index(row, 0),
                    kstCallsFrame->kstCallModel.index(row, kstCallsFrame->kstCallModel.columnCount() - 1));
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

        kstCallsFrame->kstCallModel.setCallVector(callVector);
        kstMsgFrame->kstMessageModel.setChatVector(messageVector);

        kstCallsFrame->kstCallFilterModel.invalidate();
        kstMsgFrame->kstMessageFilterModel.invalidate();

        kstMsgFrame->scrollMesToBottom();

        kstTomeFrame->scrollMeepToBottom();

        if (!firstName.isEmpty() && recName != firstName) {
            // setnam isn't valid, and not likely to be
            //            QString msg = "MSG|" + QString::number(activeChat) +
            //            "|0|/SETNAM " + firstName + "|0|"; sendKST(msg);

            kstclient->disconnectFromHost();
            kstLoggedIn.clear();

            kstCallsFrame->kstCallModel.reset();
            callVector->clear();
            callMap.clear();
            kstMsgFrame->kstMessageModel.reset();
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
                "MSG|" + QString::number(getActiveChat()) + "|0|/SETLOC " + myLoc + "|0|";
            sendKST(msg);
            recLoc = myLoc;
            kstCallsFrame->kstCallModel.locator = myLoc;   // change loc with /SETLOC
            for (auto const &l : QASCONST(*callVector)) {
                l->distance = -1;
            }
            kstCallsFrame->kstCallFilterModel.invalidate();
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
            emit kstCallsFrame->kstCallModel.dataChanged(
                kstCallsFrame->kstCallModel.index(row, 0),
                kstCallsFrame->kstCallModel.index(row, kstCallsFrame->kstCallModel.columnCount() - 1));
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
            emit kstCallsFrame->kstCallModel.dataChanged(
                kstCallsFrame->kstCallModel.index(row, 0),
                kstCallsFrame->kstCallModel.index(row, kstCallsFrame->kstCallModel.columnCount() - 1));
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

            kstCallsFrame->kstCallModel.removeRow(row);
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
            kstCallsFrame->kstCallModel.insertRow(row, test);
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
        emit kstMsgFrame->kstMessageModel.dataChanged(
            kstMsgFrame->kstMessageModel.index(0, 0),
            kstMsgFrame->kstMessageModel.index(kstMsgFrame->kstMessageModel.rowCount() - 1,
                                  kstMsgFrame->kstMessageModel.columnCount() - 1));
    }

    checkAwayButton();
}
void KSTMainWindow::checkAwayButton()
{
    kstButtonsFrame->checkAwayButton();

    QSharedPointer<KstUser> user = mainWindow->getUser(KstUser(mainWindow->myCallsign, mainWindow->getActiveChat()));
    if (user)
    {
        if (user->away)
        {
            awayAction->setText(tr("Set Back"));
        }
        else
        {
            awayAction->setText(tr("Set Away"));
        }
    }
}
void KSTMainWindow::do_connectButton_clicked()
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

    conf.lcf = lcf;

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

        bool doRebuild = false;
        if (conf.lcf != lcf)
        {
            lcf = conf.lcf;
            settings.setValue("lcf", lcf);
            // and we need to trigger a rebuild
            doRebuild = true;
        }

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
            kstCallsFrame->kstCallFilterModel.invalidate();
            kstMsgFrame->kstMessageFilterModel.invalidate();

            if (kstASActiveFrame->getASActive())
            {
                asl.reset();

                asl = QSharedPointer<AirScoutLink>(new AirScoutLink());
                connect(asl.data(), &AirScoutLink::acChanged, kstPlanesFrame, &KSTPlanesFrame::acChanged);
            }
            if  (kstconnected)
            {
                reconnect();
            }
        }
        if (doRebuild)
        {
            appStart.emitListCompressionChanged(lcf/100.0);
        }
        return true;
    }
    return false;
}

void KSTMainWindow::do_configureButton_clicked()
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
            kstCallsFrame->kstCallModel.reset();
            callVector->clear();
            callMap.clear();
        }
    }
    else
    {
        if (kstChatSelection.count())
        {
            kstCallsFrame->kstCallModel.locator = myLoc;   // set from config
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
        kstCallsFrame->kstCallModel.setCallVector(newCallVector);
        callVector = newCallVector;
        callMap.clear();
        for(auto const &i: QASCONST(*callVector))
        {
            callMap[*i.data()] = i;
        }
    }

    kstLoginFrame->checkActive();
}

void KSTMainWindow::do_awayButton_clicked()
{
    QSharedPointer<KstUser> user = getUser(KstUser(myCallsign, getActiveChat()));
    if (user)
    {
        if (user->away)
        {
            QString msg = "MSG|" + QString::number(getActiveChat()) + "|0|/BACK|0|";
            sendKST(msg);

        }
        else
        {
            QString msg = "MSG|" + QString::number(getActiveChat()) + "|0|/AWAY|0|";
            sendKST(msg);
        }
    }
}

QSharedPointer<KstUser> KSTMainWindow::getUser(const KstUser &test)
{
    if (callMap.contains(test))
    {
        return callMap[test];
    }
    return QSharedPointer<KstUser>();
}

void KSTMainWindow::do_logsButton_clicked()
{
    monitoredLogs->show();
    monitoredLogs->raise();
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

void KSTMainWindow::do_KSTTestButton_clicked()
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
void KSTMainWindow::do_layoutButton_clicked()
{
    QString cur = curScreenLayout;
    QString def = defaultLayoutName();
    QString prot;

    ScreenConfigManager sc(this, cur, def, prot);
    connect(&sc, &ScreenConfigManager::screenConfigApply, this, &KSTMainWindow::onScreenConfigApply);
    connect(&sc, &ScreenConfigManager::setDefaultName, this, &KSTMainWindow::onSetDefaultName);
    connect(&sc, &ScreenConfigManager::setProtectedName, this, &KSTMainWindow::onSetProtectedName);

    sc.exec();

}
void KSTMainWindow::selectLayout(QString layout)
{
    curScreenLayout = layout;

    QSettings settings(iniName, QSettings::IniFormat);

    settings.setValue("screenLayout", curScreenLayout);

    clearScreenLayout();

    setCurScreenLayout(layout);
    applyScreenLayout();

    appStart.emitFontChanged();

    delayedAction(this, [=]{
    kstButtonsAction->setEnabled(kstButtonsFrame->parent() == this);
    kstCallsAction->setEnabled(kstCallsFrame->parent() == this);
    kstLoginAction->setEnabled(kstLoginFrame->parent() == this);
    kstMsgAction->setEnabled(kstMsgFrame->parent() == this);
    kstASActiveAction->setEnabled(kstASActiveFrame->parent() == this);
    kstPlanesAction->setEnabled(kstPlanesFrame->parent() == this);
    kstSendMeepAction->setEnabled(kstSendMeepFrame->parent() == this);
    kstTomeAction->setEnabled(kstTomeFrame->parent() == this);
    });
}

void KSTMainWindow::onScreenConfigApply(QString curConfigName)
{
    selectLayout(curConfigName);
}
void KSTMainWindow::onSetDefaultName(QString /*def*/)
{
}
void KSTMainWindow::onSetProtectedName(QString /*prot*/)
{
    // no protected layout for KST
}

void KSTMainWindow::do_closeButton_clicked()
{
    close();
}
void KSTMainWindow::do_clearLogsButton_clicked()
{
    kstMsgFrame->kstMessageModel.reset();
    for (auto const &l: QASCONST(*callVector))
    {
        l->messageCount = 0;
    }
    kstCallsFrame->kstCallFilterModel.invalidate();

}

#ifdef Q_OS_WIN
void KSTMainWindow::do_splitIcons(bool)
{
    splitIcons = splitIconsAction->isChecked();
    QSettings settings(iniName, QSettings::IniFormat);

    settings.setValue("splitIcons", splitIcons);
    selectLayout(curScreenLayout);
}
#endif

void KSTMainWindow::do_ASActive(bool s)
{
    if (started && kstASActiveFrame)
    {
        ASActive = s;
        kstASActiveFrame->setASActive(ASActive);

        QSettings settings(iniName, QSettings::IniFormat);

        settings.setValue("ASActive", ASActive);
    }
}
QMenu *KSTMainWindow::newMenu(QMenu *m, const char *text)
{
    QMenu *menu = m->addMenu(tr(text));
    //menuList[menu] = text;
    return menu;
}
QAction *KSTMainWindow::newAction(const char *text, QMenu *m, void (KSTMainWindow::*slotparam)(),QAction::MenuRole mr )
{
    QAction * newAct = new QAction( tr(text), this );
    newAct->setMenuRole(mr);
    //actionList[newAct] = text;
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}
QAction *KSTMainWindow::newAction(int n, QMenu *m, void (KSTMainWindow::*slotparam)(),QAction::MenuRole mr )
{
    QAction * newAct = new QAction( QString::number(n), this );
    newAct->setMenuRole(mr);
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}

QAction *KSTMainWindow::newCheckableAction( const char *text, QMenu *m, void (KSTMainWindow::*slotparam)(bool) )
{
    QAction * newAct = new QAction( tr(text), this );
    //actionList[newAct] = text;
    newAct->setCheckable( true );
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}
QAction *KSTMainWindow::newCheckableAction(const QString text, QMenu *m, void (KSTMainWindow::*slotparam)(bool) )
{
    QAction * newAct = new QAction( text, this );
    newAct->setCheckable( true );
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}
