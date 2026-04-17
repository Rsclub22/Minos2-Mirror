#include <QStyleFactory>
#include <QFontDialog>
#include <QDesktopServices>
#include <QToolTip>
#include <QFileDialog>
#include <QLabel>
#include <QScreen>

#include "manageadifdialog.h"
#include "managebandmapspotsdb.h"
#include "regsettings.h"
#include "AppStartup.h"
#include "MMessageDialog.h"
#include "MShowMessageDlg.h"
#include "MinosLoggerEvents.h"
#include "PubSubClient.h"
#include "SecondInstall.h"
#include "ContestApp.h"
#include "LoggerContest.h"
#include "WindowsAppId.h"

#include "checkupdates.h"
#include "fileutils.h"
#include "list.h"
#include "tcalendardownload.h"
#include "tsinglelogframe.h"
#include "taboutbox.h"
#include "contestdetails.h"
#include "tmanagelistsdlg.h"
#include "tsettingseditdlg.h"
#include "tclockdlg.h"
#include "tloccalcform.h"
#include "TSessionManager.h"
#include "StartConfigManager.h"
#include "ConfigFile.h"
#include "SendRPCDM.h"
#include "AdifImport.h"
#include "ScreenConfigManager.h"
#include "MinosTestImport.h"
#include "singleapplication.h"
#include "WsjtxServer.h"
#include "ChatServer.h"
#include "clusterClientServer.h"
#include "MatchThread.h"
#include "delayedaction.h"
#include "ContestPageControl.h"
#include "OptionsDialog.h"
#include "bandmapclientframe.h"
#include "StatisticsDisplay.h"
#include "managehamlib.h"
#include "MTrace.h"
#include "RPCPubSub.h"
#include "MinosConnection.h"
#include "waitcursor.h"

#include "tlogcontainer.h"
#include "ui_tlogcontainer.h"

TLogContainer *LogContainer = nullptr;

bool TLogContainer::loggerClosing = false;

SetMemoryAction::SetMemoryAction(QString t, QObject *p):QAction(t, p)
{}

void TLogContainer::openSerialTVSwitch()
{
    if (!serialTVSw)
    {
        serialTVSw = new SerialTVSwitch();     // create local serial sw for band switching
    }
    else
    {
        serialTVSw->closeComport();
    }

    if (readEnableBandSwitchFromIni() && readEnableSerialBandSwitchFromIni())
    {
        trace(QString("Opening Bandswitch comport"));
        QString comport = readSerialComportBandSwitchFromIni();
        if (comport.isEmpty())
        {
            trace(QString("BandSwitch Comport is empty"));
        }
        else
        {
            if (serialTVSw->openComport(comport))
            {
                trace(QString("Bandswitch comport %1 opened OK").arg(comport));
            }
            else
            {
                QString errMsg = serialTVSw->error();
                trace(QString("Bandswitch Comport failed to open = %1 Error = %2").arg(comport, errMsg));
            }
        }
    }
}
bool TLogContainer::inspectGeometry(const QByteArray &geometry)
{
    if (geometry.size() < 4)
        return false;
    QDataStream stream(geometry);
    stream.setVersion(QDataStream::Qt_4_0);

    const quint32 magicNumber = 0x1D9D0CB;
    quint32 storedMagicNumber;
    stream >> storedMagicNumber;
    if (storedMagicNumber != magicNumber)
        return false;

    const quint16 currentMajorVersion = 3;
    quint16 majorVersion = 0;
    quint16 minorVersion = 0;

    stream >> majorVersion >> minorVersion;

    if (majorVersion > currentMajorVersion)
        return false;
    // (Allow all minor versions.)

    QRect restoredFrameGeometry;
    QRect restoredGeometry;
    QRect restoredNormalGeometry;
    qint32 restoredScreenNumber;
    quint8 maximized;
    quint8 fullScreen;
    qint32 restoredScreenWidth = 0;

    stream >> restoredFrameGeometry // Only used for sanity checks in version 0
        >> restoredNormalGeometry
        >> restoredScreenNumber
        >> maximized
        >> fullScreen;

    if (majorVersion > 1)
        stream >> restoredScreenWidth;
    if (majorVersion > 2)
        stream >> restoredGeometry;

    // ### Qt 6 - Perhaps it makes sense to dumb down the restoreGeometry() logic, see QTBUG-69104

    if (restoredScreenNumber >= qMax(QGuiApplication::screens().size(), 1))
        restoredScreenNumber = 0;
    const QScreen *restoredScreen = QGuiApplication::screens().value(restoredScreenNumber, nullptr);
    const qreal screenWidthF = restoredScreen ? qreal(restoredScreen->geometry().width()) : 0;
    // Sanity check bailing out when large variations of screen sizes occur due to
    // high DPI scaling or different levels of DPI awareness.
    if (restoredScreenWidth) {
        const qreal factor = qreal(restoredScreenWidth) / screenWidthF;
        if (factor < 0.8 || factor > 1.25)
            return false;
    } else {
        // Saved by Qt 5.3 and earlier, try to prevent too large windows
        // unless the size will be adapted by maximized or fullscreen.
        if (!maximized && !fullScreen && qreal(restoredFrameGeometry.width()) / screenWidthF > 1.5)
            return false;
    }

    const int frameHeight = QApplication::style()
                                ? QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight)
                                : 20;

    if (!restoredNormalGeometry.isValid())
        restoredNormalGeometry = QRect(QPoint(0, frameHeight), sizeHint());
    if (!restoredNormalGeometry.isValid()) {
        // use the widget's adjustedSize if the sizeHint() doesn't help
//        restoredNormalGeometry.setSize(restoredNormalGeometry
//                                           .size()
//                                           .expandedTo(d_func()->adjustedSize()));
    }

    const QRect availableGeometry = restoredScreen ? restoredScreen->availableGeometry()
                                                   : QRect();

    // Modify the restored geometry if we are about to restore to coordinates
    // that would make the window "lost". This happens if:
    // - The restored geometry is completely or partly oustside the available geometry
    // - The title bar is outside the available geometry.

//    QWidgetPrivate::checkRestoredGeometry(availableGeometry, &restoredGeometry, frameHeight);
//    QWidgetPrivate::checkRestoredGeometry(availableGeometry, &restoredNormalGeometry, frameHeight);

    if (maximized || fullScreen) {
        // set geometry before setting the window state to make
        // sure the window is maximized to the right screen.
        Qt::WindowStates ws = windowState();
#ifndef Q_OS_WIN
        setGeometry(restoredNormalGeometry);
#else
        if (ws & Qt::WindowFullScreen) {
            // Full screen is not a real window state on Windows.
            move(availableGeometry.topLeft());
        } else if (ws & Qt::WindowMaximized) {
            // Setting a geometry on an already maximized window causes this to be
            // restored into a broken, half-maximized state, non-resizable state (QTBUG-4397).
            // Move the window in normal state if needed.
            if (restoredScreen != screen()) {
                setWindowState(Qt::WindowNoState);
                setGeometry(restoredNormalGeometry);
            }
        } else {
            setGeometry(restoredNormalGeometry);
        }
#endif // Q_OS_WIN
        if (maximized)
            ws |= Qt::WindowMaximized;
        if (fullScreen)
            ws |= Qt::WindowFullScreen;
        setWindowState(ws);
//        d_func()->topData()->normalGeometry = restoredNormalGeometry;
    } else {
        setWindowState(windowState() & ~(Qt::WindowMaximized | Qt::WindowFullScreen));

        // FIXME: Why fall back to restoredNormalGeometry if majorVersion <= 2?
        if (majorVersion > 2)
            setGeometry(restoredGeometry);
        else
            setGeometry(restoredNormalGeometry);
    }
    return true;
}

TLogContainer::TLogContainer(QWidget *parent) :
    QMainWindow(parent)
  , ui(new Ui::TLogContainer)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->kbframe->setVisible(false);

    LogContainer = this;

    // we may need to delay this to get the container fully constructed
    TContestApp::getContestApp(); // initialise all the infrastructure

    setDefLogDir(getDefaultDirectory(false));
    setDefListDir(getDefaultDirectory(true));

    // make the tab control fill the window
    ui->centralWidget->layout()->setContentsMargins(0,0,0,0);

    setCaption(QString());

    setupMenus();

    // These are specific to THIS ContestPageControl
    ui->contestPageControl->setTabsClosable(true);
    connect(ui->contestPageControl->tabBar(), &QTabBar::tabCloseRequested, this, &TLogContainer::onTabClosebutton);
    connect(ui->contestPageControl->tabBar(), &QTabBar::tabMoved, this, &TLogContainer::onTabMoved);

    trace(QString("TLogContainer geometry T %1 L %2 B %3 R %4").arg(geometry().top()).arg(geometry().left())
              .arg(geometry().bottom()).arg(geometry().right()));

    {
        RegSettings settings;
        QByteArray ageometry = settings.getSettings().value("geometry").toByteArray();
        int t = settings.getSettings().value("geoT").toInt();
        int l = settings.getSettings().value("geoL").toInt();
        int b = settings.getSettings().value("geoB").toInt();
        int r = settings.getSettings().value("geoR").toInt();
        if (ageometry.size() > 0)
            restoreGeometry(ageometry);

        trace(QString("TLogContainer geometry T %1 L %2 B %3 R %4 (%5 %6 %7 %8)").arg(geometry().top()).arg(geometry().left())
              .arg(geometry().bottom()).arg(geometry().right()).arg(t).arg(l).arg(b).arg(r));

        if (t != 0 && l != 0 && b != 0 && r != 0)
        if (geometry().top() != t
            ||geometry().left() != l
            ||geometry().bottom() != b
            ||geometry().right() != r
            )
        {
            //inspectGeometry(ageometry);
            trace("Bad geometry!");
            QRect geoRect(l, t, r - l, b - t);
            setGeometry(geoRect);
        }

    }

    sblabel0 = new QLabel( "" );
    statusBar() ->addWidget( sblabel0, 6 );
    sblabel1 = new QLabel( "" );
    statusBar() ->addWidget( sblabel1, 1 );

    sblabel1->installEventFilter(this);

    sblabel2 = new QLabel( "" );
    statusBar() ->addWidget( sblabel2, 2 );

    sendDM = new TSendDM(this);

    QString station = MinosConfig::getMinosConfig()->getThisRouterName();
    RPCPubSub::publish(rpcConstants::LoggerCategory, station, "", psPublished);

    connect(&MinosConfigEvents::mce, &MinosConfigEvents::stealFocus, this, &TLogContainer::stealFocus);

    ScreenConfigFile::getScreenConfigFile(this);  // get configs loaded

    openSerialTVSwitch();


    contestPageControls.append(ui->contestPageControl);

    BandMapSpotDB::startDB();

}
TLogContainer::~TLogContainer()
{
    setAppClosing();

    delete ui;
    delete sendDM;
    delete MinosConfig::getMinosConfig();
    clearPubSub();

    if (MinosRPC::getMinosRPC())
    {
        delete ChatServer::getChatServer();
        delete ClusterClientServer::getClusterClientServer();

        delete MinosRPC::getMinosRPC();
        delete MinosAppConnection::minosAppConnection;
    }
    MinosRPCObj::clearRPCObjects();
    ScreenConfigFile::getScreenConfigFile(this).configs.clear();
    for(auto cpc: QASCONST(contestPageControls))
    {
        if (cpc && cpc->getInstance() > 0)
        {
            cpc->close();
        }
        delete cpc;
        cpc = nullptr;
    }

    delete serialTVSw;
    delete n1mmBroadcast;
}

