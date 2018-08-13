#include "MonitoredContestLog.h"
#include "DisplayContestContact.h"
//=============================================================================================

MonitoredContestLog::MonitoredContestLog() : BaseContestLog()
{
}
MonitoredContestLog::~MonitoredContestLog()
{
}

void MonitoredContestLog::makeContact(bool timeNow, QSharedPointer<BaseContact> &lct )
{
   lct = QSharedPointer<BaseContact>(new DisplayContestContact( this, timeNow ));
}
//=============================================================================================
