#ifndef DTG_H
#define DTG_H
#include "XMPP_pch.h"
#include "minositem.h"

extern QString TDTToCanonical( QString d );
extern QDateTime CanonicalToTDT( QString cdtg );
extern int DTGToInt ( const QString &datg );
extern int DTGToInt ( const QDateTime &datg );
extern QDateTime DTGToDTG ( const QString &sdtg );

enum DTG {DTGLOG, DTGDISP, DTGReg1Test, DTGFULL, DTGPRINT, DTGACCURATE, DTGADIF};
class dtg
{
   private:
      MinosStringItem<QString> sdate;
      MinosStringItem<QString> stime;

      bool baddtg = true;
      QDateTime qdatetime;
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
      QString getCabrilloDTG();

      QDate getDate();
      QTime getTime();

      QDateTime getQDT();

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
         return ( sdate.isDirty() || stime.isDirty() );
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

#endif // DTG_H
