#include "base_pch.h"
#include "MinosLoggerEvents.h"

#include <QFontDialog>
#include <QDesktopServices>

#include "ContestApp.h"
#include "LoggerContest.h"

#include "tsinglelogframe.h"
#include "taboutbox.h"
#include "Calendar.h"
#include "CalendarList.h"
#include "contestdetails.h"
#include "tmanagelistsdlg.h"
#include "tsettingseditdlg.h"
#include "tclockdlg.h"
#include "tloccalcform.h"
#include "TSessionManager.h"
#include "StartConfig.h"
#include "ConfigFile.h"
#include "SendRPCDM.h"
#include "MatchTreeFrame.h"
#include "enqdlg.h"
#include "AdifImport.h"
#include "ScreenConfigManager.h"
#include "MinosTestImport.h"
#include "singleapplication.h"
#include "helpbrowser.h"
#include "WsjtxServer.h"
#include "WsjtxConfigure.h"
#include "minoscontestloaddialog.h"
#include "ChatServer.h"
#include "clusterClientServer.h"
#include "MatchThread.h"

#include "tlogcontainer.h"
#include "ui_tlogcontainer.h"

TLogContainer *LogContainer = nullptr;
static QString defLayoutText = " (default)";


SetMemoryAction::SetMemoryAction(QString t, QObject *p):QAction(t, p)
{}

TLogContainer::TLogContainer(QWidget *parent) :
    QMainWindow(parent)
  , ui(new Ui::TLogContainer)
  , lastSessionSelected(nullptr)
  , lastLayoutSelected(nullptr)
{
    ui->setupUi(this);

    ui->kbframe->setVisible(false);

    LogContainer = this;

    // we may need to delay this to get the container fully constructed
    TContestApp::getContestApp(); // initialise all the infrastructure

    // make the tab control fill the window
    ui->centralWidget->layout()->setContentsMargins(0,0,0,0);

    setWindowTitle("Minos Contest Logger");

    setupMenus();

    ui->ContestPageControl->setContextMenuPolicy( Qt::CustomContextMenu );
    ui->ContestPageControl->setTabsClosable(true);
    connect(ui->ContestPageControl->tabBar(), SIGNAL(tabCloseRequested(int)), this, SLOT(onTabClosebutton(int)));
    connect(ui->ContestPageControl->tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(onTabMoved(int,int)));

    QSettings settings;
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    sblabel0 = new QLabel( "" );
    statusBar() ->addWidget( sblabel0, 6 );
    sblabel1 = new QLabel( "" );
    statusBar() ->addWidget( sblabel1, 1 );
    sblabel2 = new QLabel( "" );
    statusBar() ->addWidget( sblabel2, 2 );

    sendDM = new TSendDM(this);

    QString station = MinosConfig::getMinosConfig()->getThisServerName();
    RPCPubSub::publish(rpcConstants::LoggerCategory, station, "", psPublished);
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
        delete WsjtxServer::getWsjtxServer();

        delete MinosRPC::getMinosRPC();
        delete MinosAppConnection::minosAppConnection;
    }
    MinosRPCObj::clearRPCObjects();
}


bool TLogContainer::show(int argc, char *argv[])
{
    bool so = isShowOperators();
    ShowOperatorsAction->setChecked(so);

    bool autoFill;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpAutoFill, autoFill );
    ReportAutofillAction->setChecked(autoFill);
    TContestApp::getContestApp() ->loggerBundle.flushProfile();

    TimerUpdateQSOTimer.start(1000);
    connect(&TimerUpdateQSOTimer, SIGNAL(timeout()), this, SLOT(on_TimeDisplayTimer()));

    connect(&MinosLoggerEvents::mle, SIGNAL(ReportOverstrike(bool , BaseContestLog * )),
            this, SLOT(on_ReportOverstrike(bool , BaseContestLog * )), Qt::QueuedConnection);

    connect(&MinosLoggerEvents::mle, SIGNAL(setMemoryAction(BaseContestLog *, QString, QString)),
            this, SLOT(mleSetMemoryAction(BaseContestLog *, QString, QString)));

    QMainWindow::show();
    if ( TAboutBox::ShowAboutBox( this, true ) == false )
    {
       close();
       return false;
    }
    sendDM->subscribeApps();

    if ( contestAppLoadFiles() )
    {
       // here need to pre-open the contest list
       QString conarg;
       if ( argc > 1 )
       {
          conarg = argv[1];
       }
       preloadLists();
       preloadFiles( conarg );
       enableActions();

    }
    TContestApp::getContestApp()->setPreloadComplete();

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

#ifdef FINDFOCUS
       QWidget *f = QApplication::focusWidget ();
       if(f)
            sblabel1->setText(f->metaObject()->className());
       else
           sblabel1->setText("<unknown>");
#endif

       QString statbuf;
      if ( ct )
      {
         ct->setScore( statbuf );
      }
      sblabel0->setText( statbuf );

      if (ct)
      {
          TSingleLogFrame * lf = LogContainer ->findContest( ct );
          ui->menuKeyer->menuAction()->setVisible(lf && lf->isKeyerLoaded());
      }
      else
          ui->menuKeyer->menuAction()->setVisible(false);

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
      sblabel1->setText(overstrike ? "Overwrite" : "Insert");
   }
}

void TLogContainer::closeEvent(QCloseEvent *event)
{
    TimerUpdateQSOTimer.stop();

    TContestApp::getContestApp() ->writeContestList();
    TContestApp::getContestApp() ->suppressWritePreload = true;
    TContestApp::getContestApp() ->clearPreloadComplete();

    while ( ui->ContestPageControl->count())
    {
       // Keep closing the current (and hence visible) contest
       closeSlot(0, true);
    }

    for ( ListSlotIterator i = TContestApp::getContestApp() ->listSlotList.begin(); i != TContestApp::getContestApp() ->listSlotList.end(); i++ )
    {
       if ( ( *i ) )
       {
          TContestApp::getContestApp() ->closeListFile( ( *i ) ->slot );
       }
    }
    closeContestApp();

    QWidget::closeEvent(event);
}
void TLogContainer::moveEvent(QMoveEvent *event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::moveEvent(event);
}
void TLogContainer::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    QWidget::resizeEvent(event);
}
void TLogContainer::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
    }
}
QAction *TLogContainer::newAction( const QString &text, QMenu *m, const char *atype )
{
    QAction * newAct = new QAction( text, this );
    m->addAction( newAct );
    connect( newAct, SIGNAL( triggered() ), this, atype );
    return newAct;
}
SetMemoryAction *TLogContainer::newMemoryAction( const QString &text, QMenu *m, const char *atype )
{
    SetMemoryAction * newAct = new SetMemoryAction( text, this );
    m->addAction( newAct );
    connect( newAct, SIGNAL( triggered() ), this, atype );
    return newAct;
}
QAction *TLogContainer::newCheckableAction( const QString &text, QMenu *m, const char *atype )
{
    QAction * newAct = new QAction( text, this );
    newAct->setCheckable( true );
    m->addAction( newAct );
    connect( newAct, SIGNAL( triggered( bool ) ), this, atype );
    return newAct;
}

