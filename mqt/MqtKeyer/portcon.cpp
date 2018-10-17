/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"
#include "cutils.h"
#include "keyctrl.h"
#include "keyers.h"
#include "portcon.h"
#include "windowMonitor.h"
#include "MinosLines.h"

// For serial port
//#include "serial.h"     // serial port constants

//#include "K8055D.h"        // Velleman K8055 interface

#include "KeyerRPCServer.h" 
//==============================================================================

// This module needs splitting up into common, serial, parallel
// (and later anything else, such as USB, or whatever)

//==============================================================================
/*
      Configuration: we need INI file sections for each possible port and ticker
      so we can set each one up as it is registered to a keyer.
 
      Each keyer INI section defines which port(s) it uses, and which
      ticker it uses;
      then when it registers we set up the corresponding port object
      and let the keyer object register to it.
 
*/

my_deque < commonPort *> portChain;
static WinMonitor *winp = nullptr;
/*
*/ 
//==============================================================================
commonPort *createPort( const PortConfig &port )
{
   commonPort * cp = portChain.find( port.name );

   if ( cp )
      return cp;
   else
   {
      // create the port

      switch ( port.portType )
      {
         case PortConfig::eptWindows:
            cp = new WindowsMonitorPort( port );
            break;

         case PortConfig::eptMinosControl:
            cp = new LineEventsPort( port );
            break;
      }

      if ( !cp->initialise( port ) )
      {
         delete cp;
         cp = nullptr;
      }
      if ( cp && !winp )
      {
         winp = new WinMonitor();
         cp->monitors.push_back( winp );
      }
   }
   return cp;
}
//==============================================================================
commonPort::commonPort( const PortConfig &port ) :
   pName( port.name )
{

}
commonPort::~commonPort()
{
   if ( sblog )
   {
      trace( "~commonPort" );
   }
   monitors.freeAll();
}
bool commonPort::initialise( const PortConfig &/*port*/ )
{
   if ( initialisePort() && openPort() )
   {
      portChain.push_back( this );
      return true;
   }
   return false;
}
void commonPort::registerMonitor( lineMonitor *k )
{
   monitors.push_back( k );
}
//==============================================================================
void commonPort::pttChanged( int state )
{
   for ( my_deque < lineMonitor * >::iterator l = monitors.begin(); l != monitors.end(); l++ )
      ( *l ) ->pttChanged( state );
}
void commonPort::L1Changed( int state )
{
   for ( my_deque < lineMonitor * >::iterator l = monitors.begin(); l != monitors.end(); l++ )
      ( *l ) ->L1Changed( state );
}
void commonPort::L2Changed( int state )
{
   for ( my_deque < lineMonitor * >::iterator l = monitors.begin(); l != monitors.end(); l++ )
      ( *l ) ->L2Changed( state );
}
void commonPort::linesModeChanged( int state )
{
   for ( my_deque < lineMonitor * >::iterator l = monitors.begin(); l != monitors.end(); l++ )
      ( *l ) ->linesModeChanged( state );
}
void commonPort::transverterSwitchChanged(int s)
{
    for ( my_deque < lineMonitor * >::iterator l = monitors.begin(); l != monitors.end(); l++ )
       ( *l ) ->transverterSwitchChanged( s );
}
//=============================================================================
LineCallBack WindowsMonitorPort::WinLineCallback = nullptr;
bool WindowsMonitorPort::PTTInState = false;
bool WindowsMonitorPort::L1State = false;
bool WindowsMonitorPort::L2State = false;
int WindowsMonitorPort::linesMode = 0;

WindowsMonitorPort::WindowsMonitorPort( const PortConfig &port ) : commonPort( port ),
      PTTState( false ), winMonForm( nullptr )
{
}
WindowsMonitorPort::~WindowsMonitorPort()
{
   if ( sblog )
   {
      trace( "WindowsMonitorPort::~WindowsMonitorPort" );
   }
   closePort();
}


bool WindowsMonitorPort::initialisePort()
{
   winMonForm = new windowMonitor( nullptr );
   connect(&LineCheck, SIGNAL(timeout()), this, SLOT(checkWinLines()));
   LineCheck.start(10);
   winMonForm->show();
   return true;
}
void WindowsMonitorPort::checkWinLines()
{
    checkControls();
}

bool WindowsMonitorPort::openPort()
{
   return true;
}

bool WindowsMonitorPort::closePort()
{
   delete winMonForm;
   winMonForm = nullptr;

   return true;
}


void WindowsMonitorPort::ptt( int state )
{
   if ( currentKeyer && currentKeyer->started )
   {
      PTTState = state;
      if ( WinLineCallback )
         ( *WinLineCallback ) ( PTTState, PTTInState, L1State, L2State, linesMode );
   }
}

void WindowsMonitorPort::checkControls( )
{
   // check and action PTT, L1, L2
    L1State = winMonForm->L1Checked();
    L2State = winMonForm->L2Checked();
    PTTInState = winMonForm->PTTChecked();

    linesMode = (winMonForm->L3Checked()?1:0) * 8 +
            (winMonForm->L4Checked()?1:0) * 4 +
            (winMonForm->L5Checked()?1:0) * 2 +
            (winMonForm->L6Checked()?1:0) * 1;


   if ( L2State != lastL2State )
   {
      lastL2State = L2State;
      if ( sblog )
      {
         trace( "L2 changed to " + makeStr( lastL2State ) );
      }
      L2Changed( lastL2State );
   }
   if ( L1State != lastL1State )
   {
      lastL1State = L1State;
      if ( sblog )
      {
         trace( "L1 changed to " + makeStr( lastL1State ) );
      }
      L1Changed( lastL1State );
   }
   if ( PTTInState != lastPTTState )
   {
      lastPTTState = PTTInState;
      if ( sblog )
      {
         trace( "PTT changed to " + makeStr( PTTInState ) );
      }
      pttChanged( lastPTTState );
   }
   if (linesMode != lastLinesMode)
   {
       lastLinesMode = linesMode;
       if ( sblog )
       {
          trace( "lines mode changed to " + QString::number( linesMode ) );
       }
       linesModeChanged( linesMode );
   }
}
//==============================================================================
LineCallBack WinMonitor::WinLineCallback = nullptr;
bool WinMonitor::PTTInState = false;
bool WinMonitor::L1State = false;
bool WinMonitor::L2State = false;
int WinMonitor::linesMode = false;

