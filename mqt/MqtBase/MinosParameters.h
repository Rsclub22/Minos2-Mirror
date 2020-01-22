/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#ifndef MinosParametersH
#define MinosParametersH 
#include <QString>
#include <QWidget>
class BaseContestLog;
//---------------------------------------------------------------------------
class BaseContestLog;
class ContactList;
class MinosParameters
{
    Q_DECLARE_TR_FUNCTIONS(MinosParameters)

   private:
      static MinosParameters *mp;
   protected:
      static void setMinosParameters( MinosParameters *m )
      {
         mp = m;
      }
   public:
      MinosParameters();
      virtual ~MinosParameters();

      static MinosParameters *getMinosParameters();
      virtual bool insertContest( BaseContestLog *p, int sno ) = 0;
      virtual int getMagneticVariation() = 0;
      virtual void getDisplayColumnWidth( const QString &key, int &val, int def ) = 0;
      virtual void setDisplayColumnWidth( const QString &key, int val ) = 0;
      virtual void getBoolDisplayProfile( int enumkey, bool &value ) = 0;
      virtual void setBoolDisplayProfile( int enumkey, bool value ) = 0;
      virtual void getIntDisplayProfile(int enumkey, int &value) = 0;
      virtual void setIntDisplayProfile(int enumkey, int value) = 0;
      virtual void getStringDisplayProfile( int enumkey, QString &value ) = 0;
      virtual void setStringDisplayProfile( int enumkey, QString value ) = 0;
      virtual void flushDisplayProfile( ) = 0;
      virtual qint64 getBigClockCorrection() = 0;
      virtual int getStatsPeriod1() = 0;
      virtual int getStatsPeriod2() = 0;
      virtual void setStatsPeriod1( int ) = 0;
      virtual void setStatsPeriod2( int ) = 0;
      virtual bool getAllowLoc4() = 0;
      virtual bool getAllowLoc8() = 0;
      virtual bool yesNoMessage( QWidget* Owner, const QString &mess ) = 0;
      virtual void mshowMessage( const QString &mess, QWidget* Owner = nullptr ) = 0;
      virtual BaseContestLog *getCurrentContest() = 0;
      virtual bool insertList( ContactList *p, int sno ) = 0;
      virtual bool isContestOpen( const QString fn ) = 0;
      virtual bool isListOpen( const QString fn ) = 0;
      virtual QVector<BaseContestLog *> getContestList() = 0;
      virtual int getContestSlotCount() = 0;
      virtual BaseContestLog *getContestSlot(int) = 0;
      virtual int getListSlotCount() = 0;
      virtual ContactList *getListSlot(int) = 0;

};
class MinosParametersAdapter : public MinosParameters
{
   public:
      MinosParametersAdapter()
      {}
      virtual ~MinosParametersAdapter() override
      {}
      virtual bool insertContest( BaseContestLog *p, int sno ) override;
      virtual int getMagneticVariation() override;
      virtual void getDisplayColumnWidth( const QString &key, int &val, int def ) override;
      virtual void setDisplayColumnWidth( const QString &key, int val ) override;
      virtual void getBoolDisplayProfile( int enumkey, bool &value ) override;
      virtual void setBoolDisplayProfile( int enumkey, bool value ) override;
      virtual void getIntDisplayProfile(int enumkey, int &value) override;
      virtual void setIntDisplayProfile(int enumkey, int value) override;
      virtual void getStringDisplayProfile( int enumkey, QString &value ) override;
      virtual void setStringDisplayProfile( int enumkey, QString value ) override;
      virtual void flushDisplayProfile( ) override;
      virtual qint64 getBigClockCorrection() override;
      virtual int getStatsPeriod1() override;
      virtual int getStatsPeriod2() override;
      virtual void setStatsPeriod1( int ) override;
      virtual void setStatsPeriod2( int ) override;
      virtual bool getAllowLoc4() override;
      virtual bool getAllowLoc8() override;
      virtual bool yesNoMessage( QWidget* Owner, const QString &mess ) override;
      virtual void mshowMessage(const QString &mess, QWidget* Owner = nullptr ) override;
      virtual BaseContestLog *getCurrentContest() override;
      virtual bool insertList( ContactList *p, int sno ) override;
      virtual bool isContestOpen( const QString fn ) override;
      virtual bool isListOpen(const QString fn ) override;
      virtual QVector<BaseContestLog *> getContestList() override;
      virtual int getContestSlotCount() override;
      virtual BaseContestLog *getContestSlot(int) override;
      virtual int getListSlotCount() override;
      virtual ContactList *getListSlot(int) override;
};
#endif
