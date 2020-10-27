/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "MinosLoggerEvents.h"
#include "cutils.h"
#include "MinosParameters.h"

#include "MatchContact.h"
#include "MatchThread.h"
//#include "tsinglelogframe.h"
//#include "tlogcontainer.h"
#include "ScreenContact.h"
#include "contest.h"
#include "ListContact.h"
//---------------------------------------------------------------------------
TMatchThread *TMatchThread::matchThread = nullptr;

// wild card comparison, search string e for the wild card string in s
// At the moment we are using "space" as the wildcard.
// we always scan down s for the first char in e
bool wildComp( const QString &ss, const QString &ee )
{
   int s = 0;
   int sl = ss.length();
   int e = 0;
   int el = ee.length();

   while (s < sl && ss[s] == ' ' )
      s++;
   if ( s == sl )
      return false;
   while (e < el && ee[e] == ' ' )
      e++;
   if ( e == el )
      return false;


   int estart = e;

   // scan for first char of e in s

   int sstart = s;	// where to restart search

   while ( sstart < sl )
   {
      s = sstart;		// position moving pointer
      e = estart;		// go back to the start of the searching string

      while ( s < sl && e < el && ( ss[s] != ee[e] ) )
         s++;
      if ( s >= sl )
         return false;		// s has ended without a match on char 1 of e

      sstart = ++s;			// next time start one on from this match
      e++;						// first char has matched
      // now attempt to match
      while ( s < sl && e < el )
      {
         if (
            (ss[s] == ee[e] )
            || ( ( ee[e] == ' ' ) || ( ee[e] == '*' ) || ( ee[e] == '?' ) )
         )
         {
            s++;
            e++;
            continue;
         }
         break;		// match failed, break out
      }
      if ( e >= el )
         return true;		// we are at the end of the searching string, so matched

      // otherwise try again at next matching start char
   }
   return false;
}

TMatchThread::TMatchThread()
      : QThread(   ), myThisMatches( nullptr ), myOtherMatches( nullptr ),myListMatches( nullptr ),
        Terminated(false),
        contactToMatch(nullptr)
{
   thisLogMatch = new ThisLogMatcher();
   otherLogMatch = new OtherLogMatcher();
   listMatch = new ListMatcher();
   connect(&MinosLoggerEvents::mle, SIGNAL(ScreenContactChanged(ScreenContact*,BaseContestLog*,QString)), this, SLOT(on_ScreenContactChanged(ScreenContact*,BaseContestLog*,QString)));

   connect(&MinosLoggerEvents::mle, SIGNAL(CountrySelect(QString,BaseContestLog*)), this, SLOT(on_CountrySelect(QString,BaseContestLog*)));
   connect(&MinosLoggerEvents::mle, SIGNAL(DistrictSelect(QString,BaseContestLog*)), this, SLOT(on_DistrictSelect(QString,BaseContestLog*)));
   connect(&MinosLoggerEvents::mle, SIGNAL(LocSelect(QString,BaseContestLog*)), this, SLOT(on_LocatorSelect(QString,BaseContestLog*)));
}

QString TMatchThread::getBaseName() const
{
    return baseName;
}
void TMatchThread::FinishMatchThread()
{
    if ( matchThread )
    {
      matchThread->Terminate();
      trace( "WaitFor TMatchThread" );
      matchThread->wait(ULONG_MAX);
      delete matchThread;
      matchThread = nullptr;
   }
   trace( "TMatchThread close complete" );
}
void TMatchThread::InitialiseMatchThread()
{
   if ( !matchThread )
   {
      matchThread = new TMatchThread();
      matchThread->start(LowPriority);
   }
}
void TMatchThread::on_ScreenContactChanged(ScreenContact *sct, BaseContestLog *context, QString b)
{
   BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
   if (context == ct)
   {
       baseName = b;    // baseName makes sure that results are only acted on by the correct windows - log, edit, monitor
       contactToMatch = sct;
      if (contactToMatch)
      {
         // we want to initialise the search from the screen contact - break what couplings we can
         // we need to take care over thread safety as well!
         startMatch();
      }
   }
}
//---------------------------------------------------------------------------
void TMatchThread::on_CountrySelect(QString sel, BaseContestLog *c)
{
    BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
   if (c == ct)
   {
      contactToMatch = nullptr;
      QSharedPointer<CountryEntry> ce = MultLists::getMultLists() ->getCtryForPrefix( sel );
      startMatch(ce);
   }
}

