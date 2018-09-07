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
#define IO_BUF_SIZE 4096
int MinosTestImport::readTestFile(QSharedPointer<QFile> ctfile )
{
    // read the stream as a sequence of Minos stanzas
    QByteArray rdcbuffer;
    bool fileComplete = false;

    // NB - old versions might not have a proper header
    bool firstRead = true;
    QString buffer;
    while ( !fileComplete )
    {
        rdcbuffer = ctfile->read( IO_BUF_SIZE);
        if ( rdcbuffer.size() > 0 )
        {
            QString rdbuffer(rdcbuffer);

            buffer += rdbuffer;
            if ( firstRead )
            {
                firstRead = false;
                iqOffset = 0;
                if ( strnicmp( rdbuffer, header1, header1.length() ) == 0 )
                {
                    buffer = buffer.mid( header1.length(), buffer.length() - header1.length() );  // overwriting what is there already
                    iqOffset = header1.length();
                }
                else
                    if ( strnicmp( rdbuffer, header2, header2.length() ) == 0 )
                    {
                        buffer = buffer.mid( header2.length(), buffer.length() - header2.length() );  // overwriting what is there already
                        iqOffset = header2.length();
                    }
                buffer = QString( stubHeader ) + buffer;
            }
        }
        else
        {
            fileComplete = true;
        }
    }
    // and now we parse the document, and grind through the root
    buffer += "</stream:stream>";

    TiXmlBase::SetCondenseWhiteSpace( false );
    TiXmlDocument xdoc;
    TIXML_STRING sb = buffer.toStdString();
    xdoc.Parse( sb.c_str(), nullptr );
    TiXmlElement *tix = xdoc.RootElement();
    if ( !tix || !checkElementName( tix, "stream:stream" ) )
    {
        return 0;
    }
    int stanzas = 0;
    for ( TiXmlElement * e = tix->FirstChildElement(); e; e = e->NextSiblingElement() )
    {
        stanzas++;

        // Tiny gives us a "cursor" of row/column on the parsed data, but this
        // doesn't actually translate at all easily to a file position

        // I've added the code to get a data offset (DataPos)

        // as we get a position of the START rather than the END of a stanza,
        // we don't always get a good length, so we may need to do a "large" read in thos cases

        curfpos = iqOffset + e->DataPos() - 1 - stubHeader.length();
        analyseNode( this, e );
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

