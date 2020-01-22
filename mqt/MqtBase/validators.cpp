/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#include "base_pch.h"
#include "MinosLoggerEvents.h"
#include "ScreenContact.h"
#include "rigutils.h"

ErrEntry errDefs[] =
   {
      ErrEntry( 20, QT_TR_NOOP("null validator failed") ),   //ERR_0
      ErrEntry( 11, QT_TR_NOOP("Invalid time") ),        //ERR_1
      ErrEntry( 10, QT_TR_NOOP("Invalid date") ),        //ERR_2
      ErrEntry( 9, QT_TR_NOOP("Invalid power(0.01 - 1000 W)") ),      //ERR_3
      ErrEntry( 15, QT_TR_NOOP("Invalid locator") ),     //ERR_4
      ErrEntry( 14, QT_TR_NOOP("Invalid Serial") ),      //ERR_5
      ErrEntry( 13, QT_TR_NOOP("Invalid RS(T)") ),       //ERR_6
      ErrEntry( 9, QT_TR_NOOP("Invalid power(-40 - +40 dBW)") ),      //ERR_7
      ErrEntry( 10, QT_TR_NOOP("Invalid district") ),      //ERR_8
      ErrEntry( 10, QT_TR_NOOP("Invalid band") ),        //ERR_9
      ErrEntry( 12, QT_TR_NOOP("!!! Callsign not validated !!!" )),   //ERR_10
      ErrEntry( 12, QT_TR_NOOP("!!! No Callsign present !!!") ),   //ERR_11
      ErrEntry( 1, QT_TR_NOOP("!!! Duplicate Callsign !!!") ),   //ERR_12
      ErrEntry( 8, QT_TR_NOOP("Invalid Callsign!") ),         //ERR_13
      ErrEntry( 10, QT_TR_NOOP("No RX Serial Number") ),      //ERR_14
      ErrEntry( 15, QT_TR_NOOP("Locator probably not within country.") ),   //ERR_15
      ErrEntry( 10, QT_TR_NOOP(FREQ_EDIT_ERR_MSG) ),     //ERR_16
      ErrEntry( 10, "" ),   //ERR_17
      ErrEntry( 20, QT_TR_NOOP("No Locator") ),             //ERR_18
      ErrEntry( 2, QT_TR_NOOP("!!Loc not AA00AA to RR99XX!!") ),   //ERR_19
      ErrEntry( 16, QT_TR_NOOP("Invalid district multiplier") ),          //ERR_20
      ErrEntry( 10, QT_TR_NOOP("QTH required") ),           //ERR_21
      ErrEntry( 10, "" ),         //ERR_22
      ErrEntry( 5, QT_TR_NOOP("Unknown Country") ),         //ERR_23
      ErrEntry( 2, QT_TR_NOOP("!!! Non Scoring Contact !!!") ),         //ERR_24
      ErrEntry( 1, QT_TR_NOOP("!!! Comment Only !!!") ),         //ERR_25
      ErrEntry( 1,QT_TR_NOOP( "!!! DELETED !!!") ),         //ERR_26
      ErrEntry( 90, "" ),         //ERR_27
      ErrEntry( 90, "" ),         //ERR_28
      ErrEntry( 0, "" )          //ERR_29
   };
void setError( int err )   // No CS
{
//#warning this really needs the log pointer for context - but where from?
   MinosLoggerEvents::SendValidateError(err);
}

