/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

#include "LoggerContest.h"
#include "ContestApp.h"
#include "MatchThread.h"

//---------------------------------------------------------------------------

#define INITIAL_CONTEST_SLOTS 2
#define INITIAL_LIST_SLOTS 10

qint64 bigClockCorr = 0;
TContestApp *TContestApp::contestApp = nullptr;

TContestApp *TContestApp::getContestApp()
{
   static bool firstTime = true;
   if ( !contestApp && firstTime )
   {
      firstTime = false;
      contestApp = new TContestApp;
      contestApp->initialise();
   }
   return contestApp;
}
//---------------------------------------------------------------------------

bool isOpen(QSharedPointer<ContestSlot> cs, const QString &fn )
{
   if (cs.isNull())
       return false;

   if (!FileExists(fn) )
       return false;    // doesn't exist, so can't be open

   BaseContestLog * con = cs->slot;
   if ( !con )
      return false;

   QFileInfo confi(con->cfileName);
   QFileInfo fnfi(fn);

   if ( confi == fnfi )
      return true;
   return false;
}
bool isOpen(QSharedPointer<ListSlot> cs, const QString &fn )
{
    if (cs.isNull())
        return false;
   ContactList * con = cs->slot;
   if ( !con )
      return false;
   if ( con->cfileName.compare(fn, Qt::CaseInsensitive ) == 0 )
      return true;
   return false;
}
ContestSlot::ContestSlot( ) : slotno( -1 ), slot( nullptr )
{}
ContestSlot::~ContestSlot()
{
   if ( slot )
   {
      delete slot;
      slot = nullptr;
   }
}
ListSlot::ListSlot( ) : slotno( -1 ), slot( nullptr )
{}
ListSlot::~ListSlot()
{
   if ( slot )
   {
      delete slot;
      slot = nullptr;
   }
}