void TMatchThread::on_DistrictSelect(QString
                                     #ifndef NDEBUG
                                     sel
                                     #endif
                                     , BaseContestLog *c)
{
    BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
   if (c == ct)
   {

   }
}

void TMatchThread::on_LocatorSelect(QString
                                    #ifndef NDEBUG
                                    sel
                                    #endif
                                    , BaseContestLog *c)
{
    BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
   if (c == ct)
   {

   }
}
//---------------------------------------------------------------------------
// limit the total  number of hits
const int MATCH_LIM = 20;
void TMatchThread::Execute()
{
   //---- Place thread code here ----
   try
   {
      while ( !Terminated )
      {
         thisLogMatch->initMatch();  // does nothing unless matchRequired is true
         otherLogMatch->initMatch();  // does nothing unless matchRequired is true
         listMatch->initMatch();

         // so it only does a max of 20+20 contacts before switching
         // to "other" of log/list
         if ( !thisLogMatch->idleMatch( 20 ) && !otherLogMatch->idleMatch( 20 ) && !listMatch->idleMatch( 20 ) )
            msleep(100);
      }

      // make sure everything is cleared down, so we can stop matching while managing lists
      replaceThisContestList(SharedMatchCollection(new TMatchCollection));
      replaceOtherContestList(SharedMatchCollection(new TMatchCollection));
      replaceListList(SharedMatchCollection(new TMatchCollection));

      delete thisLogMatch;
      thisLogMatch = nullptr;
      delete otherLogMatch;
      otherLogMatch = nullptr;
      delete listMatch;
      listMatch = nullptr;
   }
   catch ( ... )
   {
      // exception caught - in wrong thread, so we can't do much
      trace( "Exception in match thread" );
   }
   trace( "Exiting match thread" );
}
void TMatchThread::run()
{
    Execute();
}

//---------------------------------------------------------------------------
/*static*/ void TMatchThread::startMatch(   QSharedPointer<CountryEntry> ce )
{
   BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
   MinosLoggerEvents::SendMatchStarting(ct);

   matchThread->thisLogMatch->startMatch( ce );
   matchThread->otherLogMatch->startMatch( ce );
   matchThread->listMatch->startMatch( ce );

}
//---------------------------------------------------------------------------
void TMatchThread::replaceThisContestList( SharedMatchCollection matchCollection )
{
   myThisMatches = matchCollection;
   BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
   MinosLoggerEvents::SendReplaceThisLogList(myThisMatches, ct, baseName);
}
void TMatchThread::replaceOtherContestList( SharedMatchCollection matchCollection )
{
   myOtherMatches = matchCollection;
   BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
   MinosLoggerEvents::SendReplaceOtherLogList(myOtherMatches, ct, baseName);
}
//---------------------------------------------------------------------------
void TMatchThread::replaceListList(SharedMatchCollection matchCollection )
{
   myListMatches = matchCollection;
   BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
   MinosLoggerEvents::SendReplaceListList(myListMatches, ct, baseName);
}
//---------------------------------------------------------------------------
void TMatchThread::ShowThisMatchStatus( QString mess )
{
   thisMatchStatus = mess;
}
QString TMatchThread::getThisMatchStatus( )
{
   if ( matchThread )
   {
      return matchThread->thisMatchStatus;
   }
   return "";
}
void TMatchThread::ShowOtherMatchStatus( QString mess )
{
   otherMatchStatus = mess;
}
QString TMatchThread::getOtherMatchStatus( )
{
   if ( matchThread )
   {
      return matchThread->otherMatchStatus;
   }
   return "";
}
void TMatchThread::ShowListMatchStatus( QString mess )
{
   listMatchStatus = mess;
}
QString TMatchThread::getListMatchStatus( )
{
   if ( matchThread )
   {
      return matchThread->listMatchStatus;
   }
   return "";
}

//=============================================================================
void TMatchThread::matchCountry( QString cs )
{
   ctrymatch = cs;
   BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
   MinosLoggerEvents::SendScrollToCountry(ctrymatch, ct);
}
//=============================================================================
void  TMatchThread::matchDistrict( QString dist )
{
   distmatch = dist;
   BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
   MinosLoggerEvents::SendScrollToDistrict(distmatch, ct);
}

