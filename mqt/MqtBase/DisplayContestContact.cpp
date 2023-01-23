/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "TreeUtils.h"
#include "cutils.h"
#include "contest.h"
#include "MinosTestImport.h"
#include "ScreenContact.h"
#include "DisplayContestContact.h"
#include "BandList.h"
#include "rigcontrolcommonconstants.h"
#include "MinosParameters.h"
#include "calcs.h"

//==========================================================================
DisplayContestContact::DisplayContestContact( BaseContestLog * ct, bool time_now, bool rInit )
      : BaseContact( ct, time_now ),
      modificationCount( 0 )
{
   BaseContestLog * clp = ct;

   int ms = clp->maxSerial + 1;
   QString bclMode = clp->currentMode.getValue();

   QString curMode = bclMode;

   if (bclMode == "FT8" || bclMode == "FT4")
   {
       curMode = hamlibData::MGM;
       mgmSubmode.setValue(bclMode);
   }

   serialr.setInitialValue( QString( SERIALLENGTH, ' ' ) );

   if (curMode != hamlibData::MGM)
   {
       QString cb = clp->currentBand.getValue().trimmed();
       BandList &blist = BandList::getBandList();
       QSharedPointer<BandInfo>  bi;
       bool bandOK = blist.findBand(cb, bi);
       bool hf = false;
       if (bandOK)
       {
          hf = bi->getType() == HF_BANDTYPE;
       }
       else
       {
           if (cb == allHF)
           {
               hf = true;
           }
       }
       if (hf)
       {
            if (curMode == hamlibData::CW || curMode == hamlibData::RY || curMode == hamlibData::PSK)
            {
                repr.setInitialValue( "599" );
                reps.setInitialValue( "599" );
            }
            else
            {
                repr.setInitialValue( "59 " );
                reps.setInitialValue( "59 " );
            }
       }
       else
       {
           if (rInit)
           {
               repr.setInitialValue( "5  " );
               reps.setInitialValue( "5  " );
           }
           else
           {
               repr.setInitialValue( "   " );
               reps.setInitialValue( "   " );
           }
       }
   }
   else
   {
       repr.setInitialValue( "   " );
       reps.setInitialValue( "   " );
   }
   clearDirty();

   QString temp = QString("%1").arg(ms, 3);
   serials.setValue( temp );

   mode.setValue( curMode );


}
DisplayContestContact::~DisplayContestContact()
{}

void DisplayContestContact::copyFromArg( ScreenContact &cct )
{
   //   logSequence = cct.logSequence; // addContact or whatever will already have it correct
   loc = cct.loc;
   extraText = cct.extraText;

   cs = cct.cs;

   timeOn = cct.timeOn ;
   timeOff = cct.timeOff ;

   reps.setValue( cct.reps );
   serials.setValue( cct.serials );
   repr.setValue( cct.repr );
   serialr.setValue( cct.serialr );

   QSOValid = cct.QSOValid;

   districtMult = cct.districtMult;
   ctryMult = cct.ctryMult;
   multCount = cct.multCount;
   forcedMult = cct.forcedMult ;
   frequency = cct.frequency;
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

   comments = cct.comments ;

   contactFlags = cct.contactFlags ;

   contactScore = cct.contactScore;
   bearing = cct.bearing;
   mode.setValue( cct.mode );
   mgmSubmode.setValue( cct.mgmSubmode );

   cqResponse.setValue(cct.cqResponse);

}
// used to test if anything has changed

