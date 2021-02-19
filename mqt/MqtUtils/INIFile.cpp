/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "mqtUtils_pch.h"
#include "fileutils.h"
#include "MLogFile.h"
#include "INIFile.h"
#include "MShowMessageDlg.h"

class INIFile;
class INISection;
class INIEntry //: public NameChain
{
private:
    QString entryValue;
    bool entryDirty;
    bool entryValid;

public:
    QString name;

    INIEntry( INISection *cb,
              const QString &name, bool Valid );  // Constructor.

    ~INIEntry();        // Destructor.

    QString getValue( );
    void setValue( const QString &Value );
    bool isValidEntry( );
    bool isDirty()
    {
        return entryDirty;
    }
    void setClean()
    {
        entryDirty = false;
    }

};
typedef INIEntry *IniEntryPtr;
struct INIEntryCmp
{
    QString cmpstr;
    INIEntryCmp( const QString &s ) : cmpstr( s )
    {}

    bool operator() ( IniEntryPtr &s1 ) const
    {
        return s1->name.compare(cmpstr, Qt::CaseInsensitive ) == 0;
    }
};
class INISection
{
private:
    bool entryValid;
    bool sectDirty;
public:
    QString name;

    INISection( INIFile *cb,
                const QString &name, bool Valid );

    ~INISection();   // Destructor.

    bool isValidSection( );

    QVector <IniEntryPtr> entries;

    bool isDirty();
    void setClean();

};
struct INISectionCmp
{
    QString cmpstr;
    INISectionCmp( const QString &s ) : cmpstr( s )
    {}

    bool operator() ( IniSectionPtr &s1 ) const
    {
        return s1->name.compare(cmpstr, Qt::CaseInsensitive ) == 0;
    }
};
//==============================================================================
INISection::INISection( INIFile *cb, const QString &name, bool valid )
    :
    entryValid( valid )
    , sectDirty( false )
    , name( name.trimmed() )
{
    cb->sections.push_back( this );
}

INISection::~INISection()
{
    // delete all entries
    for ( auto const &this_ent: entries)
    {
        delete this_ent ;
    }
    entries.clear();
}

bool INISection::isValidSection( )
{
    return entryValid;
}
bool INISection::isDirty( )
{
    if ( sectDirty )
        return true;

    for ( auto const &thisEntry: entries)
    {
        if ( thisEntry->isDirty() )
            return true;
    }
    return false;   // for now, just in case...
}
void INISection::setClean( )
{
    //walk all sections and set each clean
    for ( auto const &thisEntry: entries)
    {
        thisEntry->setClean();
    }
}
//==============================================================================


INIEntry::INIEntry( INISection *cb, const QString &name, bool valid )
    : entryDirty( false )
    , entryValid( valid )
    , name( name.trimmed() )
{
    cb->entries.push_back( this );
}

INIEntry::~INIEntry()
{}

QString INIEntry::getValue( )
{
    return entryValue;
}

void INIEntry::setValue( const QString &value )
{
    if ( value != entryValue )
    {
        entryValue = value.trimmed();
        entryDirty = true;
    }
}

bool INIEntry::isValidEntry( )
{
    return entryValid;
}
//==============================================================================

INIFile::INIFile( const QString &name ) :
    fileDirty( false )
    , fileLoaded( false )
{
    loadedFileName = name.trimmed() ;

    QFileInfo checkFile( loadedFileName );
    if ( !checkFile.exists() || !checkFile.isFile() )
    {
        CreateDir( ExtractFilePath( loadedFileName ) );
        QFile fh(loadedFileName);
        bool ret = fh.open(QIODevice::ReadWrite);
        if ( ret )
            fh.close();

        checkFile.refresh();
    }
    loadedFileName = checkFile.canonicalFilePath();
}
INIFile::~INIFile()
{
    writePrivateProfileString( "", "", "" );
    // delete all sections
    for ( auto const &thisSect: sections )
    {
        delete thisSect;
    }
    sections.clear();
    fileLoaded = false;
}
bool INIFile::dupSection( const QString &oldname, const QString &newname )
{
    QVector<IniSectionPtr>::iterator newSect = std::find_if( sections.begin(), sections.end(), INISectionCmp( newname ) );
    if ( newSect == sections.end() )
    {
        QVector<IniSectionPtr>::iterator thisSect = std::find_if( sections.begin(), sections.end(), INISectionCmp( oldname ) );
        if ( thisSect != sections.end() )
        {
            INISection * oldsect = ( *thisSect );
            INISection *newsect = new INISection( this, newname, true );
            for ( auto const &this_ent: oldsect->entries )
            {
                INIEntry *i = this_ent;
                QString n = i->name;
                QString v = i->getValue();
                INIEntry *newent = new INIEntry( newsect, n, true );
                newent->setValue( v );
            }
            return true;
        }
        return false;
    }
    return false;  // section already exists
}

