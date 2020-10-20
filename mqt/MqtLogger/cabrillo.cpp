#include "base_pch.h"

#include "LoggerContest.h"
#include "contacts.h"
#include "cabrillo.h"

/*
https://wwrof.org/cabrillo/cabrillo-specification-v3/

Cabrillo Specification V3
Last update: 26 April 2020

This page details the tags supported by the Cabrillo Specification V3. The format for each line is <TAG>: followed by a space. The tags may be listed in any order in the log file.

Required Tags
START-OF-LOG:version-number
Must be the first line of the log submission. The current version-number is 3.0.

END-OF-LOG:
Must be the last line of the log submission.

Common Tags
CALLSIGN:callsign
The callsign used during the contest.

CONTEST:text
String to identify the contest. Valid characters are A-Z, 0-9, and hyphen (-). Contest text values are not an official part of the specification. Contest sponsors may define their own contest values. Some examples listed below for convenience.

ARRL-10
ARRL-10-GHZ
ARRL-160
ARRL-222
ARRL-DX-CW
ARRL-DX-SSB
ARRL-EME
ARRL-RR-PH
ARRL-RR-DIG
ARRL-RR-CW
ARRL-SCR
ARRL-SS-CW
ARRL-SS-SSB
ARRL-UHF-AUG
ARRL-VHF-JAN
ARRL-VHF-JUN
ARRL-VHF-SEP
ARRL-RTTY
BARTG-RTTY
CQ-160-CW
CQ-160-SSB
CQ-WPX-CW
CQ-WPX-RTTY
CQ-WPX-SSB
CQ-VHF
CQ-WW-CW
CQ-WW-RTTY
CQ-WW-SSB
IARU-HF
NAQP-CW
NAQP-SSB
NAQP-RTTY
NA-SPRINT-CW
NA-SPRINT-SSB
NA-SPRINT-RTTY
OCEANIA-DX-CW
OCEANIA-DX-SSB
RDXC
RSGB-IOTA
SAC-CW
SAC-SSB
SPDXC
SPDXC-RTTY
TARA-RTTY
WAG
WW-DIGI
Additional contest name examples can be seen here.

CATEGORY-ASSISTED:
The category-assisted must be one of the following.

ASSISTED
NON-ASSISTED
CATEGORY-BAND:
The category-band must be one of the following. If you are entering an all band category, use ALL, otherwise select one of the individual bands shown.

ALL
160M
80M
40M
20M
15M
10M
6M
4M
2M
222
432
902
1.2G
2.3G
3.4G
5.7G
10G
24G
47G
75G
123G
134G
241G
Light
VHF-3-BAND and VHF-FM-ONLY (ARRL VHF Contests only)
CATEGORY-MODE:
The category-mode must be one of the following.

CW
DIGI
FM
RTTY
SSB
MIXED
CATEGORY-OPERATOR:
The category-operator must be one of the following.

SINGLE-OP
MULTI-OP
CHECKLOG
CATEGORY-POWER:
The category-power must be one of the following.

HIGH
LOW
QRP
CATEGORY-STATION:
Type of station

FIXED
MOBILE
PORTABLE
ROVER
ROVER-LIMITED
ROVER-UNLIMITED
EXPEDITION
HQ
SCHOOL
CATEGORY-TIME:

6-HOURS
12-HOURS
24-HOURS
CATEGORY-TRANSMITTER:
The category-transmitter is required for multi-operator entries and must be one of the following.

ONE
TWO
LIMITED
UNLIMITED
SWL
CATEGORY-OVERLAY:

CLASSIC
ROOKIE
TB-WIRES
NOVICE-TECH
OVER-50
CERTIFICATE: YES | NO
Indicate if you wish to receive, if eligible, a paper certificate sent via postal mail by the contest sponsor. The contest sponsor may or may not honor this tag, and if so may or may not use opt-in or opt-out as the default. YES is the default.

CLAIMED-SCORE: nnnnn
The claimed-score of the log submission, in integer form such as 1217315 or 19113 or 12. Do not include commas, decimals, or other text.

CLUB: text
Name of the radio club to which the score should be applied.

CREATED-BY: text
Name and version of the logging program used to create the Cabrillo file.

EMAIL: text
Contact email address for the entrant. Must be valid email or blank.

GRID-LOCATOR: text
Used to indicate the Maidenhead Grid Square where the station was operating from. E.g., FN42EB

LOCATION:
Used to indicate the location where the station was operating from.

ARRL/RAC Sections	For USA and Canada stations LOCATION must be the ARRL section abbreviation. For foreign stations LOCATION must be ‘DX’. This information is required for IARU-HF and for all ARRL and CQ contests.
IOTA Island Name	This information is required for RSGB-IOTA contest and includes Island Name (not IOTA reference number).
RDA Number	This information is required for RDXC contest.
NAME: text
Name. Maximum of 75 characters long.

ADDRESS: text
Mailing address. Each line should be a maximum of 45 characters long. Up to 6 address lines are permitted.

ADDRESS-CITY: text
ADDRESS-STATE-PROVINCE: text
ADDRESS-POSTALCODE: text
ADDRESS-COUNTRY: text
Optional fields for providing mailing address details.

OPERATORS: callsign1 [callsign2 callsign3…]
A space or comma-delimited list of operator callsign(s). You may also list the callsign of the host station by placing an “@” character in front of the callsign within the operator list, such as

OPERATORS: K1ABC N5XYZ @N6IJ
OPERATORS: K1ABC, N5XYZ, @N6IJ

The OPERATOR line is a maximum of 75 characters long and must begin with OPERATORS: followed by a space. Use multiple OPERATOR lines if needed.

OFFTIME: begin-time end-time
This tag is used to indicate off-time.

OFFTIME: 2002-03-22 0300 2002-03-22 0743

         yyyy-mm-dd nnnn yyyy-mm-dd nnnn
         -----begin----- ------end------

Note: About on-time and off-time:

QSOs in Cabrillo are logged with a granularity of one minute. The use of one second granularity would lead to the question “during which exact second did the QSO take place?”

The ARRL has taken the very sensible interpretation that on-time and off-time also should be considered with a granularity of one minute. In other words, during any given minute you can either be on or off. Therefore 1801Z-1830Z would be a 30 minute off-time, with 1800Z being your last minute on before the break and 1831Z being the minute you resume operation.

If you make a QSO at 1800Z, take a break, and resume operating at 1830Z, then you have completed a 29 minute off-time.

SOAPBOX: text
Soapbox comments. Enter as many lines of soapbox text as you wish. Each line is a maximum of 75 characters long and must begin with SOAPBOX: followed by a space.

QSO: qso-data
QSO data as specified by the Cabrillo QSO data format. All QSO lines must appear in chronological order. See QSO data specification.

X-<anything>: text
Lines beginning with X- are ignored by the robot and log checking software.  Use this for adding comment lines to your log. Exception: See X-QSO below.

X-QSO: qso-data
Any QSO marked with this tag will be ignored in your log. Use to mark QSOs made that you do not want to count toward your score.

Developer Tags
DEBUG: n

The DEBUG: tag is intended for software developers. If an ARRL or CQ email robot receives a log with the DEBUG: tag set, then it will do all its regular processing and generate an auto-reply message, but it will not save the log into data set provided to the log checking team.

Sponsors


DX Engineering Logo

World Wide Radio Operators FoundationProudly powered by WordPress.
 */

