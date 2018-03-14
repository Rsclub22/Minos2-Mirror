/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

//==========================================================================
DisplayContestContact::DisplayContestContact( BaseContestLog * ct, bool time_now )
      : BaseContact( ct, time_now ),
      logSequence( 0 ),
      modificationCount( 0 )
{
   BaseContestLog * clp = ct;

   int ms = clp->maxSerial + 1;
   QString curmode = clp->currentMode.getValue();

   serialr.setInitialValue( QString( SERIALLENGTH, ' ' ) );

   if (curmode != hamlibData::MGM)
   {
       repr.setInitialValue( "5  " );
       reps.setInitialValue( "5  " );
   }
   else
   {
       repr.setInitialValue( "   " );
       reps.setInitialValue( "   " );
   }
   clearDirty();

   QString temp = QString("%1").arg(ms, 3);
   serials.setValue( temp );

   mode.setValue( curmode );


}
DisplayContestContact::~DisplayContestContact()
{}

void DisplayContestContact::copyFromArg( ScreenContact &cct )
{
   //   logSequence = cct.logSequence; // addContact or whatever will already have it correct
   loc.loc.setValue( cct.loc.loc );
   loc.validate();
   extraText.setValue( cct.extraText );

   cs.fullCall.setValue( cct.cs.fullCall );
   cs.validate();

   time.setValue( cct.time );

   reps.setValue( cct.reps );
   serials.setValue( cct.serials );
   repr.setValue( cct.repr );
   serialr.setValue( cct.serialr );

   QSOValid = cct.screenQSOValid;

   districtMult = cct.districtMult;
   ctryMult = cct.ctryMult;
   multCount = cct.multCount;
   forcedMult.setValue( cct.forcedMult );
   frequency.setValue(cct.frequency);
   rotatorHeading.setValue(cct.rotatorHeading);
   rigName.setValue(cct.rigName);
   bonus = cct.bonus;
   newBonus = cct.newBonus;

   op1.setValue( cct.op1 );
   op2.setValue( cct.op2 );

   locCount = cct.locCount;
   newGLoc = cct.newGLoc;
   newNonGLoc = cct.newNonGLoc;
   newDistrict = cct.newDistrict;
   newCtry = cct.newCtry;

   comments.setValue( cct.comments );

   contactFlags.setValue( cct.contactFlags );

   contactScore.setValue( cct.contactScore );
   bearing = cct.bearing;
   mode.setValue( cct.mode );

}
// used to test if anything has changed

bool DisplayContestContact::ne(const ScreenContact &mct) const
{
   QString ne_temp_date = mct.time.getDate( DTGDISP );
   QString ne_temp_time = mct.time.getTime( DTGDISP );

    if ( strcmpsp( ne_temp_date, time.getDate( DTGDISP ) ) )
     return true; // i.e. not equal

    if ( strcmpsp( ne_temp_time, time.getTime( DTGDISP ) ) )
     return true; // i.e. not equal

   if ( strcmpsp( mct.cs.fullCall.getValue(), cs.fullCall.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.reps, reps.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.serials, serials.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.repr, repr.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.serialr, serialr.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.reps, reps.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.loc.loc.getValue(), loc.loc.getValue() ) )
      return true; // i.e. not equal

   if ( stricmpsp( mct.extraText, extraText.getValue() ) )       // we force exchange upper case if dist code
      return true; // i.e. not equal

   if ( strcmpsp( mct.comments, comments.getValue() ) )
      return true; // i.e. not equal

   if ( mct.contactFlags != contactFlags.getValue() )
      return true; // i.e. not equal

   if ( strcmpsp( mct.mode, mode.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.forcedMult, forcedMult.getValue() ) )
      return true; // i.e. not equal

   QString newfreq = frequency.getValue();
   QString newmctfreq = mct.frequency;

   newfreq = newfreq.remove('.');
   newmctfreq = newmctfreq.remove('.');

   if ( strcmpsp( newfreq, newmctfreq ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.rotatorHeading, rotatorHeading.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.rigName, rigName.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.op1, op1.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.op2, op2.getValue() ) )
      return true; // i.e. not equal


//   if ( ( mct.contactScore != contactScore.getValue() ) && ( contactScore.getValue() != -1 ) )
//      return true;	// not equal

   return false;  // i.e. equal
}

