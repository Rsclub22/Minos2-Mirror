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
#include "contacts.h"
class MinosTestImport;
//----------------------------------------------------------------------------
class DisplayContestContact: public BaseContact
{
    Q_OBJECT

      int modificationCount;
   protected:
      DisplayContestContact( const DisplayContestContact & );
      DisplayContestContact& operator =( const DisplayContestContact & );
   public:

      virtual int getModificationCount() const override
      {
         return modificationCount;
      }
      virtual void setModificationCount( int c )
      {
         modificationCount = c;
      }

      virtual bool commonSave(QSharedPointer<BaseContact>  ) override
      {
         return false;
      }

      DisplayContestContact(BaseContestLog *contest, bool time_now , bool initReport);
      ~DisplayContestContact() override;
      virtual bool ne( const ScreenContact& ) const override;
      virtual int checkContact(bool adddup) override;

      virtual void copyFromArg( ScreenContact & ) override;

      virtual QString getField( int ACol, const BaseContestLog * const curcon ) const override;
      virtual void processMinosStanza( const QString &methodName, MinosTestImport * const mt ) override;

};

#endif