WinMonitor::WinMonitor() : lineMonitor( "WinMon" ),
      PTTState( false )
{}
WinMonitor::~WinMonitor()
{
   if ( sblog )
   {
      trace( "WinMonitor::~WinMonitor" );
   }
}

bool WinMonitor::initialise( const KeyerConfig &/*keyer*/, const PortConfig &/*port*/ )
{
   return true;
}

void WinMonitor::ptt( int state )
{
   if ( currentKeyer && currentKeyer->started )
   {
      PTTState = state;
      if ( WinLineCallback )
         ( *WinLineCallback ) ( PTTState, PTTInState, L1State, L2State, linesMode );
   }
}

void WinMonitor::checkControls( )
{}
bool WinMonitor::L1Changed( int state )
{
   L1State = state;
   if ( WinLineCallback )
      ( *WinLineCallback ) ( PTTState, PTTInState, L1State, L2State, linesMode );
   return true;
}
bool WinMonitor::L2Changed( int state )
{
   L2State = state;
   if ( WinLineCallback )
      ( *WinLineCallback ) ( PTTState, PTTInState, L1State, L2State, linesMode );
   return true;
}
bool WinMonitor::pttChanged( int state )
{
   PTTInState = state;
   if ( WinLineCallback )
      ( *WinLineCallback ) ( PTTState, PTTInState, L1State, L2State, linesMode );
   return true;
}
bool WinMonitor::linesModeChanged( int state )
{
   linesMode = state;
   if ( WinLineCallback )
      ( *WinLineCallback ) ( PTTState, PTTInState, L1State, L2State, linesMode );
   return true;
}
//==============================================================================
LineCallBack LineEventsPort::WinLineCallback = nullptr;

void LineLog( const QString &msg )
{
   trace( msg );
}
LineEventsPort::LineEventsPort( const PortConfig &port ) : commonPort( port )
{
    /*LineSet * ls =*/ LineSet::GetLineSet();   // make sure it is initialised
}
LineEventsPort::~LineEventsPort()
{
   closing = true;
   closePort();
}

void LineEventsPort::linesChangedEvent()
{
    LineSet * ls = LineSet::GetLineSet();
    ls->readLines();
    linePTTState = ls->getState( "PTTIn" );
    lineL1State = ls->getState( "L1" );
    lineL2State = ls->getState( "L2" );

    linesMode = (ls->getState("L3")?1:0) * 8 +
            (ls->getState("L4")?1:0) * 4 +
            (ls->getState("L5")?1:0) * 2 +
            (ls->getState("L6")?1:0) * 1;

    transverterSwitch = (ls->getState("T1")?1:0) * 2 +
            (ls->getState("T2")?1:0) * 1;
}

bool LineEventsPort::initialisePort()
{
   LineSet * ls = LineSet::GetLineSet();
   ls->lsLog = LineLog;

   connect(ls, SIGNAL(linesChanged()), this, SLOT(linesChangedEvent()));
   return true;
}

bool LineEventsPort::openPort()
{
   return true;
}
bool LineEventsPort::closePort()
{
   return true;
}

void LineEventsPort::ptt( int state )
{
   if ( currentKeyer && currentKeyer->started )
   {
      LineSet * ls = LineSet::GetLineSet();
      ls->publish( "PTTOut", state );
      PTTState = state;
      if ( WinLineCallback )
         ( *WinLineCallback ) ( PTTState, lastPTTState, lastL1State, lastL2State, lastLinesMode );
   }
}

void LineEventsPort::checkControls()
{
   if ( lineL2State != lastL2State )       //L2
   {
      lastL2State = !lastL2State;
      L2Changed( lastL2State );
      if ( WinLineCallback )
         ( *WinLineCallback ) ( PTTState, lastPTTState, lastL1State, lastL2State, lastLinesMode );
   }
   if ( lineL1State != lastL1State )       //L1
   {
      lastL1State = !lastL1State;
      L1Changed( lastL1State );
      if ( WinLineCallback )
         ( *WinLineCallback ) ( PTTState, lastPTTState, lastL1State, lastL2State, lastLinesMode );
   }
   if ( linePTTState != lastPTTState )       //PTT
   {
      lastPTTState = !lastPTTState;
      pttChanged( lastPTTState );
      if ( WinLineCallback )
         ( *WinLineCallback ) ( PTTState, lastPTTState, lastL1State, lastL2State, lastLinesMode );
   }
   if (linesMode != lastLinesMode)
   {
       lastLinesMode = linesMode;
       linesModeChanged( lastLinesMode );
   }
   if (transverterSwitch != lastTransverterSwitch)
   {
       lastTransverterSwitch = transverterSwitch;
       transverterSwitchChanged(lastTransverterSwitch);
   }
   if ( WinLineCallback )
      ( *WinLineCallback ) ( PTTState, lastPTTState, lastL1State, lastL2State, lastLinesMode );
}
//==============================================================================
