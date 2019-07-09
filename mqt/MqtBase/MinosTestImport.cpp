/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "TinyUtils.h"
#include "cutils.h"
#include "contest.h"
#include "MinosTestImport.h"

MinosTestImport::MinosTestImport( BaseContestLog * const ct ) : ct( ct )
  , curfpos( 0 )
  , imp_stanzaCount( ct->getCtStanzaCount() )
{}
MinosTestImport::MinosTestImport( ) : ct( nullptr )
  , curfpos( 0 )
  , imp_stanzaCount( 0 )
{}
MinosTestImport::~MinosTestImport()
{}
bool MinosTestImport::getStructArgMemberValueDTG( QSharedPointer<RPCParam>rpm, const QString &name, QString &val )
{
   QSharedPointer<RPCParam> res;
   if ( rpm && rpm->getMember( name, res ) )
   {
      QString psval;
      if ( res->getDtg( psval ) )
      {
         val = psval;
         return true;
      }
   }
   return false;
}
bool MinosTestImport::getStructArgMemberValueDTG( QSharedPointer<RPCParam>rpm, const QString &name, MinosStringItem<QString> &val )
{
   QString temp;
   bool ret = getStructArgMemberValueDTG( rpm, name, temp );
   if ( ret )
   {
      val.setInitialValue( temp );
   }
   return ret;
}
bool MinosTestImport::getStructArgMemberValue( QSharedPointer<RPCParam>rpm, const QString &name, QString &val )
{
   QSharedPointer<RPCParam> res;
   if ( rpm && rpm->getMember( name, res ) )
   {
      QString psval;
      if ( res->getString( psval ) )
      {
         val = psval;
         return true;
      }
   }
   return false;
}
bool MinosTestImport::getStructArgMemberValue( QSharedPointer<RPCParam>rpm, const QString &name, MinosStringItem<QString> &val )
{
   QString temp;
   bool ret = getStructArgMemberValue( rpm, name, temp );
   if ( ret )
   {
      val.setInitialValue( temp );
   }
   return ret;
}
bool MinosTestImport::getStructArgMemberValue( QSharedPointer<RPCParam>rpm, const QString &name, int &val )
{
   QSharedPointer<RPCParam> res;
   if ( rpm && rpm->getMember( name, res ) )
   {
      int psval;
      if ( res->getInt( psval ) )
      {
         val = psval;
         return true;
      }
   }
   return false;
}
bool MinosTestImport::getStructArgMemberValue( QSharedPointer<RPCParam>rpm, const QString &name, MinosItem<int> &val )
{
   int temp;
   bool ret = getStructArgMemberValue( rpm, name, temp );
   if ( ret )
   {
      val.setInitialValue( temp );
   }
   return ret;
}
bool MinosTestImport::getStructArgMemberValue( QSharedPointer<RPCParam>rpm, const QString &name, bool &val )
{
   QSharedPointer<RPCParam> res;
   if ( rpm && rpm->getMember( name, res ) )
   {
      bool psval;
      if ( res->getBoolean( psval ) )
      {
         val = psval;
         return true;
      }
   }
   return false;
}
bool MinosTestImport::getStructArgMemberValue( QSharedPointer<RPCParam> rpm, const QString &name, MinosItem<bool> &val )
{
   bool temp;
   bool ret = getStructArgMemberValue( rpm, name, temp );
   if ( ret )
   {
      val.setInitialValue( temp );
   }
   return ret;
}
bool MinosTestImport::getStructArgMemberValueDTG( const QString &name, QString &val )
{
   return getStructArgMemberValueDTG( body, name, val );
}
bool MinosTestImport::getStructArgMemberValueDTG( const QString &name, MinosStringItem<QString> &val )
{
   return getStructArgMemberValueDTG( body, name, val );
}
bool MinosTestImport::getStructArgMemberValue( const QString &name, QString &val )
{
   return getStructArgMemberValue( body, name, val );
}
bool MinosTestImport::getStructArgMemberValue( const QString &name, MinosStringItem<QString> &val )
{
   return getStructArgMemberValue( body, name, val );
}
bool MinosTestImport::getStructArgMemberValue( const QString &name, int &val )
{
   return getStructArgMemberValue( body, name, val );
}
bool MinosTestImport::getStructArgMemberValue( const QString &name, MinosItem<int> &val )
{
   return getStructArgMemberValue( body, name, val );
}
bool MinosTestImport::getStructArgMemberValue( const QString &name, bool &val )
{
   return getStructArgMemberValue( body, name, val );
}
bool MinosTestImport::getStructArgMemberValue( const QString &name, MinosItem<bool> &val )
{
   return getStructArgMemberValue( body, name, val );
}
void MinosTestImport::setBit( unsigned short &cf, unsigned short bit, bool set
                               )
{
   cf &= ~bit;
   if ( set
         )
      {
         cf |= bit;
      }
}
void MinosTestImport::processMinosStanza( RPCRequest *rq )
{

   // This needs to be "inverted" and passed to the contest object to get the bits it
   // understands

   imp_stanzaCount++;

   //   fpos is a COUNT not a position - so why do we end up 1 short?

   body = rq->args[ 0 ];

   ct->setStanza( static_cast<unsigned int>(imp_stanzaCount), curfpos );
   ct->processMinosStanza( rq->methodName, this );

}

