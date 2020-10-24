#include <QString>
#include <QtTest>

#include "BandList.h"
#include "Calendar.h"
#include "CalendarList.h"

#include "base_pch.h"


class RPCTestTest : public QObject, RPCDispatcher
{
    Q_OBJECT

public:
    RPCTestTest();

    typedef void ( RPCTestTest::* DispatchCallback ) ( XStanza *stanza ) ;
    DispatchCallback dispatchCallback;

    virtual void dispatchResponse( XStanza *a );
    void analyseRequestTest( XStanza * );

private Q_SLOTS:
    void testCase1();

    void testInt();
    void testBool();
    void testDouble();
    void testString();
    void testBase64();
    void testDtg();
    void testStruct();
    void testArray();
    void testMultiParam();
    void testAddParams();

    void testRequest();

    void testParse();
    void testBands();

    void testCalendarParse();
    void testContests();
    void testDates();
    void testParse2009();
    void testContests2009();
    void testDates2009();
    void testBST();

    void testMults();
};

RPCTestTest::RPCTestTest()
{
}

void RPCTestTest::testCase1()
{

    QVERIFY2(true, "Failure");
}

void RPCTestTest::testInt()
{
   int i1 = 1;

   RPCArgs * xms = new RPCArgs();
   xms->addParam( i1 );

   QString pstr = xms->makeParamsString();

   delete xms;

   RPCArgs xm;
   TIXML_STRING sstr = pstr.toStdString();
   bool parseOK = xm.parseParams( sstr);
   QVERIFY( parseOK );

   int ires;

   bool pOK = xm.getIntArg( 0, ires );
   QVERIFY(pOK );
   QTRY_COMPARE( i1, ires );
}

void RPCTestTest::testBool()
{
   bool b1 = true;

   RPCArgs * xms = new RPCArgs();
   xms->addParam( b1 );
   xms->addParam( !b1 );

   QString pstr = xms->makeParamsString();

   delete xms;

   RPCArgs xm;
   TIXML_STRING sstr = pstr.toStdString();
   bool parseOK = xm.parseParams( sstr);
   QVERIFY(parseOK );

   bool bres;

   bool pOK = xm.getBooleanArg( 0, bres );
   QVERIFY( true == pOK );
   QVERIFY( b1 == bres );

   pOK = xm.getBooleanArg( 1, bres );
   QVERIFY( true == pOK );
   QVERIFY( b1 != bres );
}

void RPCTestTest::testDouble()
{
   double d1 = 3.14159;

   RPCArgs * xms = new RPCArgs();
   xms->addParam( d1 );

   QString pstr = xms->makeParamsString();

   delete xms;

   RPCArgs xm;
   TIXML_STRING sstr = pstr.toStdString();
   bool parseOK = xm.parseParams( sstr);
   QVERIFY( true == parseOK );

   double dres;

   bool pOK = xm.getDoubleArg( 0, dres );
   QVERIFY( true == pOK );
   QVERIFY( d1 == dres );
}

void RPCTestTest::testString()
{
   QString s1( "This is a longish string which we are using for testing" );
   QString s2( "Try it <with some XML \"escape   characters\"> & some that aren't" );

   RPCArgs * xms = new RPCArgs();
   xms->addParam( s1 );
   xms->addParam( s2 );

   QString pstr = xms->makeParamsString();

   delete xms;

   RPCArgs xm;
   TIXML_STRING sstr = pstr.toStdString();
   bool parseOK = xm.parseParams( sstr);
   QVERIFY( true == parseOK );

   QString sres;

   bool pOK = xm.getStringArg( 0, sres );
   QVERIFY( true == pOK );
   QVERIFY( s1 == sres );

   pOK = xm.getStringArg( 1, sres );
   QVERIFY( true == pOK );
   QVERIFY( s2 == sres );
}

void RPCTestTest::testBase64()
{
   QString s1( "This is a longish string which we are using for testing" );

   RPCArgs * xms = new RPCArgs();
   xms->addBase64Param( s1 );

   QString pstr = xms->makeParamsString();

   delete xms;

   RPCArgs xm;
   TIXML_STRING sstr = pstr.toStdString();
   bool parseOK = xm.parseParams( sstr);
   QVERIFY( true == parseOK );

   QString sres;

   bool pOK = xm.getBase64Arg( 0, sres );
   QVERIFY( true == pOK );
   QVERIFY( s1 == sres );
}

void RPCTestTest::testDtg()
{
   QString s1( "This is a longish string which we are using for testing" );

   RPCArgs * xms = new RPCArgs();
   xms->addDtgParam( s1 );

   QString pstr = xms->makeParamsString();

   delete xms;

   RPCArgs xm;
   TIXML_STRING sstr = pstr.toStdString();
   bool parseOK = xm.parseParams( sstr);
   QVERIFY( true == parseOK );

   QString sres;

   bool pOK = xm.getDtgArg( 0, sres );
   QVERIFY( true == pOK );
   QVERIFY( s1 == sres );
}

