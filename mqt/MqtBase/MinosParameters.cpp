/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

MinosParameters *MinosParameters::mp = nullptr;
MinosParameters::MinosParameters()
{
   mp = this;
}
MinosParameters::~MinosParameters()
{
   mp = nullptr;
}
MinosParameters *MinosParameters::getMinosParameters()
{
   if ( !mp )
   {
      mShowMessage( "MinosParameters not initialised", nullptr );
   }
   return mp;
}

bool MinosParametersAdapter::insertContest( BaseContestLog * /*p*/, int /*sno*/ )
{
   return true;
}
int MinosParametersAdapter::getMagneticVariation()
{
   return 0;
}
void MinosParametersAdapter::getDisplayColumnWidth( const QString &/*key*/, int &val, int def )
{
   val = def;
}
void MinosParametersAdapter::setDisplayColumnWidth( const QString &/*key*/, int /*val*/ )
{
}
void MinosParametersAdapter::getBoolDisplayProfile( int /*enumkey*/, bool &value )
{
   value = true;
}
void MinosParametersAdapter::setBoolDisplayProfile( int /*enumkey*/, bool /*value*/ )
{
}
void MinosParametersAdapter::getIntDisplayProfile( int /*enumkey*/, int &value )
{
   value = 0;
}
void MinosParametersAdapter::setIntDisplayProfile( int /*enumkey*/, int /*value*/ )
{
}
void MinosParametersAdapter::getStringDisplayProfile( int /*enumkey*/, QString &/*value*/ )
{

}
void MinosParametersAdapter::setStringDisplayProfile( int /*enumkey*/, QString /*value*/ )
{

}

void MinosParametersAdapter::flushDisplayProfile()
{
}
qint64 MinosParametersAdapter::getBigClockCorrection()
{
   return 0;
}
int MinosParametersAdapter::getStatsPeriod1()
{
   return 0;
}
int MinosParametersAdapter::getStatsPeriod2()
{
   return 0;
}
void MinosParametersAdapter::setStatsPeriod1( int /*p*/ )
{
}
void MinosParametersAdapter::setStatsPeriod2( int /*p*/ )
{
}
bool MinosParametersAdapter::getAllowLoc4()
{
   return false;
}
bool MinosParametersAdapter::getAllowLoc8()
{
   return false;
}
bool MinosParametersAdapter::yesNoMessage(QWidget * /*Owner*/, const QString & /*mess*/ )
{
   return false;
}
void MinosParametersAdapter::mshowMessage( const QString &mess, QWidget* Owner )
{
    ::mShowMessage( mess, Owner );
}
BaseContestLog * MinosParametersAdapter::getCurrentContest()
{
   return nullptr;
}
bool MinosParametersAdapter::insertList(ContactList * /*p*/, int /*sno*/ )
{
   return false;
}
bool MinosParametersAdapter::isContestOpen(const QString /*fn*/ )
{
   return false;
}
bool MinosParametersAdapter::isListOpen(const QString /*fn*/ )
{
   return false;
}
QVector<BaseContestLog *> MinosParametersAdapter::getContestList()
{
   QVector<BaseContestLog *> logList;
   return logList;
}