//=============================================================================
//=============================================================================
matchElement::matchElement() : match( false ), empty( true )
{
   mstr.clear();
   rawstr.clear();
}
unsigned char matchElement::checkGreater( const QString &s, QString &match_temp )
{
   unsigned char res = 0;
   if ( !s.length() )
   {
      return res;
   }

   strcpysp( match_temp, s, EXTRALENGTH );
   match_temp = match_temp.toUpper();

   int oldlen = rawstr.length();
   int newlen = match_temp.length();

   if ( newlen == oldlen )
   {
      if ( match_temp.compare( rawstr, Qt::CaseInsensitive ) == 0 )
         return res;		// no change
      res = SET_CHANGED | SET_NOT_GREATER;
   }
   else
   {
      res = SET_CHANGED | SET_NOT_GREATER;
      if ( ( oldlen == 0 ) || ( ( newlen > oldlen ) && ( strnicmp( rawstr, match_temp, oldlen ) == 0 ) ) )
         res &= ~SET_NOT_GREATER;
   }

   return res;
}
unsigned char matchElement::set( const QString &s )
{
   unsigned char res = 0;
   QString match_temp;
   match = false;
   empty = true;

   if ( !s.length() )
   {
      // Null argument used to clear it out
      mstr.clear();
      rawstr.clear();
      return res;
   }

   res = checkGreater( s, match_temp );

   if ( !match_temp.isEmpty() )
   {
      empty = false;
      match = true;
   }

   mstr = match_temp;
   rawstr = match_temp;
   return res;
}
bool matchElement::checkMatch( const QString &s )
{
   bool ismatch = true;
   if ( match && !empty )
   {
      if ( !wildComp( s, mstr ) )
         ismatch = false;
   }

   return ismatch;
}

//=============================================================================

Matcher::Matcher() :
      matchRequired( false ),
      matchPhase( empExact ),
      matchStarted( false ),
      firstMatch( ecpStarting ),
      tickct( 0 ),
      contestIndex( -1 ),
      contactIndex( -1 ),
      countryEntry( nullptr )
{
   matchCollection = SharedMatchCollection(new TMatchCollection);
}
Matcher::~Matcher()
{
}

void Matcher::clearmatchall( )
{
   contestIndex = -1;
   contactIndex = -1;
   matchStarted = false;
   matchRequired = false;

   matchCollection->contestMatchList.clear();

   // don't want to keep the old, so clear down the memory of selection terms
   matchcs.set( "" );
   matchloc.set( "" );
   matchqth.set( "" );
   countryEntry.reset();
   thisContestMatched = 0;

   TMatchThread::getMatchThread() ->ShowThisMatchStatus( "" );
   TMatchThread::getMatchThread() ->ShowOtherMatchStatus( "" );
}
// mark things that we want to start a match
void Matcher::startMatch( QSharedPointer<CountryEntry> c )
{
   matchRequired = true;
   matchStarted = false;				// if it started we need to stop it...
   tickct = 0;
   countryEntry = c;
   thisContestMatched = 0;
}