void RPCTestTest::testStruct()
{
   RPCArgs * xms = new RPCArgs();
   QSharedPointer<RPCParam>ps(new RPCParamStruct);
   ps->addMember( QString( "name value" ), "fred" );
   ps->addMember( QString( "value value" ), "joe" );
   xms->addParam( ps );

   QString pstr = xms->makeParamsString();

   delete xms;

   RPCArgs xm;
   TIXML_STRING sstr = pstr.toStdString();
   bool parseOK = xm.parseParams( sstr);
   QVERIFY( true == parseOK );

   QSharedPointer<RPCParam> pres;
   QString sres;
   int ires;

   bool pOK = xm.getStructArgMember( 0, "fred", pres );
   QVERIFY( true == pOK );
   pOK = pres->getString( sres );
   QVERIFY( true == pOK );
   QVERIFY( QString( "name value" ) == sres );

   pOK = pres->getInt( ires );
   QVERIFY( false == pOK );

   pOK = xm.getStructArgMember( 0, "test", pres );
   QVERIFY( false == pOK );

   pOK = xm.getStructArgMember( 0, 1, pres );
   QVERIFY( true == pOK );
   pOK = pres->getString( sres );
   QVERIFY( true == pOK );
   QVERIFY( QString( "value value" ) == sres );

   pOK = xm.getStructArgMember( 0, 2, pres );
   QVERIFY( false == pOK );
}

RPCParamArray *RPCArrayFromStringVector( const QVector< QString> &plist )
{
   RPCParamArray * rpc = new RPCParamArray();
   for ( int i = 0; i < plist.size(); i++ )
   {
      rpc->addElement( plist[ i ] );
   }
   return rpc;
}

void RPCTestTest::testArray()
{
   RPCArgs * xms = new RPCArgs();

   QVector < QString > testvec;
   testvec.push_back( "string 1" );
   testvec.push_back( "string 2" );
   testvec.push_back( "string 3" );
   testvec.push_back( "string 4" );
   testvec.push_back( "" );


   QSharedPointer<RPCParam>rpca(RPCArrayFromStringVector( testvec ));
   xms->addParam( rpca );

   QString pstr = xms->makeParamsString();

   delete xms;

   RPCArgs xm;
   TIXML_STRING sstr = pstr.toStdString();
   bool parseOK = xm.parseParams( sstr);
   QVERIFY( true == parseOK );

   //      bool GetArrayArgElements( unsigned int argno, unsigned int &size );
   //      bool GetArrayArgElement( unsigned int argno, unsigned int eleno, RPCParam *&res );

   int asize;

   bool pOK = xm.getArrayArgElements( 0, asize );
   QVERIFY( true == pOK );
   QVERIFY( testvec.size() == asize );

   for ( int i = 0; i < testvec.size(); i++ )
   {
      QSharedPointer<RPCParam> pres;
      QString sres;

      bool pOK = xm.getArrayArgElement( 0, i, pres );
      QVERIFY( true == pOK );
      QVERIFY( pres != nullptr );

      pOK = pres->getString( sres );
      QVERIFY( true == pOK );
      QVERIFY( testvec[ i ] == sres );
   }
}

void
RPCTestTest::testMultiParam()
{
   QString s1( "BarPosition" );
   QString s2( "HOVenue" );
   int i1 = 1;

   RPCArgs * xms = new RPCArgs();
   xms->addParam( s1 );
   xms->addParam( s2 );
   xms->addParam( i1 );

   QString pstr = xms->makeParamsString();

   delete xms;

   RPCArgs xm;
   TIXML_STRING sstr = pstr.toStdString();
   bool parseOK = xm.parseParams( sstr);
   QVERIFY( true == parseOK );

   QString sres;
   int ires;

   bool pOK = xm.getStringArg( 0, sres );
   QVERIFY( true == pOK );
   QVERIFY( s1 == sres );

   pOK = xm.getStringArg( 1, sres );
   QVERIFY( true == pOK );
   QVERIFY( s2 == sres );

   pOK = xm.getIntArg( 2, ires );
   QVERIFY( true == pOK );
   QVERIFY( i1 == ires );
}

void RPCTestTest::testAddParams()
{
   // This is more complicated, and needs a real iks node
}

// Here we REALLY need to find a way of using DaemonThread and its descendants
// so that we can do a REAL test!

void RPCTestTest::dispatchResponse( XStanza *xs )
{
   ( this->*dispatchCallback ) ( xs ); // What a horrid syntax for calling through a member pointer!
}

