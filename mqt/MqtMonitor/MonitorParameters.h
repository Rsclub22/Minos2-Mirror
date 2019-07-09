#ifndef MONITORPARAMETERS_H
#define MONITORPARAMETERS_H
#include "base_pch.h"

class MonitorParameters : public MinosParametersAdapter
{
      int p1;
      int p2;
   public:
      MonitorParameters() : p1( 10 ), p2( 60 )
      {}
      ~MonitorParameters() override
      {}
      virtual int getStatsPeriod1() override;
      virtual int getStatsPeriod2() override;
      virtual void setStatsPeriod1( int ) override;
      virtual void setStatsPeriod2( int ) override;
      virtual bool getAllowLoc4() override;
      virtual bool getAllowLoc8() override;
      virtual bool yesNoMessage( QWidget* Owner, const QString &mess ) override;
      virtual void mshowMessage(const QString &mess, QWidget* Owner = nullptr ) override;
      virtual int getContestSlotCount() override;
      virtual BaseContestLog *getContestSlot(int) override;
      virtual BaseContestLog *getCurrentContest() override;
};

#endif // MONITORPARAMETERS_H