void TLogContainer::setupMenus()
{
    FileOpenAction = newAction("&Open Contest...", ui->menuFile, SLOT(FileOpenActionExecute()));
    FileImportAction = newAction("&Import Contest...", ui->menuFile, SLOT(FileImportActionExecute()));
    recentFilesMenu = ui->menuFile->addMenu("Reopen Contest");

    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        recentFileActs.push_back( new QAction(this));
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
        recentFilesMenu->addAction(recentFileActs[i]);
    }
    updateRecentFileActions();

    FileNewAction = newAction("&New Contest...", ui->menuFile, SLOT(FileNewActionExecute()));
    FileCloseAction = newAction("Close Contest", ui->menuFile, SLOT(FileCloseActionExecute()));
    CloseAllAction = newAction("Close all Contests", ui->menuFile, SLOT(CloseAllActionExecute()));
    CloseAllButAction = newAction("Close all but this Contest", ui->menuFile, SLOT(CloseAllButActionExecute()));
    ui->menuFile->addSeparator();

    ui->menuFile->addSeparator();
    ContestDetailsAction = newAction("Contest Details...", ui->menuFile, SLOT(ContestDetailsActionExecute()));
    MakeEntryAction = newAction("Produce Entry/Export File...", ui->menuFile, SLOT(MakeEntryActionExecute()));
    ui->menuFile->addSeparator();

    AppendAdifAction = newAction("Append ADIF file to contest...", ui->menuFile, SLOT(AppendAdifActionExecute()));
    ui->menuFile->addSeparator();

    ListOpenAction = newAction("Open &Archive List...", ui->menuFile, SLOT(ListOpenActionExecute()));
    ManageListsAction = newAction("&Manage Archive Lists...", ui->menuFile, SLOT(ManageListsActionExecute()));
    ui->menuFile->addSeparator();
    OptionsAction = newAction("Options...", ui->menuFile, SLOT(OptionsActionExecute()));
#ifdef Q_OS_WIN
    ExitClearAction = newAction("E&xit Minos Contest Logger and Clear registry", ui->menuFile, SLOT(ExitClearActionExecute()));
#endif
    ui->menuFile->addSeparator();
    ExitAction = newAction("E&xit Minos Contest Logger", ui->menuFile, SLOT(ExitActionExecute()));
// end of file menu

    GoToSerialAction = newAction("&Go To Contact Serial...", ui->menuSearch, SLOT(GoToSerialActionExecute()));
    NextUnfilledAction = newAction("Goto First Unfilled Contact", ui->menuSearch, SLOT(NextUnfilledActionExecute()));
// end of search menu

    startConfigAction = newAction("Startup Apps Configuration", ui->menuTools, SLOT(StartConfigActionExecute()));

    screenLayoutMenu = ui->menuTools->addMenu("Screen Layouts");
    updateLayoutsMenu();

    ui->menuTools->addSeparator();
    LocCalcAction = newAction("Locator Calculator", ui->menuTools, SLOT(LocCalcActionExecute()));
//    AnalyseMinosLogAction = newAction("Analyse Minos Log", ui->menuTools, SLOT(AnalyseMinosLogActionExecute()));
//    ui->menuTools->addSeparator();

    FontEditAcceptAction = newAction("Select &Font...", ui->menuTools, SLOT(FontEditAcceptActionExecute()));
    WSJTXConfigAction = newAction("WSJT-X link configuration", ui->menuTools, SLOT(WsjtConfigActionExecute()));
    ReportAutofillAction = newCheckableAction("Signal Report AutoFill", ui->menuTools, SLOT(ReportAutofillActionExecute()));
    CorrectDateTimeAction = newAction("Correct Date/Time", ui->menuTools, SLOT(CorrectDateTimeActionExecute()));

    // end of tools manu

    setMemoryAction = newMemoryAction(QString("Add as new memory..."), &TabPopup, SLOT(onSetMemoryActionExecute()));

    TabPopup.addAction(FileOpenAction);
    TabPopup.addAction(FileImportAction);
    TabPopup.addMenu(recentFilesMenu);
    TabPopup.addAction(FileNewAction);
    TabPopup.addAction(FileCloseAction);
    TabPopup.addAction(CloseAllAction);
    TabPopup.addAction(CloseAllButAction);
    TabPopup.addSeparator();

    TabPopup.addAction(ContestDetailsAction);
    TabPopup.addAction(MakeEntryAction);
    TabPopup.addSeparator();

    TabPopup.addAction(AppendAdifAction);
    TabPopup.addSeparator();

    TabPopup.addAction(GoToSerialAction);
    TabPopup.addAction(NextUnfilledAction);
    TabPopup.addSeparator();

    ShowOperatorsAction = newCheckableAction("Show Operators", &TabPopup, SLOT(ShowOperatorsActionExecute()));
    TabPopup.addSeparator();

    ShiftTabLeftAction = newAction("Shift Active Tab Left", &TabPopup, SLOT(ShiftTabLeftActionExecute()));
    ShiftTabRightAction = newAction("Shift Active Tab Right", &TabPopup, SLOT(ShiftTabRightActionExecute()));
    TabPopup.addAction(CorrectDateTimeAction);
    TabPopup.addSeparator();

    //TabPopup.addAction(AnalyseMinosLogAction);
    newAction( "Cancel", &TabPopup, SLOT( CancelClick() ) );

    keyerRecordMenu = ui->menuKeyer->addMenu("Record");
    keyerPlaybackMenu = ui->menuKeyer->addMenu("Playback");
    KeyerToneAction = newAction("Tune", ui->menuKeyer, SLOT(KeyerToneActionExecute()));
    KeyerTwoToneAction = newAction("Two Tone", ui->menuKeyer, SLOT(KeyerTwoToneActionExecute()));
    KeyerStopAction = newAction("Stop", ui->menuKeyer, SLOT(KeyerStopActionExecute()));

    for (int i = 1; i < 10; i++)
    {
        QString s = QString::number(i);
        KeyerRecordAction = newAction((s), keyerRecordMenu, SLOT(KeyerRecordActionExecute()));
        KeyerRecordAction->setData(i);
        KeyerPlaybackAction = newAction((s), keyerPlaybackMenu, SLOT(KeyerPlaybackActionExecute()));
        KeyerPlaybackAction->setData(i);
    }
    HelpAction = newAction("Help...", ui->menuHelp, SLOT(HelpActionExecute()));
    HelpAboutAction = newAction("About...", ui->menuHelp, SLOT(HelpAboutActionExecute()));
}