//==============================================================================
// These are used for log monitoring
void MinosTestImport::startImportTest()
{}
int MinosTestImport::importTestBuffer( const QString &buffer )
{
   // called from monitoredLog when we get a "random" stanza in

   DispatchCallback oldcall = dispatchCallback;
   dispatchCallback = &MinosTestImport::analyseImportTest;

   analyseNode( this, buffer.toStdString() );

   dispatchCallback = oldcall;

   return imp_stanzaCount;
}
void MinosTestImport::endImportTest()
{}
//==============================================================================
const QString header1 = "<?xml version='1.0'?><stream:stream xmlns:stream='http://minos.goodey.org.uk/streams' xmlns='minos:client' version='1.0'>";
const QString header2 = "<?xml version='1.0'?><stream:stream xmlns:stream='http://etherx.jabber.org/streams' xmlns='jabber:client' version='1.0'>";
const QString stubHeader = "<stream:stream>";

void MinosTestImport::dispatchResponse( XStanza *xs )
{
   ( this->*dispatchCallback ) ( xs ); // What a horrid syntax for calling through a member pointer!
}
#define IO_BUF_SIZE 64000
int MinosTestImport::readTestFile(QSharedPointer<QFile> ctfile )
{
    // read the stream as a sequence of Minos stanzas

    int stanzas = 0;

    // First stanza is the first "<iq", missing the (header1)<xml and comment (fileHeader form MinosTestExport)
    // curfpos is the fileoffset to be able to read it later

    curfpos = 1;    // to get us started

    startImportTest();

    QString buffer;
    int bufend = 0;

    int bufOffset = 0;

    QByteArray rdcbuffer;
    bool fileComplete = false;

    // NB - old versions might not have a proper header

    while ( !fileComplete )
    {
        rdcbuffer = ctfile->read( IO_BUF_SIZE);
        if ( rdcbuffer.size() > 0 )
        {
            QString rdbuffer(rdcbuffer);

            try {
            buffer += rdbuffer;
            }
            catch(std::exception &a)
            {
                int s = buffer.count();
                const char * aa = a.what();
               trace("broken");
            }
            catch(...)
            {
                int s = buffer.count();
               trace("broken");
            }
            int curfstart = buffer.indexOf("<iq", 0);
            int curfend = buffer.indexOf("</iq>", 0);

            while (curfstart >= 0 && curfend >= 0)
            {
                curfpos = curfstart + bufOffset;
                QString iqbuff = buffer.mid(curfstart, curfend - curfstart + 4 + 1);
                stanzas = importTestBuffer(iqbuff);
                curfpos = curfend + 4 + 1;

                buffer = buffer.right(buffer.size() - (curfend + 4 + 1));
                bufOffset += curfend + 4 + 1;
                curfstart = buffer.indexOf("<iq", 0);
                curfend = buffer.indexOf("</iq>", 0);
            }
        }
        else
        {
            fileComplete = true;
        }
    }

    return stanzas;
}
//=============================================================================
int MinosTestImport::importTest(QSharedPointer<QFile> ctfile )
{
   imp_stanzaCount = 0;

   dispatchCallback = &MinosTestImport::analyseImportTest;

   if ( readTestFile( ctfile ) )
      return imp_stanzaCount;

   return 0;
}
void MinosTestImport::analyseImportTest( XStanza *xs )
{
   RPCRequest * req = dynamic_cast<RPCRequest *>( xs );
   if ( req )
   {
      processMinosStanza( req );
   }
}
//=============================================================================
bool MinosTestImport::analyseTest(QSharedPointer<QFile> ctfile )
{
   dispatchCallback = &MinosTestImport::analyseImportLog;
   return readTestFile( ctfile );
}
void MinosTestImport::analyseImportLog( XStanza *xs )
{
   RPCRequest * req = dynamic_cast<RPCRequest *>( xs );
   if ( req )
   {
      processLogMinosStanza( req );
   }
}
void MinosTestImport::processLogMinosStanza( RPCRequest *rq )
{
   // We need to go through and log the tags and values in this node

   QString log = rq->analyse();

//   #warning here is where we need to analyse the sequence number - or should we do it in the analyse with a callback
   // to here with the name?
   trace( rq->methodName + " : " + log );
}
//==============================================================================

