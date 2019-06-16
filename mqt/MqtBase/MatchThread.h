/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#include "base_pch.h"
#include "MatchCollection.h"

#ifndef MatchThreadH
#define MatchThreadH 
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define SET_CHANGED 1
#define SET_NOT_GREATER 2



class Matcher
{
      bool matchRequired;  // use getter and setter below
   protected:
      enum MatchPhase {Exact, NoSuffix, NoLoc, Body, Country, District, Locator};
      enum ContestPhase {Starting, MainContest, Rest};

      MatchPhase mp;
      bool matchStarted;
      ContestPhase firstMatch;
      int tickct;

      int contestIndex;
      int contactIndex;

      matchElement matchcs;
      matchElement matchloc;
      matchElement matchqth;

      QSharedPointer<CountryEntry> ce;

      int thisContestMatched;
      virtual void matchDistrict( const QString &extraText ) = 0;
      virtual void matchCountry( const QString &cs ) = 0;
      virtual void replaceList( ) = 0;
   public:

      Matcher( );
      virtual ~Matcher();
      SharedMatchCollection matchCollection;

      void startMatch(QSharedPointer<CountryEntry> ce = QSharedPointer<CountryEntry>() );
      void initMatch();
      void clearmatchall();
      virtual bool idleMatch( int limit ) = 0;

      void setMatchRequired( bool b )
      {
         matchRequired = b;
      }
      bool getMatchRequired()
      {
         return matchRequired;
      }

};
class ThisLogMatcher: public Matcher
{
      virtual void matchDistrict( const QString &extraText );
      virtual void matchCountry( const QString &cs );
      virtual void replaceList(  );
   public:
      ThisLogMatcher( );
      virtual ~ThisLogMatcher();

      virtual bool idleMatch( int limit );
      void addMatch(QSharedPointer<BaseContact>, BaseContestLog * );
};
class OtherLogMatcher: public Matcher
{
      virtual void matchDistrict( const QString &extraText );
      virtual void matchCountry( const QString &cs );
      virtual void replaceList( );
   public:
      OtherLogMatcher( );
      virtual ~OtherLogMatcher();

      virtual bool idleMatch( int limit );
      void addMatch(QSharedPointer<BaseContact>, BaseContestLog * );
};
class ListMatcher: public Matcher
{
      virtual void matchDistrict( const QString &extraText );
      virtual void matchCountry( const QString &cs );
      virtual void replaceList(  );
   public:
      ListMatcher( );
      ~ListMatcher();

      virtual bool idleMatch( int limit );
      void addMatch( ListContact *, ContactList * );
};
//---------------------------------------------------------------------------
class TMatchThread : public QThread
{
    Q_OBJECT

private:

      static void startMatch(QSharedPointer<CountryEntry> ce = QSharedPointer<CountryEntry>() );

      static TMatchThread *matchThread;
      TMatchThread();

      QString baseName;
      ThisLogMatcher *thisLogMatch;
      OtherLogMatcher *otherLogMatch;
      ListMatcher *listMatch;

      SharedMatchCollection myThisMatches;       // used to pass the match list out
      SharedMatchCollection myOtherMatches;      // used to pass the match list out
      SharedMatchCollection myListMatches;       // used to pass the match list out

      QString ctrymatch;
      QString distmatch;

      QString thisMatchStatus;
      QString otherMatchStatus;

      bool Terminated;

   protected:
      virtual void Execute();    // TThread method
   public:
      ScreenContact * mct = nullptr;
      void Terminate()
      {
          Terminated = true;
      }

      void replaceThisContestList(SharedMatchCollection matchCollection );
      void replaceOtherContestList( SharedMatchCollection matchCollection );
      void replaceListList( SharedMatchCollection matchCollection );
      void matchCountry( QString cs );
      void matchDistrict( QString dist );

      void ShowThisMatchStatus( QString mess );
      void ShowOtherMatchStatus( QString mess );
      static QString getThisMatchStatus( );
      static QString getOtherMatchStatus( );
      static void InitialiseMatchThread();
      static void FinishMatchThread();
      static TMatchThread *getMatchThread()
      {
         return matchThread;
      }

      // QThread interface
      QString getBaseName() const;

protected:
      virtual void run() override;
private slots:
      void on_ScreenContactChanged(ScreenContact *mct, BaseContestLog *context, QString b);
      void on_CountrySelect(QString cty, BaseContestLog *c);
      void on_DistrictSelect(QString dist, BaseContestLog *c);
      void on_LocatorSelect(QString dist, BaseContestLog *c);
};
//---------------------------------------------------------------------------
#endif