void TLogContainer::enableActions()
{
   bool f = ( ui->ContestPageControl->currentIndex() >= 0 );

   LocCalcAction->setEnabled(true);
   FileNewAction->setEnabled(true);
   HelpAction->setEnabled(true);
   HelpAboutAction->setEnabled(true);

   FileCloseAction->setEnabled(f);
   CloseAllAction->setEnabled(f);
   CloseAllButAction->setEnabled(f);

   ContestDetailsAction->setEnabled(f);
   GoToSerialAction->setEnabled(f);
   NextUnfilledAction->setEnabled(f);
   MakeEntryAction->setEnabled(f);
   AppendAdifAction->setEnabled(f);

   ManageListsAction->setEnabled( TContestApp::getContestApp() ->getOccupiedListSlotCount() > 0 );

   if ( ui->ContestPageControl->currentIndex() >= 0 )
   {
      int tno = ui->ContestPageControl->currentIndex();
      ShiftTabLeftAction->setEnabled( tno > 0 );
      ShiftTabRightAction->setEnabled( tno < ui->ContestPageControl->count() - 1 );
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
           BaseContestLog *ct = addSlot( &pced, FileName, false, -1 );
           if (ct)
           {
              selectContest(ct, QSharedPointer<BaseContact>());
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
    QSettings settings;
    QStringList files = settings.value("dbmru").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("dbmru", files);

    updateRecentFileActions();

}
void TLogContainer::removeCurrentFile(const QString &fileName)
{
    QSettings settings;
    QStringList files = settings.value("dbmru").toStringList();
    files.removeAll(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    settings.setValue("dbmru", files);

    updateRecentFileActions();

}
QString TLogContainer::getCurrentFile()
{
    QSettings settings;
    QStringList files = settings.value("dbmru").toStringList();
    if (files.size())
        return files[0];
    return QString();
}
void TLogContainer::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("dbmru").toStringList();

    int numRecentFiles = qMin(files.size(), static_cast< int >(MaxRecentFiles));

    for (int i = 0; i < numRecentFiles; ++i)
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
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
    //  Action method for Help Browser button.

    //  Creates a HelpBrowser instance and sets the collection and startUrl.

    QString collectionFile;
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpHelpFile, collectionFile );

    if (FileExists(collectionFile))
    {
        QString url;
        TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpHelpEntryURL, url );
        QUrl startUrl = QUrl(url);

        if (!helpBrowser)
            helpBrowser = QSharedPointer<HelpBrowser>(new HelpBrowser(collectionFile, startUrl, this));
        helpBrowser->show();
    }
    else
    {
        if (helpBrowser)
        {
            helpBrowser.clear();
        }
        TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpPDFFile, collectionFile );
        if (FileExists(collectionFile))
        {
            QUrl url = QUrl().fromLocalFile(collectionFile);
            QDesktopServices::openUrl(url);
        }
        else
        {
            mShowMessage(QString("Documentation File %1 doesn't exist.").arg(collectionFile), this);
        }
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
   }

   // we default to "./logs" or "./lists. On Vista this won't be relative to
   // the executable, but to the user data directory (I think). This is why Minos
   // invents a very strange default directory which you then cannot find.

   if ( fileName.size() && DirectoryExists( fileName ) )
   {
      // need to check possible validity of directory string
      if ( fileName[ fileName.size() ] != '/' )
         fileName += "/";
      if ( fileName[ fileName.size() ] == '/' )
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

    emit MinosLoggerEvents::sendSetMemory(setMemoryAction->ct, setMemoryAction->call, setMemoryAction->loc);
}