bool TLogContainer::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ToolTip && obj == sblabel1)
    {
        QHelpEvent *helpEvent = dynamic_cast<QHelpEvent *>(event);

        QString toolTip;
        QWidget *f = QApplication::focusWidget ();
        if(f)
        {
            QString name = f->objectName();
            if (name.isEmpty())
            {
                QToolButton *tb = dynamic_cast<QToolButton *>(f);
                if (tb)
                {
                    name = tb->text();
                }
            }
            QWidget *p = f->parentWidget();
            while ( p)
            {
                name += " | " + p->objectName();
                p = p->parentWidget();
            }
             toolTip = f->metaObject()->className() + QString(" | ") + name;
        }
        else
        {
            toolTip = "<unknown>";
        }

        QToolTip::showText(helpEvent->globalPos(), toolTip);
        return true;
    }
    if (event->type() == QEvent::Close)
    {
        clearWinAppId(this);
    }


    return false;
}


bool TLogContainer::show(int argc, char *argv[])
{
    QMainWindow::show();

    TContestApp::getContestApp() ->loggerBundle.flushProfile();

    TimerUpdateQSOTimer.start(1000);
    connect(&TimerUpdateQSOTimer, &QTimer::timeout, this, &TLogContainer::on_TimeDisplayTimer);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ReportOverstrike, this, &TLogContainer::on_ReportOverstrike, Qt::QueuedConnection);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::setMemoryAction, this, &TLogContainer::mleSetMemoryAction);

    setWinAppId(this, SecondInstall::getOrgName() + QString(".MqtLogger.SubScreen%1").arg(0) );

    if ( TAboutBox::ShowAboutBox( this, true ) == false )
    {
        trace("exit called from About box");
       close();
       return false;
    }

    if ( contestAppLoadFiles() )
    {
       // here need to pre-open the contest list
       QString conarg;
       for(int i = 1; i < argc; i++)
       {
           if ( argv[i][0] != '/' && argv[i][0] != '-' ) // i.e. not a switch character
           {
              conarg = argv[i];
              break;
           }
       }
       preloadLists();
       preloadFiles( conarg );
       enableActions();

    }
    TContestApp::getContestApp()->setPreloadComplete();
    sendDM->subscribeApps();
    if (!n1mmBroadcast)
    {
        n1mmBroadcast = new N1MMBroadcast();
    }
    n1mmBroadcast->configure();
    WsjtxServer::getWsjtxServer()->start();

    return true;
}
void TLogContainer::onArgsReceived(QString conarg)
{
    preloadFiles( conarg );
}

void TLogContainer::on_TimeDisplayTimer( )
{

   if ( TContestApp::getContestApp() )
   {
       BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();

       QDateTime t = QDateTime::currentDateTimeUtc().addSecs( MinosParameters::getMinosParameters() ->getBigClockCorrection());
       QString disp = t.toString( "dd/MM/yyyy HH:mm:ss" ) + " UTC       ";

       QString fc;
       bool timeOK = false;
       if (ct)
       {
            timeOK = ct->checkTime(t);
            if (!timeOK)

                fc = HtmlFontColour(Qt::red) + "<b>";
            else
                fc = HtmlFontColour(Qt::blue);
       }

       sblabel2 ->setText(fc + disp);

       MinosLoggerEvents::SendTimerDistribution();

      QString statbuf;
      QString otBuff;
      if ( ct )
      {
         ct->setScore( statbuf );
         int temp;
         TContestApp::getContestApp() ->loggerBundle.getIntProfile(elpShowOperateTime, temp);
         ct->getOpTime(otBuff, static_cast<SHOWOPERATINGTIME>(temp));

      }
      sblabel0->setText( QString(statbuf + " " + otBuff).trimmed());
   }

}
void TLogContainer::on_ReportOverstrike(bool overstrike, BaseContestLog *econtest )
{
    // this can be a "double queued Qt::QueuedConnection" event, and it can get
    // delayed until we are closing down and TContestApp has already gone

    TContestApp *tca = TContestApp::getContestApp();
    if (!tca)
        return;

   BaseContestLog * ct = tca->getCurrentContest();
   if (ct == econtest)
   {
      sblabel1->setText(overstrike ? tr("Overwrite") : tr("Insert"));
   }
}

void TLogContainer::closeEvent(QCloseEvent *event)
{
    trace("closeEvent:Start");
    if (loggerClosing)
    {
        trace("Logger already closing");
        return;
    }
    loggerClosing = true;
//    MinosConfig::getMinosConfig() ->askStop();

    TimerUpdateQSOTimer.stop();

    delete WsjtxServer::getWsjtxServer();

    TContestApp::getContestApp() ->writeContestList();
    TContestApp::getContestApp() ->suppressWritePreload = true;

    // close all current slots, but don't write preload
    CloseAllActionExecute();

    TContestApp::getContestApp() ->clearPreloadComplete();

    CloseAllActionExecute();
    trace("closeEvent:Contest slots closed");

    MinosConfig::getMinosConfig() ->askStop();

    for ( auto const &l: QASCONST(TContestApp::getContestApp() ->listSlotList ))
    {
       if ( l )
       {
          TContestApp::getContestApp() ->closeListFile( l->slot );
       }
    }
    trace("closeEvent:List slots closed");
    // no need to force close apps - when logger terminates they will be killed
    // but they may need to close cleanly
    MinosConfig::getMinosConfig() ->forceStop();
    trace("closeEvent:Apps closed");

    delete statusBar();
    closeContestApp();

    QWidget::closeEvent(event);
}
void TLogContainer::moveEvent(QMoveEvent *event)
{
    trace(QString("TLogContainer moveEvent geometry T %1 L %2 B %3 R %4").arg(geometry().top()).arg(geometry().left())
              .arg(geometry().bottom()).arg(geometry().right()));

    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());

    settings.getSettings().setValue("geoT", geometry().top());
    settings.getSettings().setValue("geoL", geometry().left());
    settings.getSettings().setValue("geoB", geometry().bottom());
    settings.getSettings().setValue("geoR", geometry().right());

    QWidget::moveEvent(event);
}
void TLogContainer::resizeEvent(QResizeEvent * event)
{
    trace(QString("TLogContainer resizeEvent geometry T %1 L %2 B %3 R %4").arg(geometry().top()).arg(geometry().left())
              .arg(geometry().bottom()).arg(geometry().right()));

    RegSettings settings;
    settings.getSettings().setValue("geometry", saveGeometry());

    settings.getSettings().setValue("geoT", geometry().top());
    settings.getSettings().setValue("geoL", geometry().left());
    settings.getSettings().setValue("geoB", geometry().bottom());
    settings.getSettings().setValue("geoR", geometry().right());

    QWidget::resizeEvent(event);
}
void TLogContainer::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
       trace(QString("TLogContainer changeEvent geometry T %1 L %2 B %3 R %4").arg(geometry().top()).arg(geometry().left())
                 .arg(geometry().bottom()).arg(geometry().right()));
        RegSettings settings;
        settings.getSettings().setValue("geometry", saveGeometry());

        settings.getSettings().setValue("geoT", geometry().top());
        settings.getSettings().setValue("geoL", geometry().left());
        settings.getSettings().setValue("geoB", geometry().bottom());
        settings.getSettings().setValue("geoR", geometry().right());
    }

    if (e->type() == QEvent::LanguageChange)
    {
        // when language changes force a complete rebuild
        TWaitCursor wc(this);
        selectSession(TContestApp::getContestApp()->currSession);

        // clear and rebuild the menus, in their new language
        clearMenus();

        setupMenus();

        TSingleLogFrame *tslf = getCurrentLogFrame();
        int tab = ui->contestPageControl->indexOf(tslf);
        setMenuLog(tab);

        ui->retranslateUi(this);
        setCaption(QString());
    }
    QMainWindow::changeEvent(e);
}
QMenu *TLogContainer::newMenu(QMenu *m, const char *text)
{
    QMenu *menu = m->addMenu(tr(text));
    menuList[menu] = text;
    return menu;
}
QAction *TLogContainer::newAction(const char *text, QMenu *m, void (TLogContainer::*slotparam)(),QAction::MenuRole mr )
{
    QAction * newAct = new QAction( tr(text), this );
    newAct->setMenuRole(mr);
    actionList[newAct] = text;
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}
QAction *TLogContainer::newAction(int n, QMenu *m, void (TLogContainer::*slotparam)(),QAction::MenuRole mr )
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
SetMemoryAction *TLogContainer::newMemoryAction(const char *text, QMenu *m, void (TLogContainer::*slotparam)() )
{
    SetMemoryAction * newAct = new SetMemoryAction( tr(text), this );
    actionList[newAct] = text;
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}
QAction *TLogContainer::newCheckableAction( const char *text, QMenu *m, void (TLogContainer::*slotparam)(bool) )
{
    QAction * newAct = new QAction( tr(text), this );
    actionList[newAct] = text;
    newAct->setCheckable( true );
    m->addAction( newAct );
    if (slotparam)
    {
        connect( newAct, &QAction::triggered, this, slotparam );
    }
    return newAct;
}
QAction *TLogContainer::newCheckableAction(const QString text, QMenu *m, void (TLogContainer::*slotparam)(bool) )
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

