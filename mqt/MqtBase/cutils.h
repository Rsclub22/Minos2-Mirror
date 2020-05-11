/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef CutilsH
#define CutilsH
#include "base_pch.h"

//----------------------------------------------------------------------------
extern const double pi /* = (double )3.141592653 */;  /* pi */
extern const double dr /* = pi/180.0*/;      			  // degree to radian conversion factor

extern void clearBuffer( );
extern void strtobuf( const QString &str );
extern void strtobuf( const MinosStringItem<QString> &str );
extern void strtobuf();
extern int strnicmp(const QString &s1, const QString &s2, int len );
extern void opyn( bool b );
void opyn( const MinosItem<bool> &b );
extern void buftostr( QString &str );
extern void buftostr( MinosStringItem<QString> &str );
extern bool inyn();
extern size_t strcpysp( char *s1, const QString &s2, int maxlen = 255 );
extern int strcpysp( QString &s1, const QString &s2, int maxlen = 255 );
extern int strcmpsp( const QString &s1, const QString &s2 );
extern int stricmpsp( const QString &s1, const QString &s2 );
extern int placestr(QString &buff, const QString &str, int start, int len );
extern int strcspn(const QString &s, const QString &chars);
int parseLine(QString buff, char sep, QStringList &a, int count, char sep2, bool &sep2seen );

extern QString trimr( const QString & );

extern QString makeADIFField( const QString &fieldname, const QString &content );
extern QString makeADIFField( const QString &fieldname, int content );
extern QString TDTToCanonical( QString d );
extern QDateTime CanonicalToTDT( QString cdtg );

const int bsize = 256;
extern char diskBuffer[ bsize + 1 ];

extern int buffpt;

const QList<QChar> illegalChars = {QChar('|'), QChar('/'), QChar('\\'), QChar('['), QChar(']'), QChar('&'), QChar(']'), QChar('*'),
                                   QChar('!'), QChar('^'), QChar('?'), QChar('<'), QChar('>'), QChar('.'), QChar(','), QChar('~'),
                                   QChar('@'), QChar(':'), QChar(';'), QChar('='), QChar('{'), QChar('}'), QChar('$')};

extern bool containsChars(QString s, const QList<QChar> chars);
extern QString replaceChars(QString s, const QChar c, const QList<QChar> cChars );
extern QString removeChars(QString s, const QList<QChar> chars);
extern QString escapeXML (const QString &value);


class writer
{
      QSharedPointer<QFile> expfd;
      //      char *lbuff;

   public:
      void lwrite( const QString & );
      void lwrite( const char * );
      void lwriteLine( );
      void lwriteNl( );
      void lwriteFf( );
      writer( QSharedPointer<QFile> f );
      ~writer();
};

extern int toInt ( const QString &s, int def = 0 );
extern double toDouble ( const QString &s, double def = 0.0 );
extern QString makeStr( bool i );

// and disallow any other versions
template <class T>
QString makeStr(T) = delete; // C++11

extern QString HtmlFontColour( const QColor &c );

template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
    almost_equal(T x, T y, int ulp)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::abs(x-y) <= std::numeric_limits<T>::epsilon() * std::abs(x+y) * ulp
    // unless the result is subnormal
           || std::abs(x-y) < std::numeric_limits<T>::min();
}

class UpperCaseValidator:public QValidator
{
public:
    UpperCaseValidator();
    virtual ~UpperCaseValidator() override
    {}
    QValidator::State validate(QString & input, int & /*pos*/) const override;
};

//class CsvReader
//{
//    void checkString(QString &temp, QChar character, QList<QStringList> &csv);
//    QStringList itemList;
//public:
//    CsvReader();

//    bool parseCsv(const QString &fileName, QList<QStringList> &csv);
//};
class CsvReader
{
public:
    CsvReader(QChar sep = ',');

    void parseCsvLine(const QString &line, QStringList &csv);
    bool parseCsv(const QString &fileName, QList<QStringList> &csv);

private:
    QChar sep;
    QStringList itemList;

    void checkString(QString &temp, QChar character, QStringList &csv);
    void checkString(QString &temp, QChar character, QList<QStringList> &csv);
};
void CSVToStringList( const QString &s, QStringList &sl );
void TSVToStringList( const QString &s, QStringList &sl );

#endif