void TLogContainer::FileNewActionExecute()
{
    QString InitialDir = getDefaultDirectory( false );

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
    BaseContestLog * c = addSlot( &pced, initName, true, -1 );

    if (!c)
    {
       if ( !QFile::remove( initName ) )
       {
          MinosParameters::getMinosParameters() ->mshowMessage( QString( "Failed to delete " ) + initName );
       }
       return;
    }
    bool repeatDialog = true;
   QString suggestedfName;
   c->mycall.validate();
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
   QString band = c->band.getValue();
   if ( band.size() )
   {
      suggestedfName += '_';
      suggestedfName += band;
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
   closeSlot(ui->ContestPageControl->currentIndex(), false );

   while ( repeatDialog )
   {
       QString fileName = QFileDialog::getSaveFileName( this,
                          "Save new contest as",
                          InitialDir + "/" + suggestedfName,
                          "Minos contest files (*.minos *.Minos)",
                          nullptr,
                          QFileDialog::DontConfirmOverwrite
                                                      );
       if ( !fileName.isEmpty() )
       {
           if (FileExists(fileName) )
           {
               MinosParameters::getMinosParameters() ->mshowMessage( fileName + "\nalready exists.\n\nPlease choose a new name." );

               InitialDir = ExtractFilePath(fileName);
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
             MinosParameters::getMinosParameters() ->mshowMessage( QString( "Failed to rename\n" ) + initName + "\n as \n" + suggestedfName +
                                                                   "\n\nPlease choose a new name." );
             suggestedfName = initName;
          }

          // we want to (re)open it WITHOUT using the dialog!
          addSlot( nullptr, suggestedfName, false, -1 );
          repeatDialog = false;
       }
       else
       {
            repeatDialog = false;   // never go back to the dialog
            if ( !QFile::remove( initName ) )
            {
                MinosParameters::getMinosParameters() ->mshowMessage( QString( "Failed to delete " ) + initName );
            }
       }
    }
    selectContest(c, QSharedPointer<BaseContact>());
}
void TLogContainer::FileOpenActionExecute()
{
    // first choose file
//"Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"
    QString InitialDir = getDefaultDirectory( false );

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();

    QString Filter = "Minos contest files (*.minos *.Minos);;"
                     "All Files (*.*)" ;

    QStringList fnames = QFileDialog::getOpenFileNames( this,
                       "Open contests",
                       InitialDir,  // dir
                       Filter
                       );
    for (int i = 0; i < fnames.size(); i++)
    {
        QString fname = fnames[i];
        BaseContestLog *ct = nullptr;
        if ( !fname.isEmpty() )
        {
            ContestDetails pced(this );
            ct = addSlot( &pced, fname, false, -1 );   // not automatically read only
            if (ct)
            {
                selectContest(ct, QSharedPointer<BaseContact>());
            }
        }
    }
}
void TLogContainer::FileImportActionExecute()
{
    // first choose file
//"Images (*.png *.xpm *.jpg);;Text files (*.txt);;XML files (*.xml)"
    QString InitialDir = getDefaultDirectory( false );

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();

    QString Filter = "Use this combo for file types (*.*);;"
                     "Reg1Test Files (*.edi);;"
                     "GJV contest files (*.gjv);;"
                     "RSGB Log Files (*.log);;"
                     "ADIF Files (*.adi);;"
                     "All Files (*.*)" ;

    QStringList fnames = QFileDialog::getOpenFileNames( this,
                       "Import contests",
                       InitialDir,  // dir
                       Filter
                       );
    for (int i = 0; i < fnames.size(); i++)
    {
        QString fname = fnames[i];
        BaseContestLog *ct = nullptr;
        if ( !fname.isEmpty() )
        {
            ContestDetails pced(this );
            ct = addSlot( &pced, fname, false, -1 );   // not automatically read only
            if (ct)
            {
                selectContest(ct, QSharedPointer<BaseContact>());
            }
        }
    }
}


void TLogContainer::ContestDetailsActionExecute()
{
    QWidget *tw = ui->ContestPageControl->currentWidget();
    TSingleLogFrame *f = dynamic_cast<TSingleLogFrame *>( tw );

    if (f)
    {
        LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( f->getContest());


        if (ct)
        {
            QString curConfig = ct->screenLayout.getValue();
            ContestDetails pced( this );


            pced.setDetails( ct );
            if ( pced.exec() == QDialog::Accepted )
            {
                if (ct->screenLayout.getValue() != curConfig)
                {
                    f->applyScreenLayout();
                }

                sendDM->subscribeApps();
                // and we need to do some re-init on the display
                f->updateQSODisplay();
                ct->scanContest();
                f->refreshMults();

                updateLayoutsMenu();
            }

        }
    }
}
//---------------------------------------------------------------------------

void TLogContainer::FileCloseActionExecute()
{
   int t = ui->ContestPageControl->currentIndex();
   closeSlot(t, true);
}
void TLogContainer::onTabClosebutton(int t)
{
    closeSlot(t, true);
}
//---------------------------------------------------------------------------

void TLogContainer::CloseAllActionExecute()
{
   while ( ui->ContestPageControl->count())
   {
      // Keep closing the current (and hence visible) contest
      closeSlot(0, true);
   }
   on_ContestPageControl_currentChanged(-1);
   enableActions();
}
//---------------------------------------------------------------------------

void TLogContainer::CloseAllButActionExecute()
{
   QWidget *thisContest = ui->ContestPageControl->currentWidget();
   while ( ui->ContestPageControl->count() > 1)
   {
       int t = ui->ContestPageControl->count() - 1;
      QWidget *ctab = ui->ContestPageControl->widget(t);
      if (ctab == thisContest)
      {
         t -= 1;
      }
      closeSlot(t, true );
   }
   on_ContestPageControl_currentChanged(-1);
   enableActions();
}
//---------------------------------------------------------------------------

void TLogContainer::ExitActionExecute()
{
    close();
}
void TLogContainer::ExitClearActionExecute()
{
    // Confirm...

#ifdef Q_OS_WIN
    if (!mShowYesNoMessage(this, "This action will clear registry entries for all of the apps within the Minos V2 Logger.\r\n\r\n"
                                  "Please confirm this action by pressing \"Yes\"." ))
    {
       return;
    }

    MinosConfig::getMinosConfig() ->stop();
    SingleApplication *sa = dynamic_cast<SingleApplication *>(QApplication::instance());

    QObject::connect(sa, SIGNAL(aboutToQuit()), sa, SLOT(clearRegistry()));

    close();
#else
    mShowMessage("Clear registry only works under Windows", this);
#endif
}
void TLogContainer::AppendAdifActionExecute()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();

    if (!ct)
        return;

    QString InitialDir = getDefaultDirectory( false );

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();

    QString Filter = "ADIF files (*.adi);;"
                     "All Files (*.*)" ;

    QString fname = QFileDialog::getOpenFileName( this,
                       "Open ADIF for append",
                       InitialDir,  // dir
                       Filter
                       );

    if (!fname.isEmpty())
    {
        QIODevice::OpenMode om = QIODevice::ReadOnly;

        QSharedPointer<QFile> adifFile(new QFile(fname));

        if (!adifFile->open(om))
        {
           QString lerr = adifFile->errorString();
           QString emess = "Failed to open ADIF file " + fname + " : " + lerr;
           MinosParameters::getMinosParameters() ->mshowMessage( emess );
           return;
        }

        int spoint = ct->ctList.count();
        if (! ADIFImport::doImportADIFLog(dynamic_cast<LoggerContestLog *>(ct),  adifFile ))
        {
            MinosParameters::getMinosParameters() ->mshowMessage( "Failed to append " + fname );
        }
        ct->scanContest();
        ct->validateLoc();
        for ( int i = spoint; i != ct->ctList.count(); i++ )
        {
            QSharedPointer<BaseContact> bct = ct->pcontactAt(i);
            bct->commonSave(bct);
        }
        ct->commonSave( false );
        MinosLoggerEvents::SendAfterLogContact(ct);
        TSingleLogFrame * tslf = LogContainer ->findContest( ct );

        tslf->showQSOs();
    }
}

