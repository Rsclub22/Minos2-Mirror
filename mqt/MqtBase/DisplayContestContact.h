//--------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef DisplayContestContactH
#define DisplayContestContactH 
#include "base_pch.h"
#include "contacts.h"
class MinosTestImport;
//----------------------------------------------------------------------------
class DisplayContestContact: public BaseContact
{
      int modificationCount;
      unsigned long logSequence; // sparse sequence, used to provide sort key
   protected:
      DisplayContestContact( const DisplayContestContact & );
      DisplayContestContact& operator =( const DisplayContestContact & );
   public:

      virtual void setLogSequence( unsigned long ul ) override
      {
         logSequence = ul;
      }
      virtual unsigned long getLogSequence() const override
      {
         return logSequence;
      }
      virtual int getModificationCount() const override
      {
         return modificationCount;
      }
      virtual void setModificationCount( int c )
      {
         modificationCount = c;
      }

      // Contact items are all in BaseContact


      // end of Contact items

      virtual bool commonSave(QSharedPointer<BaseContact>  ) override
      {
         return false;
      }
      //      virtual bool GJVload( int diskBlock ){return false;}   // moved to BAseCointact


      DisplayContestContact( BaseContestLog *contest, bool time_now );
      ~DisplayContestContact() override;
      virtual bool ne( const ScreenContact& ) const override;
      virtual void checkContact(bool inScan) override;

      virtual void copyFromArg( ScreenContact & ) override;

      virtual QString getField( int ACol, const BaseContestLog * const curcon ) const override;
      virtual void processMinosStanza( const QString &methodName, MinosTestImport * const mt ) override;

};

#endif