void TLogContainer::setupMenus()
{
    FileOpenAction = newAction(QT_TR_NOOP("&Open Contest..."), ui->menuFile, &TLogContainer::FileOpenActionExecute);
    FileImportVHFAction = newAction(QT_TR_NOOP("&Import VHF Contest..."), ui->menuFile, &TLogContainer::FileImportVHFActionExecute);
    FileImportHFAction = newAction(QT_TR_NOOP("&Import HF Contest..."), ui->menuFile, &TLogContainer::FileImportHFActionExecute);
    recentFilesMenu = newMenu(ui->menuFile, QT_TR_NOOP("Reopen Contest"));

    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        recentFileActs.push_back( new QAction(this));
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], &QAction::triggered, this, &TLogContainer::openRecentFile);
        recentFilesMenu->addAction(recentFileActs[i]);
    }
    updateRecentFileActions();

    VHFFileNewAction = newAction(QT_TR_NOOP("&New VHF Contest..."), ui->menuFile, &TLogContainer::VHFFileNewActionExecute);
    HFFileNewAction = newAction(QT_TR_NOOP("&New HF Contest..."), ui->menuFile, &TLogContainer::HFFileNewActionExecute);

    FileCloseAction = newAction(QT_TR_NOOP("Close Contest"), ui->menuFile, &TLogContainer::FileCloseActionExecute);
    CloseAllAction = newAction(QT_TR_NOOP("Close all Contests"), ui->menuFile, &TLogContainer::CloseAllActionExecute);
    CloseAllButAction = newAction(QT_TR_NOOP("Close all but this Contest"), ui->menuFile, &TLogContainer::CloseAllButActionExecute);

    ui->menuFile->addSeparator();
    ContestDetailsAction = newAction(QT_TR_NOOP("Contest Details..."), ui->menuFile, &TLogContainer::ContestDetailsActionExecute);
    MakeEntryAction = newAction(QT_TR_NOOP("Produce Entry/Export File..."), ui->menuFile, &TLogContainer::MakeEntryActionExecute);
    StatsAction = newAction(QT_TR_NOOP("Show Contest Statistics..."), ui->menuFile, &TLogContainer::StatsActionExecute);
    ui->menuFile->addSeparator();

    AppendAdifAction = newAction(QT_TR_NOOP("Manage ADIF files"), ui->menuFile, &TLogContainer::ManageAdifActionExecute);
    ui->menuFile->addSeparator();

    ListOpenAction = newAction(QT_TR_NOOP("Open &Archive List..."), ui->menuFile, &TLogContainer::ListOpenActionExecute);
    ManageListsAction = newAction(QT_TR_NOOP("&Manage Archive Lists..."), ui->menuFile, &TLogContainer::ManageListsActionExecute);
    ui->menuFile->addSeparator();

    ui->menuFile->addSeparator();
#ifdef Q_OS_WIN
    ExitClearAction = newAction(QT_TR_NOOP("E&xit Minos Contest Logger and Clear registry..."), ui->menuFile, &TLogContainer::ExitClearActionExecute, QAction::ApplicationSpecificRole);
#endif
    ui->menuFile->addSeparator();
    ExitAction = newAction(QT_TR_NOOP("E&xit Minos Contest Logger"), ui->menuFile, &TLogContainer::ExitActionExecute, QAction::QuitRole);
// end of file menu

    GoToSerialAction = newAction(QT_TR_NOOP("&Go To Contact Serial..."), ui->menuSearch, &TLogContainer::GoToSerialActionExecute);
    NextUnfilledAction = newAction(QT_TR_NOOP("Goto First Unfilled Contact"), ui->menuSearch, &TLogContainer::NextUnfilledActionExecute);
// end of search menu

    startConfigAction = newAction(QT_TR_NOOP("Startup Apps Configuration..."), ui->menuTools, &TLogContainer::ManageAppConfigsActionExecute);

    screenLayoutMenu = newMenu(ui->menuTools, QT_TR_NOOP("Screen Layouts"));
    updateLayoutsMenu();
    ui->menuTools->addSeparator();
    LocCalcAction = newAction(QT_TR_NOOP("Locator Calculator..."), ui->menuTools, &TLogContainer::LocCalcActionExecute);

    CorrectDateTimeAction = newAction(QT_TR_NOOP("Correct Date/Time..."), ui->menuTools, &TLogContainer::CorrectDateTimeActionExecute);
#ifdef Q_OS_WIN
    ManageHamlibAction = newAction(QT_TR_NOOP("Manage Hamlib..."), ui->menuTools, &TLogContainer::ManageHamlibActionExecute);
#endif
    manageSpotDatabaseAction = newAction(QT_TR_NOOP("Manage Bandmap Spots Database..."), ui->menuTools, &TLogContainer::on_manageSpotsDatabaseActionSelected);
    DownloadFilesAction = newAction(QT_TR_NOOP("Download Latest Calendar"), ui->menuTools, &TLogContainer::on_downloadFilesActionSelected);

    ui->menuTools->addSeparator();

    OptionsAction = newAction(QT_TR_NOOP("Options..."), ui->menuTools, &TLogContainer::OptionsActionExecute, QAction::PreferencesRole);

    AdvancedOptionsAction = newAction(QT_TR_NOOP("Advanced Options..."), ui->menuTools, &TLogContainer::AdvancedOptionsActionExecute);
    AdvancedOptionsAction->setVisible(false);

#ifndef NDEBUG
    // until it works, don't show it!

    // EnterAction = newAction(QT_TR_NOOP("Create Entry and send to RSGB"), ui->menuTools, &TLogContainer::EnterActionExecute);
#endif

    // end of tools manu

    setMemoryAction = newMemoryAction(QT_TR_NOOP("Add as new memory..."), &TabPopup, &TLogContainer::onSetMemoryActionExecute);

    TabPopup.addAction(FileOpenAction);
    TabPopup.addAction(FileImportVHFAction);
    if (FileImportHFAction)
    {
        TabPopup.addAction(FileImportHFAction);
    }
    TabPopup.addMenu(recentFilesMenu);
    TabPopup.addAction(VHFFileNewAction);
    if (HFFileNewAction)
    {
        TabPopup.addAction(HFFileNewAction);
    }
    TabPopup.addAction(FileCloseAction);
    TabPopup.addAction(CloseAllAction);
    TabPopup.addAction(CloseAllButAction);
    TabPopup.addSeparator();

    TabPopup.addAction(ContestDetailsAction);
    TabPopup.addAction(MakeEntryAction);
    TabPopup.addAction(StatsAction);
    TabPopup.addSeparator();

    TabPopup.addAction(AppendAdifAction);
    TabPopup.addSeparator();

    TabPopup.addAction(GoToSerialAction);
    TabPopup.addAction(NextUnfilledAction);
    TabPopup.addSeparator();

    ShiftTabLeftAction = newAction(QT_TR_NOOP("Shift Active Tab Left"), &TabPopup, &TLogContainer::ShiftTabLeftActionExecute);
    ShiftTabRightAction = newAction(QT_TR_NOOP("Shift Active Tab Right"), &TabPopup, &TLogContainer::ShiftTabRightActionExecute);
    TabPopup.addAction(CorrectDateTimeAction);
    TabPopup.addSeparator();

    TabPopup.addAction(OptionsAction);

    //TabPopup.addAction(AnalyseMinosLogAction);
    newAction( QT_TR_NOOP("Cancel"), &TabPopup, &TLogContainer::CancelClick);


    HelpAction = newAction(QT_TR_NOOP("Help..."), ui->menuHelp, &TLogContainer::HelpActionExecute);
    CheckUpdatesAction = newAction(QT_TR_NOOP("Check For Updates..."), ui->menuHelp, &TLogContainer::CheckUpdatesActionExecute);
    HelpAboutAction = newAction(QT_TR_NOOP("About..."), ui->menuHelp, &TLogContainer::HelpAboutActionExecute, QAction::AboutRole);
}
void TLogContainer::clearMenus()
{
    for(QMap<QMenu *, const char *>::iterator i = menuList.begin(); i != menuList.end(); i++)
    {
        i.key()->deleteLater();
    }
    for(QMap<QAction *, const char *>::iterator i = actionList.begin(); i != actionList.end(); i++)
    {
        i.key()->deleteLater();
    }
    for(QVector<QAction *>::iterator i = recentFileActs.begin(); i != recentFileActs.end(); i++)
    {
        (*i)->deleteLater();
    }
    for(QVector<QAction *>::iterator i = sessionActs.begin(); i != sessionActs.end(); i++)
    {
        (*i)->deleteLater();
    }
    ui->menuLogs->clear();
    menuLogsActions.clear();

    menuList.clear();
    actionList.clear();
    recentFileActs.clear();
    sessionActs.clear();

}