//---------------------------------------------------------------------------
static void initClock( )
{
    QFile lf( "./Configuration/time.correction");

    if (!lf.open(QIODevice::ReadOnly))
    {
        return;
    }
    QTextStream strm(&lf);

   if ( !strm.atEnd() )
   {
      long big_corr;
      strm >> big_corr;
      bigClockCorr = big_corr;
   }
}
bool TContestApp::initialise()
{
    suppressWritePreload = false;

    // we need to open our bundles...
    // and we need to discover the defaults from the initial splash screen

    // Where do we get the initial defaults from? I think we need an "initial"
    // config file that we can use to store the others...


    //----------------------------------

    BundleFile::bundleFiles[ epLOGGERPROFILE ] = QSharedPointer<BundleFile>( new BundleFile( epLOGGERPROFILE ) );
    BundleFile::bundleFiles[ epPRELOADPROFILE ] = QSharedPointer<BundleFile>( new BundleFile( epPRELOADPROFILE ) );
    BundleFile::bundleFiles[ epLISTSPROFILE ] = QSharedPointer<BundleFile>( new BundleFile( epLISTSPROFILE ) );
    BundleFile::bundleFiles[ epDISPLAYPROFILE ] = QSharedPointer<BundleFile>( new BundleFile( epDISPLAYPROFILE ) );
    BundleFile::bundleFiles[ epENTRYPROFILE ] = QSharedPointer<BundleFile>( new BundleFile( epENTRYPROFILE ) );
    BundleFile::bundleFiles[ epQTHPROFILE ] = QSharedPointer<BundleFile>( new BundleFile( epQTHPROFILE ) );
    BundleFile::bundleFiles[ epSTATIONPROFILE ] = QSharedPointer<BundleFile>( new BundleFile( epSTATIONPROFILE ) );
    BundleFile::bundleFiles[ epLOCSQUARESPROFILE ] = QSharedPointer<BundleFile>( new BundleFile( epLOCSQUARESPROFILE ) );

    //----------------------------------
    BundleFile::bundleFiles[ epLOGGERPROFILE ] ->openProfile( "./Configuration/MinosLogger.ini", tr("Advanced Options") );
    loggerBundle.setProfile( BundleFile::bundleFiles[ epLOGGERPROFILE ] );
    loggerBundle.openSection( "Default" );

    loggerBundle.startGroup();
    //----------------------------------
    QString preloadfile;
    loggerBundle.getStringProfile( elpPreloadFile, preloadfile );
    BundleFile::bundleFiles[ epPRELOADPROFILE ] ->openProfile( preloadfile, "Preload options" );

    logsPreloadBundle.setProfile( BundleFile::bundleFiles[ epPRELOADPROFILE ] );

    //   QString preloadsect;   // moved to class
    loggerBundle.getStringProfile( elpPreloadSection, preloadsect );
    logsPreloadBundle.openSection( preloadsect );
    //----------------------------------
    QString listsfile;
    loggerBundle.getStringProfile( elpListsFile, listsfile );
    BundleFile::bundleFiles[ epLISTSPROFILE ] ->openProfile( listsfile, "List Preload options" );

    listsPreloadBundle.setProfile( BundleFile::bundleFiles[ epLISTSPROFILE ] );

    QString listsect;
    loggerBundle.getStringProfile( elpListsSection, listsect );
    listsPreloadBundle.openSection( listsect );
    //----------------------------------

    QString dispfile;
    loggerBundle.getStringProfile( elpDisplayFile, dispfile );
    BundleFile::bundleFiles[ epDISPLAYPROFILE ] ->openProfile( dispfile, "Display defaults" );

    displayBundle.setProfile( BundleFile::bundleFiles[ epDISPLAYPROFILE ] );

    QString dispsect;
    loggerBundle.getStringProfile( elpDisplaySection, dispsect );
    displayBundle.openSection( dispsect );
    //----------------------------------

    QString entfile;
    loggerBundle.getStringProfile( elpEntryFile, entfile );
    BundleFile::bundleFiles[ epENTRYPROFILE ] ->openProfile( entfile, tr("Contest Entry Details") );
    //----------------------------------

    QString qthfile;
    loggerBundle.getStringProfile( elpQTHFile, qthfile );
    BundleFile::bundleFiles[ epQTHPROFILE ] ->openProfile( qthfile, tr("QTH Details") );
    //----------------------------------

    QString stationfile;
    loggerBundle.getStringProfile( elpStationFile, stationfile );
    BundleFile::bundleFiles[ epSTATIONPROFILE ] ->openProfile( stationfile, tr("Station Details") );
    //----------------------------------

    QString locsfile;
    loggerBundle.getStringProfile( elpLocsFile, locsfile );
    BundleFile::bundleFiles[ epLOCSQUARESPROFILE ] ->openProfile( locsfile, "Valid locator squares" );

    locsBundle.setProfile( BundleFile::bundleFiles[ epLOCSQUARESPROFILE ] );
    //----------------------------------
    loggerBundle.endGroup();

    initClock();
    TMatchThread::InitialiseMatchThread();

    return true;
}
bool contestAppLoadFiles( )
{
   return MultLists::getMultLists(); // ->loadMultFiles();
}
void closeContestApp( )
{
   if ( TContestApp::getContestApp() )
   {
      TContestApp::getContestApp() ->close();
   }
   delete TContestApp::getContestApp();
}
void TContestApp::close()
{
   writeContestList();

   for (auto &b: BundleFile::bundleFiles)
   {
        b.clear();
   }

   TMatchThread::FinishMatchThread();
}
TContestApp::TContestApp() : MinosParameters(),
      currentContest( nullptr ),
      magneticVariation( 0 ),
      preloadComplete(false),
      period1( 5 ),
      period2( 20 ),
      reverseBearing( false ),
      dispCountry( false ),
      dispBearing( true ),
    dispScore( true )

{

   for ( int i = 0; i < INITIAL_CONTEST_SLOTS; i++ )
   {
      QSharedPointer<ContestSlot> cs(new ContestSlot);
      cs->slotno = i;
      contestSlotList.insert( cs->slotno, cs );
   }
   for ( int i = 0; i < INITIAL_LIST_SLOTS; i++ )
   {
      QSharedPointer<ListSlot> cs(new ListSlot);
      cs->slotno = i;
      listSlotList.insert( cs->slotno, cs );
   }
}
TContestApp::~TContestApp()
{
   loggerBundle.closeProfile();
   logsPreloadBundle.closeProfile();
   listsPreloadBundle.closeProfile();
   displayBundle.closeProfile();
   contestApp = nullptr;

   delete MultLists::getMultLists();
   CsGuard::ClearDown();
}

