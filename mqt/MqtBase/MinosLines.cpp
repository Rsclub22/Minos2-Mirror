#include "base_pch.h"
#include "MinosLines.h"

#define MAX_LINE_SIZE 4096
// Windows version  re-implemented using QSharedMemory

// It is used to provide a direct (not through server comms) link
// between the control app and the keyer

void ( *LineSet::lsLog ) ( const QString & ) = nullptr;
static void Log( const QString &msg )
{
   if ( LineSet::lsLog )
   {
      ( *LineSet::lsLog ) ( msg );
   }
}
class LineFileMapper
{
   private:
      char *lpPtr;
   public:
      QSharedMemory hFile;
      QMap < QString, int > LineMap;

      LineFileMapper();
      ~LineFileMapper();
      bool Initialise();
      void Close();
      char *GetMap()
      {
         return static_cast<char *>(hFile.data());
      }
};
class LineFileBlock
{
private:
    LineFileMapper *lfm;
public:
      LineFileBlock(LineFileMapper *);
      ~LineFileBlock();
};

LineFileBlock::LineFileBlock(LineFileMapper *l):lfm(l)
{
    lfm->hFile.lock();
}
LineFileBlock::~LineFileBlock()
{
    lfm->hFile.unlock();
}
//===================================================================
LineFileMapper::LineFileMapper() :
    lpPtr( nullptr ),
    hFile( "LineMapFile" )
{
    hFile.attach();
}
LineFileMapper::~LineFileMapper()
{
   Close();
}
void LineFileMapper::Close()
{
   if ( lpPtr )
   {
      hFile.detach();
   }
   lpPtr = nullptr;
}

bool LineFileMapper::Initialise()
{
    {
       LineFileBlock cfb(this);

       hFile.create(MAX_LINE_SIZE);
    }
   return true;
}
//---------------------------------------------------------------------------
LineSet::LineSet() : FileMap( new LineFileMapper )
{
}
LineSet *LineSet::myLineSet = nullptr;
LineSet *LineSet::GetLineSet()
{
   if ( !myLineSet )
   {
      myLineSet = new LineSet( );
      myLineSet->initialise();
   }

   return myLineSet;
}
bool LineSet::initialise()
{
   bool ret = FileMap->Initialise();
   if ( ret )
   {
       writeSet();
      connect(&checkTimer, SIGNAL(timeout()), this, SLOT(checkTimerTimeout()));
      checkTimer.start(5);
      return readSet();
   }
   return false;
}
LineSet::~LineSet()
{
   delete FileMap;
}
void LineSet::writeSet()
{
   QString cxml;
   //  cxml += "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
   cxml += "<Lines>";
   for ( QMap < QString, bool > ::iterator i = LineMap.begin(); i != LineMap.end(); i++ )
   {
      QString name = i.key();
      QString val = ( i.value() ? "1" : "0" );

      cxml += QString( "<" ) + name + " state=\"" + val + "\"/>";
   }
   cxml += "</Lines>\0\0";
   // modify memory mapped file
   char linebuff[ MAX_LINE_SIZE ];
   memset( linebuff, 0, MAX_LINE_SIZE );
   std::string scxml = cxml.toStdString();      // allowed conversion through std::string
   memcpy( linebuff, scxml.c_str(), scxml.size() ); //include zero termination
   linebuff[ scxml.size() ] = 0;
   memcpy( FileMap->GetMap(), linebuff, MAX_LINE_SIZE ); //include zero termination
   Log( QString( "writeSet (publish)  " ) + FileMap->GetMap() );
}
bool LineSet::readSet()
{
   TiXmlBase::SetCondenseWhiteSpace( false );
   TiXmlDocument linedoc;
   linedoc.Parse( FileMap->GetMap() );
   TiXmlElement *root = linedoc.RootElement();
   if ( !root )
   {
      //err = "No XML root element (does file exist?)";
      return false;
   }
   QString rootName = root->Value();
   if ( !( rootName == "Lines" ) )
   {
      Log( "readSet Invalid XML root element " + rootName );
      return false;
   }
   for ( TiXmlElement * e = root->FirstChildElement(); e; e = e->NextSiblingElement() )
   {
      QString name = e->Value();
      const char * a = e->Attribute( "state" );
      if ( a[ 0 ] == '1' )
      {
         LineMap[ name ] = true;
      }
      else
      {
         LineMap[ name ] = false;
      }
   }
   return true;
}

void LineSet::publish( const QString &name, bool state )
{
   LineFileBlock guard(FileMap);
   readSet();
   Log( QString( "readSet  (publish)  " ) + FileMap->GetMap() );
   LineMap[ name ] = state;
   writeSet();
}
void LineSet::readLines()
{
   LineFileBlock guard(FileMap);
   readSet();
   Log( QString( "readSet (readLines) " ) + FileMap->GetMap() );

}
bool LineSet::getState( const QString name )
{
   for ( QMap <QString, bool>::iterator i = LineMap.begin(); i != LineMap.end(); i++ )
   {
      if ( name == i.key() )
      {
         return i.value();
      }
   }

   return false;
}
void LineSet::checkTimerTimeout()
{
    QMap <QString, bool> oldMap = LineMap;
    readSet();
    for ( QMap <QString, bool>::iterator i = LineMap.begin(); i != LineMap.end(); i++ )
    {
       if ( oldMap[i.key()] != i.value() )
       {
           Log( QString( "checkTimerTimeout emitting lineChanged " ) + FileMap->GetMap() );
            emit linesChanged();
            return;
       }
    }
}

//---------------------------------------------------------------------------
