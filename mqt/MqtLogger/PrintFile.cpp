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
#include "contacts.h"
#include "printfile.h"

//==============================================================================
// This (printfile) will be in english...

const char *PrintFile::fileHeader =
                         QT_TR_NOOP( "====================================================\r\n"
                         "\r\nDO NOT SEND THIS FILE AS YOUR ENTRY!\r\n\r\n"
                         "Use \"File\" | \"Produce Entry/Export File...\"\r\n"
                         "Export as Reg1Test(entry)\r\n"
                         "and send the .EDI file produced.\r\n"
                         "====================================================\r\n")
                         ;
//==============================================================================
enum PrintFile_order
{
   TName, TdDate, PCall, PWWLo, PExch, PAdr1, PAdr2, PSect, PBand,
   PClub, RName, RCall, RAdr1, RAdr2, RPoCo, RCity, RCoun, RPhon,
   RHBBS, MOpe1, MOpe2, STXEq, SPowe, SRXEq, SAnte, SAntH, CVQSO, CQSOs,
   CQSOP, CWWLs, /*CWWLB,*/ CExcs, /*CExcB,*/ CDXCs, /*CDXCB,*/ CToSc, CODXC,
   LineCount
};
// We want to export as a human readable file
PrintFileLine::PrintFileLine( const QString &pre, const QString &dat ) :
      prefix( pre ), data( dat )
{}
PrintFileLine::PrintFileLine( )
{}
PrintFileLine::~PrintFileLine()
{}
PrintFileLine &PrintFileLine::operator = ( const PrintFileLine &l )
{
   prefix = l.prefix;

   data = l.data;
   return *this;
}

PrintFile::PrintFile( LoggerContestLog * const ct )
      : ct( ct )
{}
PrintFile::~PrintFile()
{}

