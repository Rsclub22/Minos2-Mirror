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
#include "baseloglist.h"
#include "contacts.h"
//---------------------------------------------------------------------------
class ListContact;
class ContactList;
class BaseMatchContest;
class BaseContestLog;
class MatchContact;

typedef QMap < MapKeyWrapper<BaseMatchContest>, MapWrapper<BaseMatchContest> > ContestMatchList;
typedef ContestMatchList::iterator ContestMatchIterator;

typedef QMap < MapKeyWrapper<MatchContact>, MapWrapper<MatchContact> > MatchList;
typedef MatchList::iterator MatchIterator;

class MatchContact:public QObject
{
    Q_OBJECT
   public:
      MatchContact( );
       MatchContact(const MatchContact &/*rhs*/);
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
      virtual CheckableContact * getBaseContact() const
      {
         return nullptr;
      }
      virtual bool operator<( const MatchContact& rhs ) const = 0;
      virtual bool operator==( const MatchContact& rhs ) const = 0;
      virtual bool operator!=( const MatchContact& rhs ) const = 0;
};
class BaseMatchContest:public QObject
{
    Q_OBJECT
public:
   virtual ~BaseMatchContest();

   BaseLogList *matchedContest;

   MatchList contactMatchList;

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
    virtual ~MatchContactList() override
    {}
   virtual ContactList *getContactList() const override;
   virtual bool operator<( const BaseMatchContest& rhs ) const override;
};
class MatchContactLog : public BaseMatchContest
{
public:
    virtual ~MatchContactLog() override
    {}
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
};
class MatchLogContact: public MatchContact
{
   public:
      BaseContestLog *matchedContest;
      CheckableContact *matchedContact;
      MatchLogContact(BaseContestLog * ct, CheckableContact *lc );
      virtual ~MatchLogContact() override;
      virtual BaseContestLog * getContactLog() const override
      {
         return matchedContest;
      }
      virtual CheckableContact * getBaseContact() const override
      {
         return matchedContact;
      }
      virtual bool operator<( const MatchContact& rhs ) const override;
      virtual bool operator==( const MatchContact& rhs ) const override;
      virtual bool operator!=( const MatchContact& rhs ) const override;
};
#endif