void TLogContainer::MakeEntryActionExecute()
{
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
    MinosLoggerEvents::SendMakeEntry(ct);
}
void TLogContainer::LocCalcActionExecute()
{
    TLocCalcForm loccalc( this );
    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
    if (ct)
    {
       loccalc.S1Loc = ct->myloc.loc.getValue();
    }
    loccalc.exec();
}
void TLogContainer::AnalyseMinosLogActionExecute()
{
    QString InitialDir = getDefaultDirectory( false );

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();

    QString Filter = "Minos contest files (*.minos *.Minos);;"
                     "All Files (*.*)" ;

    QString fname = QFileDialog::getOpenFileName( this,
                       "Open contest for Analysis",
                       InitialDir,  // dir
                       Filter
                       );

    QIODevice::OpenMode om = QIODevice::ReadOnly;

    QSharedPointer<QFile> contestFile(new QFile(fname));

    if (!contestFile->open(om))
    {
       QString lerr = contestFile->errorString();
       QString emess = "Failed to open Contest Log file " + fname + " : " + lerr;
       MinosParameters::getMinosParameters() ->mshowMessage( emess );
       return;
    }

    MinosTestImport mt;
    mt.analyseTest( contestFile );

    MinosParameters::getMinosParameters() ->mshowMessage( "Analysis of " + fname + " complete; look in the trace log for analysis." );
}

void TLogContainer::CorrectDateTimeActionExecute()
{
    TClockDlg cdlg(this);
    cdlg.exec();
}

void TLogContainer::ShowOperatorsActionExecute()
{
    bool so = !isShowOperators();
    ShowOperatorsAction->setChecked(so);
    TContestApp::getContestApp() ->displayBundle.setBoolProfile( edpShowOperators, so );
    TContestApp::getContestApp() ->displayBundle.flushProfile();
    MinosLoggerEvents::SendShowOperators();
}

void TLogContainer::OptionsActionExecute()
{
    TSettingsEditDlg ed(this, &TContestApp::getContestApp() ->loggerBundle );

    ed.ShowCurrentSectionOnly();
    if (ed.exec() == QDialog::Accepted)
    {
       mShowMessage("You may need to close and reload Minos to have these settings applied", this);
    }
}

void TLogContainer::FontEditAcceptActionExecute()
{
    QString qpa = qgetenv("QT_QPA_PLATFORMTHEME");
    if (qpa.compare("qt5ct", Qt::CaseInsensitive) == 0)
    {
        mShowMessage("Font setting will not work while the QT_QPA_PLATFORMTHEME environment variable is set to qt5ct", this);
        QSettings settings;
        settings.remove( "font");
    }
    else
    {
        QFont f = font();
        bool ok;
        f = QFontDialog::getFont( &ok, f );
        if (ok)
        {
            QApplication::setFont( f );

            foreach ( QWidget * widget, QApplication::allWidgets() )
            {
                widget->setFont(f);
                widget->update();
            }

            QSettings settings;
            settings.setValue( "font", font() );

            MinosLoggerEvents::SendFontChanged();
        }
    }
}
void TLogContainer::WsjtConfigActionExecute()
{
    WsjtxConfigure wsjtConfig;

    wsjtConfig.exec();
}
void TLogContainer::ReportAutofillActionExecute()
{
    bool autoFill = ReportAutofillAction->isChecked();
    TContestApp::getContestApp() ->loggerBundle.setBoolProfile( elpAutoFill, autoFill );
    TContestApp::getContestApp() ->loggerBundle.flushProfile();

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
void TLogContainer::KeyerToneActionExecute()
{
    emit sendKeyerTone( );
}

void TLogContainer::KeyerTwoToneActionExecute()
{
    emit sendKeyerTwoTone( );
}

void TLogContainer::KeyerStopActionExecute()
{
    emit sendKeyerStop( );
}
void TLogContainer::KeyerRecordActionExecute()
{
    QAction *qa = qobject_cast<QAction *>(sender());
    if (qa)
    {
        int k = qa->data().toInt();
        emit sendKeyerRecord( k );
    }
}
void TLogContainer::KeyerPlaybackActionExecute()
{
    QAction *qa = qobject_cast<QAction *>(sender());
    if (qa)
    {
        int k = qa->data().toInt();
        emit sendKeyerPlay( k );
    }
}

void TLogContainer::menuLogsActionExecute()
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
    ScreenConfigManager sc(this);
    sc.exec();
    updateLayoutsMenu();
}
void TLogContainer::StartConfigActionExecute()
{
    StartConfig configBox( this, false);
    configBox.exec();
    // in case we are now running more apps
    sendDM->subscribeApps();
}
void TLogContainer::listCompressionActionExecute()
{
    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpListCompression, lcf);
    if (enquireDialog(this, "Set List Spacing Compression Value as percentage", lcf, 50, 150))
    {
        TContestApp::getContestApp() ->setIntDisplayProfile(edpListCompression, lcf);
        MinosLoggerEvents::sendListCompressionChanged(lcf/100.0);

        TWaitCursor wc(this);
        selectSession(TContestApp::getContestApp()->currSession);
    }
}

void TLogContainer::on_ContestPageControl_currentChanged(int index)
{
    trace(QString("TLogContainer::on_ContestPageControl_currentChanged index %1").arg(index));
    enableActions();

    if (index >= 0)
    {
        MinosLoggerEvents::SendContestPageChanged();
    }

    TContestApp::getContestApp() ->writeContestList();
    enableActions();

    updateLayoutsMenu();

    ui->menuLogs->clear();
    menuLogsActions.clear();

    ui->menuLogs->addAction(FileOpenAction);
    ui->menuLogs->addMenu(recentFilesMenu);
    ui->menuLogs->addAction(FileNewAction);
    ui->menuLogs->addAction(FileCloseAction);
    ui->menuLogs->addAction(CloseAllAction);
    ui->menuLogs->addAction(CloseAllButAction);
    ui->menuLogs->addSeparator();

    sessionsMenu = ui->menuLogs->addMenu("Contest Sets");
    updateSessionActions();

    for (int i = 0; i < ui->ContestPageControl->count(); i++)
    {
        QWidget *ctab = ui->ContestPageControl->widget(i);
        TSingleLogFrame * f = dynamic_cast<TSingleLogFrame *>( ctab );
        BaseContestLog *pc = f->getContest();

        QSharedPointer<QAction> ma(newCheckableAction(ui->ContestPageControl->tabText(i), ui->menuLogs, SLOT(menuLogsActionExecute())));

        QVariant qpc(i);
        ma->setData(qpc);
        menuLogsActions.push_back(ma);

        if ( f )
        {

            if (f == ui->ContestPageControl->currentWidget())
            {
                ui->ContestPageControl->setTabColor(i, Qt::red);
                ma->setChecked(true);
            }
            else if (pc->isReadOnly())
            {
                ui->ContestPageControl->setTabColor(i, Qt::darkGreen);
            }
            else
            {
                ui->ContestPageControl->setTabColor(i, Qt::darkBlue);
            }
        }
    }
}

