/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#include "base_pch.h"

#include "tinyxml.h"
#include "portconf.h"
#include "controlport.h"

#include "CommonMonitor.h"
#include "MinosLines.h"

#define TIMER_INTERVAL 55U         // 55-millisecond target interval

//==============================================================================
timerTicker::timerTicker()
{
    connect(&b, &QTimer::timeout, this, &timerTicker::tick);
    b.start(TIMER_INTERVAL);
}
timerTicker::~timerTicker()
{
}

//==============================================================================
commonController::commonController(QWidget *p ):parent(p)
{
}
void commonController::closeDown()
{
   // close down each port
   for ( auto const &icp: qAsConst(portChain) )
   {
      if ( icp )
         icp ->closePort();
      delete ( icp );
   }
   portChain.clear();
}
commonController::~commonController()
{
}
bool commonController::initialise( )
{
   return true;
}


void commonController::tickEvent()        // this will often be an interrupt routine
{
   checkControls();
}
void commonController::checkControls( )
{
   // loop through ports, checkControls on each
    for ( auto const &icp: qAsConst(portChain) )
   {
      icp->checkControls();
   }
}
commonPort *commonController::createPort( const PortConfig &port )
{
    commonPort * cp = nullptr;
    for(auto const &ni: qAsConst(portChain))
    {
        if (ni->portName == port.name)
        {
            cp = ni;
            break;
        }
    }

   if ( cp )
      return cp;
   else
   {
      // create the port

      switch ( port.portType )
      {
         case PortConfig::eptPiGPIO:
            cp = new PiGPIOPort( port );
            break;

        case PortConfig::eptWindows:
            cp = new WindowsMonitorPort( parent, port );
            break;

         case PortConfig::eptK8055:
            //cp = new K8055Port( port );
            break;

      }

   }
   return cp;
}
commonLineControl *commonController::findLine( const QString &name, bool lineIn )
{
   commonLineControl * clc = nullptr;
   for ( auto const &icp: qAsConst(portChain) )
   {
      clc = icp->findLine( name, lineIn );
      if ( clc )
      {
         break;
      }
   }
   return clc;
}
void commonController::lineChange( commonLineControl *line )
{
   commonLineControl * pttout = findLine( "PTTOut", false );
   commonLineControl *pttin = findLine( "PTTIn", true );
   commonLineControl *l1 = findLine( "L1", true );
   commonLineControl *l2 = findLine( "L2", true );
   commonLineControl *l3 = findLine( "L3", true );
   commonLineControl *l4 = findLine( "L4", true );
   commonLineControl *l5 = findLine( "L5", true );
   commonLineControl *l6 = findLine( "L6", true );
   commonLineControl *t1 = findLine( "T1", false );
   commonLineControl *t2 = findLine( "T2", false );
   if ( pttout && pttin && l1 && l2 && l3 && l4 && l5 && l6 && t1 && t2 )
   {
      setLines( pttout->getState(), pttin->getState(),
                l1->getState(), l2->getState(), l3->getState(), l4->getState(), l5->getState(), l6->getState(),
                t1->getState(), t2->getState());
   }
   LineSet *ls = LineSet::GetLineSet();
   ls->publish( line->lineName, line->getState() );
}
//==============================================================================
