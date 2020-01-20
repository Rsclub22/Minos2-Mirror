/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "cutils.h"
#include "contest.h"
#include "contacts.h"
//============================================================
ContactBuffs contactBuffs;
//==========================================================================
BaseContact::BaseContact( BaseContestLog * contest, dtg time_now ) :
      contest( contest )
    , updtime( true )
    , time( time_now )
    , contactFlags( 0 )
    , contactScore( -1 )
    , bearing( -1 )
    , QSOValid( false )
    , newDistrict( false )
    , newCtry( false )
    , locCount( 0 )
    , newGLoc(false)
    , newNonGLoc(false)
    , bonus(0)
    , newBonus(false)
    , multCount( 0 )
{
}
BaseContact::BaseContact( const BaseContact &ct )
      : updtime( false )
      , time( false )
{
   *this = ct;
}
BaseContact& BaseContact::operator =( const BaseContact &ct )
{
   contest = ct.contest;
   updtime = ct.updtime;      //CONTAIN MinosItem
   cs = ct.cs;   //CONTAIN MinosItem
   loc = ct.loc;   //CONTAIN MinosItem
   time = ct.time;      //CONTAIN MinosItem

   extraText = ct.extraText;
   mode = ct.mode;
   mgmSubmode = ct.mgmSubmode;
   reps = ct.reps;
   serials = ct.serials;
   repr = ct.repr;
   serialr = ct.serialr;
   comments = ct.comments;
   contactFlags = ct.contactFlags;
   forcedMult = ct.forcedMult;
   frequency = ct.frequency;
   rotatorHeading = ct.rotatorHeading;
   rigName = ct.rigName;

   op1 = ct.op1;         // current main op - derived from contacts
   op2 = ct.op2;         // current second op - derived from contacts

   contactScore = ct.contactScore;
   bearing = ct.bearing;

   return *this;
}
//==========================================================================
bool BaseContact::operator<( const BaseContact& rhs ) const
{
   return getLogSequence() < rhs.getLogSequence();
}
//==========================================================================
void BaseContact::clearDirty()
{
   cs.fullCall.clearDirty();
   loc.loc.clearDirty();
   time.clearDirty();

   extraText.clearDirty();
   mode.clearDirty();
   mgmSubmode.clearDirty();
   reps.clearDirty();
   serials.clearDirty();
   repr.clearDirty();
   serialr.clearDirty();
   comments.clearDirty();
   contactFlags.clearDirty();
   forcedMult.clearDirty();
   frequency.clearDirty();
   rotatorHeading.clearDirty();
   rigName.clearDirty();
   op1.clearDirty();
   op2.clearDirty();
   contactScore.clearDirty();
}
void BaseContact::setDirty()
{
   cs.fullCall.setDirty();
   loc.loc.setDirty();
   time.setDirty();

   extraText.setDirty();
   mode.setDirty();
   mgmSubmode.setDirty();
   reps.setDirty();
   serials.setDirty();
   repr.setDirty();
   serialr.setDirty();
   comments.setDirty();
   contactFlags.setDirty();
   forcedMult.setDirty();
   frequency.setDirty();
   rotatorHeading.setDirty();
   rigName.setDirty();
   op1.setDirty();
   op2.setDirty();
   contactScore.setDirty();
}
//==========================================================================
QSharedPointer<CountryEntry> findCtryPrefix( const Callsign &cs )
{
   QString testpart = "/";	// look for e.g. /RVI as a country suffix
   testpart += cs.suffix;	// look for e.g. /RVI as a country suffix

   QSharedPointer<CountrySynonym> csyn;
   if ( cs.suffix.length() )
      csyn = MultLists::getMultLists() ->searchCountrySynonym( testpart );

   if ( !csyn )   	// look with number
   {
      /*
      		// eg for <pe/f0ctt/mm> (g0gjv/p) [F6CTT/RVI/P] ?F6CTT/RVI?
      		char prefix[BITLENGTH + 1]; // <pe> (g) [RVI] ???? country of location
      		char prefix2[BITLENGTH + 1];  // <f> (g) [F] ?F?country of issue
      		char number[NUMBITLENGTH + 1]; // <0> (0) [6] ?6?numeric part
      		char body[BITLENGTH + 1];  // <ctt> (gjv) [CTT] ?CTT?main body
      		char suffix[TRAILBITLENGTH + 1]; // <mm> (p) [P] ?RVI?trailer
      */
      if ( cs.locCtryPrefix != cs.dupPrefix )
      {
         // we have a leading / for a pre-pended prefix, so callsign itself is
         // not relevant
         csyn = MultLists::getMultLists() ->searchCountrySynonym( cs.locCtryPrefix );
      }

      if ( !csyn )
      {
         // take the whole callsign, extra prefix, suffix, the lot and look for the
         // longest matching synonym. If the list is incomplete then this may
         // misidentify the country - e.g. if GW were missed out then this algorithm
         // would (wrongly?) allow a match on G. This might be a benefit for e.g.
         // PA or DL, as we don't need to put all the synonyms in, just the base
         // letter

         // There are really foul callsigns, such as
         // Glorioso Island:      FR-G  e.g.  FR7GL
         // Callbook implies that FR#*/G (see below) is also valid
         // Short of entering ALL these, not sure what to do

         // How about a '#' in the synonym meaning any number?
         // Also need an "any letter" - what about '?', to match
         // DOS wild cards? As well as * to mean a sequence of letters

         // This coding cannot be fully expanded into the synonym list; maybe
         // some kind of synonym decision tree needs to be built
         // This is getting VERY nasty; maybe we just say to enter the actual call
         // as a synonym (but of what... we need a placeholder for the main country)!

// replacement algorithm - HF inspired
// just keep stripping it back until we get a match

         testpart = cs.fullCall.getValue();
         testpart = trimr( testpart );

         int clen = testpart.length();
         while ( ( clen >= 1 ) && ( !csyn ) )
         {
            // we need to stop when we get to the basic prefix...
            // otherwise RVI6ABC ends up matching R, which is UA
            testpart = testpart.left(clen);
            clen--;
            csyn = MultLists::getMultLists() ->searchCountrySynonym(testpart );
         }
      }
   }

   QSharedPointer<CountryEntry> ctryMult;
   if (csyn)
        ctryMult = csyn->country;
   return ctryMult;
}
void BaseContact::getText( QString &dest, const BaseContestLog * const curcon ) const
{
   contactBuffs.scorebuff.clear();
   contactBuffs.scorebuff.clear();
   contactBuffs.brgbuff.clear();
   contactBuffs.buff2.clear();
   contactBuffs.qthbuff.clear();
   contactBuffs.srbuff.clear();
   contactBuffs.ssbuff.clear();
   contactBuffs.buff.clear();

   if ( contactFlags.getValue() & ( LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) )
   {
      contactBuffs.buff = QString("%1 %2 %3")
               .arg(time.getTime( DTGDISP ), 5)
               .arg(( contactFlags.getValue() & DONT_PRINT ) ? tr("DELETED") : ( contactFlags.getValue() & LOCAL_COMMENT ) ? tr("LOCAL COMMENT") : tr("COMMENT FOR ADJUDICATOR"))
               .arg(comments.getValue(), 60);
   }
   else
   {
      // if contest requires a serial
      makestrings( curcon ->serialMandatoryField.getValue() );

      contactBuffs.qthbuff = extraText.getValue().left( 100 );

      if ( contactFlags.getValue() & MANUAL_SCORE )
         contactBuffs.brgbuff = tr("MAN");

      if ( contactFlags.getValue() & DONT_PRINT )
         contactBuffs.scorebuff = tr("DEL");
      else
         if ( contactFlags.getValue() & NON_SCORING )
            contactBuffs.scorebuff = tr("N/S");
         else
         {
            // look at the contest dup
            if ( ( cs.valRes == ERR_DUPCS ) && ( curcon == contest ) )
               contactBuffs.scorebuff = tr("DUP");
         }
   }

   if ( contactFlags.getValue() & VALID_DUPLICATE )
      contactBuffs.buff2 = "BP ";
   else
      if ( contactFlags.getValue() & XBAND )
         contactBuffs.buff2 = "XB ";

   strcpysp( contactBuffs.buff, comments.getValue(), 42 );
   if ( !contactBuffs.buff.isEmpty() )
   {
      strcpysp( contactBuffs.buff2, contactBuffs.qthbuff, 20 );
      contactBuffs.buff2 += " | ";
      contactBuffs.buff2 += contactBuffs.buff;
   }
   else
      strcpysp( contactBuffs.buff2, contactBuffs.qthbuff, 42 );

   contactBuffs.buff.clear();
   int next = 0;
   next = placestr( contactBuffs.buff, time.getDate( DTGDISP ), next, 8 );
   next += 2;
   next = placestr( contactBuffs.buff, time.getTime( DTGDISP ), next, 5 );

   next += 1;
   next = placestr( contactBuffs.buff, cs.fullCall.getValue(), next, 11 );

   if ( curcon ->RSTMandatoryField.getValue() )
      next = placestr( contactBuffs.buff, reps.getValue(), next, 3 );
   next = placestr( contactBuffs.buff, contactBuffs.ssbuff, next, -4 );
   if ( curcon ->RSTMandatoryField.getValue() )
      next = placestr( contactBuffs.buff, repr.getValue(), next + 1, 3 );
   next = placestr( contactBuffs.buff, contactBuffs.srbuff, next, -4 );

   next = placestr( contactBuffs.buff, loc.loc.getValue(), next + 1, ( curcon ->allowLoc8.getValue() ) ? 8 : 6 );

   next = placestr( contactBuffs.buff, contactBuffs.brgbuff, next + 1, 4 );
   next = placestr( contactBuffs.buff, contactBuffs.scorebuff, next, -5 );

   next = placestr( contactBuffs.buff, op1.getValue(), next, -8 );
   next += 1;
   next = placestr( contactBuffs.buff, op2.getValue(), next, -8 );

   next = placestr( contactBuffs.buff, contactBuffs.buff2, next + 1, 90 );

   dest = QString( contactBuffs.buff ).trimmed();
}

void BaseContact::makestrings( bool sf ) const
{
   int ss = serials.getValue().toInt();

   QString ssr = serialr.getValue();
   int sr = ssr.toInt();
   QString srs = QString("%1 ").arg(sr, 3, 10, QChar('0')); // Leading zeroes
   if (ssr == "-")
   {
       srs = "-";
       sr = -1;
   }

   if ( ss && sf )
      contactBuffs.ssbuff = QString("%1 ").arg(ss, 3, 10, QChar('0')); // Leading zeroes
   else
      contactBuffs.ssbuff.clear();

   if ( sr && sf )
       contactBuffs.srbuff = srs;
   else
      contactBuffs.srbuff.clear();
}