bool PrintFile::exportTest(QSharedPointer<QFile> expfd )
{
   PrintFileLine linelist[ static_cast< int> (LineCount) ];
   // export current contest as PrintFile
   writer wr( expfd );
   char lbuff[ 1024 ];

   // First the header info
   // calc mult count
   int ltot = 0;
   int nctry = 0;
   int ndistrict = 0;
   int nlocs = 0;
   int nvalid = 0;

   if ( ct->countryMult.getValue() )
   {
       nctry = ct->getNctry();
      ltot += nctry ;
   }
   if ( ct->districtMult.getValue() )
   {
       ndistrict = ct->getNdistrict();
      ltot += ndistrict;
   }
   if ( ct->locMult.getValue() )
   {
      nlocs = ct->getNlocs();
      ltot += nlocs;
   }

   if ( ltot == 0 )
      ltot = 1;

   QStringList remarks;
   remarks.append( ct->entCondx1.getValue() );
   remarks.append( ct->entCondx2.getValue() );
   remarks.append( ct->entCondx3.getValue() );
   remarks.append( ct->entCondx4.getValue() );

   for(auto const &dct: ct->ctList)
   {
      QSharedPointer<BaseContact> cct = dct.wt;

      if ( cct->contactScore.getValue() > 0 )
         nvalid++;
   }

   // get the best DX contact
   QSharedPointer<BaseContact> bestdx = ct->getBestDX();

   linelist[ static_cast< int> (TName) ] = PrintFileLine( "Contest Name                            ", ct->name.getValue()  /*, "Contest Name"*/ );

//   linelist[ static_cast< int> ( TdDate ] = PrintFileLine("Start Date;End Date                     ", ct->dateRange( DTGFULL )  /*, "Start Date;End Date"*/ );
   linelist[ static_cast< int> (PCall) ] = PrintFileLine( "Callsign Used                           ", ct->mycall.getFullCall()  /*, "Callsign Used"*/ );
   linelist[ static_cast< int> (PWWLo) ] = PrintFileLine( "Locator Used                            ", ct->myloc.getLoc()  /*, "Locator Used"*/ );
   linelist[ static_cast< int> (PExch) ] = PrintFileLine( "Exchange Used                           ", ct->location.getValue()  /*, "Exchange Used"*/ );
   linelist[ static_cast< int> (PAdr1) ] = PrintFileLine( "Address line 1/2 of station             ", ct->sqth1.getValue()  /*, "Address line 1/2 of station"*/ );
   linelist[ static_cast< int> (PAdr2) ] = PrintFileLine( "Address line 2/2 of station             ", ct->sqth2.getValue()  /*, "Address line 2/2 of station"*/ );
   linelist[ static_cast< int> (PSect) ] = PrintFileLine( "Section Entered                         ", ct->entSect.getValue()  /*, "Section Entered"*/ );
   linelist[ static_cast< int> (PBand) ] = PrintFileLine( "Band Used                               ", ct->contestBands.getValue()  /*, "Band Used"*/ );

   linelist[ static_cast< int> (PClub) ] = PrintFileLine( "Name of club/group                      ", ct->entrant.getValue()  /*, "Name of club/group"*/ );
   linelist[ static_cast< int> (RName) ] = PrintFileLine( "Name of responsible operator            ", ct->entName.getValue()  /*, "Name of responsible operator"*/ );
   linelist[ static_cast< int> (RCall) ] = PrintFileLine( "Callsign of responsible operator        ", ct->entCall.getValue()  /*, "Callsign of responsible operator"*/ );

   linelist[ static_cast< int> (RAdr1) ] = PrintFileLine( "Address line 1/2 of responsible operator", ct->entAddr1.getValue()  /*, "Address line 1/2 of responsible operator"*/ );
   linelist[ static_cast< int> (RAdr2) ] = PrintFileLine( "Address line 2/2 of responsible operator", ct->entAddr2.getValue()  /*, "Address line 2/2 of responsible operator"*/ );
   linelist[ static_cast< int> (RCity) ] = PrintFileLine( "City of responsible operator            ", ct->entCity.getValue()  /*, "City of responsible operator"*/ );
   linelist[ static_cast< int> (RPoCo) ] = PrintFileLine( "Post Code of responsible operator       ", ct->entPostCode.getValue()  /*, "Post Code of responsible operator"*/ );
   linelist[ static_cast< int> (RCoun) ] = PrintFileLine( "Country of responsible operator         ", ct->entCountry.getValue()  /*, "Country of responsible operator"*/ );
   linelist[ static_cast< int> (RPhon) ] = PrintFileLine( "Phone number of responsible operator    ", ct->entPhone.getValue()  /*, "Phone no. of responsible operator"*/ );

   linelist[ static_cast< int> (RHBBS) ] = PrintFileLine( "eMail address of responsible operator   ", ct->entEMail.getValue()  /*, "EMAIL address of responsible operator"*/ );
   linelist[ static_cast< int> (MOpe1) ] = PrintFileLine( "Operators line 1/2                      ", ct->ops1.getValue()  /*, "Operators line 1/2"*/ );
   linelist[ static_cast< int> (MOpe2) ] = PrintFileLine( "Operators line 2/2                      ", ct->ops2.getValue()  /*, "Operators line 2/2"*/ );
   linelist[ static_cast< int> (STXEq) ] = PrintFileLine( "TX Equipment                            ", ct->entTx.getValue()  /*, "TX Equipment"*/ );
   linelist[ static_cast< int> (SPowe) ] = PrintFileLine( "TX Power (W)                            ", ct->power.getValue()  /*, "TX Power (W)"*/ );
   linelist[ static_cast< int> (SRXEq) ] = PrintFileLine( "RX Equipment                            ", ct->entRx.getValue()  /*, "RX Equipment"*/ );
   linelist[ static_cast< int> (SAnte) ] = PrintFileLine( "Antenna                                 ", ct->entAnt.getValue()  /*, "Antenna"*/ );
   linelist[ static_cast< int> (SAntH) ] = PrintFileLine( "Antenna Height agl;height asl           ", ct->entAGL.getValue() + ";" + ct->entASL.getValue()  /*, "Antenna Height agl;height asl"*/ );

   linelist[ static_cast< int> (CVQSO) ] = PrintFileLine( "Number of valid QSOs                    ", QString::number( nvalid ) ); /*, "Claimed no. of QSO points"*/
   linelist[ static_cast< int> (CQSOP) ] = PrintFileLine( "Claimed number of QSO points            ", QString::number( ct->contestScore ) ); /*, "Claimed no. of QSO points"*/
   linelist[ static_cast< int> (CWWLs) ] = PrintFileLine( "Claimed number of WWLs                  ", QString::number( nlocs ) ); /*, "Claimed no. of WWLs;Bonus per new WWL;WWL multiplier"*/
   linelist[ static_cast< int> (CExcs) ] = PrintFileLine( "Claimed number of exchanges             ", QString::number( ndistrict )); /*, "Claimed no. of exchanges; Bonus for each new exchange; Exchange Multiplier"*/
   linelist[ static_cast< int> (CDXCs) ] = PrintFileLine( "Claimed number of DXCCs                 ", QString::number( nctry )); /*, "Claimed no. of DXCCs; Bonus for each new DXCC;DXCC multiplier"*/
   linelist[ static_cast< int> (CToSc) ] = PrintFileLine( "Claimed total score                     ", QString::number( ct->contestScore * ltot ) ); /*, "Claimed total score"*/


   QString sbestdx = bestdx
               ?( bestdx->cs.getFullCall() + ";" + bestdx->loc.getLoc() + ";" + QString::number(bestdx->contactScore.getValue()))
               : QString(";;");
   linelist[ static_cast< int> (CODXC) ] = PrintFileLine( "Best DX - Callsign; Locator; Distance   ", sbestdx ); /*, "(Best DX) Callsign; Locator; Distance"*/

   wr.lwrite(tr(fileHeader));
   wr.lwriteLine();

   for ( auto const &l:linelist )
   {
      if (l.prefix.size())
      {
         sprintf( lbuff, "%s : %s", l.prefix.toStdString().c_str(), l.data.toStdString().c_str() );
         wr.lwrite( lbuff );
      }
   }
   wr.lwrite( "" );
   wr.lwrite( "Remarks" );
   wr.lwrite( "=======" );

   for ( auto const &r: remarks )
   {
      if (r.size())
      {
         wr.lwrite( r );
      }
   }
   wr.lwrite( "" );

   // and then the contact list

   wr.lwrite( "QSOs" );
   wr.lwrite( "====" );
   for(auto const &dct: ct->ctList)
   {
       QSharedPointer<BaseContact> cct = dct.wt;

      QString sbuff;

      cct->getPrintFileText( sbuff, 120 );   // lbuff if "bsize" = 256
      if ( sbuff.length() != 0 )
      {
         sbuff = trimr( sbuff );			// No need to pad to 250!!
         wr.lwrite( sbuff );
      }
   }
   wr.lwrite( "" );

   QString ProductName = "Minos";
   QString Version = QString(STRINGVERSION) + " " + PRERELEASETYPE;

   QString pver = "Produced by " + ProductName + " version " + Version;
   wr.lwrite( pver );

   return true;

}
//====================================================================

