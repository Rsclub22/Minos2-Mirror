/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef MwinH
#define MwinH 
//----------------------------------------------------------------------------
#include "XMPP_pch.h"
// GJVVERSION is definition for the .GJV file format

#define GJVVERSION "0.41"
#define VERLENGTH (4)

#define DATELENGTH 8
#define TIMELENGTH 5
#define CALLSIGNLENGTH 15
#define LOCLENGTH 8
#define COMMENTLENGTH 27
#define EXTRALENGTH 50
//#define RSTLENGTH 3
#define SERIALLENGTH 4

//#define INISECTLENGTH 40

#define CS_OK 0
#define CS_NOT_VALIDATED -1
#define ERR_NOCS 21
#define ERR_CSDIGIT 22
#define ERR_INVCS 23
#define ERR_DUPCS 24

#define LOC_NOT_VALIDATED -1
#define LOC_OK 0
#define LOC_SHORT 1
#define LOC_PARTIAL 2
#define ERR_NOLOC 30
#define ERR_LOC_RANGE 31

enum MatchType {ThisMatch, OtherMatch, ArchiveMatch};

enum ExportType {EREG1TEST, EADIF, EG0GJV, EMINOS, EKML, EPRINTFILE };

#define STACKITEMS 10


// control block for repeated actions on contest contacts
class GJVParams
{
   public:
      int diskBlock;
      QSharedPointer<QFile> fd;
      int count;

      GJVParams( QSharedPointer<QFile> f );
      ~GJVParams();
};
//===================================================================================

// templated class for items in contest and contact
class RPCParamStruct;

template < class itemtype >
class MinosItem
{
   protected:
      bool dirty = false;
      itemtype val{};                          // C++ 11 default brace initialisation

   public:

      itemtype getValue() const
      {
         return val;
      }
      itemtype getValue( bool &d ) const
      {
         d |= dirty;
         return val;
      }
      void setValue( itemtype t )
      {
         if ( val != t )        // so all item classes need != operator
         {
            dirty = true;
            val = t;
         }
      }
      void setValue( MinosItem<itemtype> t )
      {
         if ( val != t.getValue() )        // so all item classes need != operator
         {
            dirty = true;
            val = t.getValue();
         }
      }
      void setInitialValue( itemtype t )
      {
         dirty = false;
         val = t;
      }
      bool isDirty() const
      {
         return dirty;
      }
      void clearDirty()
      {
         dirty = false;
      }
      void setDirty()
      {
         dirty = true;
      }
      MinosItem()
      {}
      MinosItem( itemtype d ) : val( d ), dirty( false )
      {}
      virtual ~MinosItem()
      {}
      MinosItem& operator = ( const MinosItem&rhs )
      {
          dirty = rhs.dirty;
          val = rhs.val;
          return *this;
      }
      void addIfDirty( RPCParamStruct *st, const QString &stName, bool &d ) const
      {
         d |= isDirty();
         if ( isDirty() )
         {
            st->addMember( getValue(), stName );
         }
      }
};
template < class QString >
class MinosStringItem : public MinosItem<QString>
{
public:
    virtual void setValue( QString t )
    {
       t = t.trimmed();
       if ( MinosItem<QString>::val != t )        // so all item classes need != operator
       {
          MinosItem<QString>::dirty = true;
          MinosItem<QString>::val = t;
       }
    }
    void setValue( MinosStringItem<QString> t )
    {
       if ( MinosItem<QString>::val != t.getValue() )        // so all item classes need != operator
       {
          MinosItem<QString>::dirty = true;
          MinosItem<QString>::val = t.getValue();
       }
    }
    virtual void setInitialValue( QString t )
    {
       MinosItem<QString>::dirty = false;
       MinosItem<QString>::val = t;
    }
    MinosStringItem& operator = ( const MinosStringItem&rhs )
    {
        MinosItem<QString>::dirty = rhs.dirty;
        MinosItem<QString>::val = rhs.val;
        return *this;
    }

};

//---------------------------------------------------------------------------
enum DTG {DTGLOG, DTGDISP, DTGReg1Test, DTGFULL, DTGPRINT, DTGACCURATE};
class dtg
{
   private:
      MinosStringItem<QString> sdate;
      MinosStringItem<QString> stime;

      bool baddtg;
   public:
      static const double daySecs;

      QString getDate( DTG, bool &d ) const;
      QString getDate( DTG ) const;
      QString getTime( DTG, bool &d ) const;
      QString getTime( DTG ) const;
      bool getDtg(QDateTime &, bool &d ) const;
      bool getDtg( QDateTime & ) const;
      void setDate( const QString &, DTG );
      void setTime( const QString &, DTG );

      QString getIsoDTG( bool &d ) const;
      QString getIsoDTG( ) const;
      void setIsoDTG( const QString & );

      QString getN1mmDTG();

      QDate getDate();
      QTime getTime();

      void setDate(QDate);
      void setTime(QTime);
      void setDateTime(QDateTime tdt);

      void setValue( const dtg &d )
      {
         sdate.setValue( d.sdate.getValue() );
         stime.setValue( d.stime.getValue() );

         if (notEntered() == 0)
         {
            baddtg = true;
         }
      }
      int notEntered( );  // returns the "entered" state

      bool isDirty() const
      {
         return ( sdate.isDirty() | stime.isDirty() );
      }
      void setDirty()
      {
         sdate.setDirty();
         stime.setDirty();
      }
      void clearDirty()
      {
         sdate.clearDirty();
         stime.clearDirty();
      }
      void setBadDtg()
      {
         baddtg = true;
      }
      bool isBadDtg()
      {
         return baddtg;
      }

      dtg( bool now );
      dtg(const dtg&);
      dtg& operator =(const dtg&);
      ~dtg();
};
//---------------------------------------------------------------------------
class Callsign
{
   #define BITLENGTH 4
	#define NUMBITLENGTH 4
	#define TRAILBITLENGTH 3
   public:
      MinosStringItem<QString> fullCall; // full call
      // eg for <pe/f0ctt/mm> (g0gjv/p) [F6CTT/RVI/P]
      QString locCtryPrefix; // <pe> (g) [/RVI]country of location
      QString dupPrefix;  // <f> (g) [F]country of issue
      QString number; // <0> (0) [6]numeric part
      QString body;  // <ctt> (gjv) [CTT]main body
      QString suffix; // <mm> (p) [P]trailer

      QString realCall; // the actual calsign, stripped of decorations
      QString wpxPrefix;
      int valRes;   // current validation result

      Callsign( );
      Callsign( const QString &orig );
      ~Callsign();
      bool operator==( const Callsign& rhs ) const;
      bool operator!=( const Callsign& rhs ) const
      {
          return !(rhs == *this);
      }
      bool operator<( const Callsign& rhs ) const;
      Callsign& operator = ( const Callsign& );


      int validate( );
      bool isUK() const;
private:
      bool isValidStructure();
      static bool isValidStructure( const QString &prefix,  const QString &number,  const QString &body );
};
//---------------------------------------------------------------------------
class Locator
{
   public:
      MinosStringItem<QString> loc;

      mutable char valRes;

      Locator();
      Locator(const QString &);
      //    locator(const locator&);
      //    locator& operator =(const locator&);
      ~Locator();
      char validate( );
      char validate( double &lon, double &lat );
      Locator &operator =( const Locator& );


};

class BaseLogList: public QObject
{
public:
    BaseLogList();
    BaseLogList(const BaseLogList &);
    virtual ~BaseLogList();
};

#endif
