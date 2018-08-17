/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#ifndef MatchContactH
#define MatchContactH 
#include "base_pch.h"
#include "contacts.h"
//---------------------------------------------------------------------------
class ListContact;
class ContactList;
class BaseMatchContest;
class BaseContestLog;
class MatchContact;

typedef QMap < MapWrapper<BaseMatchContest>, MapWrapper<BaseMatchContest> > ContestMatchList;
typedef ContestMatchList::iterator ContestMatchIterator;

typedef QMap < MapWrapper<MatchContact>, MapWrapper<MatchContact> > MatchList;
typedef MatchList::iterator MatchIterator;

class MatchContact
{
   public:
      MatchContact( );
      virtual ~MatchContact();
      virtual ContactList * getContactList() const
      {
         return nullptr;
      }
      virtual ListContact * getListContact() const
      {
         return nullptr;
      }
      virtual BaseContestLog * getContactLog() const
      {
         return nullptr;
      }
      virtual QSharedPointer<BaseContact> getBaseContact() const
      {
         return QSharedPointer<BaseContact>();
      }
      virtual bool operator<( const MatchContact& rhs ) const = 0;
      virtual bool operator==( const MatchContact& rhs ) const = 0;
      virtual bool operator!=( const MatchContact& rhs ) const = 0;
      virtual void getText( QString &dest, BaseContestLog *const ct ) const = 0;
};
class BaseMatchContest
{
public:
   virtual ~BaseMatchContest();

   BaseLogList *matchedContest;

   MatchList contactMatchList;

   virtual int getContactCount(){return contactMatchList.size();}
   QSharedPointer<MatchContact> pcontactAt( int );

   virtual const ContactList * getContactList() const
   {
      return nullptr;
   }
   virtual const BaseContestLog * getContactLog() const
   {
      return nullptr;
   }
   virtual bool operator<( const BaseMatchContest& rhs ) const = 0;
};
class MatchContactList : public BaseMatchContest
{
public:
   virtual ContactList *getContactList() const override;
   virtual bool operator<( const BaseMatchContest& rhs ) const override;
};
class MatchContactLog : public BaseMatchContest
{
public:
   virtual const BaseContestLog * getContactLog() const override;
   virtual bool operator<( const BaseMatchContest& rhs ) const override;
};


class MatchListContact: public MatchContact
{
   public:
      ContactList *matchedList;
      ListContact *matchedListContact;
      MatchListContact( ContactList * ct, ListContact * lc );
      MatchListContact( );
      virtual ~MatchListContact() override;
      virtual ContactList * getContactList() const override
      {
         return matchedList;
      }
      virtual ListContact * getListContact() const override
      {
         return matchedListContact;
      }
      virtual bool operator<( const MatchContact& rhs ) const override;
      virtual bool operator==( const MatchContact& rhs ) const override;
      virtual bool operator!=( const MatchContact& rhs ) const override;
      virtual void getText(QString &dest, BaseContestLog *const ct ) const override;
};
class MatchLogContact: public MatchContact
{
   public:
      BaseContestLog *matchedContest;
      QSharedPointer<BaseContact> matchedContact;
      MatchLogContact(BaseContestLog * ct, QSharedPointer<BaseContact> lc );
      virtual ~MatchLogContact() override;
      virtual BaseContestLog * getContactLog() const override
      {
         return matchedContest;
      }
      virtual QSharedPointer<BaseContact> getBaseContact() const override
      {
         return matchedContact;
      }
      virtual bool operator<( const MatchContact& rhs ) const override;
      virtual bool operator==( const MatchContact& rhs ) const override;
      virtual bool operator!=( const MatchContact& rhs ) const override;
      virtual void getText( QString &dest, BaseContestLog *const ct ) const override;
};
#endif
