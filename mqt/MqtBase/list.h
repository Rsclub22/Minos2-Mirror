/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////

#ifndef listH
#define listH 
#include "base_pch.h"
//---------------------------------------------------------------------------
class ListContact;
typedef QVector < ListContact *> ListList;
typedef ListList::iterator ListIterator;


class ContactList : public BaseLogList
{
   private:
      bool cslFile;
      int slotno;
      bool errMessShown;
      bool cslLoad();
      bool cslLoadContacts();
   public:
      QString cfileName;
      QString name;

      ListList ctList;
      ContactList();
      ~ContactList();
      bool initialise( int slotno );
      bool initialise( const QString &, int slotno );
      void getMatchText(ListContact *, QString &, const BaseContestLog *const ct ) const;
      void getMatchField( ListContact *pct, int col, QString &disp, const BaseContestLog *const ct ) const;
      int getContactCount( )
      {
         return ctList.size();
      }
      ListContact *pcontactAt(int offset );
      int getSlotNo() const{return slotno;}
      void setSlotNo(int n)
      {
          slotno = n;
      }
};


class CsvReader
{
    void checkString(QString &temp, QChar character, QList<QStringList> &csv);
    QStringList itemList;
public:
    CsvReader();

    bool parseCsv(const QString &fileName, QList<QStringList> &csv);
};

#endif