// on timer, start it off NB this NOT in the main thread
void Matcher::initMatch( )
{
   try
   {
      if ( matchRequired )
      {
         tickct++;
         if ( tickct < 2 )
            return ;
      }
      else
      {
         return ;
      }

      if ( !MinosParameters::getMinosParameters() ->getCurrentContest() )
      {
         clearmatchall();
         return ;
      }

      if ( !countryEntry )
      {
         clearmatchall();
         // we need to have been sent this... in a start match action?
         ScreenContact * mct = TMatchThread::getMatchThread()->contactToMatch;
         if ( !mct )
            return ;

         unsigned char qth_changed = matchqth.set( mct->extraText );
         if ( qth_changed & SET_CHANGED )
         {
            matchDistrict( mct->extraText );
         }
         unsigned char changed = matchcs.set( mct->cs.getFullCall() );	// we rely on set to set up the search terms
         if ( changed & SET_CHANGED )
         {
            matchCountry( mct->cs.getFullCall() );   // scroll to
         }
         changed |= matchloc.set( mct->loc.getLoc() );			// so don't do this all on one line
         changed |= qth_changed;

         if ( matchcs.empty && matchloc.empty && matchqth.empty )
         {
            clearmatchall();
            replaceList( );
            return ;
         }

         if ( !(changed & SET_CHANGED) )
            return ;

         if ( changed & SET_NOT_GREATER )
         {
            replaceList( );
         }
         // we have something matchable, so go to it!

         matchPhase = empExact;		// set phase
      }
      else
      {
         QSharedPointer<CountryEntry> c = countryEntry;	// preserve over cleardown
         clearmatchall();
         matchPhase = empCountry;
         countryEntry = c;
      }

      contestIndex = 0;
      contactIndex = 0;
      matchStarted = true;
      firstMatch = ecpStarting;
   }
   catch ( ... )
   {
      trace( "Exception in init match" );
   }
}
//==============================================================================
ThisLogMatcher::ThisLogMatcher()
{}
ThisLogMatcher::~ThisLogMatcher()
{
    clearmatchall();
}
void ThisLogMatcher::matchDistrict( const QString &extraText )
{
   TMatchThread::getMatchThread() ->matchDistrict( extraText );   // scroll to
}
void ThisLogMatcher::matchCountry( const QString &cs )
{
   TMatchThread::getMatchThread() ->matchCountry( cs );   // scroll to
}
void ThisLogMatcher::addMatch( QSharedPointer<BaseContact> cct, BaseContestLog *ccon )
{
   if ( !cct )
      return ;

   ContestMatchList &contestMatchList = matchCollection->contestMatchList;
   MapWrapper<BaseMatchContest> wmc = MapWrapper<BaseMatchContest>(new MatchContactLog);
   if (contestMatchList.size() == 0)
   {
       wmc.wt->matchedContest = ccon;
       contestMatchList.insert(wmc, wmc);
   }
   QSharedPointer<BaseMatchContest> found;
   for(auto const &test: contestMatchList)
   {
       if (test.wt->getContactLog() == ccon)
       {
            found = test.wt;
            MapWrapper<MatchContact> wmct(new MatchLogContact( ccon, cct ));
            found->contactMatchList.insert( wmct, wmct );
            break;
       }
   }

   thisContestMatched = matchCollection->contactCount();
}
bool Matcher::reduceScanAccuracy()
{
    bool EndScan = true;
    bool dropthrough = true;
    while (dropthrough)
    {
        dropthrough = false;
        switch ( matchPhase )
        {
           case empExact:
              {
                // we strip back to the basic callsign, as prefixes and suffixes aren't
                // enough to stop a callsign match in main contest
                 if ( matchcs.match )
                 {
                    QString smstr = matchcs.mstr;
                    int c = 0;
                    // need to trim out any leading and trailing
                    while ( c < smstr.length() && smstr[c] != '/' )
                       c++;

                    int c2 = c;
                    if ( c2 < smstr.length() )
                       c2++;	// skip the first /
                    while ( c2 < smstr.length() && smstr[c2] != '/' )
                       c2++;

                    if ( c < smstr.length() && c2 < smstr.length() )
                    {
                       // both prefix and suffix found
                       // main part of callsign - pa/g0gjv/p goes to g0gjv
                       matchcs.mstr = smstr.mid(c + 1, c2 - c - 1 );	// copy back over ourselves
                    }
                    else
                       if ( c < smstr.length() && ( c < 3 ) && smstr.length() - c > 2  )
                       {
                           // e.g. pa/g0gjv
                          // prefix less than 3 chars and suffix more than 1 character
                          matchcs.mstr = smstr.mid( c + 1 );	// copy back over ourselves
                       }
                       else
                          if ( c < smstr.length() && smstr[c] == '/' )
                          {
                              // e.g. g0gjv/p
                             // remove suffix
                             matchcs.mstr = matchcs.mstr.left( c );	// copy back over ourselves
                          }
                          else
                          {
                             // neither leading or trailing /
                             // want to drop through to reduce loc/qth
                             dropthrough = true;
                          }

                    if ( !dropthrough )
                    {
                       //TMatchThread::getMatchThread() ->ShowThisMatchStatus( tr(" - No exact match") );
                       matchPhase = empNoSuffix;
                       contestIndex = 0;
                       contactIndex = 0;
                       firstMatch = ecpMainContest;
                       EndScan = false;
                       break;
                    }
                 }
              }
            matchPhase = empNoSuffix;
            dropthrough = true;
            break;

           case empNoSuffix:
              {
                // we have stripped back to a basic callsign
                // or no /P anyway, so drop through
                 if ( matchcs.match && ( matchloc.match || matchqth.match ) )
                 {
                    matchloc.set( "" );
                    matchqth.set( "" );
                    matchPhase = empNoLoc;
                    contestIndex = 0;
                    contactIndex = 0;
                    firstMatch = ecpMainContest;
                    EndScan = false;
                    //TMatchThread::getMatchThread() ->ShowThisMatchStatus( tr(" - No match No Suffix") );
                    break;
                 }
              }
            dropthrough = true;
            matchPhase = empNoLoc;
            break;

           case empNoLoc:
             {
            // or no loc anyway, so drop through
                 if ( matchcs.match )
                 {
                    // We know that cs is not empty
                    // strip temp cs of its leading country and number
                    // e.g. g0gjv to gjv
                    QString smstrStart = matchcs.mstr;
                    int c = smstrStart.length();

                    while ( c > 0 )
                    {
                       if ( !smstrStart[c - 1].isDigit())
                          c--;
                       else
                          break;
                    }

                    if ( c > 0 )   	// i.e. we havent got back to the start
                    {
                       if ( smstrStart.length() - c >= 2 )   			// we would be left with something
                       {
                          matchcs.mstr = smstrStart.mid(c);	// copy back over
                          matchPhase = empBody;
                          contestIndex = 0;
                          contactIndex = 0;
                          firstMatch = ecpMainContest;
                          EndScan = false;
                          //TMatchThread::getMatchThread() ->ShowThisMatchStatus( tr(" - No match No LOC") );
                          break;
                       }
                    }
                 }
              }
            matchPhase = empBody;
            dropthrough = true;
            break;

           case empCountry:
           case empDistrict:
           case empLocator:
           case empBody:
              {
            // else we have already done what we can, so drop through
                 // we have finished.
                 break;
              }
        }
    }
    return EndScan;
}

