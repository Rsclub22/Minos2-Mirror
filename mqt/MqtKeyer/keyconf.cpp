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
#include "keyers.h"
#include "keyconf.h"
#include "keyerlog.h"

//==============================================================================
// The keyer factory
commonKeyer *KeyerConfigure::createKeyer( const KeyerConfig &keyer, const PortConfig &port )
{
   commonKeyer * ck = nullptr;

   ck = new voiceKeyer( keyer, port );
   if ( sblog )
   {
      trace( "keyer created" );
   }
   if ( !ck->initialise( keyer, port ) )
   {
      delete ck;
      ck = nullptr;
   }

   return ck;
}
//==============================================================================
//==============================================================================
void KeyerConfigure::SetTrace( TiXmlElement *e )
{
   bool tval;
   if ( GetBoolAttribute( e, "enabled", tval, false ) == TIXML_SUCCESS )
   {
      sblog = tval;
   }
}
void KeyerConfigure::SetTune( TiXmlElement *e )
{
   int tval;
   if ( GetIntAttribute( e, "time", tval, 20 ) == TIXML_SUCCESS )
   {
      if ( tval > 0 && tval <= 30 )
         tuneTime = tval;
      else
      {
         trace( "Incorrect argument to attribute `Tune Time' - must be > 0 and <= 30 (secs)" );
      }
   }
   if ( GetIntAttribute( e, "level", tval, 80 ) == TIXML_SUCCESS )
   {
      if ( tval > 0 && tval <= 100 )
         tuneLevel = tval;
      else
      {
         trace( "Incorrect argument to attribute `Tune Level' - must be > 0 and <= 100 (%)" );
      }
   }
}
void KeyerConfigure::SetPorts( TiXmlElement *e )
{
   for ( TiXmlElement * c = e->FirstChildElement(); c; c = c->NextSiblingElement() )
   {
      if ( checkElementName( c, "Port" ) )
      {
         // attribs name, type, port
         QString n;
         QString t;
         if ( GetStringAttribute( c, "name", n ) == TIXML_SUCCESS
              && GetStringAttribute( c, "type", t ) == TIXML_SUCCESS )
         {
            // should all this live inside PortConfig?
            PortConfig::PortType pt;
            if ( t == "Windows" )
            {
               pt = PortConfig::eptWindows;
            }
            else
               if ( t == "MinosControl" )
               {
                  pt = PortConfig::eptMinosControl;
               }
               else
               {
                  // error...
                  continue;
               }
            PortConfig p( n, pt );
            portmap[ n ] = p;
         }
      }
   }
}
void KeyerConfigure::SetKeyers( TiXmlElement *e )
{

   for ( TiXmlElement * c = e->FirstChildElement(); c; c = c->NextSiblingElement() )
   {
      if ( checkElementName( c, "Keyer" ) )
      {
         // attribs name PipTone EnablePip StartDelay AutoRepeat PipStartDelay PlayPTTDelay
         QString name;
         unsigned int sampleRate;
         int PipTone;
         int PipLength;
         int StartDelay;
         unsigned int PipStartDelay;
         int PlayPTTDelay;
         int pipVolume;
         int ClipRecord;

         if ( GetStringAttribute( c, "name", name ) == TIXML_SUCCESS )
         {
             int temp;
            GetIntAttribute( c, "sampleRate", temp, 22050);
            sampleRate = static_cast<unsigned int>(temp);
            GetIntAttribute( c, "pipTone", PipTone, 1000 );
            GetIntAttribute( c, "pipLength", PipLength, 250 );
            GetIntAttribute( c, "pipStartDelay", temp, 0 );
            PipStartDelay = static_cast<unsigned int>(temp);
            GetIntAttribute( c, "pipVolume", pipVolume, 80 );
            GetIntAttribute( c, "startDelay", StartDelay, 0 );
            GetIntAttribute( c, "playPTTDelay", PlayPTTDelay, 0 );
            GetIntAttribute( c, "clipRecord", ClipRecord, 0 );

            KeyerConfig k( name, sampleRate, PipTone, pipVolume, PipLength, StartDelay,
                           PipStartDelay, PlayPTTDelay,
                           ClipRecord );
            keyermap[ name ] = k;
         }
      }
   }
}
void KeyerConfigure::SetEnable( TiXmlElement *e )
{
   //attribs Keyer, Port - refer to names of KeyerConfig, PortConfig
   QString k;
   QString p;
   if ( GetStringAttribute( e, "keyer", k ) == TIXML_SUCCESS
        && GetStringAttribute( e, "port", p ) == TIXML_SUCCESS )
      createKeyer( keyermap[ k ], portmap[ p ] );
}
void KeyerConfigure::SetCW( TiXmlElement *e )
{
   int tone;
   int speed;
   if ( GetIntAttribute( e, "tone", tone, 1000 ) == TIXML_SUCCESS )
   {
      CWTone = tone;
   }
   if ( GetIntAttribute( e, "speed", speed, 12 ) == TIXML_SUCCESS )
   {
      CWSpeed = speed;
   }
}
void KeyerConfigure::SetMorseCode( TiXmlElement *e )
{
   for ( TiXmlElement * c = e->FirstChildElement(); c; c = c->NextSiblingElement() )
   {
      if ( checkElementName( c, "Key" ) )
      {
         QString character;
         QString code;
         if ( GetStringAttribute( c, "character", character ) == TIXML_SUCCESS
              && GetStringAttribute( c, "code", code ) == TIXML_SUCCESS )
         {
            QString proccode;
            for ( int i = 0; i < 8 && code[ i ].unicode() != 0 && code[ i ].unicode() != ' '; i++ )
            {
               if ( code[ i ] == '.' )
                  proccode += '\x40';
               if ( code[ i ] == '-' )
                  proccode += '\x80';
            }
            char c = character[ 0 ].toLatin1();
            MORSECODE[ c ] = proccode;
         }
      }
   }
}
//==============================================================================
void KeyerConfigure::SetCWMessages( TiXmlElement *e )
{
   for ( TiXmlElement * c = e->FirstChildElement(); c; c = c->NextSiblingElement() )
   {
      if ( checkElementName( c, "Message" ) )
      {
         // Attrib id, autoRepeat; body is message
         // Create a MORSEMSG, and put it into MORSEMSGS
      }
   }
}
bool KeyerConfigure::configureKeyers()
{

   TiXmlBase::SetCondenseWhiteSpace( false );
   TiXmlDocument keyerdoc;
   keyerdoc.LoadFile( "./Configuration/keyerConfig.xml" );
   TiXmlElement *root = keyerdoc.RootElement();
   if ( !root )
   {
      //err = "No XML root element (does file exist?)";
      return false;
   }
   if ( !checkElementName( root, "KeyerConfig" ) )
   {
      //err = "Invalid XML root element";
      return false;
   }
   for ( TiXmlElement * e = root->FirstChildElement(); e; e = e->NextSiblingElement() )
   {
      if ( checkElementName( e, "Trace" ) )
      {
         SetTrace( e );
      }
      else
         if ( checkElementName( e, "Tune" ) )
         {
            SetTune( e );
         }
         else
            if ( checkElementName( e, "Ports" ) )
            {
               SetPorts( e );
            }
            else
               if ( checkElementName( e, "Keyers" ) )
               {
                  SetKeyers( e );
               }
               else
                  if ( checkElementName( e, "Enable" ) )
                  {
                     SetEnable( e );
                  }
                  else
                     if ( checkElementName( e, "CW" ) )
                     {
                        SetCW( e );
                     }
                     else
                        if ( checkElementName( e, "MorseCode" ) )
                        {
                           SetMorseCode( e );
                        }
                        else
                           if ( checkElementName( e, "CWMessages" ) )
                           {
                              SetCWMessages( e );
                           }
                           else
                           {
                              //         trace("Unknown");
                           }
   }
   return true;
}

//==============================================================================
bool configureKeyers()
{
   KeyerConfigure kc;
   return kc.configureKeyers();
}
//==============================================================================
//==============================================================================
