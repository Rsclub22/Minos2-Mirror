/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include <QApplication>
#include <QComboBox>
#include <QSplitter>
#include <QScrollArea>
#include <QSettings>
#include <QToolButton>
#include <QMenu>
#include <qnamespace.h>
#include <cstring>

#include "PubSubName.h"
#include "cutils.h"
#include "MTrace.h"
#include "MinosParameters.h"

const double pi = 3.141592653 ;  /* pi */
const double dr = pi / 180.0;      // degree to radian conversion factor

char diskBuffer[ bsize + 1 ];
//char *lbuff = &diskBuffer[ 0 ];
int buffpt = 0;

bool inClearScreenLayout = false;

void clearBuffer( )
{
   memset( diskBuffer, '#', bsize );
   diskBuffer[ bsize ] = 0;
}
static const char *noeditstr = "\r\n!!BINARY FILE DO NOT EDIT!!\r\n";
const int noeditlength = 32;

void strtobuf( const QString &str )
{
   buffpt += static_cast<int>(strcpysp( &diskBuffer[ buffpt ], str, str.length() ) + 1);
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

    strncpy(s1, ss2.toLatin1(), maxlen);
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

   qint64 ret = expfd->write(l.toUtf8());
   if ( ret != static_cast<int >(l.toUtf8().size()) )
   {
      MinosParameters::getMinosParameters() ->mshowMessage( "bad reply from write!" );
   }
}
void writer::lwriteLine()
{
    QChar hl(0x5F);
   QString l( 80, hl );					// horizontal line
   lwrite( l );
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
      tag = QString( "<%1:%2>%3 " ).arg(fieldname.toLower()).arg(len ).arg(content);
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
QString makeStr( bool i )
{
   return ( i ? "true" : "false" );
}
QString HtmlFontColour( const QColor &c )
{
    QString s = "<font color='" + c.name() + "'>";
    return s;
}
QString HtmlFontColour( const QString &c )
{
    QString s = "<font color='" + c + "'>";
    return s;
}
UpperCaseValidator::UpperCaseValidator()
{

}

QValidator::State UpperCaseValidator::validate(QString & input, int & /*pos*/) const
{
    input = input.toUpper();

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

QString escapeXML ( const QString &value )
{
    QString escaped;
    for ( auto const &c: value )
    {
        if (c == '<')
            escaped += "&lt;";
        else if (c == '>')
            escaped += "&gt;";
        else if (c == '&')
            escaped += "&amp;";
        else if (c == '\'')
            escaped += "&apos;";
        else if (c == '"')
            escaped += "&quot;";
        else
            escaped += c;
    }

    return escaped;
}
CsvReader::CsvReader(QChar sep):sep(sep){}

bool CsvReader::parseCsv(const QString &fileName, QList<QStringList> &csv)
{
    QFile file (fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        static QRegularExpression rer("\r");
        QString data = file.readAll();
        data.remove( rer ); //remove all ocurrences of CR (Carriage Return)
        QString temp;
        QChar character;

        int offset = 0;
        while (offset < data.length())
        {
            character = data[offset++];
            if (character == '\r')
            {
                // ignore it
            }
            else if (character == ',')
            {
                checkString(temp, character, csv);
            }
            else if (character == '\r' || character == '\n')
            {
                checkString(temp, character, csv);
            }
            else if (offset == data.length())
            {
                temp.append(character);
                checkString(temp, QChar(0), csv);
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
void CsvReader::parseCsvLine(const QString &line, QStringList &csv)
{
     QString temp;
     QChar character;

     int lsize = line.length();
     for (int i = 0; i < lsize; i++)
     {
         character = line[i];
         if (character == sep)
         {
             checkString(temp, character, csv);
         }
         else if (character == '\n')
         {
             checkString(temp, character, csv);
         }
         else if (character == nullptr)
         {
             checkString(temp, QChar(0), csv);
         }
         else
         {
             temp.append(character);
         }
     }
     checkString(temp, QChar(0), csv);
}
void CsvReader::checkString(QString &temp, QChar character, QStringList &csv)
{
    if(temp.count("\"")%2 == 0)
    {
        if (temp.startsWith( QChar('\"')) && temp.endsWith( QChar('\"') ) )
        {
             static QRegularExpression qre1("^\"");
             static QRegularExpression qre2("\"$");
             temp.remove( qre1 );
             temp.remove( qre2 );
        }
        //FIXME: will possibly fail if there are 4 or more reapeating double quotes
        temp.replace("\"\"", "\"");
        csv.append(temp.trimmed());
        if (character != QChar(sep))
        {
            return;
        }
        temp.clear();
    }
    else
    {
        temp.append(character);
    }
}
void CsvReader::checkString(QString &temp, QChar character, QList<QStringList> &csv)
{
    if(temp.count("\"")%2 == 0)
    {
        if (temp.startsWith( QChar('\"')) && temp.endsWith( QChar('\"') ) )
        {
            temp = temp.mid(1, temp.length() - 2);
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

void CSVToStringList( const QString &qs, QStringList &sl )
{
    sl.clear();

    CsvReader csv;

    csv.parseCsvLine(qs, sl);
}

#ifdef RUBBISH
CsvReader::CsvReader(){}

bool CsvReader::parseCsv(const QString &fileName, QList<QStringList> &csv)
{
    QFile file (fileName);
     if (file.open(QIODevice::ReadOnly))
     {
         QString data = file.readAll();
         data.remove( QRegularExpression("\r") ); //remove all ocurrences of CR (Carriage Return)
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
             temp.remove( QRegularExpression("^\"") );
             temp.remove( QRegularExpression("\"$") );
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


#endif
QString anchoredPattern(const QString &expression)
{
    return QLatin1String("\\A(?:")
           + expression
           + QLatin1String(")\\z");
}

void adjustMargins(QLayout *layout, int ls, int cml, int cmt, int cmr, int cmb)
{
    if (layout)
    {
        for(int i = 0; i < layout->count(); i++)
        {
            QLayoutItem *li = layout->itemAt(i);
            QLayout *l = li->layout();
            QWidget *w = li->widget();
            if (l)
            {
                adjustMargins(l, ls, cml, cmt, cmr, cmb);
            }
            if (w)
            {
                if (w->layout())
                {
                    adjustMargins(w->layout(), ls, cml, cmt, cmr, cmb);
                }
                else
                {
                    QSplitter *s = dynamic_cast<QSplitter *>(w);
                    if (s)
                    {
                        int sct = s->count();
                        for (int j = 0; j< sct; j++)
                        {
                            adjustMargins(s->widget(j)->layout(), ls, cml, cmt, cmr, cmb);
                        }
                    }
                    QScrollArea *sa = dynamic_cast<QScrollArea *>(w);
                    if (sa)
                    {
                        w = sa->widget();
                        adjustMargins(w->layout(), ls, cml, cmt, cmr, cmb);
                    }
                }
            }
        }
        layout->setSpacing(ls);
        layout->setContentsMargins(cml, cmt, cmr, cmb);
    }
}
void removeFrameBoxes(QLayout *layout)
{
    if (layout)
    {
        for(int i = 0; i < layout->count(); i++)
        {
            QLayoutItem *li = layout->itemAt(i);
            QLayout *l = li->layout();
            QWidget *w = li->widget();
            if (l)
            {
                removeFrameBoxes(l);
            }
            if (w)
            {
                if (w->layout())
                {
                    QFrame *f = dynamic_cast<QFrame *>(w);
                    if (f)
                    {
                        f->setFrameShape(QFrame::NoFrame);
                    }
                    removeFrameBoxes(w->layout());
                }
                else
                {
                    QSplitter *s = dynamic_cast<QSplitter *>(w);
                    if (s)
                    {
                        int sct = s->count();
                        for (int j = 0; j< sct; j++)
                        {
                            removeFrameBoxes(s->widget(j)->layout());
                        }
                    }
                    QScrollArea *sa = dynamic_cast<QScrollArea *>(w);
                    if (sa)
                    {
                        w = sa->widget();
                        removeFrameBoxes(w->layout());
                    }
                    QFrame *f = dynamic_cast<QFrame *>(w);
                    if (f)
                    {
                        f->setFrameShape(QFrame::NoFrame);
                    }
                }
            }
        }
    }
}
bool isPureNumeric ( const QString &s )
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
void saveHeaderColumns(QString fileName, QString tableName, QString layoutName, QHeaderView *hdr)
{
    trace(QString("saveHeaderColumns %1 table %2").arg(layoutName,tableName));
    if ( inClearScreenLayout)
    {
        trace("Ignoring as in clearScreenLayout");
        return;
    }
    QString hLine;
    for (int i = 0; i < hdr->count(); i++)
    {
        int w = hdr->sectionSize(i);
        int visPos = hdr->visualIndex(i);
        bool visible = !hdr->isSectionHidden(i);

        if (i != 0)
        {
            hLine += ";";
        }
        hLine += QString("( %1, %2, %3)").arg(w).arg(visPos).arg(visible);
    }
    int sort = hdr->sortIndicatorSection();
    Qt::SortOrder so = hdr->sortIndicatorOrder();
    hLine += QString(";%1,%2").arg(sort).arg(so);

    trace(QString("saveHeaderColumns %1 line %2").arg(tableName, hLine));

    QSettings hdrSettings(fileName, QSettings::IniFormat);
    hdrSettings.setValue(tableName + "/" + layoutName + "_" + "state", hLine);
    hdrSettings.sync();
}
class HdrCol
{
public:
    int logPos = 0;
    int w = 0;
    int visPos = 0;
    bool visible = false;

};

void setHeaderColumns(QString hLine, QHeaderView *hdr)
{
    trace(QString("setHeaderColumns %1").arg(hLine));
    QVector<HdrCol> hdrs;
    int sort = 0;
    int sortOrder = Qt::AscendingOrder;
    QStringList sl = hLine.split(";");

    int lp = 0;
    for(QString s:qAsConst(sl))
    {
        if (s[0] == '(')
        {
            s = s.mid(1, s.length() - 2);
            QStringList hl = s.split(",");
            if (hl.count() != 3)
            {
                continue;
            }
            HdrCol hc;
            hc.w = hl[0].toInt();
            hc.visPos = hl[1].toInt();
            hc.visible = (hl[2].toInt() != 0);
            hc.logPos = lp;
            lp++;
            hdrs.push_back(hc);
        }
        else
        {
            QStringList ss = s.split(",");
            if (ss.count() == 2)
            {
                sort = ss[0].toInt();
                sortOrder  = ss[1].toInt();
            }
        }
    }
    std::sort(hdrs.begin(), hdrs.end(),
    [=](const HdrCol &a, const HdrCol &b)->bool
      {
          return a.visPos < b.visPos;
      }
    );

    for(const auto &h:hdrs)
    {
        hdr->setSectionHidden(h.logPos, !h.visible);
        if (h.w > 0)
        {
            hdr->resizeSection(h.logPos, h.w);
        }
        else
        {
            hdr->resizeSection(h.logPos, 101);
        }
        hdr->moveSection(hdr->visualIndex(h.logPos), h.visPos);
    }
    hdr->setSortIndicator(sort, static_cast<Qt::SortOrder>(sortOrder));

    QFont cf = QApplication::font();
    hdr->setFont(cf);

}
void restoreHeaderColumns(QString fileName, QString tableName, QString layoutName, QHeaderView *hdr)
{
    trace(QString("restoreHeaderColumns %1 table %2").arg(layoutName, tableName));
    QSettings hdrSettings(fileName, QSettings::IniFormat);
    QString hLine = hdrSettings.value(tableName + "/" + layoutName + "_" + "state", "").toString();

    if (hLine.isEmpty())
    {
        resetHeaderColumns(fileName, tableName, layoutName, hdr);
        return;
    }

    trace(QString("setHeaderColumns (in restore) %1 table %2").arg(layoutName, tableName));
    setHeaderColumns(hLine, hdr);

}
void resetHeaderColumns(QString fileName, QString tableName, QString layoutName, QHeaderView *hdr)
{
    trace(QString("resetHeaderColumns %1 table %2").arg(layoutName, tableName));
    QSettings hdrSettings(fileName, QSettings::IniFormat);
    hdrSettings.setValue(tableName + "/" + layoutName + "_" + "state", "");

    QString hLine;
    for (int i = 0; i < hdr->count(); i++)
    {
        int w = -1;
        int visPos = i;
        bool visible = true;

        if (i != 0)
        {
            hLine += ";";
        }
        hLine += QString("( %1, %2, %3)").arg(w).arg(visPos).arg(visible);
    }
    int sort = 0;
    Qt::SortOrder so = Qt::AscendingOrder;
    hLine += QString(";%1,%2").arg(sort).arg(so);

    trace(QString("setHeaderColumns (in reset) %1 table %2").arg(layoutName).arg(tableName));
    setHeaderColumns(hLine, hdr);
}
void popupColumnsMenu(QMenu &menu, const QPoint &globalPos, QHeaderView *hdr)
{
    // go through columnsMenu, see which columns are visible

    for (int i = 0; i < hdr->count(); i++)
    {
        bool vis = !hdr->isSectionHidden(i );
        menu.actions().at(i + 2)->setChecked(vis);   // miss out reset and separator
    }
    menu.popup( globalPos );
}
void createColumnsMenu(QMenu &menu, QAbstractItemModel *hdrModel,  QWidget *p, std::function<void()> pred)
{
    menu.clear();

    QAction *newAct = new QAction(QWidget::tr("Reset Columns", "createColumnsMenu"), p);
    newAct->setData( -1 );
    menu.addAction( newAct );
    menu.addSeparator();

    p->connect( newAct, &QAction::triggered, p, [=]()
        {
            pred();
        }
    );
    for ( int i = 0; i < hdrModel->columnCount(); i++ )
    {
        QString h = hdrModel->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
                //p->tr(def[ i ].title);

        if (h.isEmpty())
        {
            h = "empty";
        }

        newAct = new QAction( h, p );
        newAct->setData( i );
        newAct->setCheckable( true );

        menu.addAction( newAct );

        p->connect( newAct, &QAction::triggered, p, [=]()
            {
                pred();
            }
        );
    }
}

void createColumnsMenu(QMenu &menu, QHeaderView *hdr, QWidget *p, std::function<void()> pred)
{
    createColumnsMenu(menu, hdr->model(), p, pred);
}
void comboSetUniqueNames(QStringList nameList, QComboBox *cb)
{
    QVector<PubSubName> names;

    for(const auto &s: qAsConst(nameList))
    {
        // get unique names
        names.push_back(PubSubName(s));
    }

    QStringList uniqueNames;
    for(const auto &p:names)
    {
        QString r = p.key();

        int sameNames = 0;
        QString uniqueName;
        for(const auto &p2:names)
        {
            if (p2.key() == r)
            {
                sameNames++;
            }
        }
        if (sameNames > 1)
        {
            // NB that app + key on a single machine will always be unique
            // sowe need to test for the same across machines

            // we could also look for router + key?

            sameNames = 0;
            r = p.getLocalName();
            for(const auto &p2:names)
            {
                if (p2.getLocalName() == r)
                {
                    sameNames++;
                }
                if (sameNames > 1)
                {
                    uniqueName = p.toString();
                }
                else
                {
                    uniqueName = r;
                }
            }
        }
        else
        {
            uniqueName = r;
        }

        uniqueNames.append(uniqueName);
    }

    cb->clear();
    cb->addItem("", "");
    for(int i = 0; i < names.count(); i++)
    {
        cb->addItem( uniqueNames[i], names[i].toString());
        cb->setItemData( i + 1, names[i].toString(), Qt::ToolTipRole );

    }
}
void clearLayout(QLayout *layout)
{
    trace("Enter clearLayout");
    if (layout != nullptr)
    {
        while(layout->count() > 0)
        {
            QLayoutItem *item = layout->takeAt(0);

            QWidget* widget = item->widget();
            if(widget)
            {
                QWidget *f = widget;
                QString name = f->objectName();
                if (name.isEmpty())
                {
                    QToolButton *tb = dynamic_cast<QToolButton *>(f);
                    if (tb)
                    {
                        name = tb->text();
                    }
                }
                QWidget *p = f->parentWidget();
                while ( p)
                {
                    name += " | " + p->objectName();
                    p = p->parentWidget();
                }
                QString s = f->metaObject()->className() + QString(" | ") + name;

                trace(s);
                delete widget;
            }
            else
            {
                QLayout * layout = item->layout();
                if (layout)
                {
                    trace(QString("Clear layout %1").arg(layout->objectName()));
                    clearLayout(layout);
                }
                else
                {
                    QSpacerItem * si = item->spacerItem();
                    if (si)
                    {
                        trace("clear spacer");
                        //delete si;    // the spacer IS the item
                    }
                    else
                    {
                        trace("clear something!");
                    }
                }
            }
            delete item;
        }
    }
    trace("Exit clearLayout");
}
