/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifndef LoggerContactsH
#define LoggerContactsH 
#include "base_pch.h"
#include "DisplayContestContact.h"
//----------------------------------------------------------------------------
// This header covers the structure of contests, logs, etc


class LoggerContestLog;

class ContestContact: public DisplayContestContact
{
    Q_OBJECT
   private:
      ContestContact( const ContestContact & );
      ContestContact& operator =( const ContestContact & );
   public:
      virtual void clearDirty() override;
      virtual void setDirty() override;

//      virtual bool setField(QSharedPointer<BaseContact> tct, int ACol, const QString Value );

      ContestContact( LoggerContestLog *contest, bool time_now );
      ~ContestContact() override;
      virtual void getPrintFileText(QString &, short ) override;
      virtual void addReg1TestComment(QStringList &remarks ) override;
      virtual void getReg1TestText( QString &, bool noSerials ) override;
      virtual void getCabrilloText( QString & ) override;
      virtual QString getADIFLine() override;
      bool commonSave(QSharedPointer<BaseContact>  ) override;
      bool minosSave(QSharedPointer<BaseContact> tct);
      virtual bool GJVsave( GJVParams & ) override;
      virtual bool GJVload( int diskBlock ) override;
      virtual void processMinosStanza( const QString &methodName, MinosTestImport * const mt ) override;
};


#endif
