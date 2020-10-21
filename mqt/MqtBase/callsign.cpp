#include "base_pch.h"
#include "mults.h"
#include "cutils.h"

#include "callsign.h"

Callsign::Callsign( )
{
    fullCall.setValue( "" );
}
//Callsign::Callsign(const QString &pcs )
//{
//   fullCall.setValue( pcs );
//   valRes = CS_NOT_VALIDATED;
//   validate();
//}
// default versions are good enough for now!
//callsign::callsign(const callsign&)
//{
//}
//
//Callsign& Callsign::operator =(const Callsign& rhs)
//{
//    fullCall.setValue(rhs.fullCall);
//    valRes = CS_NOT_VALIDATED;
//    validate();
//    return *this;
//}
Callsign::~Callsign()
{
   // nothing needed
}
int Callsign::setFullCall(const QString &pcs)
{
    fullCall.setValue( pcs.trimmed().toUpper() );
    valRes = CS_NOT_VALIDATED;
    validate();
    return valRes;
}

static bool isMobileString ( QString p )
{
    return ( p == "P" || p == "M" || p == "MM" || p == "A" );
}
static bool isMobileNumeric ( QString p )
{
    return ( p.length() == 1 &&  p[ 0 ].isNumber() );
}
static bool isNumeric ( const QString &s )
{
    int slen = s.length();
    if ( slen == 0 )
    {
        return false;
    }
    for ( int i = 0; i < slen; i++ )
    {
        if ( !s[ i ].isNumber() )
        {
            return false;
        }
    }
    return true;
}
static QString getPrefix ( QString p, QSharedPointer<CountrySynonym> &csyn )
{
//    QString testpart = p;

//    while ( testpart.length() >= 1 )
//    {
//        // we need to stop when we get to the basic prefix...
//        // otherwise RVI6ABC ends up matching R, which is UA

//        csyn = MultLists::getMultLists()->searchCountrySynonym ( testpart );

//        if ( csyn )
//        {
//            break;
//        }

//        testpart = testpart.left (testpart.length() - 1 );
//    }
//    return testpart;

    // we need to keep adding to testpart until we fail to have a synonym
    // then we come back one...
    QString testPart;
    QSharedPointer<CountrySynonym> lastCsyn;
    for (int i = 1; i < p.size(); i++)
    {
        testPart = p.left(i);
        lastCsyn = MultLists::getMultLists()->searchCountrySynonym ( testPart );

        if ( lastCsyn )
        {
            csyn = lastCsyn;
            continue;
        }
        testPart = p.left(i - 1);
        break;
    }
    return testPart;    // the successful part!
}

static int extraTail ( QString p, QSharedPointer<CountrySynonym> &csyn )
{

    QString testpart = getPrefix ( p, csyn );
    if ( testpart.length() == 0 )
    {
        return -1;   // we didn't find a synonym
    }
    int tail =  p.length() - testpart.length();
    return tail;
}

bool Callsign::isValidStructure()
{
    return isValidStructure( dupPrefix, number, body );
}
/*static*/ bool Callsign::isValidStructure( const QString &prefix,  const QString &number,  const QString &body )
{
    if ( prefix.length() > 0 && number.length() > 0 )
    {
        if ( body.length() > 0)// || ( body.length() == 0 && prefix == "LM" && number == "1814" ) )
        {
            return true;
        }
        return false;
    }
    return false;
}