void TLogContainer::enableActions()
{
   bool f = ( ui->contestPageControl->currentIndex() >= 0 );

   LocCalcAction->setEnabled(true);
   VHFFileNewAction->setEnabled(true);
   if (HFFileNewAction)
   {
        HFFileNewAction->setEnabled(true);
   }
   HelpAction->setEnabled(true);
   HelpAboutAction->setEnabled(true);

   FileCloseAction->setEnabled(f);
   CloseAllAction->setEnabled(f);
   CloseAllButAction->setEnabled(f);

   ContestDetailsAction->setEnabled(f);
   StatsAction->setEnabled(f);
   GoToSerialAction->setEnabled(f);
   NextUnfilledAction->setEnabled(f);
   MakeEntryAction->setEnabled(f);
   AppendAdifAction->setEnabled(f);

   if ( ui->contestPageControl->currentIndex() >= 0 )
   {
      int tno = ui->contestPageControl->currentIndex();
      ShiftTabLeftAction->setEnabled( tno > 0 );
      ShiftTabRightAction->setEnabled( tno < ui->contestPageControl->count() - 1 );
   }
   else
   {
      ShiftTabLeftAction->setEnabled(false);
      ShiftTabRightAction->setEnabled(false);
   }

}

bool TLogContainer::isShowOperators()
{
   bool ncdol;
   TContestApp::getContestApp() ->displayBundle.getBoolProfile( edpShowOperators, ncdol );
   return ncdol;
}

void TLogContainer::CancelClick()
{
    // do nothing...
}

void TLogContainer::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        QString FileName = action->data().toString();

        if ( FileExists( FileName ) )
        {
           setCurrentFile(FileName);
           ContestDetails pced( this );
           BaseContestLog *ct = addSlot( &pced, FileName, false, -1, false );
           if (ct)
           {
              sendDM->subscribeApps();
              selectContest(ct);
           }
        }
        else
        {
            removeCurrentFile( FileName );
        }
    }
}

void TLogContainer::setCurrentFile(const QString &fileName)
{
    QString f2 = GetCleanPath(fileName);
    RegSettings settings;
    QStringList files = settings.getSettings().value("dbmru").toStringList();
    for (auto &s:files)
    {
        s = GetCleanPath(s);
    }
    files.removeAll(f2);
    files.prepend(f2);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.getSettings().setValue("dbmru", files);

    updateRecentFileActions();

}
void TLogContainer::removeCurrentFile(const QString &fileName)
{
    QString f2 = GetCleanPath(fileName);
    RegSettings settings;
    QStringList files = settings.getSettings().value("dbmru").toStringList();
    for (auto &s:files)
    {
        s = GetCleanPath(s);
    }
    files.removeAll(f2);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.getSettings().setValue("dbmru", files);

    updateRecentFileActions();

}
void TLogContainer::updateRecentFileActions()
{
    RegSettings settings;
    QStringList files = settings.getSettings().value("dbmru").toStringList();

    int numRecentFiles = qMin(files.size(), static_cast< int >(MaxRecentFiles));

    for (int i = 0; i < numRecentFiles; ++i)
    {
        QString text = QString("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    recentFilesMenu->setEnabled(numRecentFiles > 0);
}

QString TLogContainer::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void TLogContainer::HelpAboutActionExecute()
{
    TAboutBox::ShowAboutBox(this, false);
    // in case we are now running more apps
    sendDM->subscribeApps();
}
void TLogContainer::HelpActionExecute()
{
    //  Action method for Help button.

    //  Brings up PDF manual.

    QString pdfFile;
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpPDFFile, pdfFile );
    if (FileExists(pdfFile))
    {
        QUrl url = QUrl().fromLocalFile(pdfFile);
        QDesktopServices::openUrl(url);
    }
    else
    {
        mShowMessage(tr("Documentation File %1 doesn't exist.").arg(pdfFile), this);
    }

}
QString TLogContainer::getDefaultDirectory( bool IsList )
{
   QString fileName;
   if ( IsList )
   {
      TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpListDirectory, fileName );

   }
   else
   {
      TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpLogDirectory, fileName );
      if (!DirectoryExists( fileName ))
      {
          CreateDir(fileName);
      }
   }

   // we default to "./logs" or "./lists. On Vista this won't be relative to
   // the executable, but to the user data directory (I think). This is why Minos
   // invents a very strange default directory which you then cannot find.

   if ( fileName.size() && DirectoryExists( fileName ) )
   {
      // need to check possible validity of directory string
      if ( fileName[ fileName.size() - 1 ] != '/' )
         fileName += "/";
      if ( fileName[ fileName.size() - 1 ] == '/' )
      {
         fileName = fileName.left( fileName.size() - 1 );
      }
   }
   else
      fileName = "";

   return fileName;
}
void TLogContainer::onSetMemoryActionExecute()
{
    // look in setMemoryaction

    emit MinosLoggerEvents::SendSetMemory(setMemoryAction->ct, setMemoryAction->call, setMemoryAction->loc);
}
void TLogContainer::FileNewActionExecute(bool hf)
{
    QString InitialDir = getDirectoryLocation(dlLogs);

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();

    QString creationDir = InitialDir;

    // generate a default filename for a new contest
    QString nfileName( "C");

    QDate d = QDate::currentDate();      // get time now
    nfileName += d.toString("ddMMyy");

    char letter = 'A';
    while ( letter < 'Z' )      // the A of A.Minos
    {
       QString fileNameBuff = creationDir + "/" + nfileName + letter + ".minos";

       if (FileExists(fileNameBuff))
       {
           letter ++;
       }
       else
          break;
    }

    QString initName = creationDir + "/" + nfileName + letter + ".minos";
    ContestDetails pced( this );
    BaseContestLog * c = addSlot( &pced, initName, true, -1, hf );

    if (!c)
    {
       if ( !QFile::remove( initName ) )
       {
          MinosParameters::getMinosParameters() ->mshowMessage( tr( "Failed to delete %1" ).arg( initName) );
       }
       return;
    }
    bool repeatDialog = true;
   QString suggestedfName;
   suggestedfName = ( c->mycall.realCall );
   suggestedfName += '_';
   if ( c->DTGStart.getValue().size() )
   {
      suggestedfName += CanonicalToTDT( c->DTGStart.getValue() ).toString( "yyyy_MM_dd" );
   }
   else
   {
      suggestedfName += QDate::currentDate().toString( "yyyy_MM_dd" );
   }
   QString band = c->contestBands.getValue();
   if (band == allHF)
   {
       band = tr("All HF");
   }
   if ( band.size() )
   {
      suggestedfName += '_';
      suggestedfName += band.replace('-', '_').replace('/','_').replace(' ','_');
   }
   QString nameBase = suggestedfName;
   int fnum = 1;
   if (FileExists(InitialDir + "/" + nameBase + ".minos"))
   {
       while (FileExists(InitialDir + "/" + nameBase + "_" + QString::number(fnum) + ".minos"))
       {
           if (fnum == 9)
               break;
           fnum++;
       }
       suggestedfName = nameBase + "_" + QString::number(fnum);
   }
   suggestedfName += ".minos";

   // close the slot - we will re-open it later under the new name
   closeSlot(ui->contestPageControl->currentIndex(), false );

   while ( repeatDialog )
   {
       QString fileName = QFileDialog::getSaveFileName( this,
                          tr("Save new contest as"),
                          InitialDir + "/" + suggestedfName,
                          tr("Minos contest files %1").arg( "(*.minos *.Minos)"),
                          nullptr,
                          QFileDialog::DontConfirmOverwrite
                                                      );
       if ( !fileName.isEmpty() )
       {
           if (FileExists(fileName) )
           {
               MinosParameters::getMinosParameters() ->mshowMessage( tr("%1 \nalready exists.\n\nPlease choose a new name.").arg(fileName) );

               InitialDir = ExtractFileDir(fileName);
               QString sfname = InitialDir + nameBase + "_" + QString::number(fnum) + ".minos";
               while (FileExists(sfname))
               {
                   if (fnum == 9)
                       break;
                   fnum++;
                   sfname = InitialDir + nameBase + "_" + QString::number(fnum) + ".minos";
               }
               suggestedfName = nameBase + "_" + QString::number(fnum);
               suggestedfName += ".minos";
               continue;
           }
          suggestedfName = fileName;
          QDir r(creationDir);
          if ( !r.rename( initName, suggestedfName ) )
          {
             MinosParameters::getMinosParameters() ->mshowMessage( tr( "Failed to rename\n%1\n as \n%2\n\nPlease choose a new name.").arg(initName, suggestedfName) );
             suggestedfName = initName;
          }

          // we want to (re)open it WITHOUT using the dialog!
          addSlot( nullptr, suggestedfName, false, -1, hf );
          repeatDialog = false;
       }
       else
       {
            repeatDialog = false;   // never go back to the dialog
            if ( !QFile::remove( initName ) )
            {
                MinosParameters::getMinosParameters() ->mshowMessage( tr( "Failed to delete %1" ).arg(initName) );
            }
       }
    }
    sendDM->subscribeApps();
    selectContest(c);
}
void TLogContainer::VHFFileNewActionExecute()
{
    trace(QString("%1 entered").arg(__func__));
    FileNewActionExecute(false);
}
void TLogContainer::HFFileNewActionExecute()
{
    trace(QString("%1 entered").arg(__func__));
    FileNewActionExecute(true);
}