bool DisplayContestContact::ne(const ScreenContact &mct) const
{
   QString ne_temp_date = mct.timeOff.getDate( DTGDISP );
   QString ne_temp_time = mct.timeOff.getTime( DTGDISP );

    if ( strcmpsp( ne_temp_date, timeOff.getDate( DTGDISP ) ) )
     return true; // i.e. not equal

    if ( strcmpsp( ne_temp_time, timeOff.getTime( DTGDISP ) ) )
     return true; // i.e. not equal

   if ( strcmpsp( mct.cs.getFullCall(), cs.getFullCall() ) )
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

   if ( strcmpsp( mct.loc.getLoc(), loc.getLoc() ) )
      return true; // i.e. not equal

   if ( stricmpsp( mct.extraText.getValue(), extraText.getValue() ) )       // we force exchange upper case if dist code
      return true; // i.e. not equal

   if ( strcmpsp( mct.comments.getValue(), comments.getValue() ) )
      return true; // i.e. not equal

   if ( mct.contactFlags.getValue() != contactFlags.getValue() )
      return true; // i.e. not equal

   if ( strcmpsp( mct.mode.getValue(), mode.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.mgmSubmode, mgmSubmode.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.forcedMult.getValue(), forcedMult.getValue() ) )
      return true; // i.e. not equal

   if (frequency.getValue() != mct.frequency.getValue())
      return true; // i.e. not equal

   if (cqResponse.getValue() != mct.cqResponse)
       return true;

   if ( strcmpsp( mct.rotatorHeading, rotatorHeading.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.rigName, rigName.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.op1, op1.getValue() ) )
      return true; // i.e. not equal

   if ( strcmpsp( mct.op2, op2.getValue() ) )
      return true; // i.e. not equal

   return false;  // i.e. equal
}