void DisplayContestContact::checkContact( )
{
   // check on country and district. If valid, return true,
   // having mapped any synonyms to their parents and
   // saved the pointers.

   int checkret = 0;
   double dist = 0.0;
   BaseContestLog * clp = contest;

   if ( contactFlags.getValue() & ( LOCAL_COMMENT | COMMENT_ONLY ) )
      return ;

   // calc the bearing and score anyway; otherwise dups get a bearing of -1

   if ( bearing < 0 )
   {
      double lon = 0.0;
      double lat = 0.0;
      int brg;

      if ( lonlat( loc.loc.getValue(), lon, lat ) == LOC_OK )
      {
         clp->disbear( lon, lat, dist, brg );
         bearing = brg;
      }
   }

   if ( contactFlags.getValue() & NON_SCORING )
      return ;

   QSOValid = false;             // initially, anyway
   int csret = cs.validate( );
//#warning scanContest may already have set ERR_DUPCS
   if ( csret != CS_OK && csret != ERR_DUPCS )
      checkret = ERR_13;

   if ( !checkret )
   {
// and if scanContest has set dup, then this won't fire tering a new QSO
// But it DOES fire when checking on en
      unsigned long valp  = clp->validationPoint;
      if ( clp->DupSheet.checkCurDup( clp, getLogSequence(), valp, false ) )
      {
         cs.valRes = ERR_DUPCS;
         checkret = ERR_12;
      }
   }

   // search for prefix in country synonym list. Have to allow for e.g. HB0 as a mult

   if ( contactFlags.getValue() & COUNTRY_FORCED )
   {
      ctryMult = MultLists::getMultLists() ->getCtryForPrefix( forcedMult.getValue() );
   }
   else
      if ( !checkret )
      {
         ctryMult = findCtryPrefix( cs );
      }

   //   extraText = trimr( extraText );   // needs to be done already

   unsigned short cf = contactFlags.getValue();
   cf &= ~UNKNOWN_COUNTRY;
   if ( !checkret && ( clp->countryMult.getValue() || clp->districtMult.getValue() ) && !ctryMult )    // need at least a valid country
   {
      cf |= UNKNOWN_COUNTRY;
   }
   contactFlags.setValue( cf );
   if ( clp->districtMult.getValue() && ctryMult )
   {
      // if CC_mult and country "has districts" search for the "extra" in the county synonym list

      // check that the district and country agree

      // if the correct parts don't exist, not a valid contact!
      // NB that the rest of the contact has to be valid as well!

      if ( ctryMult->hasDistricts() )    // continentals dont have counties
      {
         districtMult = MultLists::getMultLists() ->searchDistrict( extraText.getValue() );
         if ( !districtMult && !( cf & VALID_DISTRICT ) )
         {
            checkret = ERR_8;
         }

         if (
            !checkret &&       						// no errors
            !( cf & VALID_DISTRICT ) &&      // ? district forced OK
            districtMult &&
            ( districtMult->country1 != ctryMult ) &&     // check district in country
            ( districtMult->country2 != ctryMult )
         )
         {
            checkret = ERR_8;
         }
      }
      // so all seems OK, or checkret is set to the first error
   }
   else
      if ( !checkret )
      {
         districtMult.reset();						// just in case we have changed the type...
         if ( clp->otherExchange.getValue() )
         {
            if ( clp->districtMult.getValue() )
            {
               if ( comments.getValue().isEmpty() )
                  checkret = ERR_21;
            }
            else
               if ( extraText.getValue().isEmpty() )
                  checkret = ERR_21;
         }
      }


   if ( checkret )
      return ;

   QSOValid = true;        // for now

   // same as ScreenContact::Check up to here

   if ( districtMult )
   {
      if ( districtMult->country1 &&
           ( ( clp->districtWorked[ districtMult->listOffset ] ) ++ < districtMult->country1->districtLimit() ) )
      {
         clp->ndistrict++;
         if ( clp->districtMult.getValue() )
         {
            multCount++;
         }
         newDistrict = true;
      }
   }

   if ( ctryMult )
   {
      if ( ( clp->countryWorked[ ctryMult->listOffset ] ) ++ == 0 )
      {
         if (!clp->nonGCountryMult.getValue() || !cs.isUK())
         {
            clp->nctry++;
            if ( clp->countryMult.getValue() )
            {
               {
                  multCount++;
               }
            }
            newCtry = true;
         }
      }
   }
   clp->DupSheet.checkCurDup( clp, getLogSequence(), 0, true ); // add to duplicates list

   if ( !( contactFlags.getValue() & ( MANUAL_SCORE | NON_SCORING | LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) ) )
   {
       if (contest->MGMContestRules.getValue())
       {
            dist = clp->CalcCentres ( loc.loc.getValue() );
       }
       else if ( loc.loc.getValue().size() == 4 && clp->allowLoc4.getValue() )
       {
          dist = clp->CalcNearest( loc.loc.getValue() ); // deal with 4 char locs
       }
       contactScore.setValue( static_cast<int>(dist) );
   }

   if ( !clp->locatorField.getValue() || contactScore.getValue() >= 0 )   		// don't add -1 scores in, but DO add zero km
      // as it is 1 point.
   {
      int cscore = contactScore.getValue();
      switch ( clp->scoreMode.getValue() )
      {
         case PPKM:
            {
               if ( contactFlags.getValue() & XBAND )
               {
                  cscore = ( cscore + 1 ) / 2;
               }
               clp->contestScore += cscore;
            }
            break;

         case PPQSO:
            if ( cscore > 0 )
               clp->contestScore++;
            break;

      }
   }

   {
      // now look at the locator list
      QString letters;
      QString numbers;

      QString sloc = loc.loc.getValue().mid(0, 4);

      letters = sloc.left(2);
      numbers = sloc.mid(2, 2);

      LocSquare *ls = 0;

      for ( LocSquareIterator i = clp->locs.llist.begin(); i != clp->locs.llist.end(); i++ )
      {
          LocSquare *locsq = ( *i ).wt.data();
          if ( strnicmp ( locsq ->loc, letters, 2 ) == 0 )
          {
              ls = locsq;
              break;
          }

      }

      if ( !ls )
      {
         if ( letters[ 0 ].isLetter() && letters[ 1 ].isLetter() )
         {
            ls = new LocSquare ( letters );
            MapWrapper<LocSquare> wls(ls);
            if (!clp->locs.llist.contains(wls))
                clp->locs.llist.insert ( wls, wls );
         }
      }

      int oldMultCount = multCount;
      if ( ls )
      {
         bool UKcall = cs.isUK();
         LocCount * npt = ls->map ( numbers );
         if ( npt )
         {
            if (clp->usesBonus.getValue())
            {
               if (npt->UKLocCount == 0 &&  npt->nonUKLocCount == 0)
               {
                  clp->bonus = clp->getSquareBonus(sloc);

                  bonus += clp->bonus;
                  clp->nbonus = true;
                  newBonus = true;
               }
            }
            if (UKcall)
            {
               if (!npt->UKMultGiven)
               {
                  npt->UKMultGiven = true;
                  newGLoc = true;
                  if (npt->UKLocCount + npt->nonUKLocCount == 0)
                  {
                     // hasn't been worked at all
                     clp->nlocs += clp->UKloc_multiplier;
                     multCount += clp->UKloc_multiplier;
                  }
                  else
                  {
                     // has already been worked - must have been non-uk, so that
                     // bit of the mult has already happened.
                     clp->nlocs += clp->UKloc_multiplier - clp->NonUKloc_multiplier;
                     multCount += clp->UKloc_multiplier - clp->NonUKloc_multiplier;
                  }
               }
               npt->UKLocCount++;
            }
            else
            {
               if ( npt->UKLocCount + npt->nonUKLocCount == 0 )
               {
                  clp->nlocs += clp->NonUKloc_multiplier;
                  multCount += clp->NonUKloc_multiplier;
               }
               if (npt->nonUKLocCount == 0)
               {
                  newNonGLoc = true;
               }
               npt->nonUKLocCount++;
            }
         }
      }
      locCount = multCount - oldMultCount;
   }
}