void TLogContainer::FileOpenActionExecute()
{
    trace(QString("%1 entered").arg(__func__));
    // first choose file
//"Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"
    QString InitialDir = getDirectoryLocation(dlLogs);

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();

    QString Filter = tr("Minos contest files (*.minos *.Minos);;"
                     "All Files (*.*)") ;

    QStringList fnames = QFileDialog::getOpenFileNames( this,
                       tr("Open contests"),
                       InitialDir,  // dir
                       Filter
                       );
    for (auto const &fname: QASCONST(fnames))
    {
        BaseContestLog *ct = nullptr;
        if ( !fname.isEmpty() )
        {
            ContestDetails pced(this );
            ct = addSlot( &pced, fname, false, -1, false );   // not automatically read only
            if (ct)
            {
                sendDM->subscribeApps();
                selectContest(ct);
            }
        }
    }
}
void TLogContainer::FileImportVHFActionExecute()
{
    trace(QString("%1 entered").arg(__func__));
    FileImportActionExecute(false);
}
void TLogContainer::FileImportHFActionExecute()
{
    trace(QString("%1 entered").arg(__func__));
    FileImportActionExecute(true);
}

void TLogContainer::FileImportActionExecute(bool hf)
{
    // first choose file
//"Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"
    QString InitialDir = getDirectoryLocation(dlLogs);

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();

    QString Filter = tr("Use this combo for file types (*.*);;"
                     "Reg1Test Files (*.edi);;"
                     "GJV contest files (*.gjv);;"
                     "RSGB Log Files (*.log);;"
                     "ADIF Files (*.adi);;"
                     "All Files (*.*)") ;

    QStringList fnames = QFileDialog::getOpenFileNames( this,
                       tr("Import %1 contests").arg(hf?"HF":"VHF"),
                       InitialDir,  // dir
                       Filter
                       );
    for (int i = 0; i < fnames.size(); i++)
    {
        QString fname = fnames[i];
        BaseContestLog *ct = nullptr;
        if ( !fname.isEmpty() )
        {
            trace(QString("about to import %1 as %2").arg(fname, (hf?"HF":"VHF")));
            ContestDetails pced(this );
            ct = addSlot( &pced, fname, false, -1, hf );   // not automatically read only
            if (ct)
            {
                sendDM->subscribeApps();
                selectContest(ct);
            }
        }
    }
}


void TLogContainer::ContestDetailsActionExecute()
{
    trace(QString("%1 entered").arg(__func__));
    QWidget *tw = ui->contestPageControl->currentWidget();
    TSingleLogFrame *f = dynamic_cast<TSingleLogFrame *>( tw );

    if (f)
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( f->getContest());


        if (ct)
        {
            QString curConfig = ct->screenLayout.getValue();
            MinosItem<QString> bandsList = ct->bandsList;
            ContestDetails pced( this );


            pced.setDetails( ct );
            if ( pced.exec() == QDialog::Accepted )
            {
                if (ct->screenLayout.getValue() != curConfig || ct->bandsList != bandsList)
                {
                    f->applyScreenLayout();
                }

                sendDM->subscribeApps();

                f->FKHRigControlFrame->setContest(ct);
                f->bandmapControlFrame->setContest(ct);

                MinosLoggerEvents::SendContestBandChanged(ct);  // in case it has...
                f->FKHRigControlFrame->rigChangedFromDetails();
                f->FKHRotControlFrame->on_ContestPageChanged();
                f->FKHRotCompassFrame->on_ContestPageChanged();
                // and we need to do some re-init on the display
                f->updateQSODisplay();
                ct->scanContest();      // if contest details have changed, required
                f->refreshMults();

                updateLayoutsMenu();
            }

        }
    }
}
//---------------------------------------------------------------------------

void TLogContainer::FileCloseActionExecute()
{
    trace(QString("%1 entered").arg(__func__));
   int t = ui->contestPageControl->currentIndex();
   closeSlot(t, true);
}
void TLogContainer::onTabClosebutton(int t)
{
    trace(QString("%1 entered").arg(__func__));
    closeSlot(t, true);
}
//---------------------------------------------------------------------------

void TLogContainer::CloseAllActionExecute()
{
    trace(QString("%1 entered").arg(__func__));

    QWidget *thisContest = ui->contestPageControl->currentWidget();
    while ( ui->contestPageControl->count() > 1)
    {
        int t = ui->contestPageControl->count() - 1;
        QWidget *ctab = ui->contestPageControl->widget(t);
        if (ctab == thisContest)
        {
            t -= 1;
        }
        closeSlot(t, true );
    }
    closeSlot(0, true);
    on_contestPageControl_currentChanged(-1);
    for(auto cpc: QASCONST(contestPageControls))
    {
        if (cpc && cpc->getInstance() > 0)
        {
            cpc->close();
            cpc = nullptr;
        }
    }

    enableActions();
}
//---------------------------------------------------------------------------

void TLogContainer::CloseAllButActionExecute()
{
    trace(QString("%1 entered").arg(__func__));
    if (!mShowYesNoMessage(this, tr("Do you want to close all but the current contest?") ))
    {
        return;
    }

   QWidget *thisContest = ui->contestPageControl->currentWidget();
   while ( ui->contestPageControl->count() > 1)
   {
       int t = ui->contestPageControl->count() - 1;
      QWidget *ctab = ui->contestPageControl->widget(t);
      if (ctab == thisContest)
      {
         t -= 1;
      }
      closeSlot(t, true );
   }
   on_contestPageControl_currentChanged(-1);
   enableActions();
}
void TLogContainer::OptionsActionExecute()
{
    OptionsDialog od;

    if (od.exec() == QDialog::Accepted)
    {
       // This is a somwhat clumsy method...
        openSerialTVSwitch();
    }
}
void TLogContainer::AdvancedOptionsActionExecute()
{
    // not exposed - everything useful should be on the normal options menu
    TSettingsEditDlg ed(this, &TContestApp::getContestApp() ->loggerBundle );

    ed.ShowCurrentSectionOnly();
    if (ed.exec() == QDialog::Accepted)
    {
       mShowMessage(tr("You may need to close and reload Minos to have these settings applied"), this);
    }
}

void TLogContainer::CheckUpdatesActionExecute()
{
    CheckUpdates cu(this);
    cu.exec();
}

//---------------------------------------------------------------------------

void TLogContainer::ExitActionExecute()
{
    trace("ExitActionExecute");
    close();
}
void TLogContainer::ExitClearActionExecute()
{
    // Confirm...

#ifdef Q_OS_WIN
    if (!mShowYesNoMessage(this, tr("This action will clear geometry entries for all of the apps within Minos.\n\n"
                                  "Please confirm this action by pressing \"Yes\".") ))
    {
       return;
    }

    MinosConfig::getMinosConfig() ->askStop();
    MinosConfig::getMinosConfig() ->forceStop();
    SingleApplication *sa = dynamic_cast<SingleApplication *>(QApplication::instance());

    connect(sa, &SingleApplication::aboutToQuit, sa, &SingleApplication::clearRegistry);

    close();
#else
    mShowMessage(tr("Clear registry only works under Windows"), this);
#endif
}
void TLogContainer::ManageAdifActionExecute()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();

    if (!ct)
        return;

    ManageAdifDialog mad;

    mad.exec();
}
void TLogContainer::EnterActionExecute()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
    MinosLoggerEvents::SendMakeEntry(ct, true);
}

void TLogContainer::MakeEntryActionExecute()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
    MinosLoggerEvents::SendMakeEntry(ct, false);
}
void TLogContainer::StatsActionExecute()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
    StatisticsDisplay  sd(ct, this);
    sd.exec();
}
void TLogContainer::LocCalcActionExecute()
{
    TLocCalcForm loccalc( this );
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
    if (ct)
    {
       loccalc.S1Loc = ct->myloc.getLoc();
    }
    loccalc.exec();
}
void TLogContainer::AnalyseMinosLogActionExecute()
{
    QString InitialDir = getDirectoryLocation(dlLogs);

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();

    QString Filter = tr("Minos contest files (*.minos *.Minos);;"
                     "All Files (*.*)") ;

    QString fname = QFileDialog::getOpenFileName( this,
                       tr("Open contest for Analysis"),
                       InitialDir,  // dir
                       Filter
                       );

    QIODevice::OpenMode om = QIODevice::ReadOnly;

    QSharedPointer<QFile> contestFile(new QFile(fname));

    if (!contestFile->open(om))
    {
       QString lerr = contestFile->errorString();
       QString emess = tr("Failed to open Contest Log file %1 : %2").arg(fname, lerr);
       MinosParameters::getMinosParameters() ->mshowMessage( emess );
       return;
    }

    MinosTestImport mt;
    mt.analyseTest( contestFile );

    MinosParameters::getMinosParameters() ->mshowMessage( tr("Analysis of %1 complete; look in the trace log for analysis.").arg(fname) );
}

void TLogContainer::CorrectDateTimeActionExecute()
{
    TClockDlg cdlg(this);
    cdlg.exec();
}

void TLogContainer::ManageHamlibActionExecute()
{
    ManageHamlib mhl(this);
    mhl.exec();
}
void TLogContainer::on_manageSpotsDatabaseActionSelected()
{
    ManageBandmapSpotsDb mbsd(this);
    mbsd.exec();
}
void TLogContainer::on_downloadFilesActionSelected()
{
    TCalendarDownload dl(this);
    dl.exec();
}
void TLogContainer::GoToSerialActionExecute()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
    MinosLoggerEvents::SendGoToSerial(ct);
}

void TLogContainer::NextUnfilledActionExecute()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
    MinosLoggerEvents::SendNextUnfilled(ct);
}

void TLogContainer::menuLogsActionExecute(bool)
{
    QAction *qa = qobject_cast<QAction *>(sender());
    if (qa)
    {
        int i = qa->data().toInt();
        selectTab(i);
    }
}

