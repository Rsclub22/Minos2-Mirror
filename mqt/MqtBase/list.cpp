/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "cutils.h"
#include "contacts.h"
#include "ListContact.h"
#include "list.h"

CsvReader::CsvReader(){}

bool CsvReader::parseCsv(const QString &fileName, QList<QStringList> &csv)
{
    QFile file (fileName);
     if (file.open(QIODevice::ReadOnly))
     {
         QString data = file.readAll();
         data.remove( QRegExp("\r") ); //remove all ocurrences of CR (Carriage Return)
         QString temp;
         QChar character;
         QTextStream textStream(&data);
         while (!textStream.atEnd())
         {
             textStream >> character;
             if (character == ',')
             {
                 checkString(temp, character, csv);
             }
             else if (character == '\n')
             {
                 checkString(temp, character, csv);
             }
             else if (textStream.atEnd())
             {
                 temp.append(character);
                 checkString(temp, 0, csv);
             }
             else
             {
                 temp.append(character);
             }
         }
         itemList.clear();
         return true;
     }
     return false;
}
void CsvReader::checkString(QString &temp, QChar character, QList<QStringList> &csv)
{
    if(temp.count("\"")%2 == 0)
    {
        if (temp.startsWith( QChar('\"')) && temp.endsWith( QChar('\"') ) )
        {
             temp.remove( QRegExp("^\"") );
             temp.remove( QRegExp("\"$") );
        }
        temp.replace("\"\"", "\"");
        itemList.append(temp.trimmed());
        if (character != QChar(','))
        {
            csv.append(itemList);
            itemList.clear();
        }
        temp.clear();
    }
    else
    {
        temp.append(character);
    }
}


ContactList::ContactList() :
  cslFile( false )
  , slotno( -1 )
  , errMessShown(false)
{
}
ContactList::~ContactList()
{
    for ( ListIterator i = ctList.begin(); i != ctList.end(); i++ )
       delete ( *i );
}
bool ContactList::initialise( int sno )
{
   if ( !MinosParameters::getMinosParameters() ->insertList( this, sno ) )
   {
      return false;
   }
   slotno = sno;
   return true;
}

bool ContactList::initialise(const QString &fn, int slotno )
{

   if ( !initialise( slotno ) )
      return false;
   if ( MinosParameters::getMinosParameters() ->isListOpen( fn ) )
      return false;

   // open the List file

   cfileName = fn;
   QString ext = ExtractFileExt( fn );

   if ( ext.compare( ".csl", Qt::CaseInsensitive ) == 0 )
   {
      cslFile = true;
   }
   else
   {
      return false;
   }

   bool loadOK = false;

   if ( cslFile )
   {
      if ( !cslLoad() )     // load the header so that we can display it
         return false;
      loadOK = cslLoadContacts();
   }

   return loadOK;
}
bool ContactList::cslLoad( )
{
   // read data from file
   CsvReader csv;
   QList<QStringList> readData;
   if (csv.parseCsv(cfileName, readData))
   {
       QString fn = ExtractFileName( cfileName );
       name = fn;

       for ( int i = 0; i < readData.size(); ++i )
       {
           const QStringList &parts = readData.at(i);

           if (parts.size() == 0 || parts[0].size() == 0 ||  !(parts[0][0].isLetter() || parts[0][0].isNumber()))
               continue;

           if ( i == 0 && parts.size() > 2 && parts[0].size() == 0 && parts[1].size() == 0 )
           {
               name = parts[ 2 ];              // first line of file gives the list name
           }
           else
           {
              ListContact *rct = new ListContact();

              // a1, a2, a3 will all be set - but may point to null terminator!

              if (parts.size() > 0)
              {
                  rct->cs.fullCall.setValue( parts[0].toUpper() );
                  rct->cs.validate();
              }

              if (parts.size() > 1)
              {
                  rct->loc.loc.setValue( parts[ 1 ].toUpper() );
                  rct->loc.valRes = LOC_NOT_VALIDATED;
              }

              if (parts.size() > 2)
                  rct->extraText = parts[ 2 ];
              if (parts.size() > 3)
                  rct->comments = parts[ 3 ];

              ctList.push_back( rct );
           }
       }
       return true;
   }
   return false;
}
bool ContactList::cslLoadContacts( )
{
   return true;
}
void ContactList::getMatchText(ListContact *, QString &disp, const BaseContestLog *const /*ct*/ ) const
{
   disp = "N/A";
}
void ContactList::getMatchField(ListContact *pct, int col, QString &disp, const BaseContestLog *const ct ) const
{
   QString temp;
   if ( pct )
      temp = pct->getField( col, ct );

   disp = temp.trimmed();
}
ListContact *ContactList::pcontactAt( int i )
{
   if ( i < ctList.size() )
   {
      return ctList.at( i );
   }
   return nullptr;
}

