/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "MinosLoggerEvents.h"

#include "cutils.h"

const double pi = 3.141592653 ;  /* pi */
const double dr = pi / 180.0;      // degree to radian conversion factor

char diskBuffer[ bsize + 1 ];
//char *lbuff = &diskBuffer[ 0 ];
int buffpt = 0;

void clearBuffer( )
{
   memset( diskBuffer, '#', bsize );
   diskBuffer[ bsize ] = 0;
}
static const char *noeditstr = "\r\n!!BINARY FILE DO NOT EDIT!!\r\n";
const int noeditlength = 32;

void strtobuf( const QString &str )
{
   buffpt += strcpysp( &diskBuffer[ buffpt ], str, str.length() ) + 1;
}
void strtobuf( const MinosStringItem<QString> &str )
{
   strtobuf( str.getValue() );
}
void strtobuf()
{
   char * s1 = &diskBuffer[ buffpt ];

   // null fill the rest of the buffer
   memset( s1, 0, static_cast<unsigned int>(bsize - buffpt) );
   if ( buffpt + noeditlength < ( bsize - 3 ) )
   {
      memcpy( &diskBuffer[ bsize - noeditlength - 1 ], noeditstr, noeditlength );
   }
   diskBuffer[ bsize - 1 ] = 0;   // force termination
}
void opyn( bool b )
{
   strtobuf( b ? "Y" : "N" );
}
void opyn( const MinosItem<bool> &b )
{
   opyn( b.getValue() );
}

void buftostr( QString &str )
{
   int i;
   str = "";
   int len = static_cast<int>(strlen( &diskBuffer[ buffpt ] ));
   for ( i = 0; i < 1024 && i < len; i++ )
   {
      str += diskBuffer[ buffpt + i ];
   }
   buffpt += len + 1;   // step past null terminator
}
void buftostr( MinosStringItem<QString> &str )
{
   QString temp;
   buftostr( temp );
   str.setInitialValue( temp );
}
bool inyn()
{
   QString temp;
   buftostr( temp );
   if ( temp.size() && temp[ 0 ].toUpper() == 'Y' )
      return true;

   return false;
}
int strcpysp( QString &s1, const QString &s2, int maxlen )
{
    s1 = s2.trimmed().left(maxlen);
    return s1.size();
}
size_t strcpysp( char *s1, const QString &s2, int maxlen )
{
    QString ss2 = s2.trimmed().left(maxlen);

    strcpy(s1, ss2.toLatin1());
    return strlen(s1);
}

//============================================================

int strcmpsp( const QString &s1, const QString &s2 )
{
    QString sp1;
    QString sp2;

   strcpysp( sp1, s1, 255 );
   strcpysp( sp2, s2, 255 );

   return sp1.compare(sp2, Qt::CaseSensitive );
}
//============================================================
int stricmpsp( const QString &s1, const QString &s2 )
{
   QString sp1;
   QString sp2;

   strcpysp( sp1, s1, 255 );
   strcpysp( sp2, s2, 255 );

   return sp1.compare(sp2, Qt::CaseInsensitive );
}
//============================================================
int strcspn(const QString &s, const QString &chars)
{
    for (int i = 0; i < chars.length(); i++)
    {
        int p = s.indexOf(chars[i]);
        if (p >= 0)
        {
            return p;
        }
    }
    return -1;
}

//============================================================

int placestr( QString &buff, const QString &str, int start, int len )
{
   // if len is -ve, it means to R justify

   buff += "     ";
   buff = buff.left(start);
   buff = QString("%1%2                                                                ").arg(buff).arg(str, -len).left(start + abs(len)) ;
   return start + abs(len);
}
//============================================================
int parseLine( QString buff, char sep, QStringList &a, int count, char sep2, bool &sep2seen )
{
    int i = 0;
    int sep_count = 0;
    sep2seen = false;

    int len = buff.length();
    int lastSep = 0;

    for ( int j = 0; j < count; j++ )
    {
        // terminate the previous entry on a '<sep>'
        while ( i < len && buff[ i ] != sep && buff[ i ] != sep2 )
            i++;

        if ( i < len && ( buff[ i ] == sep || ( sep2 != 0 && buff[ i ] == sep2 ) ) )
        {
            if ( buff[ i ] == sep2 )
                sep2seen = true;
            sep_count++;
            QString part = buff.mid(lastSep, i - lastSep).trimmed();
            a.push_back(part);
            i++;
            lastSep = i;
        }
        if (i == len)
        {
            QString part = buff.mid(lastSep, i - lastSep).trimmed();
            a.push_back(part);
            break;
        }
    }
    return sep_count;
}
//============================================================
writer::writer( QSharedPointer<QFile> f ) :  /*lbuff( diskBuffer ),*/ expfd( f )
{}
writer::~writer()
{}
void writer::lwrite( const QString &s )
{
    lwrite(s.toStdString().c_str());
}