void TLogContainer::doScreenConfigAction()
{
    QWidget *tw = ui->contestPageControl->currentWidget();
    TSingleLogFrame *f = dynamic_cast<TSingleLogFrame *>( tw );

    QString cur = f->getCurScreenLayout();
    QString def;
    QString prot;

    MinosParameters::getMinosParameters() -> getStringDisplayProfile( edpDefaultLayout, def );
    MinosParameters::getMinosParameters() -> getStringDisplayProfile( edpProtectedLayout, prot );

    ScreenConfigManager sc(this, cur, def, prot);
    connect(&sc, &ScreenConfigManager::screenConfigApply, this, &TLogContainer::onScreenConfigApply);
    connect(&sc, &ScreenConfigManager::setDefaultName, this, &TLogContainer::onSetDefaultName);
    connect(&sc, &ScreenConfigManager::setProtectedName, this, &TLogContainer::onSetProtectedName);

    sc.exec();
    updateLayoutsMenu();
}
void TLogContainer::onScreenConfigApply(QString curConfigName)
{
    selectLayout(curConfigName);
    selectSession(TContestApp::getContestApp()->currSession);
}
void TLogContainer::onSetDefaultName(QString def)
{
    MinosParameters::getMinosParameters() -> setStringDisplayProfile( edpDefaultLayout, def );
}
void TLogContainer::onSetProtectedName(QString prot)
{
    MinosParameters::getMinosParameters() -> setStringDisplayProfile( edpProtectedLayout, prot );
}

void TLogContainer::ManageAppConfigsActionExecute()
{
    StartConfigManager manageApps( this, true);   // when managing sets, include autostart
    manageApps.exec();
    // in case we are now running more apps
    sendDM->subscribeApps();
}
void TLogContainer::setMenuLog(int current)
{
    // why doesn't this happen at startup?

    ui->menuLogs->clear();

    ui->menuLogs->addAction(FileOpenAction);
    ui->menuLogs->addMenu(recentFilesMenu);
    ui->menuLogs->addAction(VHFFileNewAction);
    if (HFFileNewAction)
    {
        ui->menuLogs->addAction(HFFileNewAction);
    }
    ui->menuLogs->addAction(FileCloseAction);
    ui->menuLogs->addAction(CloseAllAction);
    ui->menuLogs->addAction(CloseAllButAction);
    ui->menuLogs->addSeparator();

    // add the currently open contests - but don't add to the actions
    for (int i = 0; i < ui->contestPageControl->count(); i++)
    {
        QAction * newAct = new QAction( ui->contestPageControl->tabText(i), this );
        newAct->setCheckable( true );
        ui->menuLogs->addAction( newAct );
        connect( newAct, &QAction::triggered, this, &TLogContainer::menuLogsActionExecute );

        QVariant qpc(i);
        newAct->setData(qpc);

        if (current == i)
        {
            newAct->setChecked(true);
        }
    }

    // update the list of contest sets
    sessionsMenu = ui->menuLogs->addMenu(tr("Contest Sets"));
    updateSessionActions();

    updateRecentFileActions();

}
void TLogContainer::on_contestPageControl_currentChanged(int index)
{
    trace(QString("TLogContainer::on_contestPageControl_currentChanged index %1").arg(index));
    if (loggerClosing)
    {
        return;
    }
    enableActions();

    TContestApp::getContestApp() ->writeContestList();
    enableActions();

    updateLayoutsMenu();

    ui->menuLogs->clear();
    menuLogsActions.clear();

    if (index >= 0)
    {
        MinosLoggerEvents::SendContestPageChanged();
    }

    TSingleLogFrame *tslf = getCurrentLogFrame();
    int tab = ui->contestPageControl->indexOf(tslf);
    setMenuLog(tab);

    if (tslf)
    {
        static QString oldContestBand = "";
        QString contestBand = tslf->getContest()->currentBand.getValue();

        if (oldContestBand != contestBand)
        {
            oldContestBand = contestBand;
            QByteArray msg;

            if (readEnableBandSwitchFromIni())
            {
                trace(QString("send no rigcontrol bandswitch message, enabled"));
                // send bandswitch data to control
                msg = readBandSwitchDataFromIni(contestBand).toUtf8();
                if ( /*readEnableSerialBandSwitchFromIni() &&*/ serialTVSw->getOpenFlag())
                {
                    trace(QString("send no rigcontrol bandswitch message - serial port is open"));
                    if (!msg.isEmpty())
                    {
                        msg.prepend(TVSWMSG_START);
                        msg.append(TVSWMSG_TERM);
                        serialTVSw->sendTVSwMessage(msg);
                        trace(QString("%1LF send to bandswitch serial port").arg(QString(msg).remove('\n')));
                    }
                    else
                    {
                        trace(QString("bandswitch data is empty for band %1").arg(contestBand));
                    }
                }
                else
                {
                    trace(QString("send no rigcontrol bandswitch message - serial port is not open"));
                }
            }
        }
    }



}

void TLogContainer::selectTab(int curTab)
{
    if (curTab >= 0)
    {
        QWidget *ctab = ui->contestPageControl->widget(curTab);
        TSingleLogFrame * f = dynamic_cast<TSingleLogFrame *>( ctab );
        BaseContestLog *pc = f->getContest();
        selectContest(pc);
    }

}
BaseContestLog * TLogContainer::addSlot(ContestDetails *ced, const QString &fname, bool newfile, int slotno, bool hf )
{
    QString m;

    m += newfile?tr("Creating "):tr("Loading ");

    m += tr("Contest file ");
    m += fname;

    sblabel0->setText( m );
    repaint();

   static int namegen = 0;
   // openFile ends up calling ContestLog::initialise which then
   // calls TContestApp::insertContest

   LoggerContestLog * contest = TContestApp::getContestApp() ->openFile( fname, newfile, slotno, hf );

   if ( contest )
   {
      bool show = false;
      if ( ced )
      {
         ced->setDetails( contest );

         {
            if ( ced->exec() == QDialog::Accepted )
            {
               show = true;
            }
            else
            {
               TContestApp::getContestApp() ->closeFile( contest );
               contest = nullptr;
               show = false;
            }
         }
      }
      else
      {
         show = true;
      }

      if ( show )
      {
         TContestApp::getContestApp() ->setCurrentContest( contest );
         contest->scanContest();    // contest initially opened (addSlot) required
         QString baseFName = ExtractFileName( contest->cfileName );
         TSingleLogFrame *f = new TSingleLogFrame( this, contest );

         setUpdatesEnabled(false);
         f->buildFrame(slotno);

         f->setObjectName( QString( "LogFrame" ) + QString::number(namegen++));

         int tno = ui->contestPageControl->addTab(f, baseFName);

         ui->contestPageControl->setCurrentWidget(ui->contestPageControl->widget(tno));

         MinosLoggerEvents::SendColumnsChanged();  // also causes show QSOs
         MinosLoggerEvents::SendSplittersChanged();

         on_contestPageControl_currentChanged(tno);

         setUpdatesEnabled(true);

         if ( contest->needsExport() )      // imported from an alien format (e.g. .log)
         {
            QString expName = f->makeEntry( true, false );
            if ( expName.size() )
            {
               closeSlot(tno, true );
               addSlot( nullptr, expName, false, -1, false );
            }
         }
         else
         {
             f->addAllQSOsToBandmap();
         }
         removeCurrentFile( fname );
      }
   }
   TContestApp::getContestApp() ->writeContestList();
   enableActions();

   return contest;
}
TSingleLogFrame *TLogContainer::getCurrentLogFrame()
{
    QWidget *w = ui->contestPageControl->currentWidget();
    TSingleLogFrame *f = dynamic_cast<TSingleLogFrame *>(w);
    return f;
}

void TLogContainer::closeSlot(int t, bool addToMRU)
{
   if ( t >= 0 )
   {
      TSingleLogFrame * f = findLogFrame(t);

      if (f)
      {
          f->GJVQSOLogFrame->killPartial(); // seems to remove possible crash

          BaseContestLog *bct = f->getContest();
          if ( bct && addToMRU )
          {
             QString curPath = bct->cfileName;
             setCurrentFile( curPath );
          }

          // clear down matching, as it may have pointers to this contest
          TMatchThread::FinishMatchThread();
          f->closeContest();    // which should close the contest - and takes TSingleLogFrame out of pages
          if (!loggerClosing)
          {
            TMatchThread::InitialiseMatchThread();
          }

          for(auto cpc: QASCONST(contestPageControls))
          {
              // This deletes TSingleLogFrame first, along
              // with all of the screen components
              if (cpc)
              {
                  auto page = cpc->pages.find(bct);
                  if (page != cpc->pages.end())
                  {
                      ContestPage *cp = (*page);
                      cpc->pages.remove(bct);
                      if (cp)
                      {
                          cp->deleteLater();
                          cpc->removeTab(cpc->indexOf(cp));
                      }
                  }
              }
          }
          on_contestPageControl_currentChanged(-1);
      }
      enableActions();
   }
}
TSingleLogFrame *TLogContainer::findLogFrame(int t)
{
    // we need to find the embedded frame...
    // now ONLY used in closeSlot!
    if ( t < 0 )
        return nullptr;
    QWidget *tw = ui->contestPageControl->widget(t);
    if ( TSingleLogFrame * f = dynamic_cast<TSingleLogFrame *>( tw ))
    {
        return f;
    }
    return nullptr;
}

