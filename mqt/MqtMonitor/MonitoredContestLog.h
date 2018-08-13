#ifndef MONITOREDCONTESTLOG_H
#define MONITOREDCONTESTLOG_H
#include "base_pch.h"
#include "contest.h"

class MonitoredContestLog:public BaseContestLog
{
public:
   MonitoredContestLog();
   virtual ~MonitoredContestLog() override;

   virtual void makeContact( bool time_now, QSharedPointer<BaseContact>& ) override;
};
#endif // MONITOREDCONTESTLOG_H
