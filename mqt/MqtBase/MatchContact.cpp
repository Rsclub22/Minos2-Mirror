/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "list.h"
#include "contest.h"
#include "MatchContact.h"
#include "MinosParameters.h"
//---------------------------------------------------------------------------
BaseMatchContest::~BaseMatchContest(){}

// Here we compare pointers, just to give an actual < operator
bool MatchContactList::operator<( const BaseMatchContest& rhs ) const
{
   // p1 is from list; p2 is the one being searched for
    // p1 is from list; p2 is the one being searched for

    const ContactList * cn1 = getContactList();
    const ContactList * cn2 = rhs.getContactList();

    if ( cn2 == nullptr )       // cn1 must be good - its us!
    {
       return false;
    }
    if ( cn1 != cn2 )
    {
       if ( cn1->getSlotNo() < cn2->getSlotNo() )
          return true;
       else
          return false;
    }

    return false;
}
ContactList *MatchContactList::getContactList() const
{
  return dynamic_cast<ContactList * >(matchedContest);
}

bool MatchContactLog::operator<( const BaseMatchContest& rhs ) const
{
   // p1 is from list; p2 is the one being searched for

   const BaseContestLog * cn1 = getContactLog();
   const BaseContestLog * cn2 = rhs.getContactLog();

   if ( cn2 == nullptr )
   {
      return true;
   }
   if ( cn1 != cn2 )
   {
       if ( cn1->cslotno < cn2->cslotno )
          return true;
       else
          return false;
   }

   return false;

}
const BaseContestLog * MatchContactLog::getContactLog() const
{
   return dynamic_cast<BaseContestLog* >(matchedContest);
}

MatchContact::MatchContact( )
{}
MatchContact::~MatchContact()
{}
//==============================================================================
MatchLogContact::MatchLogContact(BaseContestLog *ct, CheckableContact *lc )
      : matchedContest( ct ), matchedContact( lc )
{}
MatchLogContact::~MatchLogContact()
{}

bool MatchLogContact::operator<( const MatchContact& rhs ) const
{
   // p1 is from list; p2 is the one being searched for

   BaseContestLog *cn1 = getContactLog();
   BaseContestLog *cn2 = rhs.getContactLog();

   if ( cn2 == nullptr )
   {
      return true;
   }
   if ( cn1 != cn2 )
   {
      // as we now separate current contest and others...
      BaseContestLog * cc = MinosParameters::getMinosParameters() ->getCurrentContest();
      if ( cn1->cslotno == cc->cslotno )
         return true;
      else
         if ( cn2->cslotno == cc->cslotno )
            return false;
      if ( cn1->cslotno < cn2->cslotno )
         return true;
      else
         return false;
   }

   unsigned long p1 = getBaseContact() ->getLogSequence();
   unsigned long p2 = rhs.getBaseContact() ->getLogSequence();

   if ( p1 == p2 )
      return false;

   // This one is more problematic
   const BaseContestLog * ct = MinosParameters::getMinosParameters() ->getCurrentContest();
   if ( bool( ct ) && ( ct->DupSheet.isCurDup( getBaseContact() ) ) )
      return true;

   if ( p1 < p2 )
      return true;

   return false;

}
bool MatchLogContact::operator==( const MatchContact& rhs ) const
{
   // p1 is from list; p2 is the one being searched for

   BaseContestLog *cn1 = getContactLog();
   BaseContestLog *cn2 = rhs.getContactLog();

   if ( cn2 == nullptr )
   {
      return true;
   }

   if ( cn1 != cn2 )
   {
      return false;
   }

   return ( getBaseContact() == rhs.getBaseContact() );
}
bool MatchLogContact::operator!=( const MatchContact& rhs ) const
{
   // p1 is from list; p2 is the one being searched for

    BaseContestLog *cn1 = getContactLog();
    BaseContestLog *cn2 = rhs.getContactLog();

   if ( cn2 == nullptr )
   {
      return true;
   }

   if ( cn1 != cn2 )
   {
      return true;
   }

   return ( getBaseContact() != rhs.getBaseContact() );
}
//==============================================================================
MatchListContact::MatchListContact( )
{}

MatchListContact::MatchListContact(ContactList *ct, ListContact *lc )
      : matchedList( ct ), matchedListContact( lc )
{}
MatchListContact::~MatchListContact()
{}

bool MatchListContact::operator<( const MatchContact& rhs ) const
{
   // p1 is from list; p2 is the one being searched for

   ContactList *cn1 = getContactList();
   ContactList *cn2 = rhs.getContactList();

   if ( cn2 == nullptr )       // cn1 must be good - its us!
   {
      return false;
   }
   if ( cn1 != cn2 )
   {
      if ( cn1 < cn2 )
         return true;
      else
         return false;
   }

   ListContact *p1 = getListContact();
   ListContact *p2 = rhs.getListContact();

   if ( p1 == p2 )
      return false;

   if ( p1 < p2 )
      return true;

   return false;

}
bool MatchListContact::operator==( const MatchContact& rhs ) const
{
   // p1 is from list; p2 is the one being searched for

   ContactList *cn1 = getContactList();
   ContactList *cn2 = rhs.getContactList();

   if ( cn1 != cn2 )
   {
      return false;
   }

   return ( getListContact() == rhs.getListContact() );
}
bool MatchListContact::operator!=( const MatchContact& rhs ) const
{
   // p1 is from list; p2 is the one being searched for

   ContactList *cn1 = getContactList();
   ContactList *cn2 = rhs.getContactList();

   if ( cn1 != cn2 )
   {
      return true;
   }

   return ( getContactList() != rhs.getContactList() );
}
//==============================================================================