QStringList TLogContainer::getSessions()
{
    TContestApp *app = TContestApp::getContestApp();
    QStringList sessionlst = app ->logsPreloadBundle.getSections();
    QStringList newSessionList;
    sessionlst.sort();
    for (auto const &s: QASCONST(sessionlst))
    {
        if (s != app ->logsPreloadBundle.noneBundle && s!= app->preloadsect)
        {
            newSessionList.append(s);
        }
    }
    return newSessionList;
}
void TLogContainer::updateLayoutsMenu()
{
    screenLayoutMenu->clear();
    ScreenConfigAction = newAction(QT_TR_NOOP("Configure Screen Layouts..."), screenLayoutMenu, &TLogContainer::doScreenConfigAction);

    screenLayoutMenu->addSeparator();

    QWidget *tw = ui->contestPageControl->currentWidget();
    TSingleLogFrame *f = dynamic_cast<TSingleLogFrame *>( tw );
    if (f)
    {
        QString currentLayout = f->getCurScreenLayout();
        QString defaultLayout;
        MinosParameters::getMinosParameters() -> getStringDisplayProfile( edpDefaultLayout, defaultLayout );
        QString protectedConfigName;
        MinosParameters::getMinosParameters() -> getStringDisplayProfile( edpProtectedLayout, protectedConfigName );

        ScreenConfigFile &scf = ScreenConfigFile::getScreenConfigFile(this);

        for(auto const &c: QASCONST(scf.configs ))
        {
            QAction *act =  new QAction(this);
            if (c.name == defaultLayout)
            {
                act->setText(c.name + " " + ScreenConfigManager::tr(ScreenConfigManager::defLayoutText));
            }
            else if (c.name == protectedConfigName)
            {
                act->setText(c.name + " " + ScreenConfigManager::tr(ScreenConfigManager::protectedLayoutText));
            }
            else
            {
                act->setText(c.name);
            }
            connect(act, &QAction::triggered, this, &TLogContainer::selectLayoutAction);
            act->setCheckable(true);

            screenLayoutMenu->addAction(act);

            if (c.name == currentLayout)
            {
                act->setChecked(true);
                lastLayoutSelected = act;
            }
        }
    }
}

void TLogContainer::selectLayoutAction()
{
    TWaitCursor wc(this);
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
        if (lastLayoutSelected)
            lastLayoutSelected->setChecked(false);
        action->setChecked(true);
        lastLayoutSelected = action;
        QString selText = action->text();
        selText = ScreenConfigManager::stripDefaultDecoration(selText);
        selectLayout(selText);

        // The action of changing layouts closes and re-loads the frame, so
        // it appears as though current has switched, so we have to switch back.

        // we are going to have to re-order the tabs as well - but I don't want to redo them all!

        selectContest( ct );
        TContestApp::getContestApp() ->setCurrentContest(ct);
    }
}
void TLogContainer::selectLayout(QString layout)
{
    QWidget *tw = ui->contestPageControl->currentWidget();
    TSingleLogFrame *f = dynamic_cast<TSingleLogFrame *>( tw );
    if (f)
    {
        f->setCurScreenLayout(layout);
        f->applyScreenLayout();
        updateLayoutsMenu();
        f->QSOListFrame->restoreQSOTableColumns();
    }
}
void TLogContainer::updateSessionActions()
{
    TContestApp *app = TContestApp::getContestApp();
    SettingsBundle &preloadBundle = app ->logsPreloadBundle;

    preloadBundle.openSection(getCurrSession());

    sessionsMenu->clear();

    sessionManagerAction  = newAction(QT_TR_NOOP("&Manage Contest Sets..."), sessionsMenu, &TLogContainer::sessionManageExecute);
    QStringList sessionlst = getSessions();
    for (auto const &s: QASCONST(sessionlst))
    {
        if (s == app->preloadsect)
        {
            continue;
        }
        QAction *act =  new QAction(this);
        act->setText(s);
        connect(act, &QAction::triggered, this, &TLogContainer::selectSessionAction);
        act->setCheckable(true);
        if (s == app->currSession)
        {
            if (lastSessionSelected)
                lastSessionSelected->setChecked(false);
            act->setChecked(true);
            lastSessionSelected = act;
        }

        sessionsMenu->addAction(act);
        sessionActs.push_back( act);
    }
}
void TLogContainer::sessionManageExecute()
{
    // present list of sessions, with contents (L/R panes)
    // allow select, remove on LH
    // and select as open, current, remove, re-order on RH

    // open new session, move contest between sessions

    TSessionManager tsm(this);
    if (tsm.exec() == QDialog::Accepted)
    {
        TWaitCursor wc(this);

        updateSessionActions();

        selectSession(TContestApp::getContestApp()->currSession);
    }
}
void TLogContainer::selectSessionAction()
{
    TWaitCursor wc(this);
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        if (lastSessionSelected)
            lastSessionSelected->setChecked(false);
        action->setChecked(true);
        lastSessionSelected = action;
        QString selText = action->text();
        TContestApp *app = TContestApp::getContestApp();
        // switch sections
        app->currSession = selText;
        selectSession(selText);
    }
}
void TLogContainer::selectSession(QString sessName)
{
    if (sessName.isEmpty())
        return;

    trace(QString("selectSession %1").arg(sessName));
    TContestApp *app = TContestApp::getContestApp();
    app->suppressWritePreload = true;

    // first, close all current slots, but don't write preload
    CloseAllActionExecute();

    // and reload
    BaseContestLog *ct = loadSession(sessName);

    // and make sure everything happens
    app->suppressWritePreload = false;
    if ( ct )
    {
        selectContest( ct );
        app->setCurrentContest(ct);
    }
    app->logsPreloadBundle.flushProfile();
    on_contestPageControl_currentChanged(-1);
    enableActions();
}

BaseContestLog *TLogContainer::loadSession( QString sessName)
{
    TContestApp *app = TContestApp::getContestApp();
    SettingsBundle &preloadBundle = app ->logsPreloadBundle;

    BaseContestLog *ct = nullptr;

    preloadBundle.startGroup();
    preloadBundle.openSection(sessName);
    int curSlot = 0;
    QStringList slotlst = preloadBundle.getProfileEntries();
    if (slotlst.count())
    {
        QStringList pathlst;
        for ( auto const &s: QASCONST(slotlst ))
        {
            // get each value
            QString ent;
            preloadBundle.getStringProfile( s, ent, "" );
            pathlst.append( ent );
        }
        preloadBundle.getIntProfile( eppCurrent, curSlot );
        for ( int i = 0; i < slotlst.size(); i++ )
        {
            // go through keys(slotlst) and values(pathlst) and load lists/contests
            QString slot = slotlst[ i ];
            bool ok;
            int slotno = slot.toInt(&ok);
            if ( ok )
            {
                addSlot( nullptr, pathlst[ i ], false, slotno, false );
                // spin the event loop...
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            }
        }

        if ( app->getContestSlotCount() )
        {
            if ( curSlot >= 0 && curSlot < app ->getContestSlotCount())
            {
                BaseContestLog *c = app ->contestSlotList[ curSlot ] ->slot;
                if (c)
                    ct = c;
            }
        }
    }
    if (ct)
    {
        sendDM->subscribeApps();
        app->setCurrentContest(ct);
    }

    preloadBundle.openSection(app->preloadsect);
    preloadBundle.setStringProfile(eppSession, sessName);
    preloadBundle.openSection(sessName);
    preloadBundle.endGroup();

    app ->writeContestList();	// to clear the unopened and changed ones

    setMenuLog(curSlot);
    // ui->menuLogs->clear();
    // menuLogsActions.clear();

    // ui->menuLogs->addAction(FileOpenAction);
    // ui->menuLogs->addMenu(recentFilesMenu);
    // ui->menuLogs->addAction(VHFFileNewAction);
    // if (HFFileNewAction)
    // {
    //     ui->menuLogs->addAction(HFFileNewAction);
    // }
    // ui->menuLogs->addAction(FileCloseAction);
    // ui->menuLogs->addAction(CloseAllAction);
    // ui->menuLogs->addAction(CloseAllButAction);
    // ui->menuLogs->addSeparator();

    // sessionsMenu = newMenu(ui->menuLogs, QT_TR_NOOP("Contest Sets"));
    // updateSessionActions();

    return ct;
}
QString TLogContainer::getCurrSession()
{
    TContestApp *app = TContestApp::getContestApp();
    SettingsBundle &preloadBundle = app ->logsPreloadBundle;
    preloadBundle.openSection(app ->preloadsect);

    preloadBundle.getStringProfile(eppDefSession, app ->defaultSession );
    preloadBundle.getStringProfile(eppSession, app ->currSession);

    if (app ->currSession == app ->preloadsect)
    {
        app ->currSession = app ->defaultSession;
    }
    return app->currSession;
}
void TLogContainer::setCurrSessionName(QString sessionName)
{
    TContestApp *app = TContestApp::getContestApp();
    SettingsBundle &preloadBundle = app ->logsPreloadBundle;
    preloadBundle.openSection(app ->preloadsect);

    preloadBundle.getStringProfile(eppDefSession, app ->defaultSession );
    if (sessionName == app ->preloadsect)
    {
        sessionName = app ->defaultSession;
    }

    preloadBundle.setStringProfile(eppSession, sessionName);
}
void TLogContainer::preloadFiles( const QString &conarg )
{
    // and here we want to pre-load lists and contests from the INI file
    // based on what was last open

    // getProfileEntries gets the Current entry as well... not good

    TContestApp *app = TContestApp::getContestApp();
    SettingsBundle &preloadBundle = app ->logsPreloadBundle;

    preloadBundle.openSection(getCurrSession());

    if (app ->currSession == app ->preloadsect)
    {
        app ->currSession = app ->defaultSession;
    }

    BaseContestLog *ct = loadSession(app->currSession);


    if ( conarg.size() )
    {
        if (!TContestApp::getContestApp()->isContestOpen(conarg))
        {
            // open the "argument" one last - which will make it current
            ct = addSlot( nullptr, conarg, false, -1, false );
            app ->writeContestList();	// or this one will not get included
        }
    }

    if ( ct )
    {
        sendDM->subscribeApps();
        selectContest( ct );
    }
     on_contestPageControl_currentChanged(-1);
}
void TLogContainer::preloadLists( )
{
    // get all the keys
    TContestApp::getContestApp() ->listsPreloadBundle.startGroup();
    QStringList slotlst = TContestApp::getContestApp() ->listsPreloadBundle.getProfileEntries();
    slotlst.sort();

    QStringList pathlst;
    for ( int i = 0; i < slotlst.count(); i++ )
    {
        // get each value
        QString ent;
        TContestApp::getContestApp() ->listsPreloadBundle.getStringProfile( slotlst[i], ent, "" );
        ent = GetCleanPath(ent);
        pathlst.append( ent );
    }

    for ( int i = 0; i < slotlst.size(); i++ )
    {
        // go through keys(slotlst) and values(pathlst) and oad lists/contests
        int slotno = slotlst[i].toInt() - 1;
        if ( slotno >= 0 )
        {
            addListSlot( this, pathlst[ i ], i, true );
        }
    }
    TContestApp::getContestApp() ->listsPreloadBundle.endGroup();

    TContestApp::getContestApp() ->writeListsList();
}