//---------------------------------------------------------------------------

void
RPCTestTest::testRequest()
{
   QString toId;
   RPCRequest *xm = new RPCRequest( toId, "testRequest" );

   QString s1( "BarPosition" );
   QString s2( "HOVenue" );
   int i1 = 1;

   xm->addParam( s1 );
   xm->addParam( s2 );
   xm->addParam( i1 );

   TIXML_STRING UTF8XML = xm->getActionMessage();

   delete xm;

   dispatchCallback = &RPCTestTest::analyseRequestTest;

   bool ret = analyseNode( this, UTF8XML );
   QVERIFY( true == ret );
}
void RPCTestTest::analyseRequestTest( XStanza *xs )
{
   RPCRequest * req = dynamic_cast<RPCRequest *>( xs );
   QVERIFY( req != 0 );

   // and now analyse req
   QVERIFY( req->methodName == "testRequest" );

   QString s;
   bool pOK = req->getStringArg( 0, s );
   QVERIFY( true == pOK );
   QVERIFY( s == "BarPosition" );

   pOK = req->getStringArg( 1, s );
   QVERIFY( true == pOK );
   QVERIFY(  s == "HOVenue" );

   int i;
   pOK = req->getIntArg( 2, i );
   QVERIFY( pOK );
   QVERIFY( i == 1 );
}
//---------------------------------------------------------------------------
void RPCTestTest::testParse()
{
   BandList blist;
   bool loaded = blist.parseFile( "C:/Projects/sfMinos/Minos/MinosLogger/Configuration/bandlist.xml" );
   QVERIFY( true == loaded );
}
void RPCTestTest::testBands()
{
   BandList blist;
   bool loaded = blist.parseFile( "C:/Projects/sfMinos/Minos/MinosLogger/Configuration/bandlist.xml" );
   QVERIFY( true == loaded );

   QSharedPointer<BandInfo>  b;
   loaded = blist.findBand(144250000.0, b);
   QVERIFY( true == loaded );
   QVERIFY( QString("VHF") == b->getType() );
   int cnt = blist.bandList.size();
   QVERIFY( 21 == cnt );

}
void RPCTestTest::testDates()
{
#ifdef TESTCALENDAR
   setYear( 2008 );

   int m = getMonth( "february" );
   QVERIFY( 2 == m );
   m = getMonth( "junk" );
   QVERIFY( 0 == m );

   int d = getDate( /*month*/1,  /*day*/1,  /*week*/1 );   // first Monday is 6th
   QVERIFY( 7 == d );  // Tuesday 1st, first Monday is 7th

   d = getDate( /*month*/5,  /*day*/6,  /*week*/3 );   // May 144
   QVERIFY( 17 == d );  // Tuesday 1st, first Monday is 7th

   d = getDate( /*month*/6,  /*day*/7,  /*week*/2 );
   QVERIFY( 8 == d );  // 2nd sunday in june

   d = getDate( /*month*/6,  /*day*/2,  /*week*/5 );
   QVERIFY( 0 == d );  // 5th Tuesday in june

   QDateTime t = QDateTime( QDate(2008, 5, 1) );

   QDateTime t2 = localToUTC( t );
   //int diff = t2.secsTo(t);
   //QVERIFY( 3600 == diff);

   t = QDateTime( QDate(2008, 10, 25 ));

   t2 = localToUTC( t );

   //int diff2 = t2.secsTo(t);
   //QVERIFY( 3600 == diff2);

   t = QDateTime( QDate(2008, 10, 26) );

   t2 = localToUTC( t );
   //QVERIFY( 0 == t2.secsTo(t));
#endif
}
void RPCTestTest::testCalendarParse()
{
   Calendar vhf(2008, ectVHF);
   bool loaded = vhf.parseFile( "C:/Projects/adjudge/adjsql/vhfcontests.xml" );
   QVERIFY( true == loaded );
}
void RPCTestTest::testContests()
{
   Calendar vhf(2008, ectVHF);
   bool loaded = vhf.parseFile( "C:/Projects/adjudge/adjsql/vhfcontests.xml" );
   QVERIFY( true == loaded );

   int nc = vhf.contests.size();
   QVERIFY( 33 == nc );

   QString lp144( "lp144" );
   CalendarContest &qrp144 = vhf.contests[ lp144 ];
   QVERIFY( lp144 == qrp144.name );

}
void RPCTestTest::testDates2009()
{
#ifdef TESTCALENDAR

   setYear( 2009 );

   int m = getMonth( "february" );
   QVERIFY( 2 == m );
   m = getMonth( "junk" );
   QVERIFY( 0 == m );

   int d = getDate( /*month*/1,  /*day*/1,  /*week*/1 );   // first Monday is 5th
   QVERIFY( 5 == d );

   d = getDate( /*month*/5,  /*day*/6,  /*week*/3 );   // May 144 - Sat 16th
   QVERIFY( 16 == d );

   d = getDate( /*month*/6,  /*day*/7,  /*week*/2 );
   QVERIFY( 14 == d );  // 2nd sunday in june

   d = getDate( /*month*/6,  /*day*/2,  /*week*/5 );
   QVERIFY( 30 == d );  // 5th Tuesday in june

   QDateTime t = QDateTime( QDate(2009, 5, 1 ));

   QDateTime t2 = localToUTC( t );
  // int diff = t2.secsTo(t);
  // QVERIFY( 3600 == diff);

   t = QDateTime( QDate(2009, 10, 24 ));

   t2 = localToUTC( t );

   //QVERIFY( 3600 == t2.secsTo(t));

   t = QDateTime( QDate(2009, 10, 25 ));

   t2 = localToUTC( t );
   QVERIFY( 0 == t2.secsTo(t));
#endif
}
void RPCTestTest::testParse2009()
{
   Calendar vhf(2009, ectVHF);
   bool loaded = vhf.parseFile( "C:/Projects/adjudge/adjsql/vhfcontests09.xml" );
   QVERIFY( true == loaded );
}
void RPCTestTest::testContests2009()
{
   Calendar vhf(2009, ectVHF);
   bool loaded = vhf.parseFile( "C:/Projects/adjudge/adjsql/vhfcontests09.xml" );
   QVERIFY( true == loaded );

   int nc = vhf.contests.size();
   QVERIFY( 33 == nc );

   QString lp144( "lp144" );
   CalendarContest &qrp144 = vhf.contests[ lp144 ];
   QVERIFY( lp144 == qrp144.name );

}
#ifdef TESTCALENDAR