bool TContestApp::insertContest(BaseContestLog * p, int sno )
{

   // called from LoggerContestLog::initialise
   if ( sno != -1 )
   {
      if ( sno >= contestSlotList.size() )
      {
         for ( int i = contestSlotList.size(); i <= sno; i++ )
         {
            QSharedPointer<ContestSlot> cs(new ContestSlot);
            cs->slotno = i;
            contestSlotList.insert( cs->slotno, cs );
         }
      }
      QSharedPointer<ContestSlot> cs = contestSlotList[ sno ];
      if ( cs->slot )
         return false;
      else
      {
         cs->slot = p;
         return true;
      }
   }
   for ( auto const &cs: qAsConst(contestSlotList ))
   {
      if ( !cs->slot )
      {
         cs->slot = p;
         return true;
      }
   }
   QSharedPointer<ContestSlot> cs(new ContestSlot);
   cs->slot = p;
   cs->slotno = contestSlotList.size();
   contestSlotList.insert( cs->slotno, cs );
   return true;
}
bool TContestApp::insertList(ContactList * p, int sno )
{

   // called from LoggerContestLog::initialise
   if ( static_cast< int > (sno) != -1 )
   {
      if ( sno >= listSlotList.size() )
      {
         for ( int i = listSlotList.size(); i <= sno; i++ )
         {
            QSharedPointer<ListSlot> cs(new ListSlot);
            cs->slotno = i;
            listSlotList.insert( cs->slotno, cs );
         }
      }
      QSharedPointer<ListSlot> cs(listSlotList[ sno ]);
      if ( cs->slot )
         return false;
      else
      {
         cs->slot = p;
         return true;
      }
   }
   for ( auto const &cs: qAsConst(listSlotList ))
   {
      if (cs && !cs->slot )
      {
         cs->slot = p;
         return true;
      }
   }
   QSharedPointer<ListSlot> cs(new ListSlot);
   cs->slot = p;
   cs->slotno = listSlotList.size();
   listSlotList.insert( cs->slotno, cs );
   return true;
}
int TContestApp::getOccupiedListSlotCount()
{
   int cnt = 0;
   for (auto const &i: qAsConst(listSlotList ))
   {
      if ( i && i->slot )
      {
         cnt++;
      }
   }
   return cnt;
}

bool TContestApp::isListOpen( const QString fn )
{
    for (auto const &i: qAsConst(listSlotList ))
   {
      if ( isOpen( i, fn ) )
      {
         mshowMessage( "File " + fn + " is already open!" );
         return true;
      }
   }
   return false;
}
bool TContestApp::isContestOpen(const QString fn )
{
   for ( auto const &i: qAsConst(TContestApp::getContestApp() ->contestSlotList ))
   {
      if ( isOpen( i, fn ) )
      {
         MinosParameters::getMinosParameters() ->mshowMessage( "File " + fn + " is already open!" );
         return true;
      }
   }
   return false;
}