void writer::lwrite( const char *b )
{
   QString l = QString( b ) + "\r\n";

   qint64 ret = expfd->write(l.toStdString().c_str(), static_cast<int>(l.toStdString().size()));
   if ( ret != static_cast<int >(l.toStdString().size()) )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( "bad reply from write!" );
   }
}
void writer::lwriteLine()
{
   QString l( 80, 0x5F );					// horizontal line
   lwrite( l );
}
void writer::lwriteNl()
{
    qint64 ret = expfd->write("\r\n", 2);
    if ( ret != 2 )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( "bad reply from write!" );
   }
}
void writer::lwriteFf()
{
   //   ::write( expfd, "\f", 1 );
   qint64 ret = expfd->write("\f", 1);
   if ( ret != 1 )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( "bad reply from write!" );
   }
}
//=============================================================================
QString trimr( const QString &r )
{
    int n = r.size() - 1;
    for (; n >= 0; --n)
    {
        if (!r.at(n).isSpace())
        {
            return r.left(n + 1);
        }
    }
    return "";
}
//=============================================================================
QString makeADIFField( const QString &fieldname, const QString &content )
{
   QString tag;
   int len = content.size();
   if ( len )
   {
      QString buff = QString( "<%1:%2>" ).arg(fieldname).arg(len );
      tag = buff + content;
   }
   return tag;
}
QString makeADIFField( const QString &fieldname, int content )
{
   return makeADIFField( fieldname, QString::number(content) );
}

//=============================================================================

int strnicmp( const QString &s1, const QString &s2, int len )
{
    return s1.left(len).compare(s2.left(len), Qt::CaseInsensitive);
}
//=============================================================================
QDateTime CanonicalToTDT(QString cdtg )
{
   QDateTime d;
   d = QDateTime::fromString(cdtg, "yyyyMMddHHmm" );
   d.setTimeSpec(Qt::UTC);
   return d;

}
QString TDTToCanonical(QString d )
{
    // comes in as dd/MM/yyy HH:mm and maybe UTC/GMT
   if (d.endsWith(" UTC"))
        d = d.left(d.length() - 4);
   QDateTime dt = QDateTime::fromString(d, "dd/MM/yyyy HH:mm");
   QString s = dt.toString( "yyyyMMddHHmm" );
   return s;
}
//=============================================================================



int toInt ( const QString &s, int def )
{
    if ( !s.isEmpty() )
    {
        bool ok;
        int i = s.toInt(&ok);
        if (ok)
            return i;
    }
    return def;
}
double toDouble ( const QString &s, double def )
{
    if ( !s.isEmpty() )
    {
        bool ok;
        double d = s.toDouble(&ok);
        if (ok)
            return d;
    }
    return def;
}
QString makeStr( bool i )
{
   return ( i ? "true" : "false" );
}
QString HtmlFontColour( const QColor &c )
{
    QString s = "<font color='" + c.name() + "'>";
    return s;
}
UpperCaseValidator::UpperCaseValidator(bool makeSignal):makeSignal(makeSignal)
{

}

QValidator::State UpperCaseValidator::validate(QString & input, int & /*pos*/) const
{
    input = input.toUpper();

    if (makeSignal)
    {
        MinosLoggerEvents::SendValidated();
    }
    return Acceptable;
}

//=======================================================================================


// returns true if any of the QChars in the QList are found in QString s

bool containsChars(QString s, const QList<QChar> chars)
{
    bool state = false;
    for (auto i : chars)
    {
        state = s.contains(i);
        if (state)
            return state;
    }

    return state;

}

// replace with QChar c when a QChar from the QList is found.

QString replaceChars(QString s, const QChar c, const QList<QChar> chars )
{

    for (auto i : chars)
    {
        s.replace(i, c);
    }
    return s;
    return s;

}

// remove any QChars found in the QList from the QString
QString removeChars(QString s, const QList<QChar> chars)
{

    for (auto i : chars)
    {
        s.remove(i, Qt::CaseInsensitive);
    }
    return s;
    return s;

}