int fsun(int month)
{
   int fSunday = getDate(month, 7, 1);    // month, day, week
   return fSunday;
}
#endif
void RPCTestTest::testBST()
{
#ifdef TESTCALENDAR

   for (int i = 0; i < 40; i++)
   {
      setYear(2005 + i);
      {
         int fs = fsun(4);
         int lsMarch = fs + 31 - 7;
         QDateTime t(QDate(2005 + i, 3, lsMarch));
         int dow = getDayOfWeek(t);
         QVERIFY( 7 == dow );
         QVERIFY( true == (dow >= 1 && dow <= 7));
      }
      {
         int fs = fsun(11);
         int lsOct = fs + 31 - 7;
         QDateTime t(QDate(2005 + i, 10, lsOct));
         int dow = getDayOfWeek(t);
         QVERIFY( 7 == dow );
         QVERIFY( true == (dow >= 1 && dow <= 7));
      }
   }
#endif
}

void RPCTestTest::testMults()
{
    MinosParametersAdapter mp;

    MultLists *m = MultLists::getMultLists();

    QFile fos("c:/temp/multlist.txt");
    if (!fos.open(QIODevice::WriteOnly|QIODevice::Text))
       return ;

    QTextStream os(&fos);


    /*
    os << "================== country entries ========================\n";
    for (MultList < CountryEntry >::iterator i = m->ctryList.begin(); i != m->ctryList.end(); i++)
    {
       os << i->wt->basePrefix + " " + i->wt->realName << "\n";
    }
    os << QString("================== country synonyms ") + QString::number(m->ctrySynList.size()) + "========================\n";
    for (MultList < CountrySynonym  >::iterator i = m->ctrySynList.begin(); i != m->ctrySynList.end(); i++)
    {
       QString temp1 = i->wt->synPrefix;
       QSharedPointer<CountryEntry> country = i->wt->country;
       QString temp2 = country->basePrefix;
       os << (temp1 + " : " + temp2) << "\n";
    }
    os << "================== district entries ========================\n";
    for (MultList < DistrictEntry >::iterator i = m->distList.begin(); i != m->distList.end(); i++)
    {
       os << i->wt->districtCode << "\n";
    }
    os << "================== district synonyms ========================\n";
    for (MultList < DistrictSynonym >::iterator i = m->distSynList.begin(); i != m->distSynList.end(); i++)
    {
       os << i->wt->synonym + " : " + (i->wt->district)->districtCode  + "\n";
    }
    os << "================== Glist ========================\n";
    for (MultList < GlistEntry >::iterator i = m->glist.begin(); i != m->glist.end(); i++)
    {
       os << i->wt->synPrefix + " : " + i->wt->dupPrefix + "\n";
    }
    */
}

QTEST_MAIN(RPCTestTest)

#include "tst_rpctesttest.moc"