BaseContestLog * TContestApp::findFirstContest()
{
    for (auto const &cs: qAsConst(contestSlotList ))
   {
      if ( cs->slot )
      {
         return cs->slot;
      }
   }
   return nullptr;
}
int TContestApp::findContest( BaseContestLog * p )
{
   for ( int i = 0; i < contestSlotList.size(); i++ )
   {
      QSharedPointer<ContestSlot> cs = contestSlotList[ i ];
      if ( cs->slot && p == cs->slot )
      {
         return i;
      }
   }
   return -1;
}
int TContestApp::findList( ContactList * p )
{
   for ( int i = 0; i < listSlotList.size(); i++ )
   {
      QSharedPointer<ListSlot> cs = listSlotList[ i ];
      if ( cs && cs->slot && p == cs->slot )
      {
         return i;
      }
   }
   return -1;
}
int TContestApp::removeContest(BaseContestLog * p )
{
   int i = findContest( p );
   {
       if ( i >= 0 )
       {
          {
             if ( getCurrentContest() == p )
                setCurrentContest( nullptr );
          }
          QSharedPointer<ContestSlot> cs = contestSlotList[ i ];
          cs->slot = nullptr;
       }
   }
   return i;
}
int TContestApp::removeList( ContactList * p )
{
   int i = findList( p );
   if ( i >= 0 )
   {
      QSharedPointer<ListSlot > cs = listSlotList[ i ];
      cs->slot = nullptr;
   }
   return i;
}
void TContestApp::writeContestList()
{
    if (preloadComplete && !suppressWritePreload)
    {
        logsPreloadBundle.clearProfileSection( false );

        // build a stripped, renumbered list
        int newSlotNo = 0;
        ContestSlotList newContestSlotList;
        for ( auto const &cs:  qAsConst(contestSlotList))
        {
            if (cs)
            {
                BaseContestLog * ct = cs->slot;
                if (ct)
                {
                    cs->slotno = newSlotNo;
                    newContestSlotList[newSlotNo] = cs;
                    newSlotNo++;
                }
            }
        }
        // replace the list
        contestSlotList = newContestSlotList;

        // and write it out
        for ( auto const &cs:  qAsConst(contestSlotList))
        {
            BaseContestLog * ct = cs->slot;
            if ( !ct )
            {
                continue;   // shouldn't happen...
            }

            QString ent = QString::number(cs->slotno);

            logsPreloadBundle.setStringProfile( ent, ct->cfileName );
            if ( currentContest == ct )
            {
                logsPreloadBundle.setIntProfile( eppCurrent, cs->slotno );
            }
        }
        if (contestSlotList.size() == 0)
        {
            logsPreloadBundle.setIntProfile( eppCurrent, 0 );
        }

        logsPreloadBundle.flushProfile();
    }
}
void TContestApp::writeListsList()
{
    if (preloadComplete)
    {

        // build a stripped, renumbered list
        int newSlotNo = 0;
        ListSlotList newListSlotList;
        for ( auto const &cs:  qAsConst(listSlotList))
        {
            if (cs) // after all, we may have just got rid of one...
            {
                ContactList * ct = cs->slot;
                if (ct)
                {
                    cs->slotno = newSlotNo;
                    ct->setSlotNo(newSlotNo);
                    newListSlotList[newSlotNo] = cs;
                    newSlotNo++;
                }
            }
        }
        // replace the list
        listSlotList = newListSlotList;

        // and write it out
        listsPreloadBundle.clearProfileSection( false );

        for ( auto const &cs: qAsConst(listSlotList))
        {
            if (cs)
            {
                ContactList * ct = cs->slot;
                if ( !ct )
                    continue;

                QString ent =  QString::number(cs->slotno + 1 );
                listsPreloadBundle.setStringProfile( ent, ct->cfileName );
            }
            // no need for any concept of a "current" list
        }
        listsPreloadBundle.flushProfile();
    }
}
QVector<BaseContestLog *> TContestApp::getContestList()
{
   QVector<BaseContestLog *> logList;
   for ( auto const &cs: qAsConst(contestSlotList ))
   {
      BaseContestLog * ct = cs->slot;
      if ( !ct )
         continue;

      logList.push_back(ct);
   }
   return logList;
}
LoggerContestLog * TContestApp::openFile( const QString &fn, bool newFile, int slotno )
{
   LoggerContestLog * contest = new LoggerContestLog();

   trace(QString("contest uuid for %1 is %2").arg(fn).arg(contest->uuid));


   if ( !contest->initialise( fn, newFile, slotno ) )    // this adds it to the slot
   {
      closeFile( contest );
      return nullptr;
   }

   return contest;
}
ContactList * TContestApp::openListFile(const QString &fn, int slotno )
{
   ContactList * list = new ContactList();
   if ( !list->initialise( fn, slotno ) )    // this adds it to the slot
   {
      removeList( list ); 		// must remove LoggerContestLog from its slot
      delete list;
      return nullptr;
   }

   return list;
}
void TContestApp::closeFile(BaseContestLog * contest)
{
   if ( contest )
   {
      removeContest( contest );
      delete contest;
   }

   writeContestList();
}
void TContestApp::closeListFile( ContactList *list )
{
   if ( list )
   {
      removeList( list );
      delete list;
   }
   writeListsList();
}
BaseContestLog * TContestApp::getCurrentContest()
{
   return currentContest;
}
void TContestApp::setCurrentContest( BaseContestLog * c )
{
   currentContest = c;
   if (!TContestApp::getContestApp() ->suppressWritePreload)
   {
       int cc = findContest( c );
       logsPreloadBundle.setIntProfile( eppCurrent, cc );
       logsPreloadBundle.flushProfile();
   }
}

