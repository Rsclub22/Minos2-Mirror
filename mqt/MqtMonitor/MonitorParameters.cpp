#include "MonitorMain.h"
#include "MonitorParameters.h"

static MonitorParameters mp;

int MonitorParameters::getStatsPeriod1()
{
   return p1;
}
int MonitorParameters::getStatsPeriod2()
{
   return p2;
}
void MonitorParameters::setStatsPeriod1( int p )
{
   p1 = p;
}
void MonitorParameters::setStatsPeriod2( int p )
{
   p2 = p;
}
bool MonitorParameters::yesNoMessage( QWidget* Owner, const QString &mess )
{
   return mShowYesNoMessage( Owner, mess );
}
void MonitorParameters::mshowMessage( const QString &mess, QWidget* Owner )
{
   mShowMessage( mess, Owner );
}
int MonitorParameters::getContestSlotCount()
{
    return monitorMain->getContestSlotCount();
}
BaseContestLog *MonitorParameters::getContestSlot(int s)
{
    return monitorMain->getContestSlot(s);
}
BaseContestLog * MonitorParameters::getCurrentContest()
{
   return monitorMain->getCurrentContest();
}