Validator::Validator( validatorTypes vt ) :
    status( false ),
    vt( vt )
{}
//enum validTypes {cmNone, cmCancel, cmValid, cmCheckValid, cmValidStatus, cmReleasedFocus};
//enum validatorTypes {vtNone, vtNotEmpty, vtNumeric, vtDate, vtTime, vtCallsign,
//                      vtSN, vtRST, vtLoc, vtQTH, vtComments};
bool Validator::validate(const QString &str, ScreenContact &screenContact )
{
   switch ( vt )
   {
      case vtNone:
         return true;

      case vtNotEmpty:
         return !allSpaces( str );

      case vtNumeric:
         return validNumber( str);

      case vtDate:
         {
            QString field = str.trimmed();
            bool res = true;
            if ( field.size() != 8 )
               res = false;
            else
               if ( ( field[ 0 ] != '0' ) && ( field[ 0 ] != '1' ) && ( field[ 0 ] != '2' ) && ( field[ 0 ] != '3' ) )
                  res = false;
               else
                  if ( !field[ 1 ].isDigit() )
                     res = false;
                  else
                     if ( field[ 2 ] != '/' )
                        res = false;
                     else
                        if ( ( field[ 3 ] != '0' ) && ( field[ 3 ] != '1' ) )
                           res = false;
                        else
                           if ( !field[ 4 ].isDigit() )
                              res = false;
                           else
                              if ( field[ 5 ] != '/' )
                                 res = false;
                              else
                                 if ( !field[ 6 ].isDigit() )
                                    res = false;
                                 else
                                    if ( !field[ 7 ].isDigit() )
                                       res = false;
                                    else
                                       return true;   // seems to match

            if ( !res )
               setError( ERR_2 );
            return false;
         }

      case vtTime:
         {
            bool res = true;
            QString field = str.trimmed();
            if ( field.size() != 5 )
               res = false;
            else
               if ( ( field[ 0 ] != '0' ) && ( field[ 0 ] != '1' ) && ( field[ 0 ] != '2' ) )
                  res = false;
               else
                  if ( !field[ 1 ].isDigit() )
                     res = false;
                  else
                     if ( field[ 2 ] != ':' )
                        res = false;
                     else
                        if (
                           ( field[ 3 ] != '0' ) && ( field[ 3 ] != '1' ) &&
                           ( field[ 3 ] != '2' ) && ( field[ 3 ] != '3' ) &&
                           ( field[ 3 ] != '4' ) && ( field[ 3 ] != '5' )
                        )
                           res = false;
                        else
                           if ( !field[ 4 ].isDigit() )
                              res = false;
                           else
                              return true;
            if ( !res )
               setError( ERR_1 );
            return false;
         }

      case vtCallsign:

        if (screenContact.cs.valRes == ERR_DUPCS)
        {
            setError(ERR_12);
            return false;
        }

        if ( allSpaces( str ) )
        {
            setError( ERR_11 ); // No CS
            return false;
        }
        return true;

      case vtSN:
         if ( validNumber( str ) || str == "-" )
         {
            return true;
         }
         else
            setError( ERR_5 );
         return false;

       case vtRST:
       {
           QString curmode = screenContact.mode;
           if (curmode == hamlibData::MGM)
           {
               return str.trimmed().length() >= 1;
           }
           else
           {
               if ( validateRST( str ) )
                   return true;
               else
                   setError( ERR_6 );
               return false;
           }
       }

      case vtLoc:
         {
            bool PXret = true;
            QString loc = str + "        ";
            bool allowLoc8 = MinosParameters::getMinosParameters() ->getAllowLoc8();
            if ( !allowLoc8 && ( loc[ 6 ].isDigit() || loc[ 7 ].isDigit() ) )
               PXret = false;

            double longitude = 0.0;
            double latitude = 0.0;

            if ( PXret && lonlat( loc, longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() ) != LOC_OK )
               PXret = false;

            if ( PXret == false )
               setError( allSpaces( str ) ? ERR_18 : ERR_4 );
            return PXret;
         }
      case vtQTH:
         {
            // We need a QTH if (a) QTH required or (b) dist mult and NOT a non-G callsign
            return true;
         }

      case vtComments:
         return true;

       case vtFreq:
       {
           QString f = str.trimmed().remove( QRegExp("^[0]*"));
           if (f.isEmpty())
               return true;
           if (!validateFreqTxtInput(f))
           {
               setError(ERR_16);
               return false;
           }
           return true;
       }
   }
   return false;
}
bool Validator::allSpaces( const QString &str )
{
   int i;
   int n = str.size();
   for ( i = 0; i < n && ( str[ i ] == ' ' ); i++ )
   {}
   if ( i >= n )
      return true;

   return false;
}
bool Validator::validNumber( const QString &str, bool trailingAlphaAllowed)
{
   int i = 0;
   int maxi = str.length();
   bool num_found = false;
   bool space_found = false;
   bool NonZ = false;
   bool invCharFound = false;

   for ( i = 0; i < maxi; i++ )
   {
      if (str[i] != ' ' && !str[ i ].isDigit())
      {
         invCharFound = true;
      }
      if ( ( num_found ) && ( str[ i ] == ' ' ) )
      {
         space_found = true;
         continue;
      }

      if ( str[ i ].isDigit() )
      {
         if ( str[ i ] != '0' )
            NonZ = true;
         if ( space_found )
         {
            return false;
         }
         num_found = true;
      }
   }
   if ( num_found && NonZ && !invCharFound )
      return true;
      
   if (trailingAlphaAllowed)
   {
      QString S = str.trimmed();
      if (S[S.size() - 1].isLetter())
      {
         S = S.left(S.size() - 1);
         return(validNumber(S));
      }
   }
   return false;
}
/*static*/bool Validator::validateRST(  const QString &t )
{
   int sz = t.size();
   bool picvalid = false;
   if ( sz >= 1 )
   {
      if ( strchr( " 54321", t[ 0 ].toLatin1() ) )
      {
         if ( sz >= 2 )
         {
            if ( strchr( " 987654321", t[ 1 ].toLatin1() ) )
            {
               if ( sz >= 3 )
               {
//                  if ( strchr( " 987654321A", t[ 2 ].toLatin1() ) )
//                  {
//                     picvalid = true;
//                  }
                  if (sz >= 4)
                  {
                      picvalid = false;
                  }
                  else if (t[2] == '0' || (t[2] !=  ' ' && !t[2].isLetterOrNumber( )))
                  {
                      picvalid = false;
                  }
                  else
                  {
                      picvalid = true;
                  }
               }
               else
               {
                  picvalid = true;
               }
            }
         }
      }
   }
   if ( picvalid )
      picvalid = validNumber( t, true );
   if ( picvalid )
   {
      // must be more than one digit in a RST
      int rst = atoi( t.toLatin1().data() );
      if ( rst < 10 )
         picvalid = false;

      // allow strength 0 to be typed, but it is still an error!

      if (sz > 1 && t[ sz - 1 ] == '0' )
         picvalid = false;
   }
   return picvalid;
}
//==============================================================================
ValidatedControl::ValidatedControl(QLineEdit *c, validatorTypes vt ) :
      wc( c )
    , validator( vt )
    , tIfValid( false )
{}
bool ValidatedControl::valid( validTypes cmd, ScreenContact &screenContact )
{
   if ( wc->isVisible() )
   {
      if ( cmd == cmValidStatus )
      {
         return tIfValid;
      }
      else //if ( cmd != cmCancel )
      {
         if ( ! validator.validate( wc->text(), screenContact ) )
         {
            tIfValid = false;
            return true; //( cmd == cmCheckValid ); // lie about it, or it stops on firstthat
         }
         tIfValid = true;  // for now, anyway!
      }
   }
   return true;
}