void TLogContainer::on_ContestPageControl_tabBarDoubleClicked(int /*index*/)
{
    ContestDetailsActionExecute();
}
void TLogContainer::selectTab(int curTab)
{
    if (curTab >= 0)
    {
        QWidget *ctab = ui->ContestPageControl->widget(curTab);
        TSingleLogFrame * f = dynamic_cast<TSingleLogFrame *>( ctab );
        BaseContestLog *pc = f->getContest();
        selectContest(pc, QSharedPointer<BaseContact>());
    }

}
void TLogContainer::on_ContestPageControl_customContextMenuRequested(const QPoint &pos)
{
    setMemoryAction->setVisible(false);

    int curtab = ui->ContestPageControl->tabBar()->tabAt(pos);
    // -1 if not on a tab
    selectTab(curtab);

    QPoint globalPos = ui->ContestPageControl->mapToGlobal( pos );

    QApplication *qa = dynamic_cast<QApplication *>(QApplication::instance());
    QObject *w = qa->widgetAt(globalPos);

    while (w)
    {
        MatchTreeFrame *mtf = dynamic_cast<MatchTreeFrame *>(w);

        if (mtf)
        {
            mtf->doCustomContextMenuRequested();
            break;
        }

        w = w->parent();
    }

    TabPopup.popup( globalPos );
}
BaseContestLog * TLogContainer::addSlot(ContestDetails *ced, const QString &fname, bool newfile, int slotno )
{
   MinosContestLoadDialog progress(this);
  //create and show a progress splash screen
   progress.setLoadMessage(fname, newfile, false);
   progress.doShow();

   static int namegen = 0;
   // openFile ends up calling ContestLog::initialise which then
   // calls TContestApp::insertContest

   LoggerContestLog * contest = TContestApp::getContestApp() ->openFile( fname, newfile, slotno );

   if ( contest )
   {
      bool show = false;
      if ( ced )
      {
         ced->setDetails( contest );

         {
            if ( ced->exec() == QDialog::Accepted )
            {
               contest->scanContest();
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
         contest->scanContest();
         show = true;
      }

      if ( show )
      {
         TContestApp::getContestApp() ->setCurrentContest( contest );
         QString baseFName = ExtractFileName( contest->cfileName );
         TSingleLogFrame *f = new TSingleLogFrame( this, contest );
         f->setObjectName( QString( "LogFrame" ) + QString::number(namegen++));

         int tno = ui->ContestPageControl->addTab(f, baseFName);
         ui->ContestPageControl->setCurrentWidget(ui->ContestPageControl->widget(tno));
         ui->ContestPageControl->setTabToolTip(tno, contest->cfileName);

         f->columnsChanged = true;  // also causes show QSOs
         f->splittersChanged = true;

         sendDM->subscribeApps();

         on_ContestPageControl_currentChanged(tno);

         if ( contest->needsExport() )      // imported from an alien format (e.g. .log)
         {
            QString expName = f->makeEntry( true );
            if ( expName.size() )
            {
               closeSlot(tno, true );
               addSlot( nullptr, expName, false, -1 );
            }
         }
         removeCurrentFile( fname );
      }
   }
   TContestApp::getContestApp() ->writeContestList();
   enableActions();

   progress.hide();

   return contest;
}
TSingleLogFrame *TLogContainer::getCurrentLogFrame()
{
    QWidget *w = ui->ContestPageControl->currentWidget();
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
          if ( addToMRU )
          {
             BaseContestLog * contest = f->getContest();
             QString curPath = contest->cfileName;
             setCurrentFile( curPath );
          }
          f->closeContest();    // which should close the contest

          QWidget *tab = ui->ContestPageControl->widget(t);
          tab->deleteLater();

          ui->ContestPageControl->removeTab(t);
          on_ContestPageControl_currentChanged(-1);
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
    QWidget *tw = ui->ContestPageControl->widget(t);
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
    qSort( sessionlst);
    for (int i = 0; i < sessionlst.size(); ++i)
    {
        if (sessionlst[i] != app ->logsPreloadBundle.noneBundle && sessionlst[i] != app->preloadsect)
        {
            newSessionList.append(sessionlst[i]);
        }
    }
    return newSessionList;
}
void TLogContainer::updateLayoutsMenu()
{
    screenLayoutMenu->clear();
    ScreenConfigAction = newAction("Configure Screen Layouts...", screenLayoutMenu, SLOT(doScreenConfigAction()));
    listCompressionAction = newAction("Set List Spacing Compression...", screenLayoutMenu, SLOT(listCompressionActionExecute()));

    screenLayoutMenu->addSeparator();

    QWidget *tw = ui->ContestPageControl->currentWidget();
    TSingleLogFrame *f = dynamic_cast<TSingleLogFrame *>( tw );
    if (f)
    {
        QString currentLayout = f->getCurScreenLayout();
        QString defaultLayout;
        MinosParameters::getMinosParameters() -> getStringDisplayProfile( edpCurrentLayout, defaultLayout );

        ScreenConfigFile scf;
        scf.loadFile(this);
        int j = 0;
        for(QMap<QString, SC>::iterator i = scf.configs.begin(); i != scf.configs.end(); i++ )
        {
            QAction *act =  new QAction(this);
            if ((*i).name == defaultLayout)
            {
                act->setText((*i).name + defLayoutText);
            }
            else
            {
                act->setText((*i).name);
            }
            connect(act, SIGNAL(triggered()),
                    this, SLOT(selectLayout()));
            act->setCheckable(true);

            screenLayoutMenu->addAction(act);

            if ((*i).name == currentLayout)
            {
                act->setChecked(true);
                lastLayoutSelected = act;
            }

            j++;
        }
    }
}

void TLogContainer::selectLayout()
{
    TWaitCursor wc(this);
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        if (lastLayoutSelected)
            lastLayoutSelected->setChecked(false);
        action->setChecked(true);
        lastLayoutSelected = action;
        QString selText = action->text();
        if (selText.endsWith(defLayoutText))
        {
            selText.chop(defLayoutText.size());

        }
        selectLayout(selText);
    }
}
void TLogContainer::selectLayout(QString layout)
{
    QWidget *tw = ui->ContestPageControl->currentWidget();
    TSingleLogFrame *f = dynamic_cast<TSingleLogFrame *>( tw );
    if (f)
    {
        f->setCurScreenLayout(layout);
        f->applyScreenLayout();
        updateLayoutsMenu();
    }
}
void TLogContainer::applyScreenLayouts()
{
    TWaitCursor wc(this);

    // clear old splitter settings
    QSettings settings;
    settings.remove("Splitters");

    for (int i = 0; i < ui->ContestPageControl->count(); i++)
    {
        QWidget *ctab = ui->ContestPageControl->widget(i);
        TSingleLogFrame * f = dynamic_cast<TSingleLogFrame *>( ctab );
        f->applyScreenLayout();
    }
}
void TLogContainer::updateSessionActions()
{
    TContestApp *app = TContestApp::getContestApp();
    SettingsBundle &preloadBundle = app ->logsPreloadBundle;

    getCurrSession();

    preloadBundle.openSection(app ->currSession);

    sessionsMenu->clear();

    sessionManagerAction  = newAction("&Manage Contest Sets...", sessionsMenu, SLOT(sessionManageExecute()));
    QStringList sessionlst = getSessions();
    for (int i = 0; i < sessionlst.size(); ++i)
    {
        if (sessionlst[i] == app->preloadsect)
        {
            continue;
        }
        QAction *act =  new QAction(this);
        act->setText(sessionlst[i]);
        connect(act, SIGNAL(triggered()),
                this, SLOT(selectSession()));
        act->setCheckable(true);
        if (sessionlst[i] == app->currSession)
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
void TLogContainer::selectSession()
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
void TLogContainer::closeSession()
{
    TContestApp *app = TContestApp::getContestApp();
    app->suppressWritePreload = true;

    // first, close all current slots, but don't write preload
    while ( ui->ContestPageControl->count())
    {
       // Keep closing the current (and hence visible) contest
       // DO add to MRU (Is that right?)
       closeSlot(0, true );
    }
    app->suppressWritePreload = false;
}
void TLogContainer::selectSession(QString sessName)
{
    TContestApp *app = TContestApp::getContestApp();
    app->suppressWritePreload = true;

    // first, close all current slots, but don't write preload
    while ( ui->ContestPageControl->count())
    {
       // Keep closing the current (and hence visible) contest
       // DO add to MRU (Is that right?)
       closeSlot(0, true );
    }

    // and reload
    BaseContestLog *ct = loadSession(sessName);

    // and make sure everything happens
    app->suppressWritePreload = false;
    if ( ct )
    {
        selectContest( ct, QSharedPointer<BaseContact>() );
        app->setCurrentContest(ct);
    }
    app->logsPreloadBundle.flushProfile();
    on_ContestPageControl_currentChanged(-1);
    enableActions();
}

BaseContestLog *TLogContainer::loadSession( QString sessName)
{
    TContestApp *app = TContestApp::getContestApp();
    SettingsBundle &preloadBundle = app ->logsPreloadBundle;

    BaseContestLog *ct = nullptr;

    preloadBundle.startGroup();
    preloadBundle.openSection(sessName);
    QStringList slotlst = preloadBundle.getProfileEntries();
    if (slotlst.count())
    {
        QStringList pathlst;
        for ( int i = 0; i < slotlst.count(); i++ )
        {
            // get each value
            QString ent;
            preloadBundle.getStringProfile( slotlst[i], ent, "" );
            pathlst.append( ent );
        }
        int curSlot = 0;
        preloadBundle.getIntProfile( eppCurrent, curSlot );
        for ( int i = 0; i < slotlst.size(); i++ )
        {
            // go through keys(slotlst) and values(pathlst) and load lists/contests
            QString slot = slotlst[ i ];
            bool ok;
            int slotno = slot.toInt(&ok);
            if ( ok )
            {
                addSlot( nullptr, pathlst[ i ], false, slotno );
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
        app->setCurrentContest(ct);

    preloadBundle.openSection(app->preloadsect);
    preloadBundle.setStringProfile(eppSession, sessName);
    preloadBundle.openSection(sessName);
    app ->writeContestList();	// to clear the unopened and changed ones

    ui->menuLogs->clear();
    menuLogsActions.clear();

    ui->menuLogs->addAction(FileOpenAction);
    ui->menuLogs->addMenu(recentFilesMenu);
    ui->menuLogs->addAction(FileNewAction);
    ui->menuLogs->addAction(FileCloseAction);
    ui->menuLogs->addAction(CloseAllAction);
    ui->menuLogs->addAction(CloseAllButAction);
    ui->menuLogs->addSeparator();

    sessionsMenu = ui->menuLogs->addMenu("Contest Sets");
    updateSessionActions();

    preloadBundle.endGroup();
    return ct;
}
void TLogContainer::getCurrSession()
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
}

void TLogContainer::preloadFiles( const QString &conarg )
{
    // and here we want to pre-load lists and contests from the INI file
    // based on what was last open

    // getProfileEntries gets the Current entry as well... not good

    TContestApp *app = TContestApp::getContestApp();
    SettingsBundle &preloadBundle = app ->logsPreloadBundle;

    getCurrSession();

    preloadBundle.openSection(app ->currSession);

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
            ct = addSlot( nullptr, conarg, false, -1 );
            app ->writeContestList();	// or this one will not get included
        }
    }

    if ( ct )
    {
        selectContest( ct, QSharedPointer<BaseContact>() );
    }
}
void TLogContainer::preloadLists( )
{
    // get all the keys
    TContestApp::getContestApp() ->listsPreloadBundle.startGroup();
    QStringList slotlst = TContestApp::getContestApp() ->listsPreloadBundle.getProfileEntries();
    QStringList pathlst;
    for ( int i = 0; i < slotlst.count(); i++ )
    {
        // get each value
        QString ent;
        TContestApp::getContestApp() ->listsPreloadBundle.getStringProfile( slotlst[i], ent, "" );
        pathlst.append( ent );
    }

    for ( int i = 0; i < slotlst.size(); i++ )
    {
        // go through keys(slotlst) and values(pathlst) and oad lists/contests
        int slotno = slotlst[i].toInt() - 1;
        if ( slotno >= 0 )
        {
            addListSlot( pathlst[ i ], slotno, true );
        }
    }
    TContestApp::getContestApp() ->listsPreloadBundle.endGroup();
}

void TLogContainer::addListSlot( const QString &fname, int slotno, bool preload )
{

    MinosContestLoadDialog progress(this);
   //create and show a progress splash screen
    progress.setLoadMessage(fname, false, true);
    progress.doShow();

    // openFile ends up calling ContactList::initialise which then
    // calls TContestApp::insertList

    ContactList * list = TContestApp::getContestApp() ->openListFile( fname, slotno );
    if ( list && !preload )
    {

        if (!mShowOKCancelMessage(this, "Open List " + list->name + "?") )
        {
            TContestApp::getContestApp() ->closeListFile( list );
            list = nullptr;
        }
    }

    TContestApp::getContestApp() ->writeListsList();
    enableActions();
    progress.hide();
}

void TLogContainer::ListOpenActionExecute()
{
    // first choose file

    QString InitialDir = getDefaultDirectory( true );

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();
    QString Filter = "Contact list files (*.csl);;"
                     "All Files (*.*)" ;

    QStringList fnames = QFileDialog::getOpenFileNames( this,
                       "Open Archive List",
                       InitialDir,
                       Filter
                       );

    for (int i = 0; i < fnames.size(); i++)
    {
         QString fname = fnames[i];
         addListSlot( fname, -1, false );
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
   if ( !ui->ContestPageControl->currentWidget() )
      return ;
   int tno = ui->ContestPageControl->currentIndex();
   if ( tno < ui->ContestPageControl->count() - 1 )
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

      ui->ContestPageControl->tabBar()->moveTab(tno, tno + 1);

      enableActions();
   }
}
void TLogContainer::onTabMoved(int from, int to)
{

    while (from < to)
    {
        if ( from < ui->ContestPageControl->count() - 1 )
        {
           QSharedPointer<ContestSlot> cs = TContestApp::getContestApp() ->contestSlotList[ from ];
           int s = cs->slotno;

           QSharedPointer<ContestSlot> csp1 = TContestApp::getContestApp() ->contestSlotList[ from + 1 ];
           int sp1 = csp1->slotno;

           TContestApp::getContestApp() ->contestSlotList[ from ] = csp1;
           csp1->slotno = s;

           TContestApp::getContestApp() ->contestSlotList[ from + 1 ] = cs;
           cs->slotno = sp1;
       }
       from++;
    }
    while (from > to)
    {

        if ( from > 1 )
        {
           QSharedPointer<ContestSlot> cs = TContestApp::getContestApp() ->contestSlotList[ from ];
           int s = cs->slotno;

           QSharedPointer<ContestSlot> csp1 = TContestApp::getContestApp() ->contestSlotList[ from - 1 ];
           int sp1 = csp1->slotno;

           TContestApp::getContestApp() ->contestSlotList[ from ] = csp1;
           csp1->slotno = s;

           TContestApp::getContestApp() ->contestSlotList[ from - 1 ] = cs;
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
   if ( !ui->ContestPageControl->currentWidget() )
      return ;
   int tno = ui->ContestPageControl->currentIndex();
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

      ui->ContestPageControl->tabBar()->moveTab(tno, tno - 1);

      enableActions();
   }
}
void TLogContainer::selectContest( BaseContestLog *pc, QSharedPointer<BaseContact> pct )
{
    // we have double clicked on a contact in "other" or "archive" trees
    // so we want to (a) switch tabs and (b) go to that contact edit


    for ( int j = 0; j < ui->ContestPageControl->count(); j++ )
    {
        QWidget *ctab = ui->ContestPageControl->widget(j);
        if ( TSingleLogFrame * f = dynamic_cast<TSingleLogFrame *>( ctab ) )
        {
            if ( f->getContest() == pc )
            {
                ui->ContestPageControl->setCurrentIndex(j);         // This doesn't call ContestPageControlChange (see TPageControl::OnChange in  help)
                on_ContestPageControl_currentChanged(-1);       // so the contest gets properly switched
                f->QSOTreeSelectContact( pct );         // which triggers edit on the contact
                return ;
            }
        }
    }
}
//---------------------------------------------------------------------------
void TLogContainer::setCaption(QString captionToSet)
{
   if ( windowTitle().length() )
   {
      if ( captionToSet != windowTitle() )
         setWindowTitle(captionToSet);
   }
   else
      if ( windowTitle() != "Minos contest Logger Application" )
         setWindowTitle("Minos contest Logger Application");
}
//---------------------------------------------------------------------------
TSingleLogFrame *TLogContainer::findContest(const QString &pubname )
{
   for ( int j = 0; j < ui->ContestPageControl->count(); j++ )
   {
       QWidget *ctab = ui->ContestPageControl->widget(j);
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
   for ( int j = 0; j < ui->ContestPageControl->count(); j++ )
   {
       QWidget *ctab = ui->ContestPageControl->widget(j);
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
//---------------------------------------------------------------------------

QVector<TSingleLogFrame *> TLogContainer::getLogFrames()
{
    QVector<TSingleLogFrame *> logs;
    for ( int j = 0; j < ui->ContestPageControl->count(); j++ )
    {
        QWidget *ctab = ui->ContestPageControl->widget(j);
        if ( TSingleLogFrame * f = dynamic_cast<TSingleLogFrame *>( ctab ) )
        {
            logs.push_back(f);
        }
    }

    return logs;
}

int TLogContainer::getLogFrameCount()
{
    return ui->ContestPageControl->count();
}