bool INIFile::checkStat( )
{
    QFileInfo tempstat(loadedFileName);
    if (
        #if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            statbuf.created() != tempstat.created()
        #else
            statbuf.birthTime() != tempstat.birthTime()
        #endif
            || statbuf.lastModified() != tempstat.lastModified()
            || statbuf.size() != tempstat.size()
            )
    {
        statbuf = tempstat;
        return true;
    }
    return false;   // no change
}
bool INIFile::isDirty( )
{
    if ( fileDirty )
        return true;

    for ( auto const &thisSect: sections )
    {
        if ( thisSect->isDirty() )
            return true;
    }
    return false;   // for now, just in case...
}
void INIFile::setClean( )
{
    //walk all sections and set each clean
    for ( auto const &thisSect: sections )
    {
        thisSect->setClean();
    }
}

bool INIFile::writeINIFile()
{
    if ( !isDirty() )
        return true;

    QFile inf(loadedFileName);

    // here we need to stat the file to see if it has changed
    // - but what do we do if it has? We should have loaded
    // it VERY recently

    if (!inf.open(QIODevice::WriteOnly|QIODevice::Text))
        return false;

    QTextStream out(&inf);

    for ( auto const &thisSect: sections )
    {
        const QString sname = thisSect->name;

        if ( thisSect->isValidSection() )
        {
            QString s = QString("[%1]\n").arg(sname);
            out << s;
        }

        for ( auto const &this_entry: thisSect ->entries )
        {
            const QString name = this_entry->name;
            const QString val = this_entry->getValue();
            if ( this_entry->isValidEntry() )
            {
                QString s = QString("%1=%2\n").arg(name).arg(val);
                out << s;
            }
            else
            {
                QString s = QString("%1\n").arg(val);
                out << s;
            }
        }
    }
    inf.close();

    setClean();
    // now stat the file so we can check for changes
    checkStat();
    return true;
}

void INIFile::startGroup()
{
    inGroup = true;
    loadINIFile();
}
void INIFile::endGroup()
{
    inGroup = false;
}
void INIFile::clear()
{
    sections.clear();
    fileDirty = true;
}
void INIFile::loadINIFile()
{
    INISection *thisSect;
    IniEntryPtr this_entry;

    if (fileLoaded && inGroup)
        return;

    if ( fileLoaded )
    {
        if ( !checkStat() )
            return;			// no change, so don't re-read
//        writePrivateProfileString( "", "", "" );    // this could overwrite anyone elses changes if we are dirty
//                                                    // if we aren't dirty, it doesn't do anything
        for ( auto const &thisSect: sections )
        {
            delete thisSect;
        }
        sections.clear();
        fileLoaded = false;
    }

    fileLoaded = true;

    {   // scoping
        QFile lf(loadedFileName);

        // here we need to stat the file to see if it has changed
        // - but what do we do if it has? We should have loaded
        // it VERY recently

        if (!lf.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            mShowMessage( QString( "Initialisation file \"" ) + loadedFileName + "\" not found.", nullptr );
            return;
        }
        QTextStream inf(&lf);

        thisSect = new INISection( this, "?Comments", false );
        // create dummy section for leading comments

        while (!inf.atEnd())
        {
            QString buffer = inf.readLine( 256 );

            QStringList p1 = buffer.split('[');
            if (p1.length() > 1)
            {
                QStringList p2 = p1[1].split(']');
                if (p2.length())
                {
                    QString Parameter = p2[0];
                    thisSect = new INISection( this, Parameter, true );
                    continue;
                }
            }

            QStringList a = buffer.split('=');
            int scnt = a.size();

            if ( scnt == 2  )
            {
                this_entry = new INIEntry( thisSect, a[ 0 ], true );
                // somewhere we need to cope with quoted parameters
                this_entry->setValue( a[ 1 ] );
                this_entry->setClean();
            }
            else
            {
                // create comment entry
                this_entry = new INIEntry( thisSect, "??", false );
                this_entry->setValue( a[ 0 ] );
                this_entry->setClean();
            }
        }
    }
    setClean();
    // now stat the file so we can check for changes
    checkStat();

    return ;
}

