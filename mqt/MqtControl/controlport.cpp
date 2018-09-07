/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2006 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

//#include <process.h>
#include "cutils.h"
#include "tinyxml.h"
#include "portconf.h"
#include "CommonMonitor.h"
#include "controlport.h"

#include "windowMonitor.h"
#include "K8055Container.h"
#include "PiGPIO.h"

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


//==============================================================================
commonLineControl::commonLineControl( QString name, bool lineIn, QString portLine, commonPort *port )
      : state( false )
      , lastState( false )
      , lineName( name )
      , portLineName( portLine )
      , port( port )
      , lineIn( lineIn )
{
}
commonLineControl::~commonLineControl()
{
}
void commonLineControl::checkLine()
{
   state = port->getLine( this );
   if ( state != lastState )
   {
      lastState = state;
      port->reportLineChange( this );
   }
}
void commonLineControl::setState( bool pstate )
{
   if ( state != pstate )
   {
      state = pstate;
      port->setLine( this );
      port->reportLineChange( this );
   }
}
//==============================================================================
commonPort::commonPort( const PortConfig &port ) :
      pName( port.name )
    , portName( port.portName )
    , openCount( 0 )
{

}
void commonPort::addLine( const LineConfig &line )
{
   commonLineControl * cll = new commonLineControl( line.name, line.lineIn, line.portLineName, this );
   lines.push_back( cll );
}

commonPort::~commonPort()
{
   for ( QVector<commonLineControl *>::iterator i = lines.begin(); i != lines.end(); i++ )
   {
      delete ( *i );
   }
   lines.clear();
}
bool commonPort::initialise( const PortConfig & /*port*/, commonController &mon )
{
   if ( initialisePort() && openPort() )
   {
      mon.portChain.push_back( this );
      monitor = &mon;
      return true;
   }
   return false;
}
commonLineControl *commonPort::findLine(const QString &name, bool lineIn )
{
   for ( QVector<commonLineControl *>::iterator i = lines.begin(); i != lines.end(); i++ )
   {
      if ( ( *i ) ->lineIn == lineIn && ( *i ) ->lineName == name )
         return ( *i );
   }
   return nullptr;
}
void commonPort::checkControls( )
{
   getLineState();
   for ( QVector<commonLineControl *>::iterator i = lines.begin(); i != lines.end(); i++ )
   {
      // output lines can't sort their state in the same way
      if ( ( *i ) ->lineIn )
      {
         ( *i ) ->checkLine();
      }
   }
}
void commonPort::reportLineChange( commonLineControl *line )
{
   monitor->lineChange( line );
}

//==============================================================================
//=============================================================================
WindowsMonitorPort::WindowsMonitorPort(QWidget *p, const PortConfig &port ) :
    commonPort( port )
    , parent(p)
{}
WindowsMonitorPort::~WindowsMonitorPort()
{
   closePort();
}

static windowMonitor *WindowsMonitorForm = nullptr;

bool WindowsMonitorPort::initialisePort()
{
   WindowsMonitorForm = new windowMonitor( parent );
   WindowsMonitorForm->show();
   return true;
}

bool WindowsMonitorPort::openPort()
{
   return true;
}

bool WindowsMonitorPort::closePort()
{
   delete WindowsMonitorForm;
   WindowsMonitorForm = nullptr;

   return true;
}

void WindowsMonitorPort::setLine( commonLineControl *line )
{
   if ( line->portLineName == "PTTOut" )
   {
      ;
   }
}
bool WindowsMonitorPort::getLine( commonLineControl *line )
{
   if ( line->portLineName == "PTTIn" )
   {
      return WindowsMonitorForm->PTTChecked();
   }
   if ( line->portLineName == "L1" )
   {
      return WindowsMonitorForm->L1Checked();
   }
   if ( line->portLineName == "L2" )
   {
      return WindowsMonitorForm->L2Checked();
   }
   if ( line->portLineName == "L3" )
   {
      return WindowsMonitorForm->L3Checked();
   }
   if ( line->portLineName == "L4" )
   {
      return WindowsMonitorForm->L4Checked();
   }
   if ( line->portLineName == "L5" )
   {
      return WindowsMonitorForm->L5Checked();
   }
   if ( line->portLineName == "L6" )
   {
      return WindowsMonitorForm->L6Checked();
   }
   return false;
}
void WindowsMonitorPort::getLineState()
{
   // no need to get it all in one go
}

//==============================================================================
PiGPIOPort::PiGPIOPort( const PortConfig &port ) : commonPort( port ),
    pigpio(new PiGPIO)
{
   trace( "PiGPIOPort::PiGPIOPort" );
}
PiGPIOPort::~PiGPIOPort()
{
   closePort();
   delete pigpio;
}
bool PiGPIOPort::initialisePort()
{
   return true;
}


bool PiGPIOPort::openPort()
{
    return true;
}

bool PiGPIOPort::closePort()
{
   return true;
}

void PiGPIOPort::addLine( const LineConfig &line )
{
    commonPort::addLine(line);

    if (line.lineIn)
    {
        pigpio->setPinInput(toInt(line.portLineName));
    }
    else
    {
        pigpio->setPinOutput(toInt(line.portLineName));
    }
}
void PiGPIOPort::setLine( commonLineControl *line )
{
   int l = toInt(line->portLineName );
   if ( line->getState() )
   {
       pigpio->setPin(l, true);
   }
   else
   {
       pigpio->setPin(l, false);
   }
}
bool PiGPIOPort::getLine( commonLineControl *line )
{
   int l = toInt( line->portLineName );
   return pigpio->readPin(l);
}
void PiGPIOPort::getLineState()
{
//   nothing to do
}
//==============================================================================
#ifdef K8055PORT
K8055Port::K8055Port( const PortConfig &port ) : commonPort( port ),
      addr( 0 ), dig( 0 )
{
   trace( "K8055Port::K8055Port" );
   addr = portName.toInt( 0 );
}
K8055Port::~K8055Port()
{
   closePort();
}
bool K8055Port::initialisePort()
{
   return true;
}


bool K8055Port::openPort()
{
   int ret = K8055::openDevice( addr );
   return ret != -1;
}

bool K8055Port::closePort()
{
   K8055::closeDevice();
   return true;
}


void K8055Port::setLine( commonLineControl *line )
{
   int l = atoi( line->portLineName.c_str() );
   if ( line->getState() )
   {
      K8055::setDigitalChannel( l );
   }
   else
   {
      K8055::clearDigitalChannel( l );
   }
   //   K8055::writeAllDigital( line->getState() ? l : 0 );
}
bool K8055Port::getLine( commonLineControl *line )
{
   int l = atoi( line->portLineName.c_str() );
   return ( dig & l );
}
void K8055Port::getLineState()
{
   dig = K8055::readAllDigital();
}
#endif
//==============================================================================