void TContestApp::getDisplayColumnWidth( const QString &key, int &val, int def )
{
   displayBundle.getIntProfile( key, val, def );
}
void TContestApp::setDisplayColumnWidth( const QString &key, int val )
{
    if ( val < 0 )
    {
        displayBundle.setIntProfile( key, 0 );
    }
    else
    {
        displayBundle.setIntProfile( key, val );
    }
}
void TContestApp::getBoolDisplayProfile( int enumkey, bool &value )
{
   displayBundle.getBoolProfile( enumkey, value );
}
void TContestApp::setBoolDisplayProfile( int enumkey, bool value )
{
   displayBundle.setBoolProfile( enumkey, value );
}
void TContestApp::getIntDisplayProfile( int enumkey, int &value )
{
   displayBundle.getIntProfile( enumkey, value );
}
void TContestApp::setIntDisplayProfile( int enumkey, int value )
{
   displayBundle.setIntProfile( enumkey, value );
}
void TContestApp::getStringDisplayProfile( int enumkey, QString &value )
{
   displayBundle.getStringProfile( enumkey, value );
}
void TContestApp::setStringDisplayProfile( int enumkey, QString value )
{
   displayBundle.setStringProfile( enumkey, value );
}
void TContestApp::flushDisplayProfile( )
{
   displayBundle.flushProfile();
}
qint64 TContestApp::getBigClockCorrection()
{
   return bigClockCorr;
}

int TContestApp::getStatsPeriod1()
{
   displayBundle.getIntProfile( edpStatisticsPeriod1, period1 );
   return period1;
}
int TContestApp::getStatsPeriod2()
{
   displayBundle.getIntProfile( edpStatisticsPeriod2, period2 );
   return period2;
}
void TContestApp::setStatsPeriod1( int s1 )
{
   period1 = s1;
   displayBundle.setIntProfile( edpStatisticsPeriod1, period1 );
   displayBundle.flushProfile();
}
void TContestApp::setStatsPeriod2( int s2 )
{
   period2 = s2;
   displayBundle.setIntProfile( edpStatisticsPeriod2, period2 );
   displayBundle.flushProfile();
}
bool TContestApp::getAllowLoc4()
{
   BaseContestLog * ct = TContestApp::getCurrentContest();
   if ( !ct )
   {
      return false;
   }
   return ct->allowLoc4.getValue();
}
bool TContestApp::getAllowLoc8()
{
   BaseContestLog * ct = TContestApp::getCurrentContest();
   if ( !ct )
   {
      return false;
   }
   return ct->allowLoc8.getValue();
}
bool TContestApp::yesNoMessage( QWidget* Owner, const QString &mess )
{
   return mShowYesNoMessage( Owner, mess );
}
void TContestApp::mshowMessage( const QString &mess, QWidget* Owner )
{
   mShowMessage( mess, Owner );
}
int TContestApp::getContestSlotCount()
{
    return contestSlotList.size();
}
BaseContestLog *TContestApp::getContestSlot(int s)
{
    return contestSlotList[s]->slot;
}
int TContestApp::getListSlotCount()
{
    return listSlotList.size();
}
ContactList *TContestApp::getListSlot(int s)
{
    if (s >= getListSlotCount() || !listSlotList[s])
    {
        return nullptr;
    }
    return listSlotList[s]->slot;
}