bool INIFile::checkKeyExists( const QString &Section,
                              const QString &Entry
                              )
{
    loadINIFile();

    QVector<IniSectionPtr>::iterator thisSect = std::find_if( sections.begin(), sections.end(), INISectionCmp( Section ) );
    if ( thisSect != sections.end() )
    {
        if ( Entry.isEmpty()  )
            return true;
        QVector<IniEntryPtr>::iterator this_entry = std::find_if( ( *thisSect ) ->entries.begin(), ( *thisSect ) ->entries.end(), INIEntryCmp( Entry ) );
        if ( this_entry != ( *thisSect ) ->entries.end() )
        {
            return true;
        }
    }
    return false;
}

int INIFile::getPrivateProfileList( const QString &Section,
                                    const QString &Entry,
                                    QStringList &Buffer )
{
    loadINIFile();

    QVector<IniSectionPtr>::iterator thisSect = std::find_if( sections.begin(), sections.end(), INISectionCmp( Section ) );
    if ( thisSect != sections.end() )
    {
        if ( Entry.isEmpty() )
        {
            /* build list of entry names in buffer */
            for ( auto const &this_entry: (*thisSect)->entries )
            {
                if ( this_entry->isValidEntry() )
                {
                    Buffer.append( this_entry->name);
                }
            }
        }
    }
    return Buffer.count();
}
int INIFile::getPrivateProfileString( const QString &Section,
                                      const QString &Entry,
                                      const QString &DefaultValue,
                                      QString &Buffer )
{
    loadINIFile();

    QVector<IniSectionPtr>::iterator thisSect = std::find_if( sections.begin(), sections.end(), INISectionCmp( Section ) );
    if ( thisSect == sections.end() )
    {
        Buffer = DefaultValue;
    }
    else
    {
        if ( !Entry.isEmpty() )
        {
            QVector<IniEntryPtr>::iterator this_entry = std::find_if( ( *thisSect ) ->entries.begin(), ( *thisSect ) ->entries.end(), INIEntryCmp( Entry ) );
            if ( this_entry == ( *thisSect ) ->entries.end() )
            {
                Buffer = DefaultValue;
            }
            else
            {
                Buffer = ( *this_entry ) ->getValue();
            }
        }
    }
    return Buffer.size();
}


int INIFile::getPrivateProfileInt(const QString &Section,
                                  const QString &Entry,
                                  int DefaultValue )
{
    if ( Section.isEmpty()|| Entry.isEmpty() )
        return 0;

    loadINIFile();

    QVector<IniSectionPtr>::iterator thisSect = std::find_if( sections.begin(), sections.end(), INISectionCmp( Section ) );
    if ( thisSect == sections.end() )
    {
        return DefaultValue;
    }

    QVector<IniEntryPtr>::iterator this_entry = std::find_if( ( *thisSect ) ->entries.begin(), ( *thisSect ) ->entries.end(), INIEntryCmp( Entry ) );
    if ( this_entry == ( *thisSect ) ->entries.end() )
    {
        return DefaultValue;
    }

    return ( *this_entry ) ->getValue().toInt();
}