enum cabrillo_order
{
    PContestName, PCallUsed,
    PAssisted,
    PBand, PMode,
    POperator1, POperator2,
    PPower, PStation,
   // PTime, PTransmitter,
    POverlay,
    //POffTime,
    PClaimedScore, PClub,
    PCreatedBy,
    PGridLoc, PLocation,
    PEmail,
    PAdr1, PAdr2,
    PCity, PCountry,
    PPoCo,
    LineCount
};
class cabrilloLine
{
   public:
      QString prefix;
      QString data;

      cabrilloLine( const QString &, const QString &dat );
      cabrilloLine( );
      ~cabrilloLine();

      cabrilloLine & operator = ( const cabrilloLine & );
};
cabrilloLine::cabrilloLine(const QString &pre, const QString &dat ) :
      prefix( pre ), data( dat )
{}
cabrilloLine::cabrilloLine()
{}
cabrilloLine::~cabrilloLine()
{}
cabrilloLine &cabrilloLine::operator = ( const cabrilloLine &l )
{
   prefix = l.prefix;

   data = l.data;
   return *this;
}

Cabrillo::Cabrillo( LoggerContestLog * const ct )
      : ct( ct )
{}
Cabrillo::~Cabrillo()
{}


bool Cabrillo::exportTest(QSharedPointer<QFile> expfd)
{
    // export current contest as CABRILLO
    writer wr( expfd );
    char lbuff[ 1024 ];

    wr.lwrite( "START-OF-LOG:3.0" );

    // First the header info
    // calc mult count
    int ltot = 0;
    int nctry = 0;
    int ndistrict = 0;
    int nlocs = 0;
    int bonus = 0;

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

    int nvalid = 0;      // valid QSOs
    int nlines = 0;      // QSO records
    foreach(MapWrapper<BaseContact> dct, ct->ctList)
    {
       QSharedPointer<BaseContact> cct = dct.wt;
       // Extract comments for "Remarks" section
       //cct->addReg1TestComment( remarks );

       if ( cct->contactFlags.getValue() & ( LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) )
          continue;

       nlines++;
       if ( cct->contactScore.getValue() > 0 )
       {
          nvalid++;
          bonus += cct->bonus;
       }
    }
    if (bonus)
         nlocs = ct->getNlocs();

    QStringList remarks;
    remarks.append( ct->entCondx1.getValue() );
    remarks.append( ct->entCondx2.getValue() );
    remarks.append( ct->entCondx3.getValue() );
    remarks.append( ct->entCondx4.getValue() );

    cabrilloLine linelist[ static_cast<int>(LineCount) ];

    QString pver = QString("Minos by G0GJV, version ") + STRINGVERSION  + " " + PRERELEASETYPE;
    linelist[ static_cast< int> (PCreatedBy) ] = cabrilloLine( "CREATED_BY", pver  /*, "logger"*/ );

    linelist[ static_cast< int> (PContestName) ] = cabrilloLine( "CONTEST", ct->name.getValue()  /*, "Contest Name"*/ );

    linelist[ static_cast< int> (PCallUsed )] = cabrilloLine( "CALLSIGN", ct->mycall.getFullCall()  /*, "Callsign Used"*/ );
    linelist[ static_cast< int> (PGridLoc )] = cabrilloLine( "GRID-LOCATOR", ct->myloc.loc.getValue()  /*, "Locator Used"*/ );
    linelist[ static_cast< int> (PLocation )] = cabrilloLine( "LOCATION", ct->location.getValue()  /*, "Exchange Used"*/ );
    linelist[ static_cast< int> (PAdr1 )] = cabrilloLine( "ADDRESS", ct->entAddr1.getValue()  /*, "Address line 1/2 of station"*/ );
    linelist[ static_cast< int> (PAdr2 )] = cabrilloLine( "ADDRESS", ct->entAddr2.getValue()  /*, "Address line 2/2 of station"*/ );
    linelist[ static_cast< int> (PCity )] = cabrilloLine( "ADDRESS-CITY", ct->entCity.getValue()  /*, "Address line 2/2 of station"*/ );
    linelist[ static_cast< int> (PCountry )] = cabrilloLine( "ADDRESS-COUNTRY", ct->entCountry.getValue()  /*, "Address line 2/2 of station"*/ );
    linelist[ static_cast< int> (PPoCo )] = cabrilloLine( "ADDRESS-POSTALCODE", ct->entPostCode.getValue()  /*, "Address line 2/2 of station"*/ );
    linelist[ static_cast< int> (PEmail )] = cabrilloLine( "EMAIL", ct->entEMail.getValue()  /*, "Address line 2/2 of station"*/ );

    linelist[ static_cast< int> (PBand )] = cabrilloLine( "BAND","ALL"  /*, "Bands"*/ );
    linelist[ static_cast< int> (PMode )] = cabrilloLine( "MODE", "MIXED"  /*, "modes"*/ );
    linelist[ static_cast< int> (PAssisted )] = cabrilloLine( "CATEGORY-ASSISTED", "ASSISTED");
    linelist[ static_cast< int> (POverlay )] = cabrilloLine( "CATEGORY-OPERATOR", "SINGLE-OP");
    linelist[ static_cast< int> (PPower )] = cabrilloLine( "CATEGORY-POWER", "LOW");
    linelist[ static_cast< int> (PStation )] = cabrilloLine( "CATEGORY-STATION", "FIXED");

    linelist[ static_cast< int> (PClub )] = cabrilloLine( "CLUB", ct->entrant.getValue()  /*, "Name of club/group"*/ );

    QString opsl1 = ct->ops1.getValue();
    QString opsl2 = ct->ops2.getValue();
    if (opsl1.size() == 0 && opsl2.size() == 0)
    {
       opsl1 = ct->opsQSO1;
       opsl2 = ct->opsQSO2;
    }

    linelist[ static_cast< int> (POperator1) ] = cabrilloLine( "OPERATORS", opsl1  /*, "Operators line 1/2"*/ );
    linelist[ static_cast< int> (POperator2) ] = cabrilloLine( "OPERATORS", opsl2  /*, "Operators line 2/2"*/ );

    linelist[ static_cast< int> (PClaimedScore) ] = cabrilloLine( "CLAIMED-SCORE", QString::number( ct->contestScore * ct->bandPointsMultiplier.getValue() * ltot + bonus ) ); /*, "Claimed total score"*/

    for ( int i = 0; i < LineCount; i++ )
    {
       sprintf( lbuff, "%s: %s", linelist[ i ].prefix.toStdString().c_str(), linelist[ i ].data.toStdString().c_str() );
       wr.lwrite( lbuff );
    }

    for ( int i = 0; i < remarks.size(); i++ )
    {
       wr.lwrite( "SOAPBOX: " + remarks[ i ] );
    }

    // and then the contact list

    foreach(MapWrapper<BaseContact> dct, ct->ctList)
    {
        QSharedPointer<BaseContact> cct = dct.wt;

       if ( cct->contactFlags.getValue() & ( LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) )
       {
          // should all COMMENT_ONLY records go into remarks?
          // and also comments on individual QSOs?, noSerials
          continue;
       }

       QString sbuff;
       cct->getCabrilloText( sbuff );   // lbuff if "bsize" = 256

       if ( sbuff.length() == 0 )
          continue;

       sbuff = trimr( sbuff );			// No need to pad to 250!!
       wr.lwrite( sbuff );
    }

    QString peol = QString("END-OF-LOG:");
    wr.lwrite( peol );

    return true;
}