int Callsign::validate( )
{
    if ( valRes != CS_NOT_VALIDATED )
        return valRes;

    QString cs = fullCall.getValue();

    valRes = ERR_NOCS;
    int cslen = cs.length();
    if ( cslen == 0 )    			// all spaces
        return valRes;

    // first check no funny characters; only alphanum and / allowed
    // (spaces are now invalid)

    valRes = ERR_INVCS;

    for ( int i = 0; i < cslen; i++ )
    {
        QChar c = cs[i];

        if (c.isDigit())
            continue;

        if (c == '/')
            continue;

        if ( c>= 0x40 && c <= 0x5A) // Basic ASCII uppercase
            continue;
        return valRes;
    }

    locCtryPrefix.clear();
    dupPrefix.clear();
    suffix.clear();
    number.clear();
    body.clear();

    // Break cs into it's 3 possible parts as divided by "/"
    QString p1;
    QString p2;
    QString p3;
    int spos = cs.indexOf( "/" );
    if ( spos != -1 )
    {
        p1 = cs.left(spos );
        cs = cs.mid ( spos + 1 );
        spos = cs.indexOf ( "/" );
        if ( spos != -1 )
        {
            p2 = cs.left ( spos );
            p3 = cs.mid ( spos + 1 );
        }
        else
        {
            p2 = cs;
        }
    }
    else
    {
        p1 = cs;
    }

    // We now need to analyse what we have

    QString countryPrefix;
    QString call;
    QString mobileSuffix;

    int p1size = p1.length();
    int p2size = p2.length();

    if ( p1size == 0 )
    {
        valRes = ERR_INVCS;
        return valRes;
    }
    if ( p1size != 0 && p2size == 0 )
    {
        call = p1;  // just a single part
    }
    else
    {
        // at least two parts
        if ( isMobileString ( p3 ) )
        {
            mobileSuffix = p3;
        }
        else
            if ( isMobileNumeric ( p3 ) )
            {
                mobileSuffix = p3;
            }

        if ( isMobileString ( p2 ) )
        {
            mobileSuffix = p2;
            call = p1;
        }
        else
            if ( isMobileNumeric ( p2 ) )
            {
                mobileSuffix = p2;
                call = p1;
            }
        if ( call.isEmpty() )
        {
            // decide which is call and which is prefix - or is the whole thing broken

            QSharedPointer<CountrySynonym> csyn1;
            QSharedPointer<CountrySynonym> csyn2;
            int p1t = extraTail ( p1, csyn1 );
            int p2t = extraTail ( p2, csyn2 );

            if ( p1t == -1 || p2t == -1 )
            {
                if ( p1size <= p2size )
                {
                    countryPrefix = p1;
                    call = p2;
                }
                else
                {
                    countryPrefix = p2;
                    call = p1;
                }
            }
            else
                if ( p1t < p2t )
                {
                    // We hope that the shortest tail will be the country of location
                    countryPrefix = p1;
                    call = p2;
                }
                else
                    if ( p1t == p2t )
                    {
                        if ( p1size <= p2size )
                        {
                            countryPrefix = p1;
                            call = p2;
                        }
                        else
                        {
                            countryPrefix = p2;
                            call = p1;
                        }
                    }
                    else
                    {
                        countryPrefix = p2;
                        call = p1;
                    }
        }
    }

    locCtryPrefix = countryPrefix;
    suffix = mobileSuffix;

    // NB that leading spaces may be genuine wildcards, and so may
    // actually be significant; we should reject as invalid
    // calls with leading spaces, even though this isn't very friendly

    // first check no funny characters; only alphanum and / allowed
    // (spaces are now invalid)

    valRes = ERR_INVCS;

    QString leadPart;
    int callOffset = 0;

    while (  call[callOffset].isDigit() )    // leading digits (2 of 2E0...)
    {
        leadPart += call[callOffset++];
    }

    int csize = call.length();
    while ( callOffset < csize )            // prefix letters (E of 2E0)
    {
        if ( call[callOffset].isDigit() )
        {
            break ;
        }
        else
        {
            leadPart += call[callOffset++];
        }
    }

    // with e.g. RT7T leadpart is RT but prefix3 is R, as cty.dat doesn't have RT
    // so it gets reduced to R. country of location is R but we want dupprefix to be RT
    // With GW4ALG location is GW but dupPrefix needs to be G

    QSharedPointer<CountrySynonym> csyn;
    QString prefix3 = getPrefix ( call, csyn );  // strip call back until we find it in the control file
    if ( prefix3.length() )
    {
        if ( locCtryPrefix.length() == 0 )
        {
            // locCtryPrefix is country of location
            // dupPrefix is country of issue
            // prefix3 is based on the issue country - but we have no xx/ leadin
            // so loc and issue are the same

            locCtryPrefix = prefix3;

            // What should we do with e.g. NH6T/W4
            if ( isNumeric ( suffix ))    //eg for w3abc/6 US call areas changes w3 to w6
            {
                locCtryPrefix += suffix;
            }
        }
    }

    if ( dupPrefix.isEmpty() )  // it SHOULD be empty!
    {
        dupPrefix = leadPart;
    }


    while ( callOffset < csize && call[callOffset].isDigit() )     // central number (0 of 2E0)
        number += call[callOffset++];


    // main CS letters
    body = call.mid(callOffset);

    realCall = dupPrefix + number + body;

    /*
     From CQWW WPX rules...
        1. A PREFIX is the letter/numeral combination which forms the first part of the amateur call.
        Examples: N8, W8, WD8, HG1, HG19, KC2, OE2, OE25, LY1000, etc. Any difference in the numbering,
        lettering, or order of same shall count as a separate prefix. A station operating from a DXCC entity
        different from that indicated by its call sign is required to sign portable. The portable prefix
        must be an authorized prefix of the country/call area of operation. In cases of portable operation,
        the portable designator will then become the prefix. Example: N8BJQ operating from Wake Island would
        sign N8BJQ/KH9 or N8BJQ/NH9. KH6XXX operating from Ohio must use an authorized prefix for the U.S.
        8th district (/W8, /AD8, etc.). Portable designators without numbers will be assigned a zero (Ø)
        after the second letter of the portable designator to form the prefix. Example: PA/N8BJQ would become
        PAØ. All calls without numbers will be assigned a zero (Ø) after the first two letters to form the prefix.
        Example: XEFTJW would count as XEØ. Maritime mobile, mobile, /A, /E, /J, /P, or other license class
        identifiers do not count as prefixes.
     */

    wpxPrefix = dupPrefix + number;     // for now...

    for (int i = 0; i < body.count(); i++)
    {
        if (body[i].isDigit())
        {
            valRes = ERR_INVCS;
            return valRes;
        }
    }

    valRes = ERR_CSDIGIT;
    if ( number.length() == 0 )
        return valRes;

    // Replace real dupPrefix with the dup check dupPrefix - for dup checking

    QSharedPointer<CountrySynonym> syn = MultLists::getMultLists()->searchCountrySynonym ( dupPrefix );
    if ( syn )
    {
        syn->getDupPrefix ( dupPrefix );
    }

    valRes = CS_OK;
    if (!isValidStructure() ||  locCtryPrefix.length() == 0 )   	// suffix can be null
    {
        valRes = ERR_INVCS;
    }
    if (fullCall.getValue().right(1) == "/" )
    {
        valRes = ERR_INVCS;
    }

    return valRes;

}
//============================================================
bool Callsign::isUK() const
{
   if (fullCall.getValue().size() == 0)
   {
      return false;
   }
   // is this callsign in one of the UK areas?
   // prefix is country of location
   // prefix2 is country of issue
   return MultLists::getMultLists()->isUKprefix(*this);
}
//============================================================
bool Callsign::operator==( const Callsign& rhs ) const
{
   const Callsign * c1 = this;    // search item
   const Callsign *c2 = &rhs;    // collection item

   int ret = c1->dupPrefix.compare(c2->dupPrefix, Qt::CaseInsensitive );
   if ( ret != 0 )
      return false;

   ret = c1->number.compare(c2->number, Qt::CaseInsensitive );
   if ( ret != 0 )
      return false;

   ret = c1->body.compare( c2->body, Qt::CaseInsensitive );
   if ( ret != 0 )
      return false;

   return true;
}

bool Callsign::operator<( const Callsign& rhs ) const
{
   const Callsign * c1 = this;    // search item
   const Callsign *c2 = &rhs;    // collection item
    int ret = c1->dupPrefix.compare(c2->dupPrefix, Qt::CaseInsensitive );
    if ( ret == 0 )
    {
       ret = c1->number.compare(c2->number, Qt::CaseInsensitive );
       if ( ret == 0 )
       {
          ret = c1->body.compare( c2->body, Qt::CaseInsensitive );
          if ( ret == 0 )
             return false;
       }
    }
    return ret < 0;
}