bool INIFile::getPrivateProfileBool(const QString &Section,
                                  const QString &Entry,
                                  bool DefaultValue )
{
    QString val;
    if (getPrivateProfileString(Section, Entry, DefaultValue?"true":"false", val))
        return val.compare("true", Qt::CaseInsensitive) == 0;

    return false;
}


bool INIFile::writePrivateProfileString(const QString &Section,
                                        const QString &Entry,
                                        const QString &Buffer )
{
    /*
   lpAppName

   Points to a null-terminated string containing the name of the section to which
   the string will be copied. If the section does not exist, it is created. The name
   of the section is case-independent; the string can be any combination of uppercase
    and lowercase letters.

   lpKeyName

   Points to the null-terminated string containing the name of the key to be associated
    with a string. If the key does not exist in the specified section, it is created.
    If this parameter is 0, the entire section, including all entries within the section, is deleted.

   lpString

   Points to a null-terminated string to be written to the file. If this parameter is 0,
   the key pointed to by the lpKeyName parameter is deleted.
   Windows 95: This platform does not support the use of the TAB (\t) character as part of this parameter.

   lpFileName

   Points to a null-terminated string that names the initialization file.


   */
    if ( ( Section.isEmpty() ) && ( Entry.isEmpty() ) && ( Buffer.isEmpty() ) )
    {
        /*    Windows 95 keeps a cached version of WIN.INI to improve performance.
          If all three parameters are 0, the function flushes the cache. The
            function always returns FALSE after flushing the cache, regardless of
            whether the flush succeeds or fails.
      */
        writeINIFile();
        return false;
    }

    loadINIFile();

    QVector<IniSectionPtr>::iterator thisSect = std::find_if( sections.begin(), sections.end(), INISectionCmp( Section ) );
    if ( Entry.isEmpty())
    {
        if ( thisSect != sections.end() )
        {
            for ( auto const &this_ent: (*thisSect)->entries )
            {
                delete this_ent;
            }
            ( *thisSect ) ->entries.clear();

            delete ( *thisSect );
            sections.erase( thisSect );
            fileDirty = true;
        }
    }
    else
    {
        if ( thisSect == sections.end() )
        {
            /* create new section */
            new INISection( this, Section, true );
            thisSect = std::find_if( sections.begin(), sections.end(), INISectionCmp( Section ) );
            fileDirty = true;
        }
        QVector<IniEntryPtr>::iterator this_entry = std::find_if( ( *thisSect ) ->entries.begin(), ( *thisSect ) ->entries.end(), INIEntryCmp( Entry ) );
/*
    // buffer empty (was originally a char *x == 0) not useful
        if ( Buffer.isEmpty() )
        {
            if ( this_entry != ( *thisSect ) ->entries.end() )
            {
                delete ( *this_entry );
                ( *thisSect ) ->entries.erase( this_entry );
                fileDirty = true;
            }
        }
        else
        */
        {
            if ( this_entry == ( *thisSect ) ->entries.end() )
            {
                /* create new entry */
                new INIEntry( ( *thisSect ), Entry, true );
                this_entry = std::find_if( ( *thisSect ) ->entries.begin(), ( *thisSect ) ->entries.end(), INIEntryCmp( Entry ) );
                fileDirty = true;
            }
            ( *this_entry ) ->setValue( Buffer );
        }
    }
    writeINIFile();

    return true;
}
bool INIFile::writePrivateProfileBool( const QString &Section,
                                const QString &Entry,
                                bool val )
{
    return writePrivateProfileString(Section, Entry, val?"true":"false");
}

bool INIFile::isSectionPresent(QString sname)
{
    loadINIFile();
    QVector<IniSectionPtr>::iterator thisSect = std::find_if( sections.begin(), sections.end(), INISectionCmp( sname ) );
    if ( thisSect == sections.end() )
    {
        return false;
    }

    if ( ( *thisSect ) ->entries.size() == 0)
    {
        return false;
    }
    return true;
}
QStringList INIFile::getSections( )
{
    loadINIFile();
    QStringList slist;
    for ( auto const &thisSect: sections )
    {
        if ( thisSect->isValidSection() )
        {
            slist.append( thisSect->name );
        }
    }

    return slist;
}