bool ThisLogMatcher::idleMatch( int limit )
{
    try
    {
       // this is called repeatedly off the timer to execute the match
       if ( !matchStarted || ( contestIndex < 0 ) )
          return false;				// nothing to do (yet)

       int cnt = matchCollection->contactCount();
       if ( ( firstMatch == ecpStarting ) && ( ( matchPhase == empExact ) || ( matchPhase == empCountry ) ) )
       {
           // right at the start, we try for a pure match
          contestIndex = 0;
          contactIndex = 0;
          firstMatch = ecpMainContest;
       }
       else
       {
          if ( ( contestIndex >= 1 ) || ( cnt > MATCH_LIM ) )
          {
             // we have done a complete pass, reduce accuracy

             bool EndScan = true;

             if ( cnt == 0 )
             {
                 // if we don't have any matches, reduce accuracy
                EndScan = reduceScanAccuracy();
             }

             if ( EndScan )
             {
                 // we are there... all passes done
                matchStarted = false;
                setMatchRequired( false );

                QString baseName = TMatchThread::getMatchThread() ->getBaseName();
                BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
                if (baseName == "Log" && bool( ct ) && ( matchPhase != empCountry )  && ( matchPhase != empDistrict )  && ( matchPhase != empLocator ) )
                {
                   addMatch( ct->DupSheet.getCurDup(), ct );	// in case it isn't already
                }

                QString buff;
                // now make it display
                replaceList( );

                if ( thisContestMatched )
                {
                   // conteste focused the top line of matches here
                   // want to manage plurals, and local/other contests
                   QString matchSuffix = tr("Possible");
                   if (matchPhase == empCountry)
                   {
                      matchSuffix = tr("Country");
                   }
                   else if (matchPhase == empDistrict)
                   {
                      matchSuffix = tr("District");
                   }
                   else if (matchPhase == empLocator)
                   {
                      matchSuffix = tr("Locator");
                   }
                   buff = tr( " - %1%2 %3 matches" )
                           .arg ( ( cnt > MATCH_LIM ) ? ">" : "" )
                           .arg(thisContestMatched)
                           .arg( ( matchPhase == empExact ) ? "" : matchSuffix );
                }
                else
                {
                   buff = " - No match";
                }
                TMatchThread::getMatchThread() ->ShowThisMatchStatus( buff );
                contestIndex = -1;
                return false;
             }
          }
       }
       BaseContestLog *ccon = MinosParameters::getMinosParameters() ->getCurrentContest();		// we always go through current FIRST! to make sure we see it

       while (  matchStarted && limit > 0 )
       {
          if ( !ccon ||
               ( contactIndex >= ccon->getContactCount() )
             )
          {
             // we need to move on

             contactIndex = 0;
             contestIndex++;
             return true;
          }
          QSharedPointer<BaseContact> cct = ccon->pcontactAt( contactIndex++ );
          if ( !cct)
             return true;

          unsigned short cf = cct->contactFlags.getValue();
          if ( cf & ( LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) )
          {
              continue;
          }
          // now do the match

          if ( matchPhase != empCountry && matchPhase != empDistrict && matchPhase != empLocator )
          {
             bool csmatch = false;
             bool locmatch = false;
             bool qthmatch = false;

             QString matchPart;
             if (matchPhase == empBody)
             {
                 matchPart = cct->cs.body;
             }
             else
             {
                 matchPart = cct->cs.getFullCall();
             }
             csmatch = matchcs.checkMatch( matchPart );

             if ( csmatch )
             {
                locmatch = matchloc.checkMatch( cct->loc.getLoc() );
             }

             if ( csmatch && locmatch )
             {
                if ( !matchqth.match || matchqth.empty )
                {
                   qthmatch = true;
                }
                else
                {
                   QString uprqth;
                   strcpysp( uprqth, cct->extraText.getValue(), EXTRALENGTH );
                   uprqth = uprqth.toUpper();
                   qthmatch = matchqth.checkMatch( uprqth );

                   if ( ccon->districtMult.getValue() )
                   {
                       if ( ccon != nullptr && !qthmatch )
                       {
                         // attempt match against district code
                         if ( cct->districtMult && wildComp( cct->districtMult->districtCode, matchqth.mstr ) )
                            qthmatch = true;
                       }
                   }
                }
             }

             if ( csmatch && locmatch && qthmatch )
             {
                addMatch( cct, ccon );
                if ( matchCollection->contactCount() > MATCH_LIM )
                   break;
             }
          }
          else if (matchPhase == empCountry)
          {
             if ( cct->ctryMult && ( cct->ctryMult == countryEntry ) )
             {
                addMatch( cct, ccon );
                if ( matchCollection->contactCount() > MATCH_LIM )
                   break;
             }

          }
          else if (matchPhase == empDistrict)
          {
 /*            if ( cct->ctryMult && ( cct->ctryMult == ce ) )
             {
                addMatch( cct, ccon );
                if ( matchCollection->contactCount() > MATCH_LIM )
                   break;
             }
 */
          }
          else if (matchPhase == empLocator)
          {
 /*            if ( cct->ctryMult && ( cct->ctryMult == ce ) )
             {
                addMatch( cct, ccon );
                if ( matchCollection->contactCount() > MATCH_LIM )
                   break;
             }
 */
          }
       }
    }
    catch ( ... )
    {
       trace( "Exception in log idle match" );
    }
    return true;
}
void ThisLogMatcher::replaceList(  )
{
   TMatchThread::getMatchThread() ->replaceThisContestList( matchCollection );
   matchCollection = SharedMatchCollection(new TMatchCollection);
}
//==============================================================================
OtherLogMatcher::OtherLogMatcher()
{}
OtherLogMatcher::~OtherLogMatcher()
{
    clearmatchall();
}
void OtherLogMatcher::matchDistrict( const QString &extraText )
{
   TMatchThread::getMatchThread() ->matchDistrict( extraText );   // scroll to
}
void OtherLogMatcher::matchCountry( const QString &cs )
{
   TMatchThread::getMatchThread() ->matchCountry( cs );   // scroll to
}
void OtherLogMatcher::addMatch( QSharedPointer<BaseContact> cct, BaseContestLog * ccon )
{

   if ( !cct )
      return ;

   ContestMatchList &contestMatchList = matchCollection->contestMatchList;
   MapWrapper<BaseMatchContest> wmc(new MatchContactLog);


   QSharedPointer<BaseMatchContest> found;
   for(auto const &test: contestMatchList)
   {
       if (test.wt->getContactLog() == ccon)
       {
            found = test.wt;
            break;
       }
   }
   if (!found)
   {
       wmc.wt->matchedContest = ccon;
       contestMatchList.insert(wmc, wmc);
   }
   found.reset();
   for(auto const &test: contestMatchList)
   {
       const BaseContestLog *tcon = test.wt->getContactLog();
       if (tcon == ccon)
       {
            found = test.wt;
            MapWrapper<MatchContact> mct(new MatchLogContact( ccon, cct ));
            found->contactMatchList.insert( mct, mct );
            break;
       }
   }

   thisContestMatched = matchCollection->contactCount();
}
bool OtherLogMatcher::idleMatch( int limit )
{
   try
   {
      // this is called repeatedly off the timer to execute the match
      if ( !matchStarted || ( contestIndex < 0 ) )
         return false;				// nothing to do (yet)

      int cnt = matchCollection->contactCount();
      if ( ( firstMatch == ecpStarting ) && ( ( matchPhase == empExact ) || ( matchPhase == empCountry ) ) )
      {
         contestIndex = 0;
         contactIndex = 0;
         firstMatch = ecpRest;
      }
      else
      {
         if ( ( contestIndex >= MinosParameters::getMinosParameters() ->getContestSlotCount() ) || ( cnt > MATCH_LIM ) )
         {
             // we have done a complete pass, reduce accuracy
            bool EndScan = true;

            if ( cnt == 0 )
            {
                EndScan = reduceScanAccuracy();

            }

            if ( EndScan )
            {
               matchStarted = false;
               setMatchRequired( false );

               // now make it display
               replaceList( );

               QString buff = " - No match";

               TMatchThread::getMatchThread() ->ShowOtherMatchStatus( buff );
               contestIndex = -1;
               return false;
            }
         }
     }
      BaseContestLog * ccon = MinosParameters::getMinosParameters()->getContestSlot(contestIndex);
      if ( ccon == MinosParameters::getMinosParameters() ->getCurrentContest() )
      {
         contactIndex = ccon->getContactCount();	// force to go on
      }

      while (  matchStarted && limit > 0 )
      {
         if ( !ccon ||
              ( contactIndex >= ccon->getContactCount() )
            )
         {
            // we need to move on

            contactIndex = 0;
            contestIndex++;
            return true;
         }
         QSharedPointer<BaseContact> cct = ccon->pcontactAt( contactIndex++ );
         if ( !cct )
            return true;

         unsigned short cf = cct->contactFlags.getValue();
         if ( cf & ( LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) )
         {
             continue;
         }
         // now do the match

         if ( matchPhase != empCountry && matchPhase != empDistrict && matchPhase != empLocator )
         {
            bool csmatch = false;
            bool locmatch = false;
            bool qthmatch = false;

            QString matchPart;
            if (matchPhase == empBody)
            {
                matchPart = cct->cs.body;
            }
            else
            {
                matchPart = cct->cs.getFullCall();
            }
            csmatch = matchcs.checkMatch( matchPart );

            if ( csmatch )
            {
               locmatch = matchloc.checkMatch( cct->loc.getLoc() );
            }

            if ( csmatch && locmatch )
            {
               if ( !matchqth.match || matchqth.empty )
               {
                  qthmatch = true;
               }
               else
               {
                  QString uprqth;
                  strcpysp( uprqth, cct->extraText.getValue(), EXTRALENGTH );
                  uprqth = uprqth.toUpper();
                  qthmatch = matchqth.checkMatch( uprqth );

                  if ( ccon != nullptr && !qthmatch )
                  {
                     if ( ccon->districtMult.getValue() )
                     {
                        // attempt match against district code
                        if ( cct->districtMult && wildComp( cct->districtMult->districtCode, matchqth.mstr ) )
                           qthmatch = true;
                     }
                  }
               }
            }

            if ( csmatch && locmatch && qthmatch )
            {
               addMatch( cct, ccon );
               if ( matchCollection->contactCount() > MATCH_LIM )
                  break;
            }
         }
         else if (matchPhase == empCountry)
         {
            if ( cct->ctryMult && ( cct->ctryMult == countryEntry ) )
            {
               addMatch( cct, ccon );
               if ( matchCollection->contactCount() > MATCH_LIM )
                  break;
            }

         }
         else if (matchPhase == empDistrict)
         {
/*            if ( cct->ctryMult && ( cct->ctryMult == ce ) )
            {
               addMatch( cct, ccon );
               if ( matchCollection->contactCount() > MATCH_LIM )
                  break;
            }
*/
         }
         else if (matchPhase == empLocator)
         {
/*            if ( cct->ctryMult && ( cct->ctryMult == ce ) )
            {
               addMatch( cct, ccon );
               if ( matchCollection->contactCount() > MATCH_LIM )
                  break;
            }
*/
         }
      }
   }
   catch ( ... )
   {
      trace( "Exception in log idle match" );
   }
   return true;
}
void OtherLogMatcher::replaceList( )
{
   TMatchThread::getMatchThread() ->replaceOtherContestList( matchCollection );
   matchCollection = SharedMatchCollection(new TMatchCollection);

}