QString DisplayContestContact::getField( int ACol, const BaseContestLog *const curcon ) const
{
   // only used to get fields for main log display
   QString res;
   if ( !curcon )
      return res;

   BaseContestLog * clp = contest;

   unsigned short cf = contactFlags.getValue();
   if ( cf & ( LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) )
   {
      switch ( ACol )
      {
         case egTime:
            res = time.getTime( DTGDISP );
            break;
         case egCall:
            res = ( cf & DONT_PRINT ) ? "DELETED" : ( cf & LOCAL_COMMENT ) ? "LOCAL COMMENT" : "COMMENT FOR ADJUDICATOR";
            break;
         case egRSTTx:
            res = comments.getValue();
            break;
      }
   }
   else
   {
      switch ( ACol )
      {
         case egTime:
            res = time.getTime( DTGDISP );
            break;
         case egCall:
            res = cs.fullCall.getValue();
            break;
         case egRSTTx:
            res = reps.getValue();
            break;
         case egSNTx:
            {
               int ss = serials.getValue().toInt();
               res = QString("%1").arg(ss, 3, 10, QChar('0'));  //leading zeros
            }
            break;
         case egRSTRx:
            res = repr.getValue();
            break;
         case egSNRx:
            {
               QString ssr = serialr.getValue();
               if (ssr == "-")
               {
                   res = ssr;
               }
               else
               {
                   int sr = ssr.toInt();
                   res = QString("%1").arg(sr, 3, 10, QChar('0'));  //leading zeros
               }
            }
            break;
         case egLoc:
            res = loc.loc.getValue();
            break;
         case egBrg:
            {
               QString brgbuff;

               if ( contactFlags.getValue() & MANUAL_SCORE )
                  brgbuff = "MAN";
               else
               {
                  if ( loc.valRes == LOC_OK )
                  {
                     if ( contest == curcon )
                     {
                        // use the existing data
						int offset = contest->bearingOffset.getValue();
                        const QChar degreeChar(0260); // octal value
                        brgbuff = QString("%1%2").arg( varBrg( bearing + offset ), 3 ).arg(degreeChar);
                     }
                     else
                     {
                        // rework to come from prime contest loc
                        double lon = 0.0;
                        double lat = 0.0;
                        if ( lonlat( loc.loc.getValue(), lon, lat ) == LOC_OK )
                        {
                           // we don't have it worked out already...
                           double lon = 0.0;
                           double lat = 0.0;
                           int brg;
                           double dist;

                           if ( lonlat( loc.loc.getValue(), lon, lat ) == LOC_OK )
                           {
                              curcon->disbear( lon, lat, dist, brg );
                           }
						   int offset = curcon->bearingOffset.getValue();
                           const QChar degreeChar(0260); // octal value
                           brgbuff = QString("%1%2").arg( varBrg( brg + offset ), 3 ).arg(degreeChar);
                        }
                     }
                  }
			   }
               res = brgbuff.toStdString().c_str();
			}
			break;

		 case egScore:
			{
               QString scorebuff;

			   if ( contactFlags.getValue() & DONT_PRINT )
				  scorebuff = "DEL";
			   else
				  if ( contactFlags.getValue() & NON_SCORING )
					 scorebuff = "N/S";
				  else
				  {
					 if ( contest == curcon )
					 {
						// use the existing data
						if ( ( cs.valRes == ERR_DUPCS ) && ( curcon == clp ) )
						   scorebuff = "DUP";
						else
						{
						   int temp = contactScore.getValue();
						   if ( temp <= 0 )
							  temp = 0;

                           switch ( contest->scoreMode.getValue() )
                           {
                              case PPKM: 	// needs a valid LOC
                                 if ( !curcon->locatorField.getValue() )
                                 {
                                    if ( temp )
                                       scorebuff = "1";
                                    else
                                       scorebuff = "0";
                                 }
                                 else
                                 {
                                    if ( contactFlags.getValue() & XBAND )
                                    {
                                       scorebuff = QString("%1XB").arg( temp, 4 );
                                    }
                                    else
                                        scorebuff = QString("%1").arg( temp, 4 );
                                 }
                                 break;

                              case PPQSO:
                                 // actually we would often want the distance here
                                 scorebuff = QString("%1").arg( temp, 4 );
                                 break;

                           }
                        }
                     }
                     else
                     {
                        // we don't have it worked out already...
                        double lon = 0.0;
                        double lat = 0.0;
                        int brg;
                        double dist = 0.0;

                        if ( lonlat( loc.loc.getValue(), lon, lat ) == LOC_OK )
                        {
                           curcon->disbear( lon, lat, dist, brg );
                        }
                        scorebuff = QString("%1").arg( static_cast< int >  (dist) );

                     }
                  }
               res = scorebuff.toStdString().c_str();
            }
            break;
         case egExchange:
            if ( districtMult )
            {
               if (districtMult->districtCode.compare( extraText.getValue(), Qt::CaseInsensitive) == 0)
               {
                  res = districtMult->districtCode;
               }
               else
               {
                  res = districtMult->districtCode + "(" + extraText.getValue() + ")";
               }
            }
            else
            {
               res = extraText.getValue();
            }
            break;
         case egComments:
            {
               res.clear();
               if (cf & TO_BE_ENTERED)
               {
                  res = "UNFILLED CONTACT ";
               }
               res += comments.getValue();
            }
            break;
      case egRigName:
          res = rigName.getValue();
          break;
      case egFrequency:
          res = frequency.getValue();
          break;
      case egRotatorHeading:
      {
          QString brg = rotatorHeading.getValue();
          if (!brg.isEmpty())
          {
              const QChar degreeChar(0260); // octal value
              res = QString("%1%2").arg( brg ).arg(degreeChar);
              break;
          }
      }

      }
   }
   return res;
}
void DisplayContestContact::processMinosStanza( const QString &methodName, MinosTestImport * const mt )
{
   QString updtg;

   int itemp;
   if ( mt->getStructArgMemberValue( "lseq", itemp ) )     // should already be done...
      logSequence = static_cast< unsigned long > (itemp);
   mt->getStructArgMemberValueDTG( "uDTG", updtg );

   if ( methodName == "MinosLogComment" )
   {
      QString ctime;
      mt->getStructArgMemberValueDTG( "logTime", ctime );
      time.setIsoDTG( ctime );

      bool btemp = false;
      unsigned short cf = contactFlags.getValue();
      if ( mt->getStructArgMemberValue( "LocalComment", btemp ) )
         mt->setBit( cf, LOCAL_COMMENT, btemp );
      if ( mt->getStructArgMemberValue( "dontPrint", btemp ) )
         mt->setBit( cf, DONT_PRINT, btemp );
      contactFlags.setInitialValue( cf );
      mt->getStructArgMemberValue( "comment", comments );

   }
   else
      if ( methodName == "MinosLogQSO" )
      {
         modificationCount++;

         updtime.setIsoDTG( updtg );

         QString ctime;
         if ( mt->getStructArgMemberValueDTG( "logTime", ctime ) )
            time.setIsoDTG( ctime );

         unsigned short cf = contactFlags.getValue();
         bool btemp = false;
         if ( mt->getStructArgMemberValue( "validDistrict", btemp ) )
            mt->setBit( cf, VALID_DISTRICT, btemp );
         mt->setBit( cf, VALID_DISTRICT, btemp );
         if ( mt->getStructArgMemberValue( "countryForced", btemp ) )
            mt->setBit( cf, COUNTRY_FORCED, btemp );
         if ( mt->getStructArgMemberValue( "unknownCountry", btemp ) )
            mt->setBit( cf, UNKNOWN_COUNTRY, btemp );
         if ( mt->getStructArgMemberValue( "nonScoring", btemp ) )
            mt->setBit( cf, NON_SCORING, btemp );
         if ( mt->getStructArgMemberValue( "manualScore", btemp ) )
            mt->setBit( cf, MANUAL_SCORE, btemp );
         if ( mt->getStructArgMemberValue( "dontPrint", btemp ) )
            mt->setBit( cf, DONT_PRINT, btemp );
         if ( mt->getStructArgMemberValue( "validDuplicate", btemp ) )
            mt->setBit( cf, VALID_DUPLICATE, btemp );
         if ( mt->getStructArgMemberValue( "toBeEntered", btemp ) )
            mt->setBit( cf, TO_BE_ENTERED, btemp );
         if ( mt->getStructArgMemberValue( "xBand", btemp ) )
            mt->setBit( cf, XBAND, btemp );
         if ( mt->getStructArgMemberValue( "Forced", btemp ) )
            mt->setBit( cf, FORCE_LOG, btemp );

         contactFlags.setInitialValue( cf );

         mt->getStructArgMemberValue( "modeTx", mode );
         mt->getStructArgMemberValue( "rstTx", reps );
         mt->getStructArgMemberValue( "serialTx", serials );
         mt->getStructArgMemberValue( "exchangeTx", contest->location );
         mt->getStructArgMemberValue( "modeRx", mode );
         mt->getStructArgMemberValue( "callRx", cs.fullCall );
         mt->getStructArgMemberValue( "rstRx", repr );
         mt->getStructArgMemberValue( "serialRx", serialr );
         mt->getStructArgMemberValue( "exchangeRx", extraText );
         if ( mt->getStructArgMemberValue( "locRx", loc.loc ) )
            loc.validate();
         mt->getStructArgMemberValue( "commentsTx", comments );
         mt->getStructArgMemberValue( "commentsRx", comments );

         mt->getStructArgMemberValue( "power", contest->power );
         mt->getStructArgMemberValue( "band", contest->band );
         mt->getStructArgMemberValue( "claimedScore", contactScore );
         mt->getStructArgMemberValue( "forcedMult", forcedMult );
         mt->getStructArgMemberValue( "frequency", frequency );
         mt->getStructArgMemberValue( "rotatorHeading", rotatorHeading );
         mt->getStructArgMemberValue( "rigName", rigName );

         mt->getStructArgMemberValue( "op1", op1 );
         mt->getStructArgMemberValue( "op2", op2 );

         int maxct = serials.getValue().toInt();
         if ( maxct > contest->maxSerial )
            contest->maxSerial = maxct;

         contest->validationPoint = getLogSequence();
         checkContact();                 // processMinosStanza - Do we need to? scanContest will repeat it. Except we push the contact in it's current state into history
         QSharedPointer<BaseContact> bc( new BaseContact(*this) );   // this should get it now??
         getHistory().push_back( bc );
      }
}

