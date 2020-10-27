/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "base_pch.h"

#include "tinyxml.h"
#include "TinyUtils.h"
#include "CommonMonitor.h"
#include "controlport.h"
#include "portconf.h"

//==============================================================================
void PortConfigure::SetPorts( TiXmlElement *e )
{
   for ( TiXmlElement * c = e->FirstChildElement(); c; c = c->NextSiblingElement() )
   {
      if ( checkElementName( c, "Port" ) )
      {
         // attribs name, type, port
         QString n;
         QString t;
         QString cn;
         if ( GetStringAttribute( c, "name", n ) == TIXML_SUCCESS
              && GetStringAttribute( c, "type", t ) == TIXML_SUCCESS
              && GetStringAttribute( c, "connection", cn ) == TIXML_SUCCESS )
         {
            // should all this live inside PortConfig?
            PortConfig::PortType pt;
            if ( t == "Windows" )
            {
               pt = PortConfig::eptWindows;
            }
            else
               if ( t == "PiGPIO" )
               {
                  pt = PortConfig::eptPiGPIO;
               }
               else
                  if ( t == "K8055" )
                  {
                     pt = PortConfig::eptK8055;
                  }
                  else
                 {
                    // error...
                    continue;
                 }
            PortConfig p( n, pt, cn );
            portmap[ n ] = p;
         }
      }
   }
}
void PortConfigure::SetLines( TiXmlElement *e )
{
   for ( TiXmlElement * c = e->FirstChildElement(); c; c = c->NextSiblingElement() )
   {
      if ( checkElementName( c, "Line" ) )
      {
         //  	<Line name="L2" direction = "in" port="K8055" line="2" />
         QString name;
         QString inLine;
         QString portName;
         QString portLineName;

         if ( GetStringAttribute( c, "name", name ) == TIXML_SUCCESS )
         {
            GetStringAttribute( c, "direction", inLine, "" );
            GetStringAttribute( c, "port", portName, "" );
            GetStringAttribute( c, "line", portLineName, "" );

            LineConfig k( name, inLine, portName, portLineName );
            linemap[ name ] = k;
         }
      }
   }
}
void PortConfigure::SetEnable( TiXmlElement *e, commonController &monitor )
{
   QString pname;
   if ( GetStringAttribute( e, "port", pname ) == TIXML_SUCCESS )
   {
      PortConfig & pc = portmap[ pname ];

      // now create the port
      commonPort *p = monitor.createPort( pc );
      // and loop through the lines adding the relevant lineConfig lines to it
      for ( auto const &i: linemap )
      {
         if ( pname == i.portName )
         {
            p->addLine( i );
         }
      }
      if ( !p->initialise( pc, monitor ) )
      {
         delete p;
         return ;
      }

   }
}
bool PortConfigure::configurePorts( commonController &monitor )
{

   TiXmlBase::SetCondenseWhiteSpace( false );
   TiXmlDocument keyerdoc;
   keyerdoc.LoadFile( "./Configuration/portConfig.xml" );
   TiXmlElement *root = keyerdoc.RootElement();
   if ( !root )
   {
      //err = "No XML root element (does file exist?)";
      return false;
   }
   if ( !checkElementName( root, "PortConfig" ) )
   {
      //err = "Invalid XML root element";
      return false;
   }
   for ( TiXmlElement * e = root->FirstChildElement(); e; e = e->NextSiblingElement() )
   {
      if ( checkElementName( e, "Ports" ) )
      {
         SetPorts( e );
      }
      else
         if ( checkElementName( e, "Lines" ) )
         {
            SetLines( e );
         }
         else
            if ( checkElementName( e, "Enable" ) )
            {
               SetEnable( e, monitor );
            }
            else
            {
               //         trace("Unknown");
            }
   }

   return true;
}

//==============================================================================
bool configurePorts( commonController &monitor )
{
   PortConfigure kc;
   return kc.configurePorts( monitor );
}
//==============================================================================