int DisplayContestContact::checkContact(bool adddup)
{
    int checkret = CheckableContact::checkContact(adddup);

    multCount = 0;
    newDistrict = false;
    newCtry = false;
    locCount = 0;
    newGLoc = false;
    newNonGLoc = false;
    bonus = 0;
    newBonus = false;

    double dist = getContactScore();    // calculated in CheckableContact
   bool dupContact = (cs.getValRes() == ERR_DUPCS);    // calculated in CheckableContact

   BaseContestLog * clp = contest;

   QString band;
   contest->getTxFreqBand(frequency.getValue(), band);

   if ( districtMult && districtMult->country1)
   {
      clp->addDistrictWorked(band, districtMult->districtCode);
      int n = clp->getDistrictsWorked(band, districtMult->districtCode);
      if ( n <= districtMult->country1->districtLimit() )
      {
         clp->ndistrict[band]++;
         if ( clp->districtMult.getValue() )
         {
            multCount++;
         }
         newDistrict = true;
      }
  }

   if ( ctryMult)
   {
       clp->addCountryWorked(band, ctryMult->getBasePrefix());
       int n = clp->getCountriesWorked(band, ctryMult->getBasePrefix());
       if ( n == 1 )
       {
           // nonGCountryMult says M6 - non UK countries only
           // since M6 died, always false
           if (!clp->nonGCountryMult.getValue() || !cs.isUK())
           {
              clp->nctry[band]++;   // DXCC mults
              if ( clp->countryMult.getValue() )
              {
                  multCount++;
              }
              newCtry = true;
           }
       }
   }

   if ( !notValidContact() )
   {
       if (!clp->locatorMandatoryField.getValue())
       {
            dist = 1;
       }
       else if (contest->MGMContestRules.getValue())
       {
           int brg;
            dist = clp->CalcCentres ( loc.getLoc(), brg );
            if ( almost_equal(dist, 1.0, 2))
                dist = 50;  // MGM same square == 50 points
       }
       else if ( loc.getLoc().size() == 4 && clp->allowLoc4.getValue() )
       {
          dist = clp->CalcNearest( loc.getLoc() ); // deal with 4 char locs
       }
       contactScore.setValue( static_cast<int>(dist) );
   }

   if (!dupContact)
   {
       if (!clp->locatorMandatoryField.getValue() || contactScore.getValue() >= 0 )   		// don't add -1 scores in, but DO add zero km
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
                   if (!dupContact)
                        clp->contestScore += cscore;
                }
                break;

             case PPQSO:
                if ( cscore > 0 && !dupContact)
                   clp->contestScore++;
                break;

          }
       }

      // now look at the locator list
      QString letters;
      QString numbers;

      QString sloc = loc.getLoc().mid(0, 4);

      letters = sloc.left(2);
      numbers = sloc.mid(2, 2);

      LocSquare *ls = nullptr;

      for ( auto const &i: qAsConst(clp->locs[band].llist) )
      {
          LocSquare *locsq = i.wt.data();
          if ( strnicmp ( locsq ->loc, letters, 2 ) == 0 )
          {
              ls = locsq;
              break;
          }

      }

      if ( !ls )
      {
         if (letters.size() >= 2 && letters[ 0 ].isLetter() && letters[ 1 ].isLetter() )
         {
            ls = new LocSquare ( letters );
            MapWrapper<LocSquare> wls(ls);
            if (!clp->locs[band].llist.contains(wls))
                clp->locs[band].llist.insert ( wls, wls );
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
                  clp->bonus[band] = clp->getSquareBonus(sloc);

                  bonus += clp->bonus[band];
                  clp->nbonus[band] = true;
                  newBonus = true;
               }
            }

            // but we set uk/non uk mult value to zero...
            if (UKcall)
            {
               if (!npt->UKMultGiven)
               {
                  npt->UKMultGiven = true;
                  newGLoc = true;
                  if (npt->UKLocCount + npt->nonUKLocCount == 0)
                  {
                     // hasn't been worked at all
                     clp->nlocs[band] += 1;
                     multCount += clp->UKloc_multiplier;
                  }
                  else
                  {
                     // has already been worked - must have been non-uk, so that
                     // bit of the mult has already happened.
                     //clp->nlocs += clp->UKloc_multiplier - clp->NonUKloc_multiplier;
                     multCount += clp->UKloc_multiplier - clp->NonUKloc_multiplier;
                  }
               }
               npt->UKLocCount++;
            }
            else
            {
               if ( npt->UKLocCount + npt->nonUKLocCount == 0 )
               {
                  clp->nlocs[band] += 1;
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
   return checkret;
}

QString DisplayContestContact::getField( int ACol, const BaseContestLog *const curcon ) const
{
   // only used to get fields for main log display
   QString res;
   if ( !curcon )
      return res;

   BaseContestLog * clp = contest;

   unsigned short cf = contactFlags.getValue();
   if ( cf & DONT_PRINT )
   {
      switch ( ACol )
      {
         case egTime:
            res = timeOff.getTime( DTGDISP );
            break;
         case egCall:
            res = tr("DELETED");
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
          case egDate:
             res = timeOff.getDate( DTGDISP );
             break;
         case egTime:
            res = timeOff.getTime( DTGDISP );
            break;
         case egBand:
            clp->getTxFreqBand(frequency.getValue(), res);
            break;
         case egMode:
            res = mode.getValue();
            break;
         case egCall:
            res = cs.getFullCall();
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
            res = loc.getLoc();
            break;
         case egBrg:
            {
               QString brgbuff;

               if ( contactFlags.getValue() & MANUAL_SCORE )
                  brgbuff = tr("MAN");
               else
               {
                  if ( loc.getValRes() == LOC_OK )
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
                        if ( lonlat( loc.getLoc(), lon, lat, MinosParameters::getMinosParameters() ->getAllowLoc4() ) == LOC_OK )
                        {
                           // we don't have it worked out already...
                           double dist;
                           int brg = 0;

                           curcon->disbeara( lon, lat, dist, brg );
						   int offset = curcon->bearingOffset.getValue();
                           const QChar degreeChar(0260); // octal value
                           brgbuff = QString("%1%2").arg( varBrg( brg + offset ), 3 ).arg(degreeChar);
                        }
                     }
                  }
                  else
                      if (loc.getValRes() == LOC_PARTIAL)
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
                             char llres = lonlat( loc.getLoc(), lon, lat, MinosParameters::getMinosParameters() ->getAllowLoc4() );
                             if ( llres == LOC_OK || llres == LOC_PARTIAL )
                             {
                                // we don't have it worked out already...
                                int brg = 0;
                                double dist;

                                curcon->disbearc( lon, lat, dist, brg );

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
                  scorebuff = tr("DEL");
			   else
				  if ( contactFlags.getValue() & NON_SCORING )
                     scorebuff = tr("N/S");
				  else
				  {
					 if ( contest == curcon )
					 {
						// use the existing data
                        if ( ( cs.getValRes() == ERR_DUPCS ) && ( curcon == clp ) )
                           scorebuff = tr("DUP");
						else
						{
						   int temp = contactScore.getValue();
						   if ( temp <= 0 )
							  temp = 0;

                           switch ( contest->scoreMode.getValue() )
                           {
                              case PPKM: 	// needs a valid LOC
                                 if ( !curcon->locatorMandatoryField.getValue() )
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
                        char llres = lonlat( loc.getLoc(), lon, lat, MinosParameters::getMinosParameters() ->getAllowLoc4() );
                        if ( llres == LOC_OK )
                        {
                           curcon->disbeara( lon, lat, dist, brg );
                        }
                        else if (llres == LOC_PARTIAL)
                        {
                            curcon->disbearc( lon, lat, dist, brg );
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
                  res = tr("UNFILLED CONTACT ");
               }
               res += comments.getValue();
            }
            break;
      case egRigName:
          res = rigName.getValue();
          break;
      case egFrequency:
          res = frequency.getValue().convertFreqStrDisp();
          break;
      case egRotatorHeading:
      {
          QString brg = rotatorHeading.getValue();
          if (!brg.isEmpty())
          {
              const QChar degreeChar(0260); // octal value
              res = QString("%1%2").arg( brg ).arg(degreeChar);
          }
          break;
      }
      case egOperator:
      {
          res = op1.getValue();
          break;
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
      setLogSequence( static_cast< unsigned long > (itemp));
   mt->getStructArgMemberValueDTG( "uDTG", updtg );

      if ( methodName == "MinosLogQSO" )
      {
         modificationCount++;

         updtime.setIsoDTG( updtg );

         QString ctimeoff;
         QString ctimeon;
         if (mt->getStructArgMemberValueDTG( "logTime", ctimeoff ))
         {
            timeOff.setIsoDTG( ctimeoff );
         }
         if (mt->getStructArgMemberValueDTG( "QSOStartTime", ctimeon ))
         {
           timeOn.setIsoDTG( ctimeon );
         }

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
         mt->getStructArgMemberValue( "mgmSubmode", mgmSubmode);
         mt->getStructArgMemberValue( "rstTx", reps );
         mt->getStructArgMemberValue( "serialTx", serials );
         mt->getStructArgMemberValue( "exchangeTx", contest->location );
         mt->getStructArgMemberValue( "modeRx", mode );
         QString temp;
         if (mt->getStructArgMemberValue( "callRx", temp ))
         {
            cs.setFullCall(temp);
         }
         mt->getStructArgMemberValue( "rstRx", repr );
         mt->getStructArgMemberValue( "serialRx", serialr );
         mt->getStructArgMemberValue( "exchangeRx", extraText );
         if ( mt->getStructArgMemberValue( "locRx", temp ) )
         {
            loc.setLoc(temp);
         }
         mt->getStructArgMemberValue( "commentsTx", comments );
         mt->getStructArgMemberValue( "commentsRx", comments );

         mt->getStructArgMemberValue( "power", contest->power );
         mt->getStructArgMemberValue( "band", contest->contestBands );
         mt->getStructArgMemberValue( "currentBand", contest->currentBand );
         mt->getStructArgMemberValue( "bandsList", contest->bandsList );
         mt->getStructArgMemberValue( "claimedScore", contactScore );
         mt->getStructArgMemberValue( "forcedMult", forcedMult );
         if (mt->getStructArgMemberValue( "frequency", temp ))
         {
            frequency.setValue( Frequency(temp) );
         }
         mt->getStructArgMemberValue( "rotatorHeading", rotatorHeading );
         mt->getStructArgMemberValue( "rigName", rigName );

         mt->getStructArgMemberValue( "op1", op1 );
         mt->getStructArgMemberValue( "op2", op2 );

         int maxct = serials.getValue().toInt();
         if ( maxct > contest->maxSerial )
         {
            contest->maxSerial = maxct;
         }
         mt->getStructArgMemberValue( "cqResponse", cqResponse );

         contest->validationPoint = getLogSequence();
         checkContact(true);                 // processMinosStanza
         QSharedPointer<BaseContact> bc( new BaseContact(*this) );   // this should get it now??
         getHistory().push_back( bc );
         contest->validationPoint = 0;
      }
}