//==============================================================================
ListMatcher::ListMatcher()
{}
ListMatcher::~ListMatcher()
{
    clearmatchall();
}
void ListMatcher::matchDistrict( const QString &/*extraText*/ )
{}
void ListMatcher::matchCountry( const QString &/*cs*/ )
{}
void ListMatcher::addMatch( ListContact *lct, ContactList * clist )
{
       if ( !lct )
          return ;

       ContestMatchList &contestMatchList = matchCollection->contestMatchList;

       MapWrapper<BaseMatchContest> wmc(new MatchContactList);

       QSharedPointer<BaseMatchContest> found;
       for(auto const &test: contestMatchList)
       {
           if (test.wt->getContactList() == clist)
           {
                found = test.wt;
                break;
           }
       }
       if (!found)
       {
           wmc.wt->matchedContest = clist;
           contestMatchList.insert(wmc, wmc);
       }
       found.reset();
       for(auto const &test: contestMatchList)
       {
           if (test.wt->getContactList() == clist)
           {
                found = test.wt;
                MapWrapper<MatchContact> mct(new MatchListContact( clist, lct ));
                found->contactMatchList.insert( mct, mct );
                break;
           }
       }

       thisContestMatched = matchCollection->contactCount();
}

bool ListMatcher::idleMatch( int limit )
{
   try
   {
      // this is called repeatedly off the timer to execute the match
      if ( !matchStarted || ( contestIndex < 0 ) )
         return false;				// nothing to do (yet)
      if ( MinosParameters::getMinosParameters() ->getListSlotCount() <= 0 )
         return false;

      int cnt = matchCollection->contactCount();
      if ( ( firstMatch == ecpStarting )
           && ( ( matchPhase == empExact )
                || ( matchPhase == empCountry )
                || ( matchPhase == empDistrict )
                || ( matchPhase == empLocator ) ) )
      {
         contestIndex = 0;
         contactIndex = 0;
         firstMatch = ecpRest;
      }
      else
      {
         if ( ( contestIndex >= MinosParameters::getMinosParameters() ->getListSlotCount() ) || ( cnt > MATCH_LIM ) )
         {
            bool EndScan = true;

            if ( cnt == 0 )
            {
                EndScan = reduceScanAccuracy();
            }

            if ( EndScan )
            {
               matchStarted = false;
               setMatchRequired( false );

               // now make it display
               replaceList( );

               contestIndex = -1;
               return false;
            }
         }
      }
      ContactList *ccon = MinosParameters::getMinosParameters()->getListSlot(contestIndex);

      while ( limit > 0  && !countryEntry)
      {
         if ( !ccon || ( contactIndex >= ccon->getContactCount() ) )
         {
            // we need to move on

           // go to the next ContestLog
           contestIndex++;
           contactIndex = 0;
           return true;
         }
         ListContact *cct = ccon->pcontactAt( contactIndex++ );
         if ( !cct )
         {
            return true;
         }

         // now do the match
// And why aren't we doing a QTH match here?
         bool csmatch = false;
         bool locmatch = false;
         bool qthmatch = false;

         QString matchPart;
         if (matchPhase == empBody)
         {
             matchPart = cct->cs.body;
         }
         else
         {
             matchPart = cct->cs.getFullCall();
         }

         csmatch = matchcs.checkMatch( matchPart );

         if ( csmatch )
         {
            locmatch = matchloc.checkMatch( cct->loc.getLoc() );
         }


         if ( csmatch && locmatch )
         {
             if ( !matchqth.match || matchqth.empty )
             {
                qthmatch = true;
             }
             else
             {
                QString uprqth;
                strcpysp( uprqth, cct->extraText, EXTRALENGTH );
                uprqth = uprqth.toUpper();
                qthmatch = matchqth.checkMatch( uprqth );

                if ( ccon != nullptr && !qthmatch )
                {
                  // attempt match against district code
                  if ( wildComp( cct->extraText, matchqth.mstr ) )
                     qthmatch = true;
                }
             }
             if ( csmatch && locmatch && qthmatch )
             {
                addMatch( cct, ccon );

                if ( matchCollection->contactCount() > MATCH_LIM )
                   break;
             }
         }
      }
   }
   catch ( ... )
   {
      trace( "Exception in list idle match" );
   }
   return true;
}
void ListMatcher::replaceList( )
{
   TMatchThread::getMatchThread() ->replaceListList( matchCollection );
   matchCollection = SharedMatchCollection(new TMatchCollection);  // pass ownership

}