void TLogContainer::addListSlot( QWidget * /*p*/, const QString &fname, int slotno, bool preload )
{
    // openFile ends up calling ContactList::initialise which then
    // calls TContestApp::insertList

    QString m;

    m += tr("Loading ");

    m += tr("List file ");
    m += fname;

    sblabel0->setText( m );
    repaint();

    ContactList * list = TContestApp::getContestApp() ->openListFile( fname, slotno );
    if ( list && !preload )
    {

        if (!mShowOKCancelMessage(this, tr("Open List %1?").arg(list->name) ))
        {
            TContestApp::getContestApp() ->closeListFile( list );
            list = nullptr;
        }
    }

    TContestApp::getContestApp() ->writeListsList();
    enableActions();
}

void TLogContainer::ListOpenActionExecute()
{
    doListOpenActionExecute(this);
}
void TLogContainer::doListOpenActionExecute(QWidget *p)
{
    // first choose file

    QString InitialDir = getDirectoryLocation(dlLists);

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();
    QString Filter = tr("Contact list files (*.csl);;"
                     "All Files (*.*)") ;

    QStringList fnames = QFileDialog::getOpenFileNames( p,
                       tr("Open Archive List"),
                       InitialDir,
                       Filter
                       );

    for (auto const &fname: QASCONST(fnames))
    {
        QString fn = GetCleanPath(fname);
         addListSlot( p, fn, -1, false );
    }
}
void TLogContainer::ManageListsActionExecute(  )
{
   TManageListsDlg manageListsDlg(this );
   TMatchThread::FinishMatchThread();
   manageListsDlg.exec();
   TMatchThread::InitialiseMatchThread();
   enableActions();
}

//---------------------------------------------------------------------------

void TLogContainer::ShiftTabRightActionExecute( )
{
   // We want to reorder the tabs so that this one goes right
   if ( !ui->contestPageControl->currentWidget() )
      return ;
   int tno = ui->contestPageControl->currentIndex();
   if ( tno < ui->contestPageControl->count() - 1 )
   {
      QSharedPointer<ContestSlot> cs = TContestApp::getContestApp() ->contestSlotList[ tno ];
      int s = cs->slotno;

      QSharedPointer<ContestSlot> csp1 = TContestApp::getContestApp() ->contestSlotList[ tno + 1 ];
      int sp1 = csp1->slotno;

      TContestApp::getContestApp() ->contestSlotList[ tno ] = csp1;
      csp1->slotno = s;

      TContestApp::getContestApp() ->contestSlotList[ tno + 1 ] = cs;
      cs->slotno = sp1;

      TContestApp::getContestApp() ->writeContestList();

      ui->contestPageControl->tabBar()->moveTab(tno, tno + 1);

      enableActions();
   }
}
void TLogContainer::onTabMoved(int from, int to)
{
    // we need to apply this across ALL ContestPageControl
    ContestSlotList &contestSlotList = TContestApp::getContestApp() ->contestSlotList;
    while (from < to)
    {
        if ( from < ui->contestPageControl->count() - 1 )
        {
           QSharedPointer<ContestSlot> cs = contestSlotList[ from ];
           int s = cs->slotno;

           QSharedPointer<ContestSlot> csp1 = contestSlotList[ from + 1 ];
           int sp1 = csp1->slotno;

           contestSlotList[ from ] = csp1;
           csp1->slotno = s;

           contestSlotList[ from + 1 ] = cs;
           cs->slotno = sp1;
       }
       from++;
    }
    while (from > to)
    {

        if ( from > 1 )
        {
           QSharedPointer<ContestSlot> cs = contestSlotList[ from ];
           int s = cs->slotno;

           QSharedPointer<ContestSlot> csp1 = contestSlotList[ from - 1 ];
           int sp1 = csp1->slotno;

           contestSlotList[ from ] = csp1;
           csp1->slotno = s;

           contestSlotList[ from - 1 ] = cs;
           cs->slotno = sp1;
       }
       from--;
    }
    TContestApp::getContestApp() ->writeContestList();

    enableActions();
}

void TLogContainer::mleSetMemoryAction(BaseContestLog *cnt, QString call, QString loc)
{
    setMemoryAction->call = call;
    setMemoryAction->loc = loc;
    setMemoryAction->ct = cnt;
    setMemoryAction->setVisible(true);
}
//---------------------------------------------------------------------------

void TLogContainer::ShiftTabLeftActionExecute( )
{
   if ( !ui->contestPageControl->currentWidget() )
      return ;
   int tno = ui->contestPageControl->currentIndex();
   if ( tno > 0 )
   {
      QSharedPointer<ContestSlot> cs = TContestApp::getContestApp() ->contestSlotList[ tno ];
      int s = cs->slotno;
      QSharedPointer<ContestSlot> csm1 = TContestApp::getContestApp() ->contestSlotList[ tno - 1 ];
      int sm1 = csm1->slotno;
      TContestApp::getContestApp() ->contestSlotList[ tno ] = csm1;
      csm1->slotno = s;

      TContestApp::getContestApp() ->contestSlotList[ tno - 1 ] = cs;
      cs->slotno = sm1;

      TContestApp::getContestApp() ->writeContestList();

      ui->contestPageControl->tabBar()->moveTab(tno, tno - 1);

      enableActions();
   }
}
void TLogContainer::selectContest( BaseContestLog *pc)
{
    // select this contest on all screens
    if (!pc)
    {
        return;
    }

    for ( int j = 0; j < ui->contestPageControl->count(); j++ )
    {
        QWidget *ctab = ui->contestPageControl->widget(j);
        if ( TSingleLogFrame * f = dynamic_cast<TSingleLogFrame *>( ctab ) )
        {
            if ( f->getContest() == pc )
            {
                ui->contestPageControl->setCurrentIndex(j);
                return ;
            }
        }
    }
}
//---------------------------------------------------------------------------
void TLogContainer::setCaption(QString captionToSet)
{
   if ( captionToSet.length() )
   {
      if ( captionToSet != windowTitle() )
         setWindowTitle(captionToSet);
   }
   else
   {
      QString trs = tr("Minos Contest Logger Application");
      if ( windowTitle() != trs )
      {
         setWindowTitle(trs);
      }
   }
}
//---------------------------------------------------------------------------
TSingleLogFrame *TLogContainer::findContest(const QString &pubname )
{
   for ( int j = 0; j < ui->contestPageControl->count(); j++ )
   {
       QWidget *ctab = ui->contestPageControl->widget(j);
       if ( TSingleLogFrame * f = dynamic_cast<TSingleLogFrame *>( ctab ) )
       {
           if ( f->getContest() ->publishedName == pubname )
           {
              return f;
           }
       }
   }

   return nullptr;
}
TSingleLogFrame *TLogContainer::findContest(BaseContestLog *ct )
{
   for ( int j = 0; j < ui->contestPageControl->count(); j++ )
   {
       QWidget *ctab = ui->contestPageControl->widget(j);
       if ( TSingleLogFrame * f = dynamic_cast<TSingleLogFrame *>( ctab ) )
       {
           if ( f->getContest() == ct )
           {
              return f;
           }
       }
   }

   return nullptr;
}



void TLogContainer::stealFocus()
{
   static bool doSteal = false;
   trace("stealFocus");
   doSteal = true;
    delayedAction(this,  [=]()
    {
       if (doSteal)
           {
        // Bring window(s) to top
        for(auto cpc: QASCONST(contestPageControls))
        {
            // it would be nice to end with the primary pane...
            if (cpc)
            {
                Qt::WindowStates css = cpc->windowState();

                cpc->setWindowState(Qt::WindowState::WindowNoState);
                cpc->setWindowState(css | Qt::WindowState::WindowActive);

                trace(QString("set WindowActive %1").arg(cpc->windowTitle()));
            }
        }
        Qt::WindowStates ss = windowState();
        setWindowState(Qt::WindowState::WindowNoState);
        setWindowState(ss | Qt::WindowState::WindowActive);
        trace(QString("set WindowActive %1").arg(windowTitle()));

        TSingleLogFrame *tslf = getCurrentLogFrame();
        if (tslf)
        {
            tslf->GJVQSOLogFrame->selectFirstInvalid();
        }
       }
       doSteal = false;
    });

}



//---------------------------------------------------------------------------

QVector<TSingleLogFrame *> TLogContainer::getLogFrames()
{
    QVector<TSingleLogFrame *> logs;
    for ( int j = 0; j < ui->contestPageControl->count(); j++ )
    {
        QWidget *ctab = ui->contestPageControl->widget(j);
        if ( TSingleLogFrame * f = dynamic_cast<TSingleLogFrame *>( ctab ) )
        {
            logs.push_back(f);
        }
    }

    return logs;
}

int TLogContainer::getSlotNo(TSingleLogFrame *f) const
{
    return ui->contestPageControl->indexOf(f);
}
